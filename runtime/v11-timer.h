/* Native timers use one cooperative event queue and managed promise handles. */
#include <pthread.h>
#include <poll.h>
#include <fcntl.h>
#include <limits.h>
#include <math.h>
#include <time.h>
#include <errno.h>
typedef struct {FLMap map;pthread_mutex_t mutex;int state;FLValue result;} VTimerPromise;
typedef struct VTimerJob {int64_t due;FLValue callback,promise;struct VTimerJob*next;} VTimerJob;
static pthread_mutex_t timer_mutex=PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t timer_dispatch;
static pthread_once_t timer_once=PTHREAD_ONCE_INIT;
static VTimerJob*timer_jobs;
static unsigned timer_count;
static int timer_stopping;
static int timer_pipe[2]={-1,-1};
static int64_t timer_now(void){struct timespec time;if(clock_gettime(CLOCK_MONOTONIC,&time))fl_throw(fl_str_val("timer clock failed"));return(int64_t)time.tv_sec*1000+time.tv_nsec/1000000;}
void fl_schedule_promise_destroy(FLValue value){VTimerPromise*p=(VTimerPromise*)value.obj;fl_heap_release(p->result);pthread_mutex_destroy(&p->mutex);free(p);}
static void timer_handle_cleanup(void*value){FLValue handle={.tag=FL_MAP,.obj=value};fl_heap_release(handle);}
static void timer_finish(FLValue handle,int state,FLValue result){VTimerPromise*p=(VTimerPromise*)handle.obj;pthread_mutex_lock(&p->mutex);p->result=result;p->state=state;pthread_mutex_unlock(&p->mutex);}
static void timer_job_free(VTimerJob*job){fl_heap_release(job->callback);fl_heap_release(job->promise);free(job);}
static void timer_exit_drain(void){
 for(;;){pthread_mutex_lock(&timer_mutex);int pending=timer_jobs!=NULL&&!timer_stopping;pthread_mutex_unlock(&timer_mutex);if(!pending)break;fl_schedule_tick(1000);}
}
static void timer_init(void){
 pthread_mutexattr_t attributes;pthread_mutexattr_init(&attributes);pthread_mutexattr_settype(&attributes,PTHREAD_MUTEX_RECURSIVE);pthread_mutex_init(&timer_dispatch,&attributes);pthread_mutexattr_destroy(&attributes);
 if(pipe(timer_pipe)==0){for(int i=0;i<2;i++){fcntl(timer_pipe[i],F_SETFL,O_NONBLOCK);fcntl(timer_pipe[i],F_SETFD,FD_CLOEXEC);}}
 else timer_pipe[0]=timer_pipe[1]=-1;
 atexit(timer_exit_drain);
}
int fl_schedule_wake_fd(void){pthread_once(&timer_once,timer_init);return timer_pipe[0];}
int fl_schedule_timeout_ms(int maximum){
 int64_t now=timer_now();pthread_mutex_lock(&timer_mutex);int64_t delay=timer_jobs?timer_jobs->due-now:maximum;pthread_mutex_unlock(&timer_mutex);
 if(delay<0)return 0;return delay>maximum?maximum:(int)delay;
}
void fl_schedule_shutdown(void){
 pthread_mutex_lock(&timer_mutex);timer_stopping=1;VTimerJob*jobs=timer_jobs;timer_jobs=NULL;timer_count=0;pthread_mutex_unlock(&timer_mutex);
 while(jobs){VTimerJob*next=jobs->next;timer_finish(jobs->promise,2,fl_heap_copy(fl_make_error("Error","timer cancelled")));timer_job_free(jobs);jobs=next;}
 if(timer_pipe[1]>=0){char byte=1;(void)write(timer_pipe[1],&byte,1);}
}
void fl_schedule_tick(int wait_ms){
 pthread_once(&timer_once,timer_init);
 int timeout=fl_schedule_timeout_ms(wait_ms<0?0:wait_ms);
 struct pollfd wake={timer_pipe[0],POLLIN,0};
 if(timeout>0){while(poll(&wake,1,timeout)<0&&errno==EINTR){break;}}
 if(timer_pipe[0]>=0){char bytes[128];while(read(timer_pipe[0],bytes,sizeof bytes)>0){}}
 int64_t cutoff=timer_now();
 pthread_mutex_lock(&timer_dispatch);
 for(;;){
  pthread_mutex_lock(&timer_mutex);VTimerJob*job=timer_jobs;
  if(timer_stopping||!job||job->due>cutoff){pthread_mutex_unlock(&timer_mutex);break;}
  timer_jobs=job->next;timer_count--;pthread_mutex_unlock(&timer_mutex);
  void*volatile previous=NULL;volatile int entered=0;FLValue result=fl_nil();int failed=0;
  if(fl_try_top>=FL_TRY_MAX){result=fl_heap_copy(fl_make_error("Error","timer exception frame limit exceeded"));failed=1;}
  else{
   FLTryFrame*frame=&fl_try_stack[fl_try_top++];
   if(setjmp(frame->buf)==0){
    previous=fl_arena_scope_enter();entered=1;
    if(job->callback.tag!=FL_FN)fl_throw(fl_make_error("TypeError","set-timeout callback must be a function"));
    result=fl_heap_copy(fl_fn_call(job->callback,0,NULL));fl_try_top--;
   }else{result=fl_heap_copy(frame->err);failed=1;fl_try_top--;}
  }
  timer_finish(job->promise,failed?2:1,result);if(entered)fl_arena_scope_leave(previous);timer_job_free(job);
 }
 pthread_mutex_unlock(&timer_dispatch);
}
FLValue fl_v11_timer(FLValue name,FLValue argv){
 const char*op=((FLString*)name.obj)->data;FLVector*args=(FLVector*)argv.obj;
 FLValue first=args->len?args->data[0]:fl_nil();
 if(!strcmp(op,"await-result")){
  if(first.tag!=FL_MAP||!first.obj||first.obj->type!=FL_NATIVE_PROMISE){fl_throw(fl_make_error("TypeError","await_result: promise required"));return fl_nil();}
  VTimerPromise*p=(VTimerPromise*)first.obj;pthread_mutex_lock(&p->mutex);int state=p->state;FLValue result=p->result;pthread_mutex_unlock(&p->mutex);
  if(!state)fl_throw(fl_make_error("Error","Promise still pending - use proper async handling"));
  if(state==2)fl_throw(result);return result;
 }
 if(!strcmp(op,"fl-yield")){
  FLValue number=fl_v11_to_number(args->len?first:fl_int(1));double value=number.f;
  int wait=!isfinite(value)||value<0?0:value>INT_MAX?INT_MAX:(int)value;fl_schedule_tick(wait);return fl_int(0);
 }
 if(args->len<2){fl_throw(fl_make_error("Error","set-timeout requires callback and delay"));return fl_nil();}
 pthread_once(&timer_once,timer_init);
 FLValue delay=fl_v11_to_number(args->data[1]);double milliseconds=delay.f;
 if(!isfinite(milliseconds)||milliseconds<1||milliseconds>INT_MAX)milliseconds=1;
 VTimerJob*job=calloc(1,sizeof(*job));VTimerPromise*p=calloc(1,sizeof(*p));
 if(!job||!p){free(job);free(p);fl_throw(fl_str_val("timer allocation failed"));return fl_nil();}
 p->map.base.type=FL_NATIVE_PROMISE;p->map.base.rc=1;p->result=fl_nil();pthread_mutex_init(&p->mutex,NULL);
 FLValue handle={.tag=FL_MAP,.obj=(FLObject*)p};
 int tracked=fl_arena_on_reset(timer_handle_cleanup,p);
 if(tracked<0){fl_heap_release(handle);free(job);fl_throw(fl_str_val("timer cleanup allocation failed"));return fl_nil();}
 job->callback=fl_heap_copy(first);job->promise=handle;job->due=timer_now()+(int64_t)milliseconds;
 pthread_mutex_lock(&timer_mutex);
 if(timer_stopping||timer_count>=4096){pthread_mutex_unlock(&timer_mutex);fl_heap_release(job->callback);free(job);if(!tracked)fl_heap_release(handle);fl_throw(fl_str_val("timer unavailable or pending limit exceeded"));return fl_nil();}
 fl_heap_retain(handle);VTimerJob**slot=&timer_jobs;while(*slot&&(*slot)->due<=job->due)slot=&(*slot)->next;job->next=*slot;*slot=job;timer_count++;pthread_mutex_unlock(&timer_mutex);
 if(timer_pipe[1]>=0){char byte=1;(void)write(timer_pipe[1],&byte,1);}return handle;
}
