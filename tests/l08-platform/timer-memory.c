#include "runtime.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
static int invoked;
static void require(int ok,const char*why){if(!ok){fprintf(stderr,"TIMER_MEMORY_FAIL %s\n",why);exit(1);}}
static FLValue capture(FLClosure*cl,int argc,FLValue*argv){(void)argc;(void)argv;fl_atom_reset(cl->env[1],fl_int(1));return cl->env[0];}
static FLValue result(FLClosure*cl,int argc,FLValue*argv){(void)cl;(void)argc;(void)argv;invoked++;return fl_int(42);}
int main(void){
 fl_arena_begin();FLValue atom=fl_atom_new(fl_int(0)),env[2]={fl_str_val("captured-한😀"),atom};
 FLValue raw=fl_fn_new(capture,2,env),owned=fl_heap_copy(raw);
 fl_arena_end();fl_arena_begin();
 for(int i=0;i<1000;i++)(void)fl_str_val("overwrite request arena");
 FLValue text=fl_fn_call(owned,0,NULL);
 require(text.tag==FL_STRING&&!strcmp(((FLString*)text.obj)->data,"captured-한😀"),"captured text");
 require(fl_atom_deref(atom).i==1,"atom identity");fl_heap_release(owned);
 FLValue args[2]={fl_fn_new(result,0,NULL),fl_int(1)};
 FLValue promise=fl_v11_timer(fl_str_val("set-timeout"),fl_vec_from(args,2)),copies[1024];
 for(int i=0;i<1024;i++)copies[i]=fl_heap_copy(promise);
 fl_arena_end();fl_arena_begin();FLValue caller=fl_str_val("caller survives nested timer arena");
 for(int i=0;i<100&&invoked==0;i++)fl_schedule_tick(5);
 require(invoked==1,"callback once");require(!strcmp(((FLString*)caller.obj)->data,"caller survives nested timer arena"),"caller arena restored");
 for(int i=0;i<1024;i++){FLValue a[1]={copies[i]};FLValue value=fl_v11_timer(fl_str_val("await-result"),fl_vec_from(a,1));require(value.tag==FL_INT&&value.i==42,"promise strong references");fl_heap_release(copies[i]);}
 fl_arena_end();fl_schedule_shutdown();puts("TIMER_MEMORY_BOUNDARIES=PASS");return 0;
}
