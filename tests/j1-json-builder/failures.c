#include "runtime.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
typedef struct {const char *text,*legacy;int legacy_error,v11_error;} Case;
static const Case cases[]={
 {"[1,2]x",NULL,1,1},
 {"[[1,2],{\"a\":[3,4]}]x",NULL,1,1},
 {"[1,2,x]","[1,2,null]",0,1},
 {"[[1,2],x]","[[1,2],null]",0,1},
 {"[1,2","[1,2]",0,1},
 {"[[1,2]","[[1,2]]",0,1},
 {"[1,\"\\q\"]","[1,\"q\"]",0,1},
 {"[1,{\"a\":[2,3]}]","[1,{\"a\":[2,3]}]",0,0},
 {"[]","[]",0,0}
};
int main(void){
    fl_cli_runtime_begin();
    for(int api=0;api<2;api++)for(size_t i=0;i<sizeof(cases)/sizeof(*cases);i++){
        for(int repeat=0;repeat<100;repeat++){
            fl_arena_begin();FLValue input=fl_str_val(cases[i].text),result=fl_nil();int threw;
            FLTryFrame *frame=&fl_try_stack[fl_try_top++];
            if(setjmp(frame->buf)==0){
                if(api){FLValue argv=fl_vec_from(&input,1);result=fl_v11_builtin(fl_str_val("json_parse"),argv);}
                else result=fl_json_parse(input);
                threw=0;
            }else{
                threw=1;
                assert(frame->err.tag==FL_STRING);
                if(api)assert(!strncmp(((FLString*)frame->err.obj)->data,"json_parse: invalid JSON:",25));
            }
            fl_try_top--;assert(threw==(api?cases[i].v11_error:cases[i].legacy_error));
            if(!threw){
                assert(result.tag==FL_VECTOR&&result.obj->rc!=0xFE);
                FLValue text=fl_json_stringify(result);
                assert(!strcmp(((FLString*)text.obj)->data,cases[i].legacy));
            }
            fl_arena_end();
        }
        printf("API=%s CASE=%zu REPEATS=100 COMPAT=PASS\n",api?"v11":"legacy",i+1);
    }
    puts("JSON_FAILURE_CASES=18 FAIL=0");return 0;
}
