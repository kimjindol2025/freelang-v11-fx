/* B17 fixed native cases: immutable aliases, normalized/binary keys, scaling. */
#include "runtime.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#define CHECK(c,n) do { if(!(c)){fprintf(stderr,"B17_%s=FAIL\n",n);exit(1);}printf("B17_%s=PASS\n",n); } while(0)
static FLValue bytes(const char *p,size_t n){
 FLString *s=fl_arena_alloc(sizeof(*s)+n+1);
 if(!s)exit(2);
 s->base.type=FL_STRING;s->base.rc=1;s->len=n;memcpy(s->data,p,n);s->data[n]=0;
 return (FLValue){.tag=FL_STRING,.obj=(FLObject*)s};
}
static double seconds(void){struct timespec t;clock_gettime(CLOCK_MONOTONIC,&t);return t.tv_sec+t.tv_nsec/1e9;}
static void fixed_cases(void){
 FLValue a=fl_vec_push(fl_vec_new(),fl_int(10)),b=a;
 b=fl_vec_push(b,fl_int(20));
 CHECK(fl_vec_len(a).i==1&&fl_vec_get(a,fl_int(0)).i==10,"SHARED_ORIGINAL");
 CHECK(fl_vec_len(b).i==2&&fl_vec_get(b,fl_int(1)).i==20,"SHARED_APPENDED");
 FLValue c=fl_vec_push(a,fl_int(30));
 CHECK(fl_vec_len(a).i==1&&fl_vec_get(b,fl_int(1)).i==20&&fl_vec_get(c,fl_int(1)).i==30,"SHARED_BRANCH");
 FLValue d=fl_vec_set(b,fl_int(0),fl_int(90));
 CHECK(fl_vec_get(a,fl_int(0)).i==10&&fl_vec_get(b,fl_int(0)).i==10&&fl_vec_get(d,fl_int(0)).i==90,"SHARED_SET");
 FLValue pairs[]={fl_int(1),fl_int(10),fl_str_val("1"),fl_int(20),bytes("a\0x",3),fl_int(30),bytes("a\0y",3),fl_int(40),bytes("a\0x",3),fl_int(50),fl_str_val("한🙂"),fl_int(60)};
 FLValue map=fl_v11_map_from_pairs(pairs,6);FLMap *m=(FLMap*)map.obj;
 CHECK(m->len==4,"MAP_NORMALIZED_COUNT");
 CHECK(fl_map_get(map,fl_v11_key(fl_str_val("1"))).i==20,"MAP_LAST_VALUE");
 CHECK(fl_map_get(map,fl_v11_key(bytes("a\0x",3))).i==50&&fl_map_get(map,fl_v11_key(bytes("a\0y",3))).i==40,"MAP_BINARY_KEYS");
 CHECK(m->entries[0].val.i==20&&m->entries[1].val.i==50&&m->entries[2].val.i==40&&m->entries[3].val.i==60,"MAP_FIRST_KEY_ORDER");
}
static void benchmark(unsigned n){
 FLValue *pairs=malloc(sizeof(*pairs)*(size_t)n*2);if(!pairs)exit(2);
 for(unsigned i=0;i<n;i++){char key[32];snprintf(key,sizeof(key),"key-%08u",i);pairs[2*i]=fl_str_val(key);pairs[2*i+1]=fl_int(i);}
 double begin=seconds();FLValue map=fl_v11_map_from_pairs(pairs,n);double elapsed=seconds()-begin;
 FLMap *m=(FLMap*)map.obj;if(m->len!=n)exit(1);
 for(unsigned i=0;i<n;i++)if(m->entries[i].val.i!=i)exit(1);
 printf("MAP_KEYS=%u ELAPSED_SECONDS=%.9f COUNT=%u VALUES=PASS\n",n,elapsed,m->len);free(pairs);
}
int main(int argc,char **argv){fl_arena_begin();if(argc==1)fixed_cases();else benchmark((unsigned)strtoul(argv[1],NULL,10));fl_arena_end();return 0;}
