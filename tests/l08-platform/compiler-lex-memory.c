#include "compiler-lex-memory.h"
#include <stdlib.h>
#include <stdint.h>
typedef struct Allocation {void *pointer;size_t bytes;struct Allocation *next;} Allocation;
static Allocation *table[4096];
static size_t live,peak;
static int enabled;
void *__real_malloc(size_t);void *__real_calloc(size_t,size_t);void *__real_realloc(void*,size_t);void __real_free(void*);
static unsigned bucket(void *p){return ((uintptr_t)p>>4)%4096;}
static void track(void *p,size_t n){
 if(!enabled||!p)return;
 Allocation *a=__real_malloc(sizeof(*a));if(!a)abort();
 unsigned key=bucket(p);*a=(Allocation){p,n,table[key]};table[key]=a;
 live+=n;if(live>peak)peak=live;
}
static int untrack(void *p,size_t *bytes){
 if(!p)return 0;
 Allocation **slot=&table[bucket(p)];
 while(*slot&&(*slot)->pointer!=p)slot=&(*slot)->next;
 if(!*slot)return 0;
 Allocation *a=*slot;*bytes=a->bytes;live-=a->bytes;*slot=a->next;__real_free(a);return 1;
}
void *__wrap_malloc(size_t n){void *p=__real_malloc(n);track(p,n);return p;}
void *__wrap_calloc(size_t n,size_t width){void *p=__real_calloc(n,width);if(p)track(p,n*width);return p;}
void *__wrap_realloc(void *old,size_t n){
 size_t before=0;int had=untrack(old,&before);
 void *p=__real_realloc(old,n);
 if(!p&&n&&had)track(old,before);else track(p,n);
 return p;
}
void __wrap_free(void *p){size_t n=0;untrack(p,&n);__real_free(p);}
FLValue compiler_lex_meter_start(void){if(enabled)fl_throw(fl_str_val("lexer meter already running"));enabled=1;return fl_nil();}
FLValue compiler_lex_meter_live(void){return fl_int(live);}
FLValue compiler_lex_meter_peak(void){return fl_int(peak);}
