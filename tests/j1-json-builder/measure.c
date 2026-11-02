#include "runtime.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/resource.h>
int main(int argc,char **argv){
    assert(argc==3);size_t n=strtoul(argv[2],NULL,10);assert(n>0&&n<=100000);
    char *text=malloc(n*2+2);assert(text);text[0]='[';
    for(size_t i=0;i<n;i++){text[1+2*i]='0';text[2+2*i]=i+1==n?']':',';}text[2*n+1]=0;
    fl_arena_begin();FLValue src=fl_str_val(text);free(text);
    struct timespec a,b;clock_gettime(CLOCK_MONOTONIC,&a);
    FLValue value;
    if(!strcmp(argv[1],"legacy"))value=fl_json_parse(src);
    else {assert(!strcmp(argv[1],"v11"));FLValue args=fl_vec_from(&src,1);value=fl_v11_builtin(fl_str_val("json_parse"),args);}
    clock_gettime(CLOCK_MONOTONIC,&b);
    assert(value.tag==FL_VECTOR);FLVector *v=(FLVector*)value.obj;assert(v->len==n&&v->base.rc!=0xFE);
    for(size_t i=0;i<n;i++)assert((v->data[i].tag==FL_INT&&v->data[i].i==0)||(v->data[i].tag==FL_FLOAT&&v->data[i].f==0.0));
    struct rusage usage;getrusage(RUSAGE_SELF,&usage);
    printf("API=%s ITEMS=%zu PARSE_SECONDS=%.9f MAX_RSS_KIB=%ld VALUES=PASS\n",argv[1],n,(b.tv_sec-a.tv_sec)+(b.tv_nsec-a.tv_nsec)/1e9,usage.ru_maxrss);
    fl_arena_end();return 0;
}
