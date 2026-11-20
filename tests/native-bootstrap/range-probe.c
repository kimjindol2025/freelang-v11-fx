#include "runtime.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static int controls;
static FLValue parse(const char *json) {
    FLValue text=fl_str_val(json);
    return fl_v11_builtin(fl_str_val("json-parse"),fl_vec_from(&text,1));
}
static void check(const char *label,const char *args,const char *expected) {
    FLValue value=fl_v11_builtin(fl_str_val("range"),parse(args));
    assert(value.tag==FL_VECTOR&&value.obj->rc==1);
    FLVector *vector=(FLVector*)value.obj;
    for(uint32_t i=0;i<vector->len;i++)assert(vector->data[i].tag==FL_FLOAT);
    FLValue json=fl_v11_builtin(fl_str_val("json-str"),fl_vec_from(&value,1));
    FLString *text=(FLString*)json.obj;
    if(text->len!=strlen(expected)||memcmp(text->data,expected,text->len)){
        fprintf(stderr,"%s: actual=%s expected=%s\n",label,text->data,expected);assert(0);
    }
    printf("%s PASS\n",label);controls++;
}
int main(int argc,char **argv) {
    fl_cli_runtime_begin();
    check("one argument","[5]","[0,1,2,3,4]");
    check("two arguments","[2,5]","[2,3,4]");
    check("positive stride","[1,8,2]","[1,3,5,7]");
    check("descending","[3,0,-1]","[3,2,1]");
    check("wrong positive direction","[3,0,1]","[]");
    check("wrong negative direction","[0,3,-1]","[]");
    check("equal endpoints","[2,2]","[]");
    check("zero step","[0,3,0]","[]");
    check("negative zero step","[0,3,-0]","[]");
    check("numeric strings","[\"2\",\"6\",\"2\"]","[2,4]");
    check("boolean true","[true,4]","[1,2,3]");
    check("boolean false","[false,3]","[0,1,2]");
    check("nil endpoint","[null,3]","[0,1,2]");
    check("array coercion","[[2],4]","[2,3]");
    check("empty array coercion","[[],3]","[0,1,2]");
    check("NaN endpoint","[0,\"bad\"]","[]");
    check("NaN step","[0,5,\"bad\"]","[]");
    check("positive infinite empty","[\"Infinity\",0,1]","[]");
    check("negative infinite empty","[\"-Infinity\",0,-1]","[]");
    check("repeated fractional addition","[0,0.5,0.1]","[0,0.1,0.2,0.30000000000000004,0.4]");
    check("descending fractional rounding","[0.6,0,-0.2]","[0.6,0.39999999999999997,0.19999999999999996]");
    FLValue original=fl_v11_builtin(fl_str_val("range"),parse("[5]"));
    FLValue appended=fl_vec_push(original,fl_int(99));
    assert(((FLVector*)original.obj)->len==5&&((FLVector*)appended.obj)->len==6);
    assert(((FLVector*)original.obj)->data[4].f==4);
    puts("published range remains immutable PASS");controls++;
    if(argc==2&&!strcmp(argv[1],"--large")){
        for(int turn=0;turn<3;turn++){
            FLValue values=fl_v11_builtin(fl_str_val("range"),parse("[16224]"));
            FLVector *v=(FLVector*)values.obj;assert(v->len==16224&&v->base.rc==1);
            for(uint32_t i=0;i<v->len;i++)assert(v->data[i].tag==FL_FLOAT&&v->data[i].f==(double)i);
        }
        puts("all 16224 values checked in three retained ranges PASS");controls++;
    }else assert(argc==1);
    printf("RANGE_CONTROLS=%d PASS=%d\n",controls,controls);return 0;
}
