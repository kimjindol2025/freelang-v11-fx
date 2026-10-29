#include "arena-cache.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <pthread.h>
void *__real_malloc(size_t size);
void __real_free(void *pointer);
void *__real_fl_arena_alloc(size_t size);
static pthread_mutex_t lock=PTHREAD_MUTEX_INITIALIZER;
static void *pointers[4096];
static size_t sizes[4096],live_bytes;
static __thread unsigned allocating,fault_mask,faults,enabled;
void *__wrap_malloc(size_t size){
 if(allocating&&fault_mask){unsigned fail=fault_mask&1;fault_mask>>=1;if(fail){faults++;return NULL;}}
 void *pointer=__real_malloc(size);
 if(pointer&&allocating&&enabled){
  pthread_mutex_lock(&lock);size_t bucket=((uintptr_t)pointer>>4)&4095,i;
  for(i=0;i<4096;i++){size_t slot=(bucket+i)&4095;if(!pointers[slot]||pointers[slot]==(void*)1){pointers[slot]=pointer;sizes[slot]=size;live_bytes+=size;break;}}
  pthread_mutex_unlock(&lock);if(i==4096)abort();
 }
 return pointer;
}
void __wrap_free(void *pointer){
 if(pointer){
  pthread_mutex_lock(&lock);size_t bucket=((uintptr_t)pointer>>4)&4095;
  for(size_t i=0;i<4096;i++){size_t slot=(bucket+i)&4095;if(!pointers[slot])break;if(pointers[slot]==pointer){live_bytes-=sizes[slot];pointers[slot]=(void*)1;break;}}
  pthread_mutex_unlock(&lock);
 }
 __real_free(pointer);
}
void *__wrap_fl_arena_alloc(size_t size){allocating++;void *pointer=__real_fl_arena_alloc(size);allocating--;return pointer;}
static size_t retained(void){pthread_mutex_lock(&lock);size_t bytes=live_bytes;pthread_mutex_unlock(&lock);return bytes;}
static void label(const char *name,int ok){printf("%s=%s\n",name,ok?"PASS":"FAIL");}
FLValue arena_cache_run(FLValue callback){
 void *previous=fl_arena_scope_enter();fl_arena_end();fl_arena_begin();
 enabled=1;
 size_t maximum=0;int bytes_ok=1;
 for(int request=1;request<=64;request++){
  FLValue size=fl_int((int64_t)request*65536);
  FLValue value=fl_fn_call(callback,1,&size);
  if(value.tag!=FL_STRING||((FLString*)value.obj)->len!=(uint32_t)size.i)bytes_ok=0;
  else for(size_t i=0;i<(size_t)size.i;i++)if(((FLString*)value.obj)->data[i]!='x'){bytes_ok=0;break;}
  fl_arena_end();size_t current=retained();if(current>maximum)maximum=current;fl_arena_begin();
 }
 puts("ARENA_CALLBACK_REQUESTS=64");label("ARENA_CALLBACK_BYTES",bytes_ok);
 printf("ARENA_CACHE_AFTER_RESET_BYTES=%zu\n",maximum);
 void *small=fl_arena_alloc(4096);memset(small,'S',4096);fl_arena_end();fl_arena_begin();
 int reused=fl_arena_alloc(4096)==small;label("ARENA_FIRST_CHUNK_REUSED",reused);
 unsigned char *parent=fl_arena_alloc(131072);memset(parent,'P',131072);
 void *outer=fl_arena_scope_enter();unsigned char *child=fl_arena_alloc(262144);memset(child,'C',262144);fl_arena_end();fl_arena_scope_leave(outer);
 int nested=1;for(size_t i=0;i<131072;i++)if(parent[i]!='P'){nested=0;break;}
 label("ARENA_NESTED_PARENT",nested);fl_arena_end();fl_arena_begin();
 fault_mask=1;faults=0;unsigned char *fallback=fl_arena_alloc(8388608);fault_mask=0;
 int fallback_ok=fallback&&faults==1;
 if(fallback){memset(fallback,'F',8388608);fallback_ok=fallback_ok&&fallback[8388607]=='F';}
 label("ARENA_ONE_SHOT_FALLBACK",fallback_ok);fl_arena_end();fl_arena_begin();
 size_t fallback_retained=retained();printf("ARENA_FALLBACK_AFTER_RESET_BYTES=%zu\n",fallback_retained);
 fault_mask=3;faults=0;int oom=fl_arena_alloc(8388608)==NULL&&faults==2;fault_mask=0;
 label("ARENA_OOM_RETURNS_NULL",oom);fl_arena_end();fl_arena_begin();
 fault_mask=5;faults=0;int cleanup_oom=fl_arena_alloc(8388608)==NULL&&faults==2;fault_mask=0;
 fl_arena_end();fl_arena_begin();cleanup_oom=cleanup_oom&&retained()==0;label("ARENA_CLEANUP_OOM_SAFE",cleanup_oom);
 int overflow=fl_arena_alloc(SIZE_MAX)==NULL;label("ARENA_SIZE_OVERFLOW_REJECTED",overflow);
 fl_arena_scope_leave(previous);enabled=0;
 return fl_bool(bytes_ok&&maximum==0&&reused&&nested&&fallback_ok&&fallback_retained==0&&oom&&cleanup_oom&&overflow);
}
