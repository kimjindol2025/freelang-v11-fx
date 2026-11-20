#define _GNU_SOURCE
#include "runtime.h"
#include "process-input.h"
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/prctl.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>
static int64_t now(void) { struct timespec t; assert(!clock_gettime(CLOCK_MONOTONIC,&t));return (int64_t)t.tv_sec*1000+t.tv_nsec/1000000; }
static void emit(int fd,const char *p,size_t n) { while(n){ssize_t w=write(fd,p,n);if(w<0&&errno==EINTR)continue;assert(w>0);p+=w;n-=(size_t)w;} }
static int child(int argc,char **argv) {
    const char *mode=argv[1];
    if(!strcmp(mode,"eof")){char c;return read(0,&c,1)==0?0:91;}
    if(!strcmp(mode,"literal")){assert(argc==3);emit(1,argv[2],strlen(argv[2]));return 0;}
    if(!strcmp(mode,"fd")){errno=0;return fcntl(77,F_GETFD)==-1&&errno==EBADF?0:92;}
    if(!strcmp(mode,"exit"))return 7;
    if(!strcmp(mode,"signal")){raise(SIGTERM);return 93;}
    if(!strcmp(mode,"sleep")){for(;;)pause();}
    if(!strcmp(mode,"descendant")){
        signal(SIGTERM,SIG_IGN);
        pid_t p=fork();assert(p>=0);
        if(!p){close(0);close(1);close(2);for(;;)pause();}
        char b[64];int n=snprintf(b,sizeof b,"%ld",(long)p);emit(1,b,(size_t)n);
        for(;;)pause();
    }
    if(!strcmp(mode,"streams")){
        char b[32768];memset(b,'o',sizeof b);emit(1,b,sizeof b);memset(b,'e',sizeof b);emit(2,b,sizeof b);return 0;
    }
    if(!strcmp(mode,"bidirectional")){
        char b[131072];memset(b,'o',sizeof b);emit(1,b,sizeof b);memset(b,'e',sizeof b);emit(2,b,sizeof b);
    }
    if(!strcmp(mode,"echo")||!strcmp(mode,"bidirectional")){
        char b[8192];for(;;){ssize_t n=read(0,b,sizeof b);if(n>0)emit(1,b,(size_t)n);else if(n<0&&errno==EINTR)continue;else return n?94:0;}
    }
    return 95;
}
static int controls;
static void check(const char *label,int condition){printf("%s %s\n",label,condition?"PASS":"FAIL");fflush(stdout);assert(condition);controls++;}
int main(int argc,char **argv) {
    if(argc>1)return child(argc,argv);
    assert(argv[0][0]=='/');fl_cli_runtime_begin();
    struct sigaction before,after;assert(!sigaction(SIGPIPE,NULL,&before));
    FLProcessInputResult r;
    const char *eof[]={argv[0],"eof",NULL};
    int rc=fl_process_input(eof,NULL,0,1000,0,&r);
    check("stdin EOF",rc==0&&r.exit_code==0&&!r.out.size&&!r.err.size);fl_process_input_release(&r);
    const char literal[]="literal ' 한글 %$\n$(touch MUST_NOT_BE_CREATED)";
    const char *lit[]={argv[0],"literal",literal,NULL};
    rc=fl_process_input(lit,NULL,0,1000,4096,&r);
    check("literal argv",!rc&&r.exit_code==0&&r.out.size==sizeof literal-1&&!memcmp(r.out.data,literal,r.out.size));fl_process_input_release(&r);
    check("no shell marker",access("MUST_NOT_BE_CREATED",F_OK)==-1&&errno==ENOENT);
    int fd=open("/dev/null",O_RDONLY);assert(fd>=0);assert(dup2(fd,77)==77);close(fd);
    const char *fds[]={argv[0],"fd",NULL};rc=fl_process_input(fds,NULL,0,1000,16,&r);
    check("inherited descriptors closed",!rc&&r.exit_code==0);fl_process_input_release(&r);close(77);
    size_t size=3u*1024u*1024u;unsigned char *data=malloc(size);assert(data);for(size_t i=0;i<size;i++)data[i]=(unsigned char)(i*37u);
    const char *echo[]={argv[0],"echo",NULL};rc=fl_process_input(echo,data,size,5000,size,&r);
    check("binary stdin including NUL and >1MiB",!rc&&r.exit_code==0&&r.out.size==size&&!memcmp(r.out.data,data,size));fl_process_input_release(&r);
    const char *duplex[]={argv[0],"bidirectional",NULL};rc=fl_process_input(duplex,data,size,5000,size+262144,&r);
    check("bidirectional pipes do not deadlock",!rc&&r.exit_code==0&&r.out.size==size+131072&&r.err.size==131072&&!memcmp(r.out.data+131072,data,size));fl_process_input_release(&r);free(data);
    const char *streams[]={argv[0],"streams",NULL};rc=fl_process_input(streams,NULL,0,1000,65536,&r);
    check("combined exact output limit",!rc&&r.exit_code==0&&r.out.size==32768&&r.err.size==32768);fl_process_input_release(&r);
    rc=fl_process_input(streams,NULL,0,1000,65535,&r);check("combined output overflow",rc==-1&&r.overflow&&r.out.size+r.err.size==65535);fl_process_input_release(&r);
    const char *nonzero[]={argv[0],"exit",NULL};rc=fl_process_input(nonzero,NULL,0,1000,16,&r);check("nonzero exit retained",!rc&&r.exit_code==7);fl_process_input_release(&r);
    const char *signaled[]={argv[0],"signal",NULL};rc=fl_process_input(signaled,NULL,0,1000,16,&r);check("signal retained",rc==-1&&r.signal_number==SIGTERM);fl_process_input_release(&r);
    const char *missing[]={"/definitely-missing-native-input-executable",NULL};rc=fl_process_input(missing,NULL,0,1000,16,&r);check("spawn failure retained",rc==-1&&r.spawn_error==ENOENT);fl_process_input_release(&r);
    const char *sleeping[]={argv[0],"sleep",NULL};int64_t started=now();rc=fl_process_input(sleeping,NULL,0,120,16,&r);int64_t duration=now()-started;
    check("whole timeout and reap",rc==-1&&r.timed_out&&duration>=120&&duration<800);fl_process_input_release(&r);
    int status;errno=0;check("no direct child remains",waitpid(-1,&status,WNOHANG)==-1&&errno==ECHILD);
    assert(!prctl(PR_SET_CHILD_SUBREAPER,1));
    const char *descendant[]={argv[0],"descendant",NULL};rc=fl_process_input(descendant,NULL,0,160,64,&r);
    check("descendant invocation timed out",rc==-1&&r.timed_out&&r.out.size>0);
    pid_t grandchild=(pid_t)strtol(r.out.data,NULL,10);assert(grandchild>0);fl_process_input_release(&r);
    pid_t reaped;do{reaped=waitpid(grandchild,&status,0);}while(reaped<0&&errno==EINTR);
    check("owned descendant killed before leader reaped",reaped==grandchild&&WIFSIGNALED(status)&&WTERMSIG(status)==SIGKILL);
    assert(!sigaction(SIGPIPE,NULL,&after));check("SIGPIPE disposition unchanged",before.sa_handler==after.sa_handler&&before.sa_flags==after.sa_flags);
    printf("PROCESS_INPUT_CONTROLS=%d PASS=%d\n",controls,controls);return 0;
}
