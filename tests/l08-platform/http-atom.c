#include "http-atom.h"
#include <pthread.h>
#include <errno.h>
#include <time.h>
static pthread_mutex_t lock=PTHREAD_MUTEX_INITIALIZER;
static pthread_barrier_t first_four;
static pthread_t workers[256];
static int worker_count,active,peak,calls;
__attribute__((constructor)) static void init(void){
 if(pthread_barrier_init(&first_four,NULL,4))fl_throw(fl_str_val("HTTP atom barrier init failed"));
}
__attribute__((destructor)) static void finish(void){pthread_barrier_destroy(&first_four);}
FLValue http_atom_begin(void){
 pthread_mutex_lock(&lock);
 pthread_t self=pthread_self();int found=0;
 for(int i=0;i<worker_count;i++)if(pthread_equal(workers[i],self))found=1;
 if(!found){
  if(worker_count==256){pthread_mutex_unlock(&lock);fl_throw(fl_str_val("HTTP atom worker count overflow"));}
  workers[worker_count++]=self;
 }
 int number=++calls;if(++active>peak)peak=active;
 pthread_mutex_unlock(&lock);
 if(number<=4){int rc=pthread_barrier_wait(&first_four);if(rc&&rc!=PTHREAD_BARRIER_SERIAL_THREAD)fl_throw(fl_str_val("HTTP atom barrier failed"));}
 return fl_nil();
}
FLValue http_atom_end(void){pthread_mutex_lock(&lock);active--;pthread_mutex_unlock(&lock);return fl_nil();}
FLValue http_atom_pause(void){struct timespec delay={0,500000};while(nanosleep(&delay,&delay)&&errno==EINTR){}return fl_nil();}
FLValue http_atom_stats(void){
 pthread_mutex_lock(&lock);int n=worker_count,p=peak,a=active;pthread_mutex_unlock(&lock);
 FLValue pairs[6]={fl_str_val("workers"),fl_int(n),fl_str_val("peak"),fl_int(p),fl_str_val("active"),fl_int(a)};
 return fl_map_from_pairs(pairs,3);
}
