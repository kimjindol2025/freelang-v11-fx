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
FLValue fl_shell_exec_result(FLValue command) {
    FLValue text = fl_v11_utf8_string(command);
    FLString *cmd = (FLString *)text.obj;
    if (memchr(cmd->data, 0, cmd->len))
        fl_throw(fl_make_error("TypeError", "shell command must not contain null bytes"));
    int pipes[2][2] = {{-1,-1},{-1,-1}}, input = -1, status = -1;
    int failed = 0, overflow = 0, killed = 0;
    pid_t child = -1;
    FLShellCapture capture[2] = {{0},{0}};
    posix_spawn_file_actions_t actions;
    int actions_ready = 0;
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
    char *argv[] = {"sh", "-c", cmd->data, NULL};
    if (posix_spawnp(&child, "sh", &actions, NULL, argv, environ)) { child = -1; goto finish; }
    close(input); input = -1;
    close(pipes[0][1]); pipes[0][1] = -1;
    close(pipes[1][1]); pipes[1][1] = -1;
    while (pipes[0][0] >= 0 || pipes[1][0] >= 0) {
        struct pollfd ready[2] = {{pipes[0][0],POLLIN,0},{pipes[1][0],POLLIN,0}};
        int result;
        do { result = poll(ready, 2, -1); } while (result < 0 && errno == EINTR);
        if (result < 0) { failed = 1; break; }
        for (int i = 0; i < 2; i++) {
            if (pipes[i][0] < 0 || !ready[i].revents) continue;
            char buffer[8192];
            for (;;) {
                ssize_t n = read(pipes[i][0], buffer, sizeof buffer);
                if (n > 0) {
                    if (shell_capture_add(&capture[i], buffer, (size_t)n, &overflow)) { failed = 1; break; }
                    if (overflow && !killed) { (void)kill(child, SIGTERM); killed = 1; }
                    if (overflow) break;
                } else if (!n) { close(pipes[i][0]); pipes[i][0] = -1; break; }
                else if (errno == EINTR) continue;
                else if (errno == EAGAIN || errno == EWOULDBLOCK) break;
                else { failed = 1; break; }
            }
            if (failed || overflow) break;
        }
        if (failed || overflow) break;
    }
finish:
    if (actions_ready) posix_spawn_file_actions_destroy(&actions);
    if (input >= 0) close(input);
    for (int i = 0; i < 2; i++) for (int j = 0; j < 2; j++)
        if (pipes[i][j] >= 0) close(pipes[i][j]);
    if (child > 0) {
        if (failed) (void)kill(child, SIGKILL);
        int wait_status;
        pid_t waited;
        do { waited = waitpid(child, &wait_status, 0); } while (waited < 0 && errno == EINTR);
        if (waited == child && !failed && !overflow && WIFEXITED(wait_status)) status = WEXITSTATUS(wait_status);
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
