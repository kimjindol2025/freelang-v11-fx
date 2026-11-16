/* Minimal, nestable terminal raw mode for native TUI applications. */
#include <errno.h>
#include <pthread.h>
#include <signal.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>

static pthread_mutex_t vt_terminal_mutex=PTHREAD_MUTEX_INITIALIZER;
static struct termios vt_terminal_saved;
static volatile sig_atomic_t vt_terminal_active;
static int vt_terminal_depth;
static int vt_terminal_fd=-1;
static int vt_terminal_atexit_registered;
static int vt_terminal_handlers_installed;
static const int vt_terminal_signals[]={SIGHUP,SIGINT,SIGQUIT,SIGTERM};
static struct sigaction vt_terminal_previous[sizeof(vt_terminal_signals)/sizeof(vt_terminal_signals[0])];

static void vt_terminal_restore_handlers(void){
    if(!vt_terminal_handlers_installed)return;
    for(size_t i=0;i<sizeof(vt_terminal_signals)/sizeof(vt_terminal_signals[0]);i++)
        (void)sigaction(vt_terminal_signals[i],&vt_terminal_previous[i],NULL);
    vt_terminal_handlers_installed=0;
}

static void vt_terminal_restore(void){
    if(vt_terminal_active){
        (void)tcsetattr(vt_terminal_fd,TCSANOW,&vt_terminal_saved);
        vt_terminal_active=0;
        vt_terminal_depth=0;
        vt_terminal_fd=-1;
    }
    vt_terminal_restore_handlers();
}

static void vt_terminal_signal(int signo,siginfo_t*info,void*context){
    size_t index=0;
    while(index<sizeof(vt_terminal_signals)/sizeof(vt_terminal_signals[0])&&vt_terminal_signals[index]!=signo)index++;
    if(index>=sizeof(vt_terminal_signals)/sizeof(vt_terminal_signals[0]))return;
    struct sigaction previous=vt_terminal_previous[index];
    if(previous.sa_handler==SIG_IGN)return;
    vt_terminal_restore();
    if(previous.sa_handler!=SIG_DFL){
        if(previous.sa_flags&SA_SIGINFO)previous.sa_sigaction(signo,info,context);
        else previous.sa_handler(signo);
        return;
    }
    if(kill(getpid(),signo))_exit(128+signo);
}

static int vt_terminal_install_handlers(void){
    size_t count=sizeof(vt_terminal_signals)/sizeof(vt_terminal_signals[0]);
    struct sigaction action={0};
    action.sa_sigaction=vt_terminal_signal;
    action.sa_flags=SA_SIGINFO;
    sigemptyset(&action.sa_mask);
    size_t installed=0;
    for(;installed<count;installed++){
        int signo=vt_terminal_signals[installed];
        if(sigaction(signo,NULL,&vt_terminal_previous[installed])||
           sigaction(signo,&action,NULL))break;
    }
    if(installed!=count){
        while(installed>0){installed--;sigaction(vt_terminal_signals[installed],&vt_terminal_previous[installed],NULL);}
        return 0;
    }
    vt_terminal_handlers_installed=1;
    return 1;
}

static FLValue fl_v11_terminal(FLValue name,FLValue argv){
    const char*op=((FLString*)name.obj)->data;
    FLVector*args=(FLVector*)argv.obj;
    if(args->len){fl_throw(fl_make_error("TypeError","tty-raw: takes no arguments"));return fl_nil();}
    if(!strcmp(op,"tty-raw-enter!")){
        if(!isatty(STDIN_FILENO)){fl_throw(fl_make_error("RuntimeError","tty-raw-enter!: stdin is not a terminal"));return fl_nil();}
        pthread_mutex_lock(&vt_terminal_mutex);
        if(vt_terminal_active){
            if(vt_terminal_fd!=STDIN_FILENO){pthread_mutex_unlock(&vt_terminal_mutex);fl_throw(fl_str_val("tty-raw-enter!: another terminal is active"));return fl_nil();}
            vt_terminal_depth++;
            pthread_mutex_unlock(&vt_terminal_mutex);
            return fl_str_val("ok");
        }
        if(tcgetattr(STDIN_FILENO,&vt_terminal_saved)){
            int saved_errno=errno;pthread_mutex_unlock(&vt_terminal_mutex);
            fl_throw(fl_make_error("RuntimeError",saved_errno==ENOTTY?"tty-raw-enter!: stdin is not a terminal":"tty-raw-enter!: cannot read terminal mode"));return fl_nil();
        }
        if(!vt_terminal_atexit_registered){
            if(atexit(vt_terminal_restore)){pthread_mutex_unlock(&vt_terminal_mutex);fl_throw(fl_str_val("tty-raw-enter!: cannot register terminal cleanup"));return fl_nil();}
            vt_terminal_atexit_registered=1;
        }
        if(!vt_terminal_install_handlers()){
            pthread_mutex_unlock(&vt_terminal_mutex);fl_throw(fl_str_val("tty-raw-enter!: cannot install termination cleanup"));return fl_nil();
        }
        struct termios raw=vt_terminal_saved;
        cfmakeraw(&raw);
        raw.c_lflag|=ISIG;
        vt_terminal_fd=STDIN_FILENO;
        vt_terminal_depth=1;
        vt_terminal_active=1;
        if(tcsetattr(STDIN_FILENO,TCSANOW,&raw)){
            vt_terminal_restore();pthread_mutex_unlock(&vt_terminal_mutex);
            fl_throw(fl_str_val("tty-raw-enter!: cannot enable raw mode"));return fl_nil();
        }
        pthread_mutex_unlock(&vt_terminal_mutex);
        return fl_str_val("ok");
    }
    if(!strcmp(op,"tty-raw-leave!")){
        pthread_mutex_lock(&vt_terminal_mutex);
        if(!vt_terminal_active||vt_terminal_fd!=STDIN_FILENO){
            pthread_mutex_unlock(&vt_terminal_mutex);
            fl_throw(fl_make_error("RuntimeError","tty-raw-leave!: raw mode is not active"));return fl_nil();
        }
        if(vt_terminal_depth>1)vt_terminal_depth--;
        else{vt_terminal_depth=0;vt_terminal_restore();}
        pthread_mutex_unlock(&vt_terminal_mutex);
        return fl_str_val("ok");
    }
    fl_throw(fl_make_error("TypeError","tty-raw: unknown operation"));
    return fl_nil();
}
