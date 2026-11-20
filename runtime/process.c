#define _GNU_SOURCE
#include "runtime.h"
#include "internal.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <spawn.h>
#include <time.h>
#include <limits.h>
#include <sys/socket.h>
#include "process-input.h"

/* Match the retained spawnSync contract: separate UTF-8 output streams,
 * EOF on stdin, a one-MiB limit per stream, and -1 for spawn/signal failures. */
#define FL_SHELL_CAPTURE_LIMIT (1024u * 1024u)
typedef struct { char *data; size_t used, capacity; } FLShellCapture;

static int shell_capture_add(FLShellCapture *out, const char *data, size_t size, int *overflow) {
    size_t available = FL_SHELL_CAPTURE_LIMIT - out->used;
    if (size > available) { size = available; *overflow = 1; }
    if (size) {
        size_t needed = out->used + size + 1;
        if (needed > out->capacity) {
            size_t capacity = out->capacity ? out->capacity : 4096;
            while (capacity < needed) capacity *= 2;
            if (capacity > FL_SHELL_CAPTURE_LIMIT + 1) capacity = FL_SHELL_CAPTURE_LIMIT + 1;
            char *next = realloc(out->data, capacity);
            if (!next) return -1;
            out->data = next; out->capacity = capacity;
        }
        memcpy(out->data + out->used, data, size);
        out->used += size; out->data[out->used] = 0;
    }
    return 0;
}
static int shell_private_fd(int fd) {
    if (fd < 0 || fd > STDERR_FILENO) return fd;
    int moved = fcntl(fd, F_DUPFD_CLOEXEC, STDERR_FILENO + 1);
    close(fd); return moved;
}
static FLValue shell_output(const char *data, size_t size) {
    FLString *s = fl_arena_alloc(sizeof(*s) + size + 1);
    if (!s) return fl_nil();
    s->base.type = FL_STRING; s->base.rc = 1; s->len = (uint32_t)size;
    if (size) memcpy(s->data, data, size);
    s->data[size] = 0;
    return (FLValue){.tag=FL_STRING,.obj=(FLObject *)s};
}
static int64_t shell_monotonic_ms(void) {
    struct timespec now;
    if (clock_gettime(CLOCK_MONOTONIC, &now)) return -1;
    return (int64_t)now.tv_sec * 1000 + now.tv_nsec / 1000000;
}
static void shell_cancel(pid_t child, int grouped, int signal_number) {
    if (child > 0) (void)kill(grouped ? -child : child, signal_number);
}
/* Optional when this object is linked without the HTTP server. */
extern int fl_http_process_cancel_requested(void) __attribute__((weak));
static int shell_http_cancel_requested(int timeout_ms) {
    return timeout_ms > 0 && fl_http_process_cancel_requested &&
           fl_http_process_cancel_requested();
}
static int shell_child_owned(pid_t child, siginfo_t *info) {
    int result;
    memset(info, 0, sizeof(*info));
    do { result = waitid(P_PID, (id_t)child, info, WEXITED | WNOHANG | WNOWAIT); }
    while (result < 0 && errno == EINTR);
    return result == 0;
}
static void shell_http_cancel(pid_t child) {
    siginfo_t info;
    if (!shell_child_owned(child, &info)) return;
    /* Keep the direct child unreaped while signalling its own group. This
     * prevents a completed leader's PID/group from being reused underneath us. */
    shell_cancel(child, 1, SIGTERM);
    int64_t started = shell_monotonic_ms();
    while (started >= 0) {
        int64_t now = shell_monotonic_ms();
        if (now < 0 || now - started >= 200) break;
        (void)poll(NULL, 0, 10);
        if (!shell_child_owned(child, &info)) return;
    }
    /* Descendants can outlive a leader that handled TERM. The unreaped child
     * still pins this group; finish cleanup before reaping it below. */
    shell_cancel(child, 1, SIGKILL);
}

void fl_process_input_release(FLProcessInputResult *result) {
    if (!result) return;
    free(result->out.data); free(result->err.data);
    memset(result, 0, sizeof(*result)); result->exit_code = -1;
}

static int input_capture_add(FLShellCapture *capture, const char *data,
                             size_t size, size_t limit, int *overflow) {
    size_t available = limit - capture->used;
    if (size > available) { size = available; *overflow = 1; }
    if (!size) return 0;
    size_t needed = capture->used + size + 1;
    if (needed > capture->capacity) {
        size_t capacity = capture->capacity ? capture->capacity : 4096;
        while (capacity < needed) {
            if (capacity > (limit + 1) / 2) { capacity = limit + 1; break; }
            capacity *= 2;
        }
        if (capacity > limit + 1) capacity = limit + 1;
        char *next = realloc(capture->data, capacity);
        if (!next) return -1;
        capture->data = next; capture->capacity = capacity;
    }
    memcpy(capture->data + capture->used, data, size);
    capture->used += size; capture->data[capture->used] = 0;
    return 0;
}

int fl_process_input(const char *const argv[], const void *input, size_t size,
                     int64_t timeout_ms, size_t output_limit,
                     FLProcessInputResult *result) {
    if (!result) return -1;
    memset(result, 0, sizeof(*result)); result->exit_code = -1;
    if (!argv || !argv[0] || argv[0][0] != '/' || (!input && size) ||
        timeout_ms < 1 || output_limit == SIZE_MAX) {
        result->io_error = EINVAL; return -1;
    }
    int input_pair[2] = {-1,-1}, pipes[2][2] = {{-1,-1},{-1,-1}};
    size_t sent = 0;
    FLShellCapture capture[2] = {{0},{0}};
    pid_t child = -1;
    posix_spawn_file_actions_t actions;
    posix_spawnattr_t attributes;
    int actions_ready = 0, attributes_ready = 0;
    int64_t started = shell_monotonic_ms(), deadline;
    if (started < 0) { result->io_error = EIO; return -1; }
    deadline = timeout_ms > INT64_MAX - started ? INT64_MAX : started + timeout_ms;
    if (fl_http_process_cancel_requested && fl_http_process_cancel_requested()) {
        result->cancelled = 1; return -1;
    }
    if (socketpair(AF_UNIX, SOCK_STREAM | SOCK_CLOEXEC, 0, input_pair)) {
        result->io_error = errno; goto finish_input;
    }
    for (int j = 0; j < 2; j++) {
        input_pair[j] = shell_private_fd(input_pair[j]);
        if (input_pair[j] < 0) { result->io_error = errno; goto finish_input; }
    }
    for (int i = 0; i < 2; i++) {
        if (pipe2(pipes[i], O_CLOEXEC)) { result->io_error = errno; goto finish_input; }
        for (int j = 0; j < 2; j++) {
            pipes[i][j] = shell_private_fd(pipes[i][j]);
            if (pipes[i][j] < 0) { result->io_error = errno; goto finish_input; }
        }
    }
    int nonblocking[] = {input_pair[0], pipes[0][0], pipes[1][0]};
    for (size_t i = 0; i < sizeof(nonblocking)/sizeof(nonblocking[0]); i++) {
        int flags = fcntl(nonblocking[i], F_GETFL);
        if (flags < 0 || fcntl(nonblocking[i], F_SETFL, flags | O_NONBLOCK)) {
            result->io_error = errno; goto finish_input;
        }
    }
    int error = posix_spawn_file_actions_init(&actions);
    if (error) { result->io_error = error; goto finish_input; }
    actions_ready = 1;
    if ((error = posix_spawn_file_actions_adddup2(&actions, input_pair[1], STDIN_FILENO)) ||
        (error = posix_spawn_file_actions_adddup2(&actions, pipes[0][1], STDOUT_FILENO)) ||
        (error = posix_spawn_file_actions_adddup2(&actions, pipes[1][1], STDERR_FILENO)) ||
        (error = posix_spawn_file_actions_addclosefrom_np(&actions, STDERR_FILENO + 1))) {
        result->io_error = error; goto finish_input;
    }
    error = posix_spawnattr_init(&attributes);
    if (error) { result->io_error = error; goto finish_input; }
    attributes_ready = 1;
    if ((error = posix_spawnattr_setpgroup(&attributes, 0)) ||
        (error = posix_spawnattr_setflags(&attributes, POSIX_SPAWN_SETPGROUP))) {
        result->io_error = error; goto finish_input;
    }
    error = posix_spawn(&child, argv[0], &actions, &attributes, (char *const *)argv, environ);
    if (error) { child = -1; result->spawn_error = error; goto finish_input; }
    close(input_pair[1]); input_pair[1] = -1;
    for (int i = 0; i < 2; i++) { close(pipes[i][1]); pipes[i][1] = -1; }
    if (!size) { close(input_pair[0]); input_pair[0] = -1; }
    for (;;) {
        if (fl_http_process_cancel_requested && fl_http_process_cancel_requested()) {
            result->cancelled = 1; break;
        }
        int64_t now = shell_monotonic_ms();
        if (now < 0) { result->io_error = EIO; break; }
        if (now >= deadline) { result->timed_out = 1; break; }
        siginfo_t observed;
        if (!shell_child_owned(child, &observed)) { result->io_error = ECHILD; break; }
        if (observed.si_pid && pipes[0][0] < 0 && pipes[1][0] < 0) {
            if (sent != size) result->io_error = EPIPE;
            break;
        }
        struct pollfd ready[3] = {{input_pair[0],POLLOUT,0},
            {pipes[0][0],POLLIN,0},{pipes[1][0],POLLIN,0}};
        int delay = deadline - now < 100 ? (int)(deadline - now) : 100;
        int polled = poll(ready, 3, delay);
        if (polled < 0 && errno == EINTR) continue;
        if (polled < 0) { result->io_error = errno; break; }
        /* Drain both streams fairly; deadline/cancellation are checked between
         * bounded chunks even when a child continuously fills one pipe. */
        for (int i = 0; i < 2; i++) if (pipes[i][0] >= 0 && ready[i+1].revents) {
            if (ready[i+1].revents & POLLNVAL) { result->io_error = EBADF; break; }
            for (int chunks = 0; chunks < 8; chunks++) {
                char bytes[8192];
                ssize_t count = read(pipes[i][0], bytes, sizeof bytes);
                if (count > 0) {
                    if (input_capture_add(&capture[i], bytes, (size_t)count,
                            output_limit - capture[1-i].used, &result->overflow)) {
                        result->io_error = ENOMEM; break;
                    }
                    if (result->overflow) break;
                } else if (!count) { close(pipes[i][0]); pipes[i][0] = -1; break; }
                else if (errno == EINTR) continue;
                else if (errno == EAGAIN || errno == EWOULDBLOCK) break;
                else { result->io_error = errno; break; }
            }
            if (result->io_error || result->overflow) break;
        }
        if (result->io_error || result->overflow) break;
        if (input_pair[0] >= 0 && ready[0].revents) {
            size_t count = size - sent;
            if (count > 65536) count = 65536;
            ssize_t written = send(input_pair[0], (const char *)input + sent, count, MSG_NOSIGNAL);
            if (written > 0) {
                sent += (size_t)written;
                if (sent == size) { close(input_pair[0]); input_pair[0] = -1; }
            } else if (written < 0 && errno != EINTR && errno != EAGAIN && errno != EWOULDBLOCK) {
                result->io_error = errno; break;
            } else if (!written) { result->io_error = EPIPE; break; }
        }
    }
finish_input:
    if (actions_ready) posix_spawn_file_actions_destroy(&actions);
    if (attributes_ready) posix_spawnattr_destroy(&attributes);
    for (int i = 0; i < 2; i++) {
        if (input_pair[i] >= 0) close(input_pair[i]);
        for (int j = 0; j < 2; j++) if (pipes[i][j] >= 0) close(pipes[i][j]);
    }
    if (child > 0) {
        siginfo_t observed;
        if (shell_child_owned(child, &observed)) {
            if (result->cancelled) shell_http_cancel(child);
            else shell_cancel(child, 1, SIGKILL);
            /* WNOWAIT above pins the child's PID and group through cleanup. */
            int status;
            pid_t waited;
            do { waited = waitpid(child, &status, 0); } while (waited < 0 && errno == EINTR);
            if (waited == child) {
                if (WIFEXITED(status)) result->exit_code = WEXITSTATUS(status);
                else if (WIFSIGNALED(status)) result->signal_number = WTERMSIG(status);
            } else result->io_error = ECHILD;
        } else result->io_error = ECHILD;
    }
    result->out = (FLProcessBytes){capture[0].data, capture[0].used};
    result->err = (FLProcessBytes){capture[1].data, capture[1].used};
    return result->spawn_error || result->io_error || result->timed_out ||
        result->overflow || result->cancelled || result->signal_number ? -1 : 0;
}

static FLValue shell_exec_result(FLValue command, int timeout_ms, int failure_status) {
    FLValue text = fl_v11_utf8_string(command);
    FLString *cmd = (FLString *)text.obj;
    if (memchr(cmd->data, 0, cmd->len))
        fl_throw(fl_make_error("TypeError", "shell command must not contain null bytes"));
    int pipes[2][2] = {{-1,-1},{-1,-1}}, input = -1, status = failure_status;
    int failed = 0, overflow = 0, killed = 0, timed_out = 0, cancelled = 0;
    int64_t deadline = 0;
    pid_t child = -1;
    FLShellCapture capture[2] = {{0},{0}};
    posix_spawn_file_actions_t actions;
    posix_spawnattr_t attributes;
    int actions_ready = 0, attributes_ready = 0;
    if (shell_http_cancel_requested(timeout_ms)) { cancelled = 1; goto finish; }
    for (int i = 0; i < 2; i++) {
        if (pipe2(pipes[i], O_CLOEXEC) < 0) goto finish;
        for (int j = 0; j < 2; j++) {
            pipes[i][j] = shell_private_fd(pipes[i][j]);
            if (pipes[i][j] < 0) goto finish;
        }
        int flags = fcntl(pipes[i][0], F_GETFL);
        if (flags < 0 || fcntl(pipes[i][0], F_SETFL, flags | O_NONBLOCK) < 0) goto finish;
    }
    input = shell_private_fd(open("/dev/null", O_RDONLY | O_CLOEXEC));
    if (input < 0 || posix_spawn_file_actions_init(&actions)) goto finish;
    actions_ready = 1;
    if (posix_spawn_file_actions_adddup2(&actions, input, STDIN_FILENO) ||
        posix_spawn_file_actions_adddup2(&actions, pipes[0][1], STDOUT_FILENO) ||
        posix_spawn_file_actions_adddup2(&actions, pipes[1][1], STDERR_FILENO)) goto finish;
    for (int i = 0; i < 2; i++) for (int j = 0; j < 2; j++)
        if (posix_spawn_file_actions_addclose(&actions, pipes[i][j])) goto finish;
    if (posix_spawn_file_actions_addclose(&actions, input)) goto finish;
    if (timeout_ms > 0) {
        if (posix_spawnattr_init(&attributes)) goto finish;
        attributes_ready = 1;
        if (posix_spawnattr_setpgroup(&attributes, 0) ||
            posix_spawnattr_setflags(&attributes, POSIX_SPAWN_SETPGROUP)) goto finish;
        int64_t now = shell_monotonic_ms();
        if (now < 0) goto finish;
        deadline = now + timeout_ms;
    }
    char *argv[] = {"sh", "-c", cmd->data, NULL};
    if (posix_spawnp(&child, "sh", &actions,
            attributes_ready ? &attributes : NULL, argv, environ)) { child = -1; goto finish; }
    close(input); input = -1;
    close(pipes[0][1]); pipes[0][1] = -1;
    close(pipes[1][1]); pipes[1][1] = -1;
    while (pipes[0][0] >= 0 || pipes[1][0] >= 0) {
        if (shell_http_cancel_requested(timeout_ms)) { cancelled = 1; break; }
        struct pollfd ready[2] = {{pipes[0][0],POLLIN,0},{pipes[1][0],POLLIN,0}};
        int wait_ms = -1;
        if (timeout_ms > 0) {
            int64_t now = shell_monotonic_ms();
            if (now < 0) { failed = 1; break; }
            int64_t remaining = deadline - now;
            if (remaining <= 0) { timed_out = 1; break; }
            wait_ms = remaining > INT_MAX ? INT_MAX : (int)remaining;
            if (wait_ms > 100) wait_ms = 100;
        }
        int result = poll(ready, 2, wait_ms);
        if (result < 0 && errno == EINTR) continue;
        if (result < 0) { failed = 1; break; }
        if (result == 0) {
            if (timeout_ms > 0) continue; /* Same deadline; also observe HTTP stop. */
            timed_out = 1; break;
        }
        for (int i = 0; i < 2; i++) {
            if (pipes[i][0] < 0 || !ready[i].revents) continue;
            char buffer[8192];
            for (;;) {
                if (shell_http_cancel_requested(timeout_ms)) { cancelled = 1; break; }
                if (timeout_ms > 0) {
                    int64_t now = shell_monotonic_ms();
                    if (now < 0) { failed = 1; break; }
                    if (now >= deadline) { timed_out = 1; break; }
                }
                ssize_t n = read(pipes[i][0], buffer, sizeof buffer);
                if (n > 0) {
                    if (shell_capture_add(&capture[i], buffer, (size_t)n, &overflow)) { failed = 1; break; }
                    if (overflow && !killed) { shell_cancel(child, timeout_ms > 0, timeout_ms > 0 ? SIGKILL : SIGTERM); killed = 1; }
                    if (overflow) break;
                } else if (!n) { close(pipes[i][0]); pipes[i][0] = -1; break; }
                else if (errno == EINTR) continue;
                else if (errno == EAGAIN || errno == EWOULDBLOCK) break;
                else { failed = 1; break; }
            }
            if (failed || overflow || timed_out || cancelled) break;
        }
        if (failed || overflow || timed_out || cancelled) break;
    }
finish:
    if (actions_ready) posix_spawn_file_actions_destroy(&actions);
    if (attributes_ready) posix_spawnattr_destroy(&attributes);
    if (input >= 0) close(input);
    for (int i = 0; i < 2; i++) for (int j = 0; j < 2; j++)
        if (pipes[i][j] >= 0) close(pipes[i][j]);
    if (child > 0) {
        if (cancelled) shell_http_cancel(child);
        if (failed || (timeout_ms > 0 && (overflow || timed_out)))
            shell_cancel(child, timeout_ms > 0, SIGKILL);
        int wait_status;
        pid_t waited;
        if (timeout_ms > 0 && !failed && !overflow && !timed_out && !cancelled) {
            for (;;) {
                waited = waitpid(child, &wait_status, WNOHANG);
                if (waited == child || (waited < 0 && errno != EINTR)) break;
                if (shell_http_cancel_requested(timeout_ms)) {
                    cancelled = 1;
                    shell_http_cancel(child);
                    do { waited = waitpid(child, &wait_status, 0); } while (waited < 0 && errno == EINTR);
                    break;
                }
                int64_t now = shell_monotonic_ms();
                if (now < 0 || now >= deadline) {
                    if (now < 0) failed = 1; else timed_out = 1;
                    shell_cancel(child, 1, SIGKILL);
                    do { waited = waitpid(child, &wait_status, 0); } while (waited < 0 && errno == EINTR);
                    break;
                }
                int delay = deadline - now < 10 ? (int)(deadline - now) : 10;
                (void)poll(NULL, 0, delay);
            }
        } else {
            do { waited = waitpid(child, &wait_status, 0); } while (waited < 0 && errno == EINTR);
        }
        if (waited == child && !failed && !overflow && !timed_out && !cancelled && WIFEXITED(wait_status)) status = WEXITSTATUS(wait_status);
    }
    FLValue output = shell_output(capture[0].data, capture[0].used);
    FLValue errors = shell_output(capture[1].data, capture[1].used);
    free(capture[0].data); free(capture[1].data);
    if (output.tag != FL_STRING || errors.tag != FL_STRING)
        fl_throw(fl_str_val("shell output allocation failed"));
    FLValue values[] = {fl_str_val("stdout"), fl_v11_utf8_string(output),
        fl_str_val("stderr"), fl_v11_utf8_string(errors),
        fl_str_val("exit"), fl_int(status), fl_str_val("ok"), fl_bool(status == 0)};
    return fl_map_from_pairs(values, 4);
}

FLValue fl_shell_exec_result(FLValue command) {
    return shell_exec_result(command, 0, -1);
}
/* shell_capture retains the module's 30-second budget and three-field result.
 * Cancellation kills only the process group created for this invocation. */
FLValue fl_shell_capture(FLValue command) {
    if (command.tag != FL_STRING || !command.obj)
        fl_throw(fl_make_error("TypeError", "shell_capture: string command required"));
    FLValue captured = shell_exec_result(command, 30000, 1);
    FLValue pairs[] = {fl_str_val("stdout"), fl_map_get(captured, fl_str_val("stdout")),
        fl_str_val("stderr"), fl_map_get(captured, fl_str_val("stderr")),
        fl_str_val("code"), fl_map_get(captured, fl_str_val("exit"))};
    return fl_map_from_pairs(pairs, 3);
}

/* Legacy generated C shares the language handler's validation and cleanup. */
FLValue process_exit(FLValue code) {
    return fl_v11_builtin(fl_str_val("process-exit"), fl_vec_from(&code, 1));
}

FLValue _fl_process_getcwd(void) {
    char buf[4096];
    if (!getcwd(buf, sizeof(buf))) return fl_nil();
    return fl_str_val(buf);
}
FLValue _fl_process_chdir(FLValue path) { return fl_bool(chdir(_strval(path)) == 0); }
FLValue _fl_process_pid(void)  { return fl_int((int64_t)getpid()); }
FLValue _fl_process_ppid(void) { return fl_int((int64_t)getppid()); }
FLValue _fl_process_kill(FLValue pid) { return fl_bool(kill((pid_t)pid.i, SIGTERM) == 0); }
FLValue _fl_process_exists(FLValue pid) { return fl_bool(kill((pid_t)pid.i, 0) == 0); }
FLValue _fl_process_wait(FLValue pid) {
    int status = 0;
    pid_t r = waitpid((pid_t)pid.i, &status, 0);
    return r < 0 ? fl_int(-1) : fl_int((int64_t)WEXITSTATUS(status));
}

/* popen으로 출력 캡처 — process-run */
FLValue _fl_process_run(FLValue cmd) {
    FILE* f = popen(_strval(cmd), "r");
    if (!f) return fl_nil();
    size_t sz = 0, cap = 4096;
    char* buf = malloc(cap);
    buf[0] = '\0';
    char tmp[1024];
    while (fgets(tmp, sizeof(tmp), f)) {
        size_t n = strlen(tmp);
        if (sz + n + 1 > cap) { cap = (sz + n + 1) * 2; buf = realloc(buf, cap); }
        memcpy(buf + sz, tmp, n); sz += n; buf[sz] = '\0';
    }
    pclose(f);
    FLValue r = fl_str_val(buf); free(buf); return r;
}

/* args 배열을 공백으로 join 후 popen */
FLValue _fl_process_run_args(FLValue cmd, FLValue args) {
    char full[8192];
    snprintf(full, sizeof(full), "%s", _strval(cmd));
    if (args.tag == FL_VECTOR) {
        FLVector* v = (FLVector*)args.obj;
        for (uint32_t i = 0; i < v->len; i++) {
            strncat(full, " ", sizeof(full) - strlen(full) - 1);
            strncat(full, _strval(v->data[i]), sizeof(full) - strlen(full) - 1);
        }
    }
    return _fl_process_run(fl_str_val(full));
}

/* system()으로 상속 stdio 실행 */
FLValue _fl_run_inherit(FLValue cmd) {
    int r = system(_strval(cmd));
    return fl_int((int64_t)(r >= 0 ? WEXITSTATUS(r) : -1));
}

/* process-exec: 현재 프로세스를 cmd로 교체 (execvp) */
FLValue _fl_process_exec(FLValue cmd) {
    char buf[4096]; strncpy(buf, _strval(cmd), sizeof(buf)-1); buf[4095]='\0';
    char* argv[256]; int argc = 0;
    char* tok = strtok(buf, " \t");
    while (tok && argc < 255) { argv[argc++] = tok; tok = strtok(NULL, " \t"); }
    argv[argc] = NULL;
    execvp(argv[0], argv);
    return fl_int(-1); /* execvp 실패 시만 도달 */
}

FLValue _fl_process_exec_args(FLValue cmd, FLValue args) {
    char full[8192]; snprintf(full, sizeof(full), "%s", _strval(cmd));
    if (args.tag == FL_VECTOR) {
        FLVector* v = (FLVector*)args.obj;
        for (uint32_t i = 0; i < v->len; i++) {
            strncat(full, " ", sizeof(full)-strlen(full)-1);
            strncat(full, _strval(v->data[i]), sizeof(full)-strlen(full)-1);
        }
    }
    return _fl_process_exec(fl_str_val(full));
}

/* fork + exec → 자식 PID 반환 */
FLValue _fl_process_spawn(FLValue cmd, FLValue args) {
    char full[8192]; snprintf(full, sizeof(full), "%s", _strval(cmd));
    if (args.tag == FL_VECTOR) {
        FLVector* v = (FLVector*)args.obj;
        for (uint32_t i = 0; i < v->len; i++) {
            strncat(full, " ", sizeof(full)-strlen(full)-1);
            strncat(full, _strval(v->data[i]), sizeof(full)-strlen(full)-1);
        }
    }
    pid_t pid = fork();
    if (pid < 0) return fl_int(-1);
    if (pid == 0) { /* 자식 */
        _fl_process_exec(fl_str_val(full));
        _exit(1);
    }
    return fl_int((int64_t)pid);
}

/* ── try/catch 런타임 ── (runtime.h의 __thread 선언 사용) */

#include "v11-tty.h"
#include "v11-timer.h"
#include "v11-tcp.h"
