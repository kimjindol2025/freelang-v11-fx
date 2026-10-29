#include "fd-concurrent.h"
#include <pthread.h>
static pthread_barrier_t barrier;
static pthread_mutex_t count_lock=PTHREAD_MUTEX_INITIALIZER;
static int count;
__attribute__((constructor)) static void init(void){if(pthread_barrier_init(&barrier,NULL,8))fl_throw(fl_str_val("test barrier init failed"));}
FLValue fd_test_barrier(void){pthread_mutex_lock(&count_lock);count++;pthread_mutex_unlock(&count_lock);int rc=pthread_barrier_wait(&barrier);if(rc&&rc!=PTHREAD_BARRIER_SERIAL_THREAD)fl_throw(fl_str_val("test barrier failed"));return fl_bool(true);}
FLValue fd_test_thread_count(void){pthread_mutex_lock(&count_lock);int n=count;pthread_mutex_unlock(&count_lock);return fl_int(n);}
__attribute__((destructor)) static void finish(void){pthread_barrier_destroy(&barrier);}
