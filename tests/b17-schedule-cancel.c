#include "runtime.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
static int calls;
static FLValue callback(FLClosure *s,int n,FLValue*a){(void)s;(void)n;(void)a;calls++;return fl_int(9);}
int main(void){
 fl_arena_begin();
 FLValue args[]={fl_int(fl_now_ms().i+10),fl_fn_new(callback,0,NULL)};
 FLValue handle=fl_v11_timer(fl_str_val("schedule-at"),fl_vec_from(args,2));
 fl_schedule_worker_cleanup();usleep(20000);fl_schedule_tick(0);
 if(calls)return 1;puts("AT_CANCEL_CALLBACKS=0");
 int saved=fl_try_top;FLTryFrame*f=&fl_try_stack[fl_try_top++];
 if(!setjmp(f->buf)){fl_v11_timer(fl_str_val("await-result"),fl_vec_from(&handle,1));return 1;}
 fl_try_top=saved;
 FLValue message=fl_map_get(f->err,fl_str_val("message"));
 if(message.tag!=FL_STRING||strcmp(((FLString*)message.obj)->data,"timer cancelled"))return 1;
 puts("AT_CANCEL_ERROR=timer cancelled");fl_arena_end();return 0;
}
