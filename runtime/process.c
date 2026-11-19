#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
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
#include <limits.h>

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

/* Complete shell-command receipt. Unlike process-run, neither stderr nor the
 * wait status is discarded. The cap bounds capture memory, not child runtime.
 * On a capture failure we still drain/reap this child before throwing. */
#define FL_SHELL_CAPTURE_LIMIT (8u * 1024u * 1024u)
#if defined(__GLIBC__)
#if __GLIBC_PREREQ(2,34)
#define FL_HAVE_SPAWN_CLOSEFROM 1
#endif
#endif
static int fl_shell_pipe(int descriptors[2]) {
    if(pipe2(descriptors,O_CLOEXEC))return -1;
    /* Keep spawn-action sources above stdio even if the parent closed one. */
    for(int i=0;i<2;i++)if(descriptors[i]<3){
        int moved=fcntl(descriptors[i],F_DUPFD_CLOEXEC,3);
        if(moved<0){int saved=errno;close(descriptors[0]);close(descriptors[1]);descriptors[0]=descriptors[1]=-1;errno=saved;return -1;}
        close(descriptors[i]);descriptors[i]=moved;
    }
    return 0;
}
typedef struct { char *bytes; size_t size, capacity; } FLShellCapture;
static int fl_shell_capture(FLShellCapture *out,const char *bytes,size_t length,size_t *total) {
    if(length>FL_SHELL_CAPTURE_LIMIT-*total)return 0;
    *total+=length;
    size_t needed=out->size+length+1;
    if(needed>out->capacity){
        size_t capacity=out->capacity?out->capacity:4096;
        while(capacity<needed)capacity*=2;
        if(capacity>FL_SHELL_CAPTURE_LIMIT+1u)capacity=FL_SHELL_CAPTURE_LIMIT+1u;
        char *next=realloc(out->bytes,capacity);
        if(!next)return 0;
        out->bytes=next;out->capacity=capacity;
    }
    memcpy(out->bytes+out->size,bytes,length);out->size+=length;out->bytes[out->size]=0;
    return 1;
}
static FLValue fl_shell_bytes(const FLShellCapture *capture) {
    FLString *value=fl_arena_alloc(sizeof(FLString)+capture->size+1);
    if(!value){fl_throw(fl_make_error("Error","shell-exec-result: result allocation failed"));return fl_nil();}
    value->base.type=FL_STRING;value->base.rc=1;value->len=(uint32_t)capture->size;
    if(capture->size)memcpy(value->data,capture->bytes,capture->size);
    value->data[capture->size]=0;
    return (FLValue){.tag=FL_STRING,.obj=(FLObject*)value};
}
FLValue fl_shell_exec_result(FLValue command) {
    if(command.tag!=FL_STRING){fl_throw(fl_make_error("TypeError","shell-exec-result: string required"));return fl_nil();}
    command=fl_v11_utf8_string(command);
    FLString *text=(FLString*)command.obj;
    if(text->len>1024u*1024u||text->len!=strlen(text->data)){
        fl_throw(fl_make_error("TypeError","shell-exec-result: invalid command bytes or length"));return fl_nil();
    }
#ifdef FL_HAVE_SPAWN_CLOSEFROM
    int out[2]={-1,-1},err[2]={-1,-1};
    int failure=0,spawned=0,status=0,action_ready=0;
    pid_t child=-1;posix_spawn_file_actions_t actions;
    FLShellCapture captures[2]={{0},{0}};size_t total=0;
    if(fl_shell_pipe(out)||fl_shell_pipe(err)){failure=1;goto done;}
    if(posix_spawn_file_actions_init(&actions)){failure=1;goto done;}
    action_ready=1;
    /* closefrom also prevents a shell from retaining HTTP/listener/file FDs.
     * Duplication is ordered first; the child's stdin cannot consume app input. */
    if(posix_spawn_file_actions_addopen(&actions,STDIN_FILENO,"/dev/null",O_RDONLY,0)||
       posix_spawn_file_actions_adddup2(&actions,out[1],STDOUT_FILENO)||
       posix_spawn_file_actions_adddup2(&actions,err[1],STDERR_FILENO)||
       posix_spawn_file_actions_addclosefrom_np(&actions,3)){failure=1;goto done;}
    char *argv[]={"sh","-c",text->data,NULL};extern char **environ;
    if(posix_spawn(&child,"/bin/sh",&actions,NULL,argv,environ)){failure=1;goto done;}
    spawned=1;close(out[1]);out[1]=-1;close(err[1]);err[1]=-1;
    struct pollfd readers[2]={{.fd=out[0],.events=POLLIN},{.fd=err[0],.events=POLLIN}};
    while(readers[0].fd>=0||readers[1].fd>=0){
        int ready=poll(readers,2,-1);
        if(ready<0){if(errno==EINTR)continue;failure=1;break;}
        for(int i=0;i<2;i++){
            if(readers[i].fd<0||!readers[i].revents)continue;
            if(readers[i].revents&POLLNVAL){failure=1;readers[i].fd=-1;continue;}
            char bytes[8192];ssize_t n;
            do{n=read(readers[i].fd,bytes,sizeof bytes);}while(n<0&&errno==EINTR);
            if(n>0){if(!failure&&!fl_shell_capture(&captures[i],bytes,(size_t)n,&total))failure=1;}
            else if(n==0){close(readers[i].fd);if(i==0)out[0]=-1;else err[0]=-1;readers[i].fd=-1;}
            else{failure=1;close(readers[i].fd);if(i==0)out[0]=-1;else err[0]=-1;readers[i].fd=-1;}
        }
    }
done:
    if(action_ready)posix_spawn_file_actions_destroy(&actions);
    for(int i=0;i<2;i++){if(out[i]>=0)close(out[i]);if(err[i]>=0)close(err[i]);}
    if(spawned){
        pid_t waited;do{waited=waitpid(child,&status,0);}while(waited<0&&errno==EINTR);
        if(waited!=child)failure=1;
    }
    if(failure){free(captures[0].bytes);free(captures[1].bytes);fl_throw(fl_make_error("Error","shell-exec-result: spawn, capture, limit or wait failed"));return fl_nil();}
    int exit_code=WIFEXITED(status)?WEXITSTATUS(status):WIFSIGNALED(status)?128+WTERMSIG(status):-1;
    FLValue stdout_value=fl_shell_bytes(&captures[0]),stderr_value=fl_shell_bytes(&captures[1]);
    free(captures[0].bytes);free(captures[1].bytes);
    FLValue result=fl_map_new();result=fl_map_set(result,fl_str_val("stdout"),stdout_value);
    result=fl_map_set(result,fl_str_val("stderr"),stderr_value);
    result=fl_map_set(result,fl_str_val("exit"),fl_int(exit_code));
    return fl_map_set(result,fl_str_val("ok"),fl_bool(exit_code==0));
#else
    /* A platform without an atomic spawn closefrom action must not silently
     * pass sensitive server descriptors into the shell. */
    fl_throw(fl_make_error("Error","shell-exec-result: protected spawn requires glibc 2.34 or later"));return fl_nil();
#endif
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

#include "v11-timer.h"
#include "v11-tcp.h"
