/* Included by io.c to reuse L07 file_open's fd-relative sandbox walk.
 * Codec dependencies stay out of the language runtime/seed link. */
#include <sys/file.h>
#include <sys/wait.h>
#include <sys/resource.h>
#include <sys/syscall.h>
#include <poll.h>
#include <signal.h>
#include <time.h>
#include <math.h>

static FLValue im_error(const char *code){
    FLValue r=fl_map_new();r=fl_map_set(r,fl_str_val("ok"),fl_bool(false));
    r=fl_map_set(r,fl_str_val("error"),fl_str_val(code));return fl_map_set(r,fl_str_val("data"),fl_nil());
}
static int im_integer(FLValue map,const char *key,uint64_t max,uint64_t *out){
    FLValue x=fl_map_get(map,fl_str_val(key));
    if(x.tag==FL_INT&&x.i>0&&(uint64_t)x.i<=max){*out=x.i;return 1;}
    if(x.tag==FL_FLOAT&&isfinite(x.f)&&x.f>0&&x.f<=(double)max&&floor(x.f)==x.f){*out=(uint64_t)x.f;return 1;}
    return 0;
}
static int im_string(FLValue v){return v.tag==FL_STRING&&v.obj&&((FLString*)v.obj)->len>0&&
    ((FLString*)v.obj)->len<PATH_MAX&&!memchr(_strval(v),0,((FLString*)v.obj)->len);}
static int im_base(void){
    const char *base=getenv("FL_FILE_BASE");if(!base||*base!='/'||strlen(base)>=PATH_MAX)return 0;
    char copy[PATH_MAX];strcpy(copy,base);char *save=NULL;
    int d=open("/",O_RDONLY|O_DIRECTORY|O_CLOEXEC);if(d<0)return 0;
    for(char *p=strtok_r(copy,"/",&save);p;p=strtok_r(NULL,"/",&save)){
        if(!strcmp(p,"..")){close(d);return 0;}
        int next=openat(d,p,O_RDONLY|O_DIRECTORY|O_NOFOLLOW|O_CLOEXEC);close(d);if(next<0)return 0;d=next;
    }
    close(d);return 1;
}
static const char *im_open_error(FileError *e){
    if(e->number==ELOOP||e->number==EACCES||e->number==ENOTDIR||
       (e->code&&!strcmp(e->code,"PATH_OUTSIDE_BASE")))return "PATH_DENIED";
    return "IO_ERROR";
}
static int64_t im_clock(void){struct timespec t;if(clock_gettime(CLOCK_MONOTONIC,&t))return -1;return (int64_t)t.tv_sec*1000+t.tv_nsec/1000000;}
/* A cleanup error must never turn into success. Retry a transient removal once
 * to leave no remnant, while retaining the original failure in the result. */
static int im_remove(int dir,const char *name){
    if(!unlinkat(dir,name,0)||errno==ENOENT)return 0;
    (void)unlinkat(dir,name,0);return 1;
}
typedef struct {int dir,fd,published;char name[NAME_MAX+1],temporary[NAME_MAX+1];uint64_t edge;} ImageOutput;
static int im_output_parent(FLValue path,ImageOutput *out){
    char copy[PATH_MAX];strcpy(copy,_strval(path));char *slash=strrchr(copy,'/');
    const char *name=slash?slash+1:copy;
    if(!*name||strlen(name)>NAME_MAX||!strcmp(name,".")||!strcmp(name,"..")){errno=EACCES;return 0;}
    strcpy(out->name,name);
    const char *parent=".";
    if(slash){*slash=0;parent=*copy?copy:"/";}
    FileError error={0};out->dir=file_open(fl_str_val(parent),O_RDONLY|O_DIRECTORY,&error);
    if(out->dir<0){errno=error.number;return 0;}
    struct stat st;
    if(fstatat(out->dir,out->name,&st,AT_SYMLINK_NOFOLLOW)==0){errno=S_ISLNK(st.st_mode)?ELOOP:EEXIST;return 0;}
    if(errno!=ENOENT)return 0;
    return 1;
}
static FLValue im_run(FLValue path,FLValue outputs,FLValue options,int processing){
    uint64_t pixels,bytes,memory,timeout,quality;
    if(!im_string(path)||options.tag!=FL_MAP||
       !im_integer(options,"max_pixels",UINT32_MAX,&pixels)||
       !im_integer(options,"max_bytes",FL_FILE_IO_MAX_BYTES,&bytes)||
       !im_integer(options,"memory_bytes",536870912,&memory)||
       !im_integer(options,"timeout_ms",5000,&timeout)||
       !im_integer(options,"jpeg_quality",100,&quality))return im_error("INVALID_ARGUMENT");
    FLValue expected=fl_map_get(options,fl_str_val("expected_mime"));
    if(expected.tag!=FL_NIL&&(!im_string(expected)||
       (strcmp(_strval(expected),"image/png")&&strcmp(_strval(expected),"image/jpeg")&&strcmp(_strval(expected),"image/gif"))))return im_error("INVALID_ARGUMENT");
    int count=0;ImageOutput dest[2];memset(dest,0,sizeof dest);
    for(int i=0;i<2;i++){dest[i].fd=-1;dest[i].dir=-1;}
    if(processing){
        if(outputs.tag!=FL_VECTOR||!outputs.obj)return im_error("INVALID_ARGUMENT");
        FLVector *v=(FLVector*)outputs.obj;if(v->len<1||v->len>2)return im_error("INVALID_ARGUMENT");
        count=v->len;
        for(int i=0;i<count;i++)if(v->data[i].tag!=FL_MAP||
           !im_string(fl_map_get(v->data[i],fl_str_val("path")))||
           !im_integer(v->data[i],"long_edge",UINT32_MAX,&dest[i].edge))return im_error("INVALID_ARGUMENT");
    }
    if(!im_base())return im_error("PATH_DENIED");
    const char *worker=getenv("FL_IMAGE_WORKER");
    if(!worker||worker[0]!='/'||strlen(worker)>=PATH_MAX)return im_error("WORKER_UNAVAILABLE");
    const char *error=NULL;FLValue result=fl_nil();FileError fe={0};
    int source=-1,lock=-1,pipefd[2]={-1,-1};pid_t child=-1;int status=0;
    int64_t start=im_clock();if(start<0)return im_error("IO_ERROR");int64_t deadline=start+timeout;
    source=file_open(path,O_RDONLY|O_NONBLOCK,&fe);
    if(source<0){error=im_open_error(&fe);goto done;}
    struct stat st;
    if(fstat(source,&st)||!S_ISREG(st.st_mode)){error="IO_ERROR";goto done;}
    if(st.st_size<0||(uint64_t)st.st_size>bytes){error="SOURCE_LIMIT";goto done;}
    fe=(FileError){0};lock=file_open(fl_str_val(".fl-image.lock"),O_RDWR|O_CREAT|O_NONBLOCK,&fe);
    if(lock<0||fstat(lock,&st)||!S_ISREG(st.st_mode)){error="IO_ERROR";goto done;}
    while(flock(lock,LOCK_EX|LOCK_NB)){
        if(errno!=EWOULDBLOCK&&errno!=EAGAIN&&errno!=EINTR){error="IO_ERROR";goto done;}
        if(im_clock()>=deadline){error="TIMEOUT";goto done;}
        struct timespec pause={.tv_nsec=1000000};nanosleep(&pause,NULL);
    }
    for(int i=0;i<count;i++){
        FLValue item=fl_vec_get(outputs,fl_int(i));
        if(!im_output_parent(fl_map_get(item,fl_str_val("path")),&dest[i])){
            error=errno==EEXIST?"OUTPUT_EXISTS":errno==EACCES||errno==ELOOP||errno==ENOTDIR?"PATH_DENIED":"IO_ERROR";goto done;
        }
        for(unsigned attempt=0;attempt<64;attempt++){
            snprintf(dest[i].temporary,sizeof dest[i].temporary,".fl-image-%ld-%d-%u",(long)getpid(),i,attempt);
            dest[i].fd=openat(dest[i].dir,dest[i].temporary,O_RDWR|O_CREAT|O_EXCL|O_NOFOLLOW|O_CLOEXEC,0600);
            if(dest[i].fd>=0||errno!=EEXIST)break;
        }
        if(dest[i].fd<0){error="IO_ERROR";goto done;}
    }
    if(pipe2(pipefd,O_CLOEXEC)){error="IO_ERROR";goto done;}
    char numbers[5][32];uint64_t values[]={pixels,bytes,quality,dest[0].edge,dest[1].edge};
    for(int i=0;i<5;i++)snprintf(numbers[i],sizeof numbers[i],"%llu",(unsigned long long)values[i]);
    child=fork();
    if(child<0){error="WORKER_FAILED";goto done;}
    if(child==0){
        struct rlimit as={memory,memory},core={0,0},cpu={6,6};
        if(setrlimit(RLIMIT_AS,&as)||setrlimit(RLIMIT_CORE,&core)||setrlimit(RLIMIT_CPU,&cpu))_exit(125);
        int from[4]={source,pipefd[1],dest[0].fd,dest[1].fd},to[4]={3,1,4,5},safe[4];
        for(int i=0;i<4;i++){safe[i]=from[i]<0?-1:fcntl(from[i],F_DUPFD_CLOEXEC,10);if(from[i]>=0&&safe[i]<0)_exit(125);}
        for(int i=0;i<4;i++)if(safe[i]>=0&&dup2(safe[i],to[i])<0)_exit(125);
        int null=open("/dev/null",O_RDWR);if(null<0||dup2(null,0)<0||dup2(null,2)<0)_exit(125);
        for(int i=count;i<2;i++)close(4+i);
#ifdef SYS_close_range
        if(syscall(SYS_close_range,6u,~0u,0)<0)
#endif
        {long max=sysconf(_SC_OPEN_MAX);if(max<0)max=65536;for(int fd=6;fd<max;fd++)close(fd);}
        char *args[]={(char*)worker,numbers[0],numbers[1],numbers[2],numbers[3],numbers[4],expected.tag==FL_NIL?"":(char*)_strval(expected),NULL};
        char *env[]={"LC_ALL=C",NULL};execve(worker,args,env);_exit(126);
    }
    close(pipefd[1]);pipefd[1]=-1;fcntl(pipefd[0],F_SETFL,O_NONBLOCK);
    char response[4096];size_t length=0;int exited=0,eof=0;
    while(!exited||!eof){
        int64_t now=im_clock();if(now<0){error="IO_ERROR";goto done;}
        if(now>=deadline){error="TIMEOUT";goto done;}
        for(;;){
            if(length==sizeof response-1){error="WORKER_FAILED";goto done;}
            ssize_t got=read(pipefd[0],response+length,sizeof response-1-length);
            if(got>0){length+=got;continue;}if(!got)eof=1;
            if(got<0&&errno!=EAGAIN&&errno!=EINTR){error="WORKER_FAILED";goto done;}break;
        }
        if(!exited){pid_t rc=waitpid(child,&status,WNOHANG);if(rc==child){exited=1;child=-1;}else if(rc<0&&errno!=EINTR){error="WORKER_FAILED";goto done;}}
        if(!exited||!eof){struct pollfd p={pipefd[0],POLLIN|POLLHUP,0};poll(&p,1,1);}
    }
    response[length]=0;
    if(!WIFEXITED(status)||WEXITSTATUS(status)>1||!length){error="WORKER_FAILED";goto done;}
    /* Existing JSON parser, no additional parser implementation. */
    result=fl_json_parse(fl_str_val(response));
    if(result.tag!=FL_MAP||fl_map_get(result,fl_str_val("ok")).tag!=FL_BOOL){error="WORKER_FAILED";goto done;}
    if(!fl_truthy(fl_map_get(result,fl_str_val("ok")))){
        FLValue message=fl_map_get(result,fl_str_val("error"));if(!im_string(message))error="WORKER_FAILED";
        goto done;
    }
    if(WEXITSTATUS(status)!=0){error="WORKER_FAILED";goto done;}
    for(int i=0;i<count;i++){
        if(fstat(dest[i].fd,&st)||!S_ISREG(st.st_mode)||st.st_size<=0||fsync(dest[i].fd)){error="IO_ERROR";goto done;}
        if(linkat(dest[i].dir,dest[i].temporary,dest[i].dir,dest[i].name,0)){error=errno==EEXIST?"OUTPUT_EXISTS":"IO_ERROR";goto done;}
        dest[i].published=1;
        if(fsync(dest[i].dir)){error="IO_ERROR";goto done;}
    }
done:
    if(child>0){kill(child,SIGKILL);while(waitpid(child,&status,0)<0&&errno==EINTR){}}
    int cleanup_failed=0;
    for(int i=0;i<2;i++){
        if(dest[i].fd>=0){
            if(close(dest[i].fd))cleanup_failed=1;
            if(im_remove(dest[i].dir,dest[i].temporary))cleanup_failed=1;
        }
    }
    if(source>=0&&close(source))cleanup_failed=1;
    if(pipefd[0]>=0&&close(pipefd[0]))cleanup_failed=1;
    if(pipefd[1]>=0&&close(pipefd[1]))cleanup_failed=1;
    if(error||cleanup_failed)for(int i=0;i<2;i++)if(dest[i].published){
        if(im_remove(dest[i].dir,dest[i].name))cleanup_failed=1;
        if(fsync(dest[i].dir))cleanup_failed=1;
    }
    for(int i=0;i<2;i++)if(dest[i].dir>=0&&close(dest[i].dir))cleanup_failed=1;
    if(lock>=0&&close(lock))cleanup_failed=1;
    if(cleanup_failed){
        FLValue failure=im_error("CLEANUP_FAILED");
        FLValue cause=error?fl_str_val(error):result.tag==FL_MAP?fl_map_get(result,fl_str_val("error")):fl_nil();
        return fl_map_set(failure,fl_str_val("cause"),cause);
    }
    return error?im_error(error):result;
}
FLValue fl_image_info_result(FLValue path,FLValue options){return im_run(path,fl_nil(),options,0);}
FLValue fl_image_process_result(FLValue path,FLValue outputs,FLValue options){return im_run(path,outputs,options,1);}
