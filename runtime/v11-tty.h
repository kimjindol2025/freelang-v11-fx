#ifndef FL_V11_TTY_H
#define FL_V11_TTY_H

/* Linux/POSIX terminal bridge. Raw access is opt-in and owned by the main
 * thread; a Native host supervises signal termination separately. */
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <math.h>
#include <poll.h>
#include <pthread.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <sys/syscall.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>

static pthread_mutex_t fl_tty_lock = PTHREAD_MUTEX_INITIALIZER;
static int fl_tty_fd = -1, fl_tty_watched, fl_tty_host_active;
static pthread_t fl_tty_owner;
static struct termios fl_tty_original;

static int fl_tty_allowed(void) {
    const char *value = getenv("FREELANG_NATIVE_TTY");
    return value && !strcmp(value, "1");
}
static int fl_tty_setattr(int fd, const struct termios *modes) {
    sigset_t blocked, previous;
    sigemptyset(&blocked); sigaddset(&blocked, SIGTTOU);
    int error = pthread_sigmask(SIG_BLOCK, &blocked, &previous);
    if (error) { errno = error; return -1; }
    int result;
    do { result = tcsetattr(fd, TCSANOW, modes); } while (result < 0 && errno == EINTR);
    int saved = errno;
    error = pthread_sigmask(SIG_SETMASK, &previous, NULL);
    if (error && !result) { result = -1; saved = error; }
    errno = saved;
    return result;
}
static void fl_tty_cleanup(void) {
    pthread_mutex_lock(&fl_tty_lock);
    if (fl_tty_fd >= 0) {
        if (fl_tty_setattr(fl_tty_fd, &fl_tty_original) < 0) {
            const char error[] = "FreeLang: terminal restoration failed\n";
            (void)write(STDERR_FILENO, error, sizeof error - 1);
        }
        close(fl_tty_fd); fl_tty_fd = -1;
    }
    pthread_mutex_unlock(&fl_tty_lock);
}
static FLValue fl_tty_error(const char *message) {
    fl_throw(fl_make_error("TerminalError", message));
    return fl_nil();
}
static FLValue fl_tty_enter(void) {
    if (!fl_tty_allowed()) return fl_tty_error("[E_TTY_CAPABILITY] Native TTY capability is not enabled");
    const char *error = NULL;
    pthread_mutex_lock(&fl_tty_lock);
    if (fl_tty_host_active) error = "[E_TTY_BUSY] a terminal host is active";
    else if (fl_tty_fd >= 0) {
        if (!pthread_equal(fl_tty_owner, pthread_self())) error = "[E_TTY_BUSY] terminal belongs to another thread";
    } else if (syscall(SYS_gettid) != getpid()) error = "[E_TTY_THREAD] raw mode requires the main thread";
    else if (!isatty(STDIN_FILENO)) error = "[E_TTY_NOT_TERMINAL] standard input is not a terminal";
    else if (tcgetpgrp(STDIN_FILENO) != getpgrp()) error = "[E_TTY_FOREGROUND] terminal is not controlled by this foreground group";
    else {
        int fd = fcntl(STDIN_FILENO, F_DUPFD_CLOEXEC, STDERR_FILENO + 1);
        struct termios original;
        if (fd < 0 || tcgetattr(fd, &original) < 0) {
            if (fd >= 0) close(fd);
            error = "[E_TTY_IO] unable to save terminal settings";
        } else if (!fl_tty_watched && atexit(fl_tty_cleanup)) {
            close(fd);
            error = "[E_TTY_IO] unable to register terminal restoration";
        } else {
            fl_tty_watched = 1;
            fl_tty_fd = fd; fl_tty_original = original; fl_tty_owner = pthread_self();
            struct termios raw = original;
            cfmakeraw(&raw);
            raw.c_cc[VMIN] = 1; raw.c_cc[VTIME] = 0;
            if (fl_tty_setattr(fd, &raw) < 0) {
                if (!fl_tty_setattr(fd, &original)) { close(fd); fl_tty_fd = -1; }
                error = "[E_TTY_IO] unable to enter terminal raw mode";
            }
        }
    }
    pthread_mutex_unlock(&fl_tty_lock);
    return error ? fl_tty_error(error) : fl_bool(true);
}
static FLValue fl_tty_leave(void) {
    const char *error = NULL;
    int restored = 0;
    pthread_mutex_lock(&fl_tty_lock);
    if (fl_tty_fd >= 0) {
        if (!pthread_equal(fl_tty_owner, pthread_self())) error = "[E_TTY_BUSY] terminal belongs to another thread";
        else if (fl_tty_setattr(fl_tty_fd, &fl_tty_original) < 0) error = "[E_TTY_IO] unable to restore terminal settings";
        else { close(fl_tty_fd); fl_tty_fd = -1; restored = 1; }
    } else if (!fl_tty_allowed()) error = "[E_TTY_CAPABILITY] Native TTY capability is not enabled";
    else if (!isatty(STDIN_FILENO)) error = "[E_TTY_NOT_TERMINAL] standard input is not a terminal";
    pthread_mutex_unlock(&fl_tty_lock);
    return error ? fl_tty_error(error) : fl_bool(restored);
}
static FLValue fl_tty_read_byte(int timeout_ms) {
    if (!fl_tty_allowed()) return fl_tty_error("[E_TTY_CAPABILITY] Native TTY capability is not enabled");
    const char *error = NULL;
    int byte = -1, fd = -1;
    pthread_mutex_lock(&fl_tty_lock);
    if (fl_tty_fd < 0) error = "[E_TTY_NOT_RAW] enter terminal raw mode before reading";
    else if (!pthread_equal(fl_tty_owner, pthread_self())) error = "[E_TTY_BUSY] terminal belongs to another thread";
    else if (tcgetpgrp(fl_tty_fd) != getpgrp()) error = "[E_TTY_FOREGROUND] terminal foreground ownership changed";
    else if ((fd = fcntl(fl_tty_fd, F_DUPFD_CLOEXEC, STDERR_FILENO + 1)) < 0)
        error = "[E_TTY_IO] unable to reserve terminal input";
    pthread_mutex_unlock(&fl_tty_lock);
    if (!error) {
        struct timespec start, now;
        int first = 1;
        if (timeout_ms >= 0 && clock_gettime(CLOCK_MONOTONIC, &start)) error = "[E_TTY_IO] terminal read clock failed";
        while (!error) {
            int delay = -1;
            if (timeout_ms >= 0) {
                if (clock_gettime(CLOCK_MONOTONIC, &now)) { error = "[E_TTY_IO] terminal read clock failed"; break; }
                int64_t remaining = (int64_t)timeout_ms * 1000000 -
                    ((int64_t)(now.tv_sec - start.tv_sec) * 1000000000 + now.tv_nsec - start.tv_nsec);
                if (remaining <= 0 && !first) break;
                delay = remaining > 0 ? (int)((remaining + 999999) / 1000000) : 0;
            }
            first = 0;
            struct pollfd input = {fd, POLLIN, 0};
            int ready = poll(&input, 1, delay);
            if (ready < 0 && errno == EINTR) continue;
            if (ready < 0) { error = "[E_TTY_IO] terminal poll failed"; break; }
            if (!ready) break;
            if (input.revents & POLLIN) {
                unsigned char value;
                ssize_t n = read(fd, &value, 1);
                if (n == 1) { byte = value; break; }
                if (n < 0 && (errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK)) continue;
                error = n ? "[E_TTY_IO] terminal read failed" : "[E_TTY_CLOSED] terminal input closed";
            } else error = "[E_TTY_CLOSED] terminal input is unavailable";
        }
    }
    if (fd >= 0) close(fd);
    return error ? fl_tty_error(error) : byte < 0 ? fl_nil() : fl_int(byte);
}

/* tty-run is a dedicated, child-free Native terminal supervisor. */
#include <dirent.h>
#include <limits.h>
#include <stdio.h>
#include <sys/eventfd.h>
#include <sys/prctl.h>
#include <sys/wait.h>

static int64_t fl_tty_now(void) {
    struct timespec now;
    return clock_gettime(CLOCK_MONOTONIC, &now) ? -1 :
        (int64_t)now.tv_sec * 1000 + now.tv_nsec / 1000000;
}
static int fl_tty_single_thread(void) {
    if (syscall(SYS_gettid) != getpid()) return 0;
    DIR *directory = opendir("/proc/self/task");
    if (!directory) return 0;
    int count = 0;
    struct dirent *entry;
    while ((entry = readdir(directory)))
        if (entry->d_name[0] >= '0' && entry->d_name[0] <= '9') count++;
    closedir(directory);
    return count == 1;
}
/* Only this dedicated host can create children during supervision. Kernel
 * child IDs remain owned until this same thread reaps them, avoiding PID reuse. */
static int fl_tty_children(int signal_number) {
    char path[80];
    snprintf(path, sizeof path, "/proc/self/task/%ld/children", (long)getpid());
    int fd = open(path, O_RDONLY | O_CLOEXEC);
    if (fd < 0) return -1;
    unsigned long number = 0;
    int have = 0, count = 0, failed = 0;
    char buffer[256];
    for (;;) {
        ssize_t n = read(fd, buffer, sizeof buffer);
        if (n < 0 && errno == EINTR) continue;
        if (n < 0) { failed = 1; break; }
        for (ssize_t i = 0; i <= n; i++) {
            char c = i == n ? ' ' : buffer[i];
            if (i == n && n) break;
            if (c >= '0' && c <= '9') {
                if (number > ((unsigned long)INT_MAX - (c - '0')) / 10) { failed = 1; break; }
                number = number * 10 + c - '0'; have = 1;
            } else if (have) {
                if (!number || count == INT_MAX) { failed = 1; break; }
                if (signal_number && kill((pid_t)number, signal_number) < 0 && errno != ESRCH) { failed = 1; break; }
                count++; number = 0; have = 0;
            }
        }
        if (failed || !n) break;
    }
    close(fd);
    return failed ? -1 : count;
}
static int fl_tty_reap_children(void) {
    int failed = fl_tty_children(SIGTERM) < 0;
    int64_t start = fl_tty_now();
    for (;;) {
        int status;
        pid_t reaped = waitpid(-1, &status, WNOHANG);
        if (reaped > 0) continue;
        if (reaped < 0) {
            if (errno == EINTR) continue;
            return errno == ECHILD && !failed ? 0 : -1;
        }
        int64_t now = fl_tty_now();
        if (start < 0 || now < 0 || now - start >= 500) {
            if (fl_tty_children(SIGKILL) < 0) return -1;
            do { reaped = waitpid(-1, &status, 0); } while (reaped < 0 && errno == EINTR);
            if (reaped < 0 && errno != ECHILD) return -1;
        } else {
            struct timespec pause = {0, 10000000};
            nanosleep(&pause, NULL);
        }
    }
}
static FLValue fl_tty_run(FLValue path, FLValue arguments, FLValue cleanup) {
    if (path.tag != FL_STRING || arguments.tag != FL_VECTOR)
        return fl_tty_error("[E_ARG_TYPE] tty-run expects an executable path and argument vector");
    FLValue encoded = fl_v11_utf8_string(path);
    FLString *binary = (FLString *)encoded.obj;
    if (!binary->len || memchr(binary->data, 0, binary->len))
        return fl_tty_error("[E_ARG_TYPE] tty-run executable path must be nonempty and NUL-free");
    if (cleanup.tag != FL_NIL && cleanup.tag != FL_STRING)
        return fl_tty_error("[E_ARG_TYPE] terminal cleanup must be a string or nil");
    FLValue cleanup_text = cleanup.tag == FL_NIL ? fl_str_val("") : fl_v11_utf8_string(cleanup);
    FLString *cleanup_bytes = (FLString *)cleanup_text.obj;
    FLVector *input = (FLVector *)arguments.obj;
    FLValue converted = fl_vec_from(input->data, input->len);
    FLVector *values = (FLVector *)converted.obj;
    for (uint32_t i = 0; i < values->len; i++) {
        if (values->data[i].tag != FL_STRING)
            return fl_tty_error("[E_ARG_TYPE] tty-run arguments must be strings");
        values->data[i] = fl_v11_utf8_string(values->data[i]);
        FLString *text = (FLString *)values->data[i].obj;
        if (memchr(text->data, 0, text->len))
            return fl_tty_error("[E_ARG_TYPE] tty-run arguments must be NUL-free");
    }
    if (!fl_tty_single_thread() || fl_tty_children(0) != 0)
        return fl_tty_error("[E_TTY_HOST_CONTEXT] tty-run requires a dedicated main thread with no existing children");
    if (!isatty(STDIN_FILENO))
        return fl_tty_error("[E_TTY_NOT_TERMINAL] standard input is not a terminal");
    pid_t original_group = tcgetpgrp(STDIN_FILENO);
    if (original_group != getpgrp())
        return fl_tty_error("[E_TTY_FOREGROUND] tty-run requires foreground terminal ownership");
    if (fl_tty_now() < 0) return fl_tty_error("[E_TTY_IO] terminal host clock failed");
    size_t env_count = 0;
    while (environ[env_count]) env_count++;
    size_t argument_count = values->len;
    if (argument_count > SIZE_MAX / sizeof(char *) - 2 ||
        env_count > SIZE_MAX / sizeof(char *) - 2)
        return fl_tty_error("[E_TTY_MEMORY] terminal host argument allocation failed");
    char **argv = calloc(argument_count + 2, sizeof *argv);
    char **environment = calloc(env_count + 2, sizeof *environment);
    if (!argv || !environment) {
        free(argv); free(environment);
        return fl_tty_error("[E_TTY_MEMORY] terminal host argument allocation failed");
    }
    argv[0] = binary->data;
    for (uint32_t i = 0; i < values->len; i++) argv[i + 1] = ((FLString *)values->data[i].obj)->data;
    size_t used = 0;
    const char capability[] = "FREELANG_NATIVE_TTY=";
    for (size_t i = 0; i < env_count; i++)
        if (strncmp(environ[i], capability, sizeof capability - 1)) environment[used++] = environ[i];
    environment[used] = "FREELANG_NATIVE_TTY=1";

    const char *error = NULL;
    int fd = -1, gate = -1, active = 0, masked = 0, installed = 0;
    int old_subreaper = 0, subreaper_saved = 0, group_ready = 0, child_owned = 0;
    int result = 1, control_signal = 0, hard_killed = 0, broken_pipe = 0, pipe_was_pending = 0;
    int64_t termination_start = -1;
    pid_t child = -1;
    struct termios original;
    int original_saved = 0;
    const int signals[] = {SIGINT, SIGQUIT, SIGHUP, SIGTERM, SIGCHLD};
    struct sigaction previous[sizeof signals / sizeof signals[0]];
    struct sigaction defaults = {.sa_handler = SIG_DFL, .sa_flags = 0};
    sigemptyset(&defaults.sa_mask);
    sigset_t catches, blocked, old_mask, old_pending;
    sigemptyset(&catches);
    for (size_t i = 0; i < sizeof signals / sizeof signals[0]; i++) sigaddset(&catches, signals[i]);
    blocked = catches; sigaddset(&blocked, SIGTTOU); sigaddset(&blocked, SIGPIPE);
    pthread_mutex_lock(&fl_tty_lock);
    if (fl_tty_fd >= 0 || fl_tty_host_active) error = "[E_TTY_BUSY] a terminal session is already active";
    else { fl_tty_host_active = 1; active = 1; }
    pthread_mutex_unlock(&fl_tty_lock);
    if (error) goto cleanup;
    fd = fcntl(STDIN_FILENO, F_DUPFD_CLOEXEC, STDERR_FILENO + 1);
    if (fd < 0 || tcgetattr(fd, &original)) { error = "[E_TTY_IO] unable to save terminal host settings"; goto cleanup; }
    original_saved = 1;
    if (pthread_sigmask(SIG_BLOCK, &blocked, &old_mask)) { error = "[E_TTY_IO] unable to protect terminal host signals"; goto cleanup; }
    masked = 1;
    if (sigpending(&old_pending)) { error = "[E_TTY_IO] unable to inspect terminal signal state"; goto cleanup; }
    pipe_was_pending = sigismember(&old_pending, SIGPIPE);
    for (size_t i = 0; i < sizeof signals / sizeof signals[0]; i++) {
        if (sigaction(signals[i], &defaults, &previous[i])) { error = "[E_TTY_IO] unable to configure terminal host signals"; goto cleanup; }
        installed++;
    }
    if (prctl(PR_GET_CHILD_SUBREAPER, &old_subreaper) ||
        prctl(PR_SET_CHILD_SUBREAPER, 1L)) { error = "[E_TTY_SUPERVISOR] child supervision is unavailable"; goto cleanup; }
    subreaper_saved = 1;
    gate = shell_private_fd(eventfd(0, EFD_CLOEXEC));
    if (gate < 0) { error = "[E_TTY_IO] terminal startup handshake failed"; goto cleanup; }
    child = fork();
    if (child < 0) { error = "[E_TTY_IO] unable to start terminal child"; goto cleanup; }
    if (!child) {
        close(fd);
        if (setpgid(0, 0)) _exit(126);
        uint64_t ready;
        ssize_t n;
        do { n = read(gate, &ready, sizeof ready); } while (n < 0 && errno == EINTR);
        close(gate);
        if (n != sizeof ready || ready != 1) _exit(126);
        for (size_t i = 0; i < sizeof signals / sizeof signals[0]; i++)
            if (sigaction(signals[i], &defaults, NULL)) _exit(126);
        if (sigprocmask(SIG_SETMASK, &old_mask, NULL)) _exit(126);
        execve(binary->data, argv, environment);
        const char message[] = "[E_TTY_EXEC] unable to execute Native terminal child\n";
        (void)write(STDERR_FILENO, message, sizeof message - 1);
        _exit(127);
    }
    child_owned = 1;
    if (setpgid(child, child)) { error = "[E_TTY_IO] unable to isolate terminal child group"; goto cleanup; }
    group_ready = 1;
    if (tcsetpgrp(fd, child)) { error = "[E_TTY_IO] unable to give the child terminal foreground ownership"; goto cleanup; }
    uint64_t ready = 1;
    ssize_t sent;
    do { sent = write(gate, &ready, sizeof ready); } while (sent < 0 && errno == EINTR);
    close(gate); gate = -1;
    if (sent != sizeof ready) { error = "[E_TTY_IO] terminal startup handshake failed"; goto cleanup; }
    for (;;) {
        siginfo_t state = {0};
        if (waitid(P_PID, (id_t)child, &state, WEXITED | WNOWAIT | WNOHANG)) {
            if (errno == EINTR) continue;
            if (errno == ECHILD) child_owned = 0;
            error = "[E_TTY_WAIT] unable to observe terminal child exit"; break;
        }
        if (state.si_pid == child) {
            result = state.si_code == CLD_EXITED ? state.si_status : 128 + state.si_status;
            break;
        }
        siginfo_t observed;
        int received;
        if (control_signal && !hard_killed) {
            int64_t now = fl_tty_now();
            int64_t remaining = now < 0 ? 0 : 500 - (now - termination_start);
            if (remaining <= 0) {
                if (fl_tty_children(SIGKILL) < 0) { error = "[E_TTY_WAIT] unable to terminate owned terminal children"; break; }
                hard_killed = 1;
                continue;
            }
            struct timespec delay = {remaining / 1000, (remaining % 1000) * 1000000};
            received = sigtimedwait(&catches, &observed, &delay);
            if (received < 0 && errno == EAGAIN) continue;
        } else received = sigwaitinfo(&catches, &observed);
        if (received < 0) {
            if (errno == EINTR) continue;
            error = "[E_TTY_WAIT] terminal signal wait failed"; break;
        }
        if (received != SIGCHLD && !control_signal) {
            control_signal = received; termination_start = fl_tty_now();
            (void)kill(-child, SIGCONT);
            (void)kill(-child, received);
        }
    }
cleanup:
    if (gate >= 0) close(gate);
    if (child_owned) {
        if (!group_ready) (void)kill(child, SIGKILL);
        if (fl_tty_reap_children() < 0) error = "[E_TTY_WAIT] unable to reap owned terminal children";
    }
    if (original_saved) {
        if (fl_tty_setattr(fd, &original) < 0) error = "[E_TTY_IO] unable to restore terminal host settings";
        if (tcsetpgrp(fd, original_group) < 0) error = "[E_TTY_IO] unable to restore terminal foreground ownership";
    }
    if (child > 0) {
        size_t written = 0;
        while (written < cleanup_bytes->len) {
            ssize_t n = write(STDOUT_FILENO, cleanup_bytes->data + written, cleanup_bytes->len - written);
            if (n < 0 && errno == EINTR) continue;
            if (n <= 0) {
                broken_pipe = n < 0 && errno == EPIPE;
                error = "[E_TTY_IO] unable to write terminal cleanup controls"; break;
            }
            written += (size_t)n;
        }
    }
    if (fd >= 0) close(fd);
    if (subreaper_saved && prctl(PR_SET_CHILD_SUBREAPER, (long)old_subreaper))
        error = "[E_TTY_SUPERVISOR] unable to restore child supervision state";
    if (masked) {
        struct timespec immediate = {0, 0};
        siginfo_t pending;
        int received;
        while ((received = sigtimedwait(&catches, &pending, &immediate)) > 0)
            if (received != SIGCHLD && !control_signal) control_signal = received;
        if (broken_pipe && !pipe_was_pending) {
            sigset_t pipe_signal;
            sigemptyset(&pipe_signal); sigaddset(&pipe_signal, SIGPIPE);
            (void)sigtimedwait(&pipe_signal, &pending, &immediate);
        }
        for (int i = installed - 1; i >= 0; i--)
            if (sigaction(signals[i], &previous[i], NULL)) error = "[E_TTY_IO] unable to restore terminal signal dispositions";
        if (pthread_sigmask(SIG_SETMASK, &old_mask, NULL)) error = "[E_TTY_IO] unable to restore terminal signal mask";
    }
    if (active) {
        pthread_mutex_lock(&fl_tty_lock); fl_tty_host_active = 0; pthread_mutex_unlock(&fl_tty_lock);
    }
    free(argv); free(environment);
    return error ? fl_tty_error(error) : fl_int(control_signal ? 128 + control_signal : result);
}

FLValue fl_v11_tty(FLValue name, FLValue argv) {
    const char *op = _strval(name);
    if (!strcmp(op, "tty-run")) {
        if (argv.tag != FL_VECTOR || ((FLVector *)argv.obj)->len < 2 || ((FLVector *)argv.obj)->len > 3)
            return fl_tty_error("[E_ARG_COUNT] tty-run expects two or three arguments");
        FLVector *args = (FLVector *)argv.obj;
        return fl_tty_run(args->data[0], args->data[1], args->len == 3 ? args->data[2] : fl_nil());
    }
    if (!strcmp(op, "tty-read-byte")) {
        if (argv.tag != FL_VECTOR || ((FLVector *)argv.obj)->len > 1)
            return fl_tty_error("[E_ARG_COUNT] tty-read-byte expects zero or one argument");
        int timeout = -1;
        FLVector *args = (FLVector *)argv.obj;
        if (args->len) {
            FLValue value = args->data[0];
            double requested = value.tag == FL_INT ? (double)value.i : value.tag == FL_FLOAT ? value.f : NAN;
            if (!isfinite(requested) || requested < 0 || requested > INT_MAX || trunc(requested) != requested)
                return fl_tty_error("[E_ARG_TYPE] terminal timeout must be a nonnegative integer");
            timeout = (int)requested;
        }
        return fl_tty_read_byte(timeout);
    }
    if (argv.tag != FL_VECTOR || ((FLVector *)argv.obj)->len)
        return fl_tty_error("[E_ARG_COUNT] terminal operation expects zero arguments");
    if (!strcmp(op, "tty-raw-enter!")) return fl_tty_enter();
    if (!strcmp(op, "tty-raw-leave!")) return fl_tty_leave();
    return fl_tty_error("[E_TTY_OPERATION] unknown terminal operation");
}
#endif
