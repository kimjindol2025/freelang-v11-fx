#define _GNU_SOURCE
#include <dlfcn.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/rand.h>
static int enabled(const char*name){const char*p=getenv(name);return p&&!strcmp(p,"1");}
int clock_gettime(clockid_t id,struct timespec*ts){if(id==CLOCK_REALTIME&&enabled("L08_AUTH_FIXED_CLOCK")){ts->tv_sec=1800000000;ts->tv_nsec=0;return 0;}int(*real)(clockid_t,struct timespec*)=dlsym(RTLD_NEXT,"clock_gettime");return real(id,ts);}
time_t time(time_t*out){if(enabled("L08_AUTH_FIXED_CLOCK")){if(out)*out=1800000000;return 1800000000;}time_t(*real)(time_t*)=dlsym(RTLD_NEXT,"time");return real(out);}
int RAND_bytes(unsigned char*out,int n){if(enabled("L08_AUTH_RANDOM_FAIL")){fputs("AUTH_RNG_FAULT=ACTIVE\n",stderr);return 0;}int(*real)(unsigned char*,int)=dlsym(RTLD_NEXT,"RAND_bytes");return real(out,n);}
