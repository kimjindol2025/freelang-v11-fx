#include "runtime.h"
#include <stdlib.h>
#include <stdio.h>
static int fail_next,cleaned;
void*__real_calloc(size_t,size_t);
void*__wrap_calloc(size_t n,size_t size){if(fail_next){fail_next=0;return NULL;}return __real_calloc(n,size);}
static void cleanup(void*unused){(void)unused;cleaned=1;}
int main(void){
 fl_arena_begin();FLTryFrame*frame=&fl_try_stack[fl_try_top++];int caught=0;
 if(setjmp(frame->buf)==0){fail_next=1;(void)fl_arena_scope_enter();fl_try_top--;}
 else{fl_try_top--;caught=1;}
 int registered=fl_arena_on_reset(cleanup,NULL);fl_arena_end();
 int ok=caught&&registered==1&&cleaned;
 printf("TIMER_SCOPE_OOM_RESTORES_CALLER=%s\n",ok?"PASS":"FAIL");return ok?0:1;
}
