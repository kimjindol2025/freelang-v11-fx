#include "runtime.h"
#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
int main(void) {
    size_t used=0,cap=4096;char *data=malloc(cap);assert(data);
    for(;;){if(used==cap){cap*=2;char *next=realloc(data,cap);assert(next);data=next;}
        ssize_t n=read(0,data+used,cap-used);if(n>0)used+=(size_t)n;else if(n<0&&errno==EINTR)continue;else{assert(!n);break;}}
    fl_cli_runtime_begin();FLValue text=fl_strn_val(data,used);free(data);
    FLValue args=fl_v11_builtin(fl_str_val("json-parse"),fl_vec_from(&text,1));assert(args.tag==FL_VECTOR);
    FLValue result=fl_v11_builtin(fl_str_val("tcp-send"),args);
    FLValue json=fl_v11_builtin(fl_str_val("json-str"),fl_vec_from(&result,1));assert(json.tag==FL_STRING);
    FLString *value=(FLString*)json.obj;assert(fwrite(value->data,1,value->len,stdout)==value->len);putchar('\n');return 0;
}
