#include "runtime.h"
#include <stdio.h>
#include <string.h>
static FLValue echo(FLClosure*cl,int argc,FLValue*argv){(void)argc;(void)argv;return cl->env[0];}
int main(void){
 FLValue text=fl_str_val("startup-한😀");
 FLValue fn=fl_fn_new(echo,1,&text);
 if(fl_arena_active())return 1;
 puts("UNSCOPED_QUERY_UNCHANGED=PASS");
 for(int i=0;i<3;i++){
  fl_arena_begin();
  for(int j=0;j<1000;j++)(void)fl_str_val("request overwrite");
  fl_arena_end();
  FLValue result=fl_fn_call(fn,0,NULL);
  if(result.tag!=FL_STRING||strcmp(((FLString*)result.obj)->data,"startup-한😀"))return 1;
 }
 puts("UNSCOPED_SURVIVES_REQUEST_RESETS=PASS");
 return 0;
}
