#define _GNU_SOURCE
#include "runtime.h"
#include <pthread.h>
#include <stdatomic.h>
#include <stdio.h>
#include <time.h>
static atomic_int active,overlap,finished;
static pthread_barrier_t barrier;
static FLValue callback(FLClosure*c,int n,FLValue*a){
 (void)c;(void)n;(void)a;
 if(atomic_fetch_add(&active,1)!=0)atomic_store(&overlap,1);
 struct timespec pause={0,50000000};nanosleep(&pause,NULL);
 atomic_fetch_sub(&active,1);atomic_fetch_add(&finished,1);return fl_nil();
}
static void*drain(void*unused){(void)unused;pthread_barrier_wait(&barrier);fl_schedule_tick(10);return NULL;}
int main(void){
 for(int i=0;i<4;i++){FLValue args[2]={fl_fn_new(callback,0,NULL),fl_int(1)};fl_v11_timer(fl_str_val("set-timeout"),fl_vec_from(args,2));}
 struct timespec pause={0,10000000};nanosleep(&pause,NULL);pthread_barrier_init(&barrier,NULL,4);pthread_t workers[4];
 for(int i=0;i<4;i++)pthread_create(&workers[i],NULL,drain,NULL);
 for(int i=0;i<4;i++)pthread_join(workers[i],NULL);
 int ok=atomic_load(&finished)==4&&atomic_load(&overlap)==0;
 printf("TIMER_DISPATCH_SERIALIZATION=%s\n",ok?"PASS":"FAIL");fl_schedule_shutdown();return ok?0:1;
}
