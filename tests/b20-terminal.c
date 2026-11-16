#define _GNU_SOURCE
#include "runtime.h"
#include <errno.h>
#include <pty.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <termios.h>
#include <unistd.h>

static FLValue call(const char *name) {
    return fl_v11_builtin(fl_str_val(name), fl_vec_new());
}

static int same_mode(const struct termios *a, const struct termios *b) {
    return a->c_iflag == b->c_iflag && a->c_oflag == b->c_oflag &&
           a->c_cflag == b->c_cflag && a->c_lflag == b->c_lflag &&
           memcmp(a->c_cc, b->c_cc, NCCS) == 0;
}

static int mode_is_raw(const struct termios *t) {
    return !(t->c_lflag & (ICANON | ECHO));
}

static int uncaught_exit_restores(int slave, const struct termios *before) {
    pid_t pid = fork();
    if (pid < 0) return 0;
    if (!pid) {
        if (dup2(slave, STDIN_FILENO) < 0) _exit(20);
        (void)call("tty-raw-enter!");
        exit(17); /* atexit must restore the PTY on an uncaught runtime exit */
    }
    int status = 0;
    if (waitpid(pid, &status, 0) != pid || !WIFEXITED(status) || WEXITSTATUS(status) != 17) return 0;
    struct termios after;
    return tcgetattr(slave, &after) == 0 && same_mode(before, &after);
}

static int signal_exit_restores(int slave, const struct termios *before) {
    pid_t pid = fork();
    if (pid < 0) return 0;
    if (!pid) {
        if (dup2(slave, STDIN_FILENO) < 0) _exit(20);
        (void)call("tty-raw-enter!");
        raise(SIGTERM);
        _exit(21);
    }
    int status = 0;
    if (waitpid(pid, &status, 0) != pid || !WIFSIGNALED(status) || WTERMSIG(status) != SIGTERM) return 0;
    struct termios after;
    return tcgetattr(slave, &after) == 0 && same_mode(before, &after);
}

int main(void) {
    if (!fl_truthy(fl_v11_has(fl_str_val("tty_raw_enter!"))) ||
        !fl_truthy(fl_v11_has(fl_str_val("tty_raw_leave!")))) return 1;
    int master = -1, slave = -1;
    if (openpty(&master, &slave, NULL, NULL, NULL)) return 2;
    struct termios before, raw, nested, after;
    if (tcgetattr(slave, &before)) return 3;
    int saved = dup(STDIN_FILENO);
    if (saved < 0 || dup2(slave, STDIN_FILENO) < 0) return 4;
    if (call("tty-raw-enter!").tag == FL_NIL || tcgetattr(slave, &raw) || !mode_is_raw(&raw)) return 5;
    (void)call("tty-raw-enter!");
    (void)call("tty-raw-leave!");
    if (tcgetattr(slave, &nested) || !mode_is_raw(&nested)) return 6;
    (void)call("tty-raw-leave!");
    if (tcgetattr(slave, &after) || !same_mode(&before, &after)) return 7;
    if (dup2(saved, STDIN_FILENO) < 0) return 8;
    close(saved);
    int pipefd[2];
    if (pipe(pipefd)) return 9;
    saved = dup(STDIN_FILENO);
    if (saved < 0 || dup2(pipefd[0], STDIN_FILENO) < 0) return 10;
    close(pipefd[0]); close(pipefd[1]);
    int frame = fl_try_top;
    FLTryFrame *try_frame = &fl_try_stack[fl_try_top++];
    if (setjmp(try_frame->buf) == 0) {
        (void)call("tty-raw-enter!");
        return 11;
    }
    fl_try_top = frame;
    FLValue message = fl_map_get(try_frame->err, fl_str_val("message"));
    if (message.tag != FL_STRING || strcmp(((FLString*)message.obj)->data,
            "tty-raw-enter!: stdin is not a terminal")) return 12;
    if (dup2(saved, STDIN_FILENO) < 0) return 13;
    close(saved);
    if (!uncaught_exit_restores(slave, &before)) return 14;
    if (!signal_exit_restores(slave, &before)) return 15;
    close(master); close(slave);
    puts("TTY_RAW_NESTING=PASS");
    puts("TTY_RAW_NON_TTY_ERROR=PASS");
    puts("TTY_RAW_UNCAUGHT_EXIT_RESTORE=PASS");
    puts("TTY_RAW_SIGNAL_RESTORE=PASS");
    return 0;
}
