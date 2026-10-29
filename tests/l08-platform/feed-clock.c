#define _POSIX_C_SOURCE 200809L
#include <time.h>
int __real_clock_gettime(clockid_t, struct timespec*);
int __wrap_clock_gettime(clockid_t id, struct timespec*out){
 if(id!=CLOCK_REALTIME)return __real_clock_gettime(id,out);
 out->tv_sec=1800000000;out->tv_nsec=0;return 0;
}
