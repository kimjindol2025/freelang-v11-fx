/* v11 evalThrow / evalTryBlock contracts, using the existing C regex engine. */
static FLValue ve_error(FLValue value){
    if(value.tag==FL_MAP&&value.obj->type==FL_NATIVE_ERROR)return value;
    FLValue message=value;
    if(value.tag==FL_MAP){
        FLValue supplied=fl_map_get(value,fl_str_val("message"));
        if(fl_v11_truthy(supplied))message=supplied;
    }
    FLValue out=fl_map_new();
    out=fl_map_set(out,fl_str_val("message"),t_js_string(message));
    out=fl_map_set(out,fl_str_val("type"),fl_str_val("RuntimeError"));
    out.obj->type=FL_NATIVE_ERROR;
    return out;
}
void fl_v11_throw(FLValue value){fl_throw(ve_error(value));}
static __thread FLCallScope*ve_context;
static __thread int ve_tco_mode;
extern __thread int __fl_callstack_depth;
static void ve_scope_enter(FLCallScope*scope){
    *scope=(FLCallScope){.previous=ve_context,.name=fl_nil(),.try_depth=fl_try_top,
        .call_depth=__fl_callstack_depth,.saved_tco=ve_tco_mode,.borrow_mark=fl_borrow_mark()};
    ve_context=scope;
}
void fl_v11_function_enter(FLCallScope*scope,int global){ve_scope_enter(scope);if(global)ve_tco_mode=1;}
void fl_v11_value_enter(FLCallScope*scope){ve_scope_enter(scope);ve_tco_mode=0;}
void fl_v11_scope_leave(FLCallScope*scope){ve_context=scope->previous;ve_tco_mode=scope->saved_tco;}
int fl_v11_tco_active(void){return ve_tco_mode;}
FLValue fl_v11_tail_call(FLValue fn,FLValue args){
    FLValue out=fl_map_set(fl_map_set(fl_map_new(),fl_str_val("fn"),fn),fl_str_val("args"),args);
    out.obj->type=FL_NATIVE_TAIL;return out;
}

FLValue fl_v11_context_call(FLValue name,FLValue fn,FLValue args,FLValue line){
    FLCallScope context;ve_scope_enter(&context);context.name=name;
    context.line=line.tag==FL_INT?(int)line.i:0;
    FLVector*values=(FLVector*)args.obj;
    FLValue result=fn.tag==FL_NIL?fl_v11_builtin(name,args):fl_fn_call(fn,(int)values->len,values->data);
    fl_v11_scope_leave(&context);
    return result;
}

/* Unwind only calls outside the selected catch; arguments ran before entry. */
FLValue fl_v11_context_error(FLValue error){
    while(ve_context&&ve_context->try_depth>=fl_try_top){
        FLCallScope context=*ve_context;ve_context=context.previous;ve_tco_mode=context.saved_tco;
        error=fl_borrow_finish(context.borrow_mark,error);
        __fl_callstack_depth=context.call_depth;
        if(context.name.tag!=FL_STRING)continue;
        FLValue out=ve_error(error),message=fl_map_get(out,fl_str_val("message"));
        FLString*name=(FLString*)context.name.obj;
        FLString*text=(FLString*)t_js_string(message).obj;
        int prefixed=text->len>=name->len+2&&
            ((text->data[0]=='['&&text->data[name->len+1]==']')||
             (text->data[0]=='('&&text->data[name->len+1]==')'))&&
            !memcmp(text->data+1,name->data,name->len);
        if(!prefixed){
            FLValue parts[4]={fl_str_val("["),context.name,fl_str_val("] "),message};
            message=fl_str_n(4,parts[0],parts[1],parts[2],parts[3]);
        }
        if(!strstr(((FLString*)message.obj)->data,"(at line ")){
            char location[80];snprintf(location,sizeof location," (at line %d, col 0)",context.line);
            message=fl_str_n(2,message,fl_str_val(location));
        }
        error=fl_map_set(out,fl_str_val("message"),message);
        error.obj->type=FL_NATIVE_ERROR;
    }
    return error;
}
static FLValue ve_regex(const char*op,FLValue input,const char*pattern,const char*replacement){
    FLValue args[3]={input,fl_str_val(pattern),fl_str_val(replacement)};
    return v11_regex(op,fl_vec_from(args,3));
}
FLValue fl_v11_catch(FLValue raw){
    if(raw.tag!=FL_MAP)raw=ve_error(raw);
    FLValue out=fl_map_new(),message=fl_map_get(raw,fl_str_val("message"));
    FLValue type=fl_map_get(raw,fl_str_val("type"));
    if(type.tag==FL_NIL)type=fl_str_val("RuntimeError");
    if(message.tag==FL_NIL)message=t_js_string(raw);
    if(raw.obj->type==FL_NATIVE_ERROR){
        out=raw;
        FLValue original=t_js_string(message);
        message=ve_regex("regex_replace_first",original,"^FreeLang line [0-9]+:\\s*","");
        if(fl_v11_truthy(ve_regex("regex_match",message,"^\\[[^\\]]+\\]",""))){
            message=ve_regex("regex_replace_first",message,"^\\[[^\\]]+\\]\\s*((?:.|\\s)*?)(?:\\s*\\(at line [0-9]+(?:.|\\s)*)?$","$1");
            FLValue trim[1]={message};
            message=fl_v11_builtin(fl_str_val("str-trim"),fl_vec_from(trim,1));
        }else message=ve_regex("regex_replace_first",message,"^([^\\n\\r\\u2028\\u2029]*?)\\s*\\(at line [0-9]+(?:.|\\s)*$","$1");
        if(!((FLString*)message.obj)->len)message=original;
        FLValue location=ve_regex("regex_find",original,"\\(at line [0-9]+","");
        if(location.tag==FL_STRING){
            FLString*s=(FLString*)location.obj;
            out=fl_map_set(out,fl_str_val("line"),t_parse(fl_str_val(s->data+9),1));
        }
    }else{
        FLValue stack=fl_map_get(raw,fl_str_val("stack"));
        if(fl_v11_truthy(stack))out=fl_map_set(out,fl_str_val("stack"),stack);
    }
    out=fl_map_set(out,fl_str_val("message"),message);
    out=fl_map_set(out,fl_str_val("type"),type);
    return fl_map_set(out,fl_str_val("raw"),raw);
}
