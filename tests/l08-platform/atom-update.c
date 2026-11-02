#include "atom-update.h"
#include <pthread.h>
#include <stdatomic.h>
#include <errno.h>
#include <stdint.h>
#include <time.h>
static pthread_barrier_t start;
static atomic_int workers,ticks;
static pthread_mutex_t peer_lock=PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t peer_cv=PTHREAD_COND_INITIALIZER;
static int owner,peer;
__attribute__((constructor)) static void init(void){
 if(pthread_barrier_init(&start,NULL,8))fl_throw(fl_str_val("atom test barrier init failed"));
}
__attribute__((destructor)) static void finish(void){pthread_barrier_destroy(&start);}
FLValue atom_test_start(void){
 atomic_fetch_add(&workers,1);
 int rc=pthread_barrier_wait(&start);
 if(rc&&rc!=PTHREAD_BARRIER_SERIAL_THREAD)fl_throw(fl_str_val("atom test barrier failed"));
 return fl_bool(true);
}
FLValue atom_test_workers(void){return fl_int(atomic_load(&workers));}
FLValue atom_test_tick(void){atomic_fetch_add(&ticks,1);return fl_nil();}
FLValue atom_test_ticks(void){return fl_int(atomic_load(&ticks));}
FLValue atom_test_pause(void){
 struct timespec delay={0,500000};
 while(nanosleep(&delay,&delay)&&errno==EINTR){}
 return fl_nil();
}
FLValue atom_test_independent_pair(void){
 FLValue buckets[64]={0};
 for(int i=0;i<65;i++){
  FLValue atom=fl_atom_new(fl_int(0));
  if(atom.tag!=FL_VECTOR)fl_throw(fl_str_val("atom test allocation failed"));
  size_t bucket=((uintptr_t)atom.obj>>4)%64;
  if(buckets[bucket].tag==FL_VECTOR){FLValue pair[2]={buckets[bucket],atom};return fl_vec_from(pair,2);}
  buckets[bucket]=atom;
 }
 fl_throw(fl_str_val("atom test collision setup failed"));return fl_nil();
}
static FLValue wait_for(int *ready){
 struct timespec limit;clock_gettime(CLOCK_REALTIME,&limit);limit.tv_sec+=5;
 int rc=0;
 while(!*ready&&!rc)rc=pthread_cond_timedwait(&peer_cv,&peer_lock,&limit);
 int ok=*ready;pthread_mutex_unlock(&peer_lock);
 return fl_bool(ok);
}
FLValue atom_test_wait_peer(void){
 pthread_mutex_lock(&peer_lock);owner=1;pthread_cond_broadcast(&peer_cv);return wait_for(&peer);
}
FLValue atom_test_wait_owner(void){pthread_mutex_lock(&peer_lock);return wait_for(&owner);}
FLValue atom_test_signal_peer(void){
 pthread_mutex_lock(&peer_lock);peer=1;pthread_cond_broadcast(&peer_cv);pthread_mutex_unlock(&peer_lock);return fl_bool(true);
}
