#define _GNU_SOURCE
#include "support.h"
#include <pthread.h>
#include <stdatomic.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
static struct {
 FLValue setup,promises;
 const char *mode;
 atomic_int cleanups,active,pending,captured;
} state;
static void cleaned(void*unused){(void)unused;atomic_fetch_add(&state.cleanups,1);}
FLValue worker_observe(FLValue text){
 atomic_fetch_add(&state.active,1);
 if(text.tag==FL_STRING&&!strcmp(((FLString*)text.obj)->data,"capture-한😀"))atomic_store(&state.captured,1);
 return fl_nil();
}
FLValue worker_pending(void){atomic_fetch_add(&state.pending,1);return fl_nil();}
FLValue worker_exit(void){pthread_exit(NULL);return fl_nil();}
static void*run(void*unused){
 (void)unused;fl_arena_begin();
 if(!strcmp(state.mode,"active")&&fl_arena_on_reset(cleaned,NULL)!=1)abort();
 state.promises=fl_heap_copy(fl_fn_call(state.setup,0,NULL));
 if(!strcmp(state.mode,"cancelled"))fl_schedule_shutdown();
 if(!strcmp(state.mode,"active")){
  struct timespec pause={0,5000000};nanosleep(&pause,NULL);
  fl_schedule_tick(0);abort(); /* The callback must exit this worker. */
 }
 fl_arena_end();
 if(fl_arena_on_reset(cleaned,NULL)!=1)abort();
 return NULL;
}
static int cancelled(FLValue promise){
 FLTryFrame*frame=&fl_try_stack[fl_try_top++];
 if(setjmp(frame->buf)==0){
  FLValue args[1]={promise};
  (void)fl_v11_timer(fl_str_val("await-result"),fl_vec_from(args,1));
  fl_try_top--;return 0;
 }
 FLValue error=frame->err;fl_try_top--;
 FLValue args[1]={error};
 FLValue message=fl_v11_builtin(fl_str_val("error-message"),fl_vec_from(args,1));
 return message.tag==FL_STRING&&!strcmp(((FLString*)message.obj)->data,"timer cancelled");
}
FLValue worker_timer_run(FLValue mode,FLValue setup){
 state.mode=((FLString*)mode.obj)->data;state.setup=fl_heap_copy(setup);
 state.promises=fl_nil();atomic_store(&state.cleanups,0);atomic_store(&state.active,0);
 atomic_store(&state.pending,0);atomic_store(&state.captured,0);
 pthread_t thread;if(pthread_create(&thread,NULL,run,NULL)||pthread_join(thread,NULL))abort();
 int n=0;
 if(state.promises.tag!=FL_VECTOR)abort();
 FLVector*promises=(FLVector*)state.promises.obj;
 for(uint32_t i=0;i<promises->len;i++)n+=cancelled(promises->data[i]);
 FLValue out=fl_map_new();
 out=fl_map_set(out,fl_str_val("cancelled"),fl_int(n));
 out=fl_map_set(out,fl_str_val("active"),fl_int(atomic_load(&state.active)));
 out=fl_map_set(out,fl_str_val("pending"),fl_int(atomic_load(&state.pending)));
 out=fl_map_set(out,fl_str_val("captured"),fl_int(atomic_load(&state.captured)));
 out=fl_map_set(out,fl_str_val("cleanups"),fl_int(atomic_load(&state.cleanups)));
 fl_heap_release(state.promises);fl_heap_release(state.setup);
 state.promises=state.setup=fl_nil();return out;
}
