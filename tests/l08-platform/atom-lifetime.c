#include "atom-lifetime.h"
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
static pthread_mutex_t meter_lock=PTHREAD_MUTEX_INITIALIZER;
static pthread_barrier_t barrier;
static void *objects[65536];
static unsigned live,created;
static int enabled;
FLValue __real_fl_atom_new(FLValue init);
void __real_free(void *pointer);
__attribute__((constructor)) static void init(void){if(pthread_barrier_init(&barrier,NULL,8))abort();}
__attribute__((destructor)) static void finish(void){pthread_barrier_destroy(&barrier);}
FLValue __wrap_fl_atom_new(FLValue init){
 FLValue value=__real_fl_atom_new(init);
 if(value.tag!=FL_VECTOR)return value;
 pthread_mutex_lock(&meter_lock);
 if(enabled){
  size_t bucket=((uintptr_t)value.obj>>4)&65535,i;
  for(i=0;i<65536;i++){size_t slot=(bucket+i)&65535;if(!objects[slot]||objects[slot]==(void*)1){objects[slot]=value.obj;break;}}
  if(i==65536){pthread_mutex_unlock(&meter_lock);fputs("atom meter capacity exceeded\n",stderr);abort();}
  live++;created++;
 }
 pthread_mutex_unlock(&meter_lock);return value;
}
void __wrap_free(void *pointer){
 if(pointer){pthread_mutex_lock(&meter_lock);if(enabled){size_t bucket=((uintptr_t)pointer>>4)&65535;for(size_t i=0;i<65536;i++){size_t slot=(bucket+i)&65535;if(!objects[slot])break;if(objects[slot]==pointer){objects[slot]=(void*)1;live--;break;}}}pthread_mutex_unlock(&meter_lock);}
 __real_free(pointer);
}
FLValue atom_meter_start(void){pthread_mutex_lock(&meter_lock);enabled=1;pthread_mutex_unlock(&meter_lock);return fl_nil();}
FLValue atom_meter_live(void){pthread_mutex_lock(&meter_lock);unsigned n=live;pthread_mutex_unlock(&meter_lock);return fl_int(n);}
FLValue atom_meter_created(void){pthread_mutex_lock(&meter_lock);unsigned n=created;pthread_mutex_unlock(&meter_lock);return fl_int(n);}
FLValue atom_meter_barrier(void){int rc=pthread_barrier_wait(&barrier);if(rc&&rc!=PTHREAD_BARRIER_SERIAL_THREAD)abort();return fl_nil();}
FLValue atom_meter_scope(FLValue fn){
 if(fl_try_top>=FL_TRY_MAX)fl_throw(fl_str_val("atom scope try limit"));
 void *previous=fl_arena_scope_enter();
 FLValue owned=fl_nil();int failed=0;
 FLTryFrame *frame=&fl_try_stack[fl_try_top++];
 if(setjmp(frame->buf)==0)owned=fl_heap_copy(fl_fn_call(fn,0,NULL));
 else {owned=fl_heap_copy(frame->err);failed=1;}
 fl_try_top--;fl_arena_scope_leave(previous);
 FLValue result=fl_arena_copy(owned);fl_heap_release(owned);
 if(failed)fl_throw(result);
 return result;
}
