#include "compiler-lex-memory.h"
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
typedef struct Allocation {void *pointer;size_t bytes;unsigned site;struct Allocation *next;} Allocation;
static struct {void *address;size_t live,total,count;} sites[64];
static unsigned site_count;
static Allocation *table[4096];
static size_t live,peak;
static int enabled;
void *__real_malloc(size_t);void *__real_calloc(size_t,size_t);void *__real_realloc(void*,size_t);void __real_free(void*);
static unsigned bucket(void *p){return ((uintptr_t)p>>4)%4096;}
static void track(void *p,size_t n,void *address){
 if(!enabled||!p)return;
 Allocation *a=__real_malloc(sizeof(*a));if(!a)abort();
 unsigned site=0;while(site<site_count&&sites[site].address!=address)site++;
 if(site==site_count&&site_count<64)sites[site_count++].address=address;
 if(site==64)abort();
 sites[site].live+=n;sites[site].total+=n;sites[site].count++;
 unsigned key=bucket(p);*a=(Allocation){p,n,site,table[key]};table[key]=a;
 live+=n;if(live>peak)peak=live;
}
static int untrack(void *p,size_t *bytes){
 if(!p)return 0;
 Allocation **slot=&table[bucket(p)];
 while(*slot&&(*slot)->pointer!=p)slot=&(*slot)->next;
 if(!*slot)return 0;
 Allocation *a=*slot;*bytes=a->bytes;live-=a->bytes;sites[a->site].live-=a->bytes;*slot=a->next;__real_free(a);return 1;
}
void *__wrap_malloc(size_t n){void *p=__real_malloc(n);track(p,n,__builtin_return_address(0));return p;}
void *__wrap_calloc(size_t n,size_t width){void *p=__real_calloc(n,width);if(p)track(p,n*width,__builtin_return_address(0));return p;}
void *__wrap_realloc(void *old,size_t n){
 size_t before=0;int had=untrack(old,&before);
 void *p=__real_realloc(old,n);
 if(!p&&n&&had)track(old,before,__builtin_return_address(0));else track(p,n,__builtin_return_address(0));
 return p;
}
void __wrap_free(void *p){size_t n=0;untrack(p,&n);__real_free(p);}
FLValue compiler_lex_meter_start(void){if(enabled)fl_throw(fl_str_val("lexer meter already running"));enabled=1;return fl_nil();}
FLValue compiler_lex_meter_live(void){
 for(unsigned i=0;i<site_count;i++)printf("COMPILER_PARSE_SITE=%p LIVE=%zu TOTAL=%zu COUNT=%zu\n",sites[i].address,sites[i].live,sites[i].total,sites[i].count);
 return fl_int(live);
}
FLValue compiler_lex_meter_peak(void){return fl_int(peak);}
