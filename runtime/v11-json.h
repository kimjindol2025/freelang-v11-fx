/* C12 JSON data contracts. Legacy C entrypoints remain available to the seed. */
#define VJ_DEPTH_LIMIT 512
#define VJ_INPUT_LIMIT (64u*1024u*1024u)
typedef struct {TUnits input;size_t at;const char*error;} VJson;
static FLValue vj_bad(VJson*p,const char*why){if(!p->error)p->error=why;return fl_nil();}
static int vj_peek(VJson*p){return p->at<p->input.n?p->input.p[p->at]:-1;}
static void vj_ws(VJson*p){while(vj_peek(p)==32||vj_peek(p)==9||vj_peek(p)==10||vj_peek(p)==13)p->at++;}
static FLValue vj_value(VJson*,unsigned);
static FLValue vj_string(VJson*p){
    TUnits out={0};p->at++;int closed=0;
    while(p->at<p->input.n&&!p->error){
        uint16_t c=p->input.p[p->at++];
        if(c=='"'){closed=1;break;}
        if(c<32){vj_bad(p,"unescaped control character");break;}
        if(c=='\\'){
            if(p->at>=p->input.n){vj_bad(p,"unterminated escape");break;}
            c=p->input.p[p->at++];
            if(c=='u'){
                if(p->input.n-p->at<4){vj_bad(p,"incomplete unicode escape");break;}
                uint16_t unit=0;
                for(int i=0;i<4;i++){int digit=t_hex(p->input.p[p->at++]);if(digit<0){vj_bad(p,"invalid unicode escape");break;}unit=(uint16_t)((unit<<4)|digit);}
                if(p->error)break;c=unit;
            }else if(c=='b')c=8;else if(c=='f')c=12;else if(c=='n')c=10;else if(c=='r')c=13;else if(c=='t')c=9;
            else if(c!='"'&&c!='\\'&&c!='/'){vj_bad(p,"invalid escape");break;}
        }
        t_unit(&out,c);
    }
    if(!closed&&!p->error)vj_bad(p,"unterminated string");
    FLValue result=p->error?fl_nil():t_from_units(out.p,out.n);free(out.p);return result;
}
static int vj_digit(int c){return c>='0'&&c<='9';}
static FLValue vj_number(VJson*p){
    size_t start=p->at;
    if(vj_peek(p)=='-')p->at++;
    if(vj_peek(p)=='0'){p->at++;if(vj_digit(vj_peek(p)))return vj_bad(p,"leading zero");}
    else{if(!vj_digit(vj_peek(p)))return vj_bad(p,"number expected");while(vj_digit(vj_peek(p)))p->at++;}
    if(vj_peek(p)=='.'){p->at++;if(!vj_digit(vj_peek(p)))return vj_bad(p,"fraction digit expected");while(vj_digit(vj_peek(p)))p->at++;}
    if(vj_peek(p)=='e'||vj_peek(p)=='E'){
        p->at++;if(vj_peek(p)=='+'||vj_peek(p)=='-')p->at++;
        if(!vj_digit(vj_peek(p)))return vj_bad(p,"exponent digit expected");
        while(vj_digit(vj_peek(p)))p->at++;
    }
    return t_parse(t_from_units(p->input.p+start,p->at-start),0);
}
static int vj_word(VJson*p,const char*word){
    size_t n=strlen(word);if(p->input.n-p->at<n)return 0;
    for(size_t i=0;i<n;i++)if(p->input.p[p->at+i]!=(unsigned char)word[i])return 0;
    p->at+=n;return 1;
}
static FLValue vj_value(VJson*p,unsigned depth){
    if(depth>VJ_DEPTH_LIMIT)return vj_bad(p,"nesting limit exceeded");
    vj_ws(p);int c=vj_peek(p);
    if(c=='"')return vj_string(p);
    if(c=='['||c=='{'){
        int object=c=='{',closing=object?'}':']';p->at++;vj_ws(p);
        FLValue result=object?fl_map_new():fl_vec_builder_new();
        if(vj_peek(p)==closing){p->at++;return object?result:fl_vec_builder_freeze(result);}
        while(!p->error){
            FLValue key=fl_nil();
            if(object){
                if(vj_peek(p)!='"'){vj_bad(p,"quoted object key expected");break;}
                key=vj_string(p);if(p->error)break;vj_ws(p);
                if(vj_peek(p)!=':'){vj_bad(p,"colon expected");break;}p->at++;
            }
            FLValue value=vj_value(p,depth+1);if(p->error)break;
            if(object)result=fl_map_set(result,key,value);else fl_vec_builder_push(result,value);
            vj_ws(p);c=vj_peek(p);
            if(c==closing){p->at++;return object?result:fl_vec_builder_freeze(result);}
            if(c!=','){vj_bad(p,"comma or closing delimiter expected");break;}
            p->at++;vj_ws(p);
        }
        if(!object){FLVector*builder=(FLVector*)result.obj;free(builder->data);free(builder);}
        return fl_nil();
    }
    if(vj_word(p,"null"))return fl_nil();
    if(vj_word(p,"true"))return fl_bool(true);
    if(vj_word(p,"false"))return fl_bool(false);
    if(c=='-'||vj_digit(c))return vj_number(p);
    return vj_bad(p,"unexpected token or end of input");
}
static FLValue vj_parse(FLValue input){
    FLValue text=t_js_string(input);
    if(((FLString*)text.obj)->len>VJ_INPUT_LIMIT)fl_throw(fl_str_val("json_parse: invalid JSON: input limit exceeded"));
    VJson p={.input=t_units(text)};
    FLValue result=vj_value(&p,0);vj_ws(&p);
    if(!p.error&&p.at!=p.input.n)p.error="trailing input";
    free(p.input.p);
    if(p.error){char message[192];snprintf(message,sizeof(message),"json_parse: invalid JSON: %s at position %zu",p.error,p.at);fl_throw(fl_str_val(message));}
    return result;
}
static int vj_proto(FLValue key){return key.tag==FL_STRING&&((FLString*)key.obj)->len==9&&!memcmp(((FLString*)key.obj)->data,"__proto__",9);}
static int vj_valid(FLValue value,unsigned depth,int keep_proto){
    if(depth>VJ_DEPTH_LIMIT||value.tag==FL_FN)return 0;
    if(value.tag==FL_VECTOR){FLVector*v=(FLVector*)value.obj;for(uint32_t i=0;i<v->len;i++)if(!vj_valid(v->data[i],depth+1,keep_proto))return 0;}
    if(value.tag==FL_MAP){FLMap*m=(FLMap*)value.obj;for(uint32_t i=0;i<m->len;i++)if((keep_proto||!vj_proto(m->entries[i].key))&&!vj_valid(m->entries[i].val,depth+1,keep_proto))return 0;}
    return 1;
}
static void vj_emit(TBuffer*b,FLValue value,int keep_proto){
    if(value.tag==FL_VECTOR){
        FLVector*v=(FLVector*)value.obj;t_ascii(b,"[");
        for(uint32_t i=0;i<v->len;i++){if(i)t_ascii(b,",");vj_emit(b,v->data[i],keep_proto);}t_ascii(b,"]");
    }else if(value.tag==FL_MAP){
        FLVector*keys=(FLVector*)t_keys(value).obj;int written=0;t_ascii(b,"{");
        for(uint32_t i=0;i<keys->len;i++){
            FLValue key=keys->data[i];if(!keep_proto&&vj_proto(key))continue;
            if(written++)t_ascii(b,",");t_quoted(b,t_key(key),1);t_ascii(b,":");vj_emit(b,fl_map_get(value,key),keep_proto);
        }
        t_ascii(b,"}");
    }else t_render(b,value,1,0);
}
static FLValue vj_stringify(FLValue value){
    if(!vj_valid(value,0,0))fl_throw(fl_str_val("json_str: unsupported value or nesting limit exceeded"));
    TBuffer out={0};vj_emit(&out,value,0);return t_finish(&out);
}
/* HTTP JSON.stringify preserves own keys, unlike v11 json_str/toSerializable. */
FLValue fl_v11_json_encode(FLValue value){
    if(!vj_valid(value,0,1))fl_throw(fl_str_val("JSON response: unsupported value or nesting limit exceeded"));
    TBuffer out={0};vj_emit(&out,value,1);return t_finish(&out);
}
FLValue fl_v11_json_try_parse(FLValue value){
    FLValue failed[2]={fl_bool(false),fl_nil()};
    if(fl_try_top>=FL_TRY_MAX)return fl_vec_from(failed,2);
    FLTryFrame*frame=&fl_try_stack[fl_try_top++];
    if(setjmp(frame->buf)==0){
        FLValue result=vj_parse(value);fl_try_top--;
        FLValue out[2]={fl_bool(true),result};return fl_vec_from(out,2);
    }
    fl_try_top--;return fl_vec_from(failed,2);
}
