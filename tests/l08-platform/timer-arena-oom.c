#include "runtime.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
static int fail_next,cleaned;
static int invoked;
static FLValue callback(FLClosure*c,int n,FLValue*a){(void)c;(void)n;(void)a;invoked++;return fl_int(42);}
void*__real_calloc(size_t,size_t);
void*__wrap_calloc(size_t n,size_t size){if(fail_next){fail_next=0;return NULL;}return __real_calloc(n,size);}
static void cleanup(void*unused){(void)unused;cleaned=1;}
int main(void){
 fl_arena_begin();FLTryFrame*frame=&fl_try_stack[fl_try_top++];int caught=0;
 if(setjmp(frame->buf)==0){fail_next=1;(void)fl_arena_scope_enter();fl_try_top--;}
 else{fl_try_top--;caught=1;}
 int registered=fl_arena_on_reset(cleanup,NULL);fl_arena_end();
 int ok=caught&&registered==1&&cleaned;
 printf("TIMER_SCOPE_OOM_RESTORES_CALLER=%s\n",ok?"PASS":"FAIL");if(!ok)return 1;
 fl_arena_begin();FLValue args[2]={fl_fn_new(callback,0,NULL),fl_int(1)};
 FLValue promise=fl_v11_timer(fl_str_val("set-timeout"),fl_vec_from(args,2));
 struct timespec pause={0,5000000};nanosleep(&pause,NULL);fail_next=1;fl_schedule_tick(0);
 frame=&fl_try_stack[fl_try_top++];caught=0;
 if(setjmp(frame->buf)==0){FLValue a[1]={promise};fl_v11_timer(fl_str_val("await-result"),fl_vec_from(a,1));fl_try_top--;}
 else{fl_try_top--;FLValue a[1]={frame->err};FLValue message=fl_v11_builtin(fl_str_val("error-message"),fl_vec_from(a,1));caught=!strcmp(((FLString*)message.obj)->data,"arena allocation failed");}
 ok=caught&&invoked==0;printf("TIMER_SCOPE_OOM_REJECTED=%s\n",ok?"PASS":"FAIL");
 args[0]=fl_fn_new(callback,0,NULL);fl_v11_timer(fl_str_val("set-timeout"),fl_vec_from(args,2));
 for(int i=0;i<100&&!invoked;i++)fl_schedule_tick(5);
 int recovered=invoked==1;printf("TIMER_SCOPE_OOM_RECOVERY=%s\n",recovered?"PASS":"FAIL");
 fl_arena_end();fl_schedule_shutdown();return ok&&recovered?0:1;
}
