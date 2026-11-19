/* Browser AST printer. Runtime algorithms live in embedded FreeLang source.
 * Only fixed grammar tokens and whitelisted primitive host operations are emitted. */
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include "browser-core-data.h"
#include "browser-island-core-data.h"
typedef struct {char *code;size_t len,cap;unsigned serial;FLValue names;} BJ;
typedef struct BJLoop {FLValue names;struct BJLoop *parent;} BJLoop;
static void bj_fail(BJ*b,const char*reason){
 char message[256];snprintf(message,sizeof message,"browser target: %s",reason);
 free(b->code);b->code=NULL;fl_throw(fl_str_val(message));
}
static void bj_out(BJ*b,const char*s){
 size_t n=strlen(s);
 if(n>FL_FILE_IO_MAX_BYTES-b->len)bj_fail(b,"output size limit");
 if(b->len+n+1>b->cap){size_t cap=b->cap?b->cap:4096;while(cap<b->len+n+1)cap*=2;
  char *p=realloc(b->code,cap);if(!p)bj_fail(b,"allocation failed");b->code=p;b->cap=cap;}
 memcpy(b->code+b->len,s,n+1);b->len+=n;
}
static void bj_num(BJ*b,unsigned n){char text[32];snprintf(text,sizeof text,"%u",n);bj_out(b,text);}
static const char *bj_text(FLValue v){return v.tag==FL_STRING?((FLString*)v.obj)->data:"";}
static FLValue bj_at(FLValue v,unsigned n){return v.tag==FL_VECTOR?fl_vec_get(v,fl_int(n)):fl_nil();}
static unsigned bj_len(FLValue v){return v.tag==FL_VECTOR?((FLVector*)v.obj)->len:0;}
static int bj_is(FLValue v,const char*s){return rb_is(v,s);}
static FLValue bj_items(FLValue n){return rb_items(n);}
static int bj_prefix(const char*s,const char*p){return !strncmp(s,p,strlen(p));}
static void bj_op(FLValue value,char out[128]){
 const char*s=bj_text(value);size_t i=0;
 for(;s[i]&&i<127;i++)out[i]=s[i]=='_'?'-':s[i];out[i]=0;
}
static int bj_reserved(FLValue name){
 const char*s=bj_text(name);char normalized[128];bj_op(name,normalized);
 return bj_prefix(s,"__fl")||bj_prefix(normalized,"jcore-")||bj_prefix(normalized,"j-host-")||
  bj_prefix(normalized,"iscore-")||bj_prefix(normalized,"i-host-")||bj_prefix(normalized,"island-core-")||
  bj_prefix(normalized,"fl-compiler-")||!strcmp(normalized,"fl-register-named-fn");
}
static void bj_validate_user(BJ*b,FLValue n,unsigned depth){
 if(depth>512)bj_fail(b,"input nesting limit");
 if(n.tag==FL_VECTOR){FLVector*v=(FLVector*)n.obj;for(unsigned i=0;i<v->len;i++)bj_validate_user(b,v->data[i],depth+1);return;}
 if(n.tag!=FL_MAP)return;
 FLValue kind=rb_get(n,"kind");
 if((bj_is(kind,"sexpr")&&bj_reserved(rb_get(n,"op")))||
    (bj_is(kind,"variable")&&bj_reserved(rb_get(n,"name")))||
    (bj_is(kind,"literal")&&bj_is(rb_get(n,"type"),"symbol")&&bj_reserved(rb_get(n,"value"))))
  bj_fail(b,"compiler-private name in user module");
 FLMap*m=(FLMap*)n.obj;for(unsigned i=0;i<m->len;i++)bj_validate_user(b,m->entries[i].val,depth+1);
}
static void bj_quote(BJ*b,FLValue value){
 FLValue a[1]={value};FLValue encoded=fl_v11_builtin(fl_str_val("json-str"),fl_vec_from(a,1));
 if(encoded.tag!=FL_STRING)bj_fail(b,"invalid string literal");bj_out(b,bj_text(encoded));
}
static void bj_ident(BJ*b,FLValue name){
 const char*s=bj_text(name),*p=s;
 if(!bj_prefix(p,"__fl_binding_"))bj_fail(b,"unresolved binding");p+=13;
 if(!*p)bj_fail(b,"invalid binding");
 for(;*p;p++)if(!isdigit((unsigned char)*p))bj_fail(b,"invalid binding");
 bj_out(b,s);
}
static const char *bj_builtin(const char*op){
 static const char *const pairs[]={
  "island-core-mount","iscore-mount","island-core-h","iscore-h",
  "island-core-atom","iscore-atom","island-core-reset","iscore-reset","island-core-swap","iscore-swap",
  "island-core-fetch","iscore-fetch","island-core-fetch-post","iscore-fetch-post",
  "island-core-by-id","iscore-by-id","island-core-node-value","iscore-node-value","island-core-node-text","iscore-node-text",
  "island-core-node-attribute","iscore-node-attribute","island-core-json-parse","iscore-json-parse",
  "island-core-form-reset","iscore-form-reset","island-core-form-submit","iscore-form-submit",
  "island-core-number","iscore-number","island-core-str-trim","iscore-str-trim","island-core-includes","iscore-includes",
  "island-core-regex-test","iscore-regex-test","island-core-random-uuid","iscore-random-uuid","island-core-navigate","iscore-navigate",
  "island-core-assoc","iscore-assoc","island-core-push","iscore-push","island-core-first","iscore-first",
  "island-core-not","iscore-not","island-core-greater","iscore-greater","island-core-less-equal","iscore-less-equal","island-core-greater-equal","iscore-greater-equal",
  "+","jcore-add","-","jcore-sub","*","jcore-mul","/","jcore-div",
  "=","jcore-equal","eq","jcore-equal","<","jcore-less",
  "str","jcore-str","html-escape","jcore-html-escape","get","jcore-get","length","jcore-length","count","jcore-length",
  "substring","jcore-substring","nil?","jcore-nil","null?","jcore-nil","string?","jcore-string?",
  "atom","jcore-atom","deref","jcore-deref","reset!","jcore-reset","swap!","jcore-swap",
  "append","jcore-append","map","jcore-map",
  "browser-by-id","jcore-by-id","browser-set-text!","jcore-set-text",
  "browser-text","jcore-text","browser-value","jcore-value","browser-set-value!","jcore-set-value",
  "browser-on!","jcore-on","browser-off!","jcore-off","browser-event-target","jcore-event-target",
  "browser-event-key","jcore-event-key","browser-prevent-default!","jcore-prevent-default",
  "browser-create","jcore-create","browser-append!","jcore-dom-append",
  "browser-insert-before!","jcore-dom-before","browser-remove!","jcore-dom-remove",
  "browser-clear!","jcore-dom-clear","browser-children","jcore-dom-children",
  "browser-attribute","jcore-dom-attribute","browser-set-attribute!","jcore-dom-set-attribute",
  "browser-set-hidden!","jcore-dom-hidden","browser-focus!","jcore-dom-focus",
  "browser-selection-read","jcore-selection-read","browser-selection-set!","jcore-selection-set",
  "browser-after!","jcore-after","browser-cancel!","jcore-cancel",
  "browser-http!","jcore-http","browser-json-parse","jcore-json-parse",
  "browser-storage-read","jcore-storage-read","browser-storage-write!","jcore-storage-write",
  "browser-storage-remove!","jcore-storage-remove",NULL};
 for(unsigned i=0;pairs[i];i+=2)if(!strcmp(op,pairs[i]))return pairs[i+1];return NULL;
}
static void bj_core_ref(BJ*b,const char*name){
 FLValue target=fl_map_get(b->names,fl_str_val(name));if(target.tag==FL_NIL)bj_fail(b,"missing FreeLang support binding");bj_ident(b,target);
}
static void bj_expr(BJ*,FLValue);
static void bj_tail(BJ*,FLValue,BJLoop*);
static void bj_args(BJ*b,FLValue args,unsigned start){
 for(unsigned i=start;i<bj_len(args);i++){if(i>start)bj_out(b,",");bj_expr(b,bj_at(args,i));}
}
static void bj_array(BJ*b,FLValue args){bj_out(b,"[");bj_args(b,args,0);bj_out(b,"]");}
static FLValue bj_body(FLValue args,unsigned start){
 FLValue body=fl_vec_new();for(unsigned i=start;i<bj_len(args);i++)body=fl_vec_push(body,bj_at(args,i));return rb_call("do",body);
}
/* Only the seven commerce Island dialects use this compatibility lowering.
 * Validate the original module first, so compiler-private primitives cannot be
 * smuggled into user input. The ordinary browser-* target is unchanged. */
static FLValue bi_lower(BJ*,FLValue,int,unsigned*);
static FLValue bi_literal(const char*value){
 FLValue n=rb_set(rb_set(fl_map_new(),"kind",fl_str_val("literal")),"type",fl_str_val("string"));
 return rb_set(n,"value",fl_str_val(value));
}
static FLValue bi_array(FLValue items){
 return rb_with_items(rb_set(rb_set(fl_map_new(),"kind",fl_str_val("block")),"type",fl_str_val("Array")),items);
}
static FLValue bi_fn(FLValue body){
 FLValue a[2]={bi_array(fl_vec_new()),body};return rb_call("fn",fl_vec_from(a,2));
}
static int bi_symbol(FLValue node,const char*name){return bj_is(rb_name(node),name);}
static int bi_string(FLValue node){return bj_is(rb_get(node,"kind"),"literal")&&bj_is(rb_get(node,"type"),"string");}
static FLValue bi_call(BJ*b,const char*name,FLValue args,unsigned skip,unsigned*serial){
 FLValue out=fl_vec_new();for(unsigned i=skip;i<bj_len(args);i++)out=fl_vec_push(out,bi_lower(b,bj_at(args,i),1,serial));
 return rb_call(name,out);
}
static FLValue bi_lower(BJ*b,FLValue node,int inside,unsigned*serial){
 if(node.tag==FL_VECTOR){FLValue out=fl_vec_new();for(unsigned i=0;i<bj_len(node);i++)out=fl_vec_push(out,bi_lower(b,bj_at(node,i),inside,serial));return out;}
 if(node.tag!=FL_MAP)return node;
 if(bj_is(rb_get(node,"kind"),"sexpr")){
  const char*op=bj_text(rb_get(node,"op"));FLValue args=rb_get(node,"args");unsigned n=bj_len(args);
  if(!strcmp(op,"island")){
   if(inside||n!=3)bj_fail(b,"Island requires a name, empty options and one outer let");
   const char*name=bj_text(rb_name(bj_at(args,0)));size_t length=strlen(name);
   if(!length||length>64)bj_fail(b,"invalid Island name");
   char id[96]="island-";size_t used=7;
   for(size_t i=0;i<length;i++){unsigned char c=(unsigned char)name[i];
    if(!((c>='A'&&c<='Z')||(c>='a'&&c<='z')||(c>='0'&&c<='9')||c=='-'||c=='_'))bj_fail(b,"invalid Island name");
    id[used++]=(char)tolower(c);
   }
   snprintf(id+used,sizeof(id)-used,"-%u",(*serial)++);
   FLValue options=bj_at(args,1),body=bj_at(args,2);
   if(!bj_is(rb_get(options,"kind"),"block")||!bj_is(rb_get(options,"type"),"Array")||bj_len(bj_items(options)))bj_fail(b,"Island options currently require []");
   if(!bj_is(rb_get(body,"kind"),"sexpr")||!bj_is(rb_get(body,"op"),"let"))bj_fail(b,"Island requires an outer let");
   FLValue bindings=rb_get(body,"args");if(bj_len(bindings)<2)bj_fail(b,"Island let body is empty");
   FLValue render=fl_vec_new();for(unsigned i=1;i<bj_len(bindings);i++)render=fl_vec_push(render,bi_lower(b,bj_at(bindings,i),1,serial));
   FLValue setup[2]={bi_lower(b,bj_at(bindings,0),1,serial),bi_fn(rb_call("do",render))};
   FLValue mount[2]={bi_literal(id),bi_fn(rb_call("let",fl_vec_from(setup,2)))};
   return rb_call("island-core-mount",fl_vec_from(mount,2));
  }
  if(inside){
   static const char*renames[]={
    "h","island-core-h","atom","island-core-atom","reset!","island-core-reset","swap!","island-core-swap",
    "fetch!","island-core-fetch","fetch-post!","island-core-fetch-post","assoc","island-core-assoc","push","island-core-push",
    "first","island-core-first","not","island-core-not",">","island-core-greater","<=","island-core-less-equal",">=","island-core-greater-equal",
    "Number","island-core-number","str-trim","island-core-str-trim",NULL};
   for(unsigned i=0;renames[i];i+=2)if(!strcmp(op,renames[i]))return bi_call(b,renames[i+1],args,0,serial);
   if(!strcmp(op,".getElementById")){
    if(n!=2||!bi_symbol(bj_at(args,0),"document"))bj_fail(b,"only document.getElementById is allowed");
    return bi_call(b,"island-core-by-id",args,1,serial);
   }
   if(!strcmp(op,".parse")){
    if(n!=2||!bi_symbol(bj_at(args,0),"JSON"))bj_fail(b,"only JSON.parse is allowed");
    return bi_call(b,"island-core-json-parse",args,1,serial);
   }
   if(!strcmp(op,".randomUUID")){
    if(n!=1||!bi_symbol(bj_at(args,0),"crypto"))bj_fail(b,"only crypto.randomUUID is allowed");
    return rb_call("island-core-random-uuid",fl_vec_new());
   }
   if(!strcmp(op,".assign")){
    FLValue location=bj_at(args,0),la=rb_get(location,"args");
    if(n!=2||!bj_is(rb_get(location,"op"),".-location")||bj_len(la)!=1||!bi_symbol(bj_at(la,0),"window"))bj_fail(b,"only window.location.assign is allowed");
    return bi_call(b,"island-core-navigate",args,1,serial);
   }
   if(!strcmp(op,".test")){
    FLValue expression=bj_at(args,0),ra=rb_get(expression,"args");
    if(n!=2||!bj_is(rb_get(expression,"op"),"RegExp")||bj_len(ra)!=1||!bi_string(bj_at(ra,0)))bj_fail(b,"Island RegExp requires an approved literal pattern");
    const char*pattern=bj_text(rb_get(bj_at(ra,0),"value"));
    if(strcmp(pattern,"^/public/[A-Za-z0-9._/-]+$")&&strcmp(pattern,"^01[0-9]-?[0-9]{3,4}-?[0-9]{4}$"))bj_fail(b,"Island RegExp pattern is not approved");
    FLValue a[2]={bj_at(ra,0),bi_lower(b,bj_at(args,1),1,serial)};return rb_call("island-core-regex-test",fl_vec_from(a,2));
   }
   static const char*interop[]={".-value","island-core-node-value","1",".-textContent","island-core-node-text","1",
    ".getAttribute","island-core-node-attribute","2",".reset","island-core-form-reset","1",".submit","island-core-form-submit","1",
    ".includes","island-core-includes","2",NULL};
   for(unsigned i=0;interop[i];i+=3)if(!strcmp(op,interop[i])){if(n!=(unsigned)atoi(interop[i+2]))bj_fail(b,"Island interop arity");return bi_call(b,interop[i+1],args,0,serial);}
   if(op[0]=='.'||!strcmp(op,"RegExp"))bj_fail(b,"unsupported Island interop");
  }
 }
 FLMap*map=(FLMap*)node.obj;FLValue out=node;
 for(unsigned i=0;i<map->len;i++){
  FLValue value=map->entries[i].val;
  if(value.tag==FL_MAP||value.tag==FL_VECTOR)value=bi_lower(b,value,inside,serial);
  out=fl_map_set(out,map->entries[i].key,value);
 }
 /* The existing Island parser pairs a trailing odd map key with undefined,
  * which its emitter represents as null. Preserve that Island-only behavior. */
 if(inside&&bj_is(rb_get(out,"kind"),"block")&&bj_is(rb_get(out,"type"),"Map")&&(bj_len(bj_items(out))%2)){
  FLValue nil=rb_set(rb_set(fl_map_new(),"kind",fl_str_val("literal")),"type",fl_str_val("nil"));
  out=rb_with_items(out,fl_vec_push(bj_items(out),nil));
 }
 return out;
}
static void bj_bindings(BJ*b,FLValue block,FLValue *names){
 if(!bj_is(rb_get(block,"kind"),"block")||!bj_is(rb_get(block,"type"),"Array"))bj_fail(b,"binding vector required");
 FLValue items=bj_items(block);unsigned length=bj_len(items);int nested=length&&bj_is(rb_get(bj_at(items,0),"kind"),"block");
 if(!nested&&length%2)bj_fail(b,"odd binding list");
 for(unsigned i=0;i<length;i+=nested?1:2){FLValue pair=nested?bj_items(bj_at(items,i)):items;unsigned at=nested?0:i;
  if(nested&&bj_len(pair)!=2)bj_fail(b,"invalid binding pair");
  FLValue name=rb_name(bj_at(pair,at));bj_out(b,"let ");bj_ident(b,name);bj_out(b,"=");bj_expr(b,bj_at(pair,at+1));bj_out(b,";\n");
  if(names)*names=fl_vec_push(*names,name);
 }
}
static void bj_function(BJ*b,FLValue args,unsigned offset){
 FLValue block=bj_at(args,offset);
 if(!bj_is(rb_get(block,"kind"),"block")||!bj_is(rb_get(block,"type"),"Array"))bj_fail(b,"function parameter vector required");
 FLValue parameters=bj_items(bj_at(args,offset));bj_out(b,"function(");
 for(unsigned i=0;i<bj_len(parameters);i++){if(i)bj_out(b,",");bj_ident(b,rb_name(bj_at(parameters,i)));bj_out(b,"=null");}
 bj_out(b,"){\nreturn ");bj_expr(b,bj_body(args,offset+1));bj_out(b,";\n}");
}
static void bj_try(BJ*b,FLValue n,BJLoop*loop){
 bj_out(b,"try{\n");bj_tail(b,rb_get(n,"body"),loop);bj_out(b,"}\n");
 FLValue clause=rb_get(n,"catch"),last=rb_get(n,"finally");
 if(clause.tag!=FL_NIL){FLValue args=rb_get(clause,"args");bj_out(b,"catch(");bj_ident(b,rb_name(bj_at(args,0)));bj_out(b,"){\n");bj_tail(b,bj_body(args,1),loop);bj_out(b,"}\n");}
 if(last.tag!=FL_NIL){bj_out(b,"finally{\n");FLValue args=rb_get(last,"args");for(unsigned i=0;i<bj_len(args);i++){bj_expr(b,bj_at(args,i));bj_out(b,";\n");}bj_out(b,"}\n");}
 if(clause.tag==FL_NIL&&last.tag==FL_NIL)bj_fail(b,"try requires catch or finally");
}
static void bj_logic(BJ*b,FLValue args,unsigned index,int is_or){
 if(index>=bj_len(args)){bj_out(b,is_or?"null":"true");return;}
 if(index+1==bj_len(args)){bj_expr(b,bj_at(args,index));return;}
 if(!is_or){bj_out(b,"(");bj_expr(b,bj_at(args,index));bj_out(b,"&&");bj_logic(b,args,index+1,0);bj_out(b,")");return;}
 unsigned id=b->serial++;bj_out(b,"((__j_value_");bj_num(b,id);bj_out(b,")=>(__j_value_");bj_num(b,id);bj_out(b,"===null||__j_value_");bj_num(b,id);bj_out(b,"===false)?");
 bj_logic(b,args,index+1,1);bj_out(b,":__j_value_");bj_num(b,id);bj_out(b,")(");bj_expr(b,bj_at(args,index));bj_out(b,")");
}
static int bj_host(BJ*b,const char*op,FLValue args){
 unsigned n=bj_len(args);const char *symbol=NULL;
 if(!strcmp(op,"i-host-node?")||!strcmp(op,"i-host-form?")){
  if(n!=1)bj_fail(b,"Island host type arity");bj_out(b,"(");bj_expr(b,bj_at(args,0));
  bj_out(b,!strcmp(op,"i-host-node?")?" instanceof Node)":" instanceof HTMLFormElement)");return 1;
 }
 if(!strcmp(op,"i-host-random-uuid")){if(n)bj_fail(b,"Island UUID arity");bj_out(b,"globalThis.crypto.randomUUID()");return 1;}
 if(!strcmp(op,"i-host-own-set!")){
  if(n!=3)bj_fail(b,"Island own property arity");bj_out(b,"Object.defineProperty(");bj_expr(b,bj_at(args,0));
  bj_out(b,",");bj_expr(b,bj_at(args,1));bj_out(b,",{value:");bj_expr(b,bj_at(args,2));
  bj_out(b,",writable:true,enumerable:true,configurable:true})");return 1;
 }
 if(!strcmp(op,"i-host-regex-test")){
  if(n!=2)bj_fail(b,"Island regex arity");bj_out(b,"(new RegExp(");bj_expr(b,bj_at(args,0));bj_out(b,")).test(");bj_expr(b,bj_at(args,1));bj_out(b,")");return 1;
 }
 const char*icalls[]={"i-host-text-new","document.createTextNode","1","i-host-json-stringify","JSON.stringify","1",
  "i-host-number","Number","1","i-host-navigate","globalThis.location.assign","1",NULL};
 for(unsigned i=0;icalls[i];i+=3)if(!strcmp(op,icalls[i])){
  if(n!=(unsigned)atoi(icalls[i+2]))bj_fail(b,"Island host call arity");
  bj_out(b,icalls[i+1]);bj_out(b,"(");bj_args(b,args,0);bj_out(b,")");return 1;
 }
 const char*imethods[]={"i-host-trim","trim","0","i-host-lower","toLowerCase","0","i-host-includes","includes","1",NULL};
 for(unsigned i=0;imethods[i];i+=3)if(!strcmp(op,imethods[i])){
  if(n!=(unsigned)atoi(imethods[i+2])+1)bj_fail(b,"Island host method arity");
  bj_out(b,"(");bj_expr(b,bj_at(args,0));bj_out(b,").");bj_out(b,imethods[i+1]);bj_out(b,"(");bj_args(b,args,1);bj_out(b,")");return 1;
 }
 if(!strcmp(op,"i-host-form-reset")||!strcmp(op,"i-host-form-submit")){
  if(n!=1)bj_fail(b,"Island form arity");
  bj_out(b,!strcmp(op,"i-host-form-reset")?"HTMLFormElement.prototype.reset.call(":"HTMLFormElement.prototype.submit.call(");
  bj_expr(b,bj_at(args,0));bj_out(b,")");return 1;
 }
 const char *binary[]={"j-host-eq","===","j-host-lt","<","j-host-gte",">=","j-host-add","+","j-host-sub","-","j-host-mul","*","j-host-div","/","j-host-and","&&","j-host-or","||",NULL};
 for(unsigned i=0;binary[i];i+=2)if(!strcmp(op,binary[i]))symbol=binary[i+1];
 if(symbol){if(n!=2)bj_fail(b,"host binary arity");bj_out(b,"(");bj_expr(b,bj_at(args,0));bj_out(b,symbol);bj_expr(b,bj_at(args,1));bj_out(b,")");return 1;}
 const char *unary=!strcmp(op,"j-host-not")?"!":!strcmp(op,"j-host-neg")?"-":!strcmp(op,"j-host-type")?"typeof ":NULL;
 if(unary){if(n!=1)bj_fail(b,"host unary arity");bj_out(b,"(");bj_out(b,unary);bj_expr(b,bj_at(args,0));bj_out(b,")");return 1;}
 if(!strcmp(op,"j-host-nullish?")){if(n!=1)bj_fail(b,"host nullish arity");bj_out(b,"(");bj_expr(b,bj_at(args,0));bj_out(b,"==null)");return 1;}
 if(!strcmp(op,"j-host-at")||!strcmp(op,"j-host-set!")){
  int set=!strcmp(op,"j-host-set!");if(n!=(set?3u:2u))bj_fail(b,"host access arity");
  bj_out(b,"((");bj_expr(b,bj_at(args,0));bj_out(b,")[");bj_expr(b,bj_at(args,1));bj_out(b,"]");if(set){bj_out(b,"=");bj_expr(b,bj_at(args,2));}bj_out(b,")");return 1;
 }
 if(!strcmp(op,"j-host-len")||!strcmp(op,"j-host-dom-set-text!")){
  int set=!strcmp(op,"j-host-dom-set-text!");if(n!=(set?2u:1u))bj_fail(b,"host property arity");
  bj_out(b,"((");bj_expr(b,bj_at(args,0));bj_out(b,set?").textContent=":").length");if(set)bj_expr(b,bj_at(args,1));bj_out(b,")");return 1;
 }
 if(!strcmp(op,"j-host-element?")){if(n!=1)bj_fail(b,"host element arity");bj_out(b,"(");bj_expr(b,bj_at(args,0));bj_out(b," instanceof Element)");return 1;}
 if(!strcmp(op,"j-host-input?")){
  if(n!=1)bj_fail(b,"host input arity");
  bj_out(b,"((");bj_expr(b,bj_at(args,0));bj_out(b," instanceof HTMLInputElement)||(");
  bj_expr(b,bj_at(args,0));bj_out(b," instanceof HTMLTextAreaElement))");return 1;
 }
 if(!strcmp(op,"j-host-event?")||!strcmp(op,"j-host-keyboard?")){
  if(n!=1)bj_fail(b,"host event arity");bj_out(b,"(");bj_expr(b,bj_at(args,0));
  bj_out(b,!strcmp(op,"j-host-event?")?" instanceof Event)":" instanceof KeyboardEvent)");return 1;
 }
 if(!strcmp(op,"j-host-weak-new")){if(n)bj_fail(b,"host weak constructor arity");bj_out(b,"new WeakSet()");return 1;}
 if(!strcmp(op,"j-host-weak-map-new")){if(n)bj_fail(b,"host weak map constructor arity");bj_out(b,"new WeakMap()");return 1;}
 if(!strcmp(op,"j-host-plain?")){
  if(n!=1)bj_fail(b,"host plain object arity");
  bj_out(b,"(Object.getPrototypeOf(");bj_expr(b,bj_at(args,0));bj_out(b,")===Object.prototype)");return 1;
 }
 if(!strcmp(op,"j-host-url")){
  if(n!=1)bj_fail(b,"host URL arity");bj_out(b,"new URL(");bj_expr(b,bj_at(args,0));bj_out(b,",document.baseURI)");return 1;
 }
 if(!strcmp(op,"j-host-origin")){if(n)bj_fail(b,"host origin arity");bj_out(b,"location.origin");return 1;}
 if(!strcmp(op,"j-host-timer-set")||!strcmp(op,"j-host-timer-clear")){
  int set=!strcmp(op,"j-host-timer-set");if(n!=(set?2u:1u))bj_fail(b,"host timer arity");
  bj_out(b,set?"globalThis.setTimeout(":"globalThis.clearTimeout(");bj_args(b,args,0);bj_out(b,")");return 1;
 }
 if(!strcmp(op,"j-host-abort-new")){if(n)bj_fail(b,"host abort constructor arity");bj_out(b,"new AbortController()");return 1;}
 if(!strcmp(op,"j-host-storage")){if(n)bj_fail(b,"host storage accessor arity");bj_out(b,"globalThis.localStorage");return 1;}
 if(!strcmp(op,"j-host-text-walker")){if(n!=1)bj_fail(b,"host text walker arity");bj_out(b,"document.createTreeWalker(");bj_expr(b,bj_at(args,0));bj_out(b,",4)");return 1;}
 const char *call=NULL;unsigned arity=0;
 const char *calls[]={"j-host-selection","globalThis.getSelection","0","j-host-range","document.createRange","0","j-host-fetch","globalThis.fetch","2","j-host-json-parse","JSON.parse","1","j-host-array?","Array.isArray","1","j-host-own?","Object.hasOwn","2","j-host-keys","Object.keys","1","j-host-integer?","Number.isInteger","1","j-host-string","String","1","j-host-dom-by-id","document.getElementById","1","j-host-dom-create","document.createElement","1","j-host-array-from","Array.from","1",NULL};
 for(unsigned i=0;calls[i];i+=3)if(!strcmp(op,calls[i])){call=calls[i+1];arity=(unsigned)atoi(calls[i+2]);}
 if(call){if(n!=arity)bj_fail(b,"host call arity");bj_out(b,call);bj_out(b,"(");bj_args(b,args,0);bj_out(b,")");return 1;}
 const char *method=NULL;unsigned argc=0;
 const char *methods[]={
  "j-host-dom-contains","contains","1","j-host-select-range!","setSelectionRange","3","j-host-range-select!","selectNodeContents","1","j-host-range-end!","setEnd","2","j-host-range-text","toString","0","j-host-selection-range","getRangeAt","1","j-host-selection-set!","setBaseAndExtent","4","j-host-walker-next","nextNode","0",
  "j-host-storage-read","getItem","1","j-host-storage-write!","setItem","2","j-host-storage-remove!","removeItem","1",
  "j-host-then","then","2","j-host-response-text","text","0","j-host-abort","abort","0","j-host-weak-has","has","1","j-host-weak-add","add","1","j-host-weak-get","get","1","j-host-weak-set!","set","2","j-host-copy","slice","0","j-host-slice","slice","1","j-host-concat","concat","1","j-host-push!","push","1","j-host-substring","substring","2","j-host-dom-on!","addEventListener","2","j-host-dom-off!","removeEventListener","2","j-host-prevent-default!","preventDefault","0","j-host-dom-append!","appendChild","1","j-host-dom-before!","insertBefore","2","j-host-dom-remove!","remove","0","j-host-dom-clear!","replaceChildren","0","j-host-dom-attribute","getAttribute","1","j-host-dom-set-attribute!","setAttribute","2","j-host-dom-focus!","focus","0",NULL};
 for(unsigned i=0;methods[i];i+=3)if(!strcmp(op,methods[i])){method=methods[i+1];argc=(unsigned)atoi(methods[i+2]);}
 if(method){if(n!=argc+1)bj_fail(b,"host method arity");bj_out(b,"(");bj_expr(b,bj_at(args,0));bj_out(b,").");bj_out(b,method);bj_out(b,"(");bj_args(b,args,1);bj_out(b,")");return 1;}
 if(!strcmp(op,"j-host-call")){if(n!=2)bj_fail(b,"host apply arity");bj_out(b,"Reflect.apply(");bj_expr(b,bj_at(args,0));bj_out(b,",null,");bj_expr(b,bj_at(args,1));bj_out(b,")");return 1;}
 return 0;
}
static void bj_tail(BJ*b,FLValue n,BJLoop*loop){
 const char*op=bj_text(rb_get(n,"op"));FLValue args=rb_get(n,"args");
 if(bj_is(rb_get(n,"kind"),"try")){bj_try(b,n,loop);return;}
 if(!strcmp(op,"do")){unsigned count=bj_len(args);for(unsigned i=0;i+1<count;i++){bj_expr(b,bj_at(args,i));bj_out(b,";\n");}if(count)bj_tail(b,bj_at(args,count-1),loop);else bj_out(b,"return null;\n");return;}
 if(!strcmp(op,"if")){if(bj_len(args)<2||bj_len(args)>3)bj_fail(b,"if arity");bj_out(b,"if(");bj_expr(b,bj_at(args,0));bj_out(b,"){\n");bj_tail(b,bj_at(args,1),loop);bj_out(b,"}else{\n");bj_tail(b,bj_at(args,2),loop);bj_out(b,"}\n");return;}
 if(!strcmp(op,"let")){bj_out(b,"{\n");bj_bindings(b,bj_at(args,0),NULL);bj_tail(b,bj_body(args,1),loop);bj_out(b,"}\n");return;}
 if(!strcmp(op,"recur")){
  if(!loop||bj_len(args)!=bj_len(loop->names))bj_fail(b,"recur outside tail loop or wrong arity");
  unsigned id=b->serial++;bj_out(b,"{const __j_next_");bj_num(b,id);bj_out(b,"=");bj_array(b,args);bj_out(b,";\n");
  for(unsigned i=0;i<bj_len(args);i++){bj_ident(b,bj_at(loop->names,i));bj_out(b,"=__j_next_");bj_num(b,id);bj_out(b,"[");bj_num(b,i);bj_out(b,"];\n");}bj_out(b,"continue;}\n");return;
 }
 bj_out(b,"return ");bj_expr(b,n);bj_out(b,";\n");
}
static void bj_expr(BJ*b,FLValue n){
 if(n.tag==FL_NIL){bj_out(b,"null");return;}
 if(n.tag!=FL_MAP)bj_fail(b,"invalid AST node");
 FLValue kind=rb_get(n,"kind");
 if(bj_is(kind,"variable")){bj_ident(b,rb_get(n,"name"));return;}
 if(bj_is(kind,"keyword")){bj_quote(b,rb_get(n,"name"));return;}
 if(bj_is(kind,"literal")){
  FLValue type=rb_get(n,"type"),value=rb_get(n,"value");
  if(bj_is(type,"nil")){bj_out(b,"null");return;}
  if(bj_is(type,"boolean")||bj_is(type,"bool")){bj_out(b,fl_truthy(value)?"true":"false");return;}
  if(bj_is(type,"string")){bj_quote(b,value);return;}
  if(bj_is(type,"number")){FLValue argv[1]={value};FLValue number=fl_v11_builtin(fl_str_val("str-to-num"),fl_vec_from(argv,1));
   if(number.tag==FL_FLOAT&&number.f==0&&signbit(number.f))bj_out(b,"-0");else bj_out(b,bj_text(fl_v11_number(number)));return;}
  if(bj_is(type,"symbol")){char op[128];bj_op(value,op);const char*core=bj_builtin(op);if(!core)bj_fail(b,"unresolved symbol");bj_out(b,"((...__j_args)=>");bj_core_ref(b,core);bj_out(b,"(__j_args))");return;}
  bj_fail(b,"unsupported literal");
 }
 if(bj_is(kind,"block")){
  FLValue items=bj_items(n),type=rb_get(n,"type");
  if(bj_is(type,"Array")){bj_array(b,items);return;}
  if(bj_is(type,"Map")){if(bj_len(items)%2)bj_fail(b,"odd map literal");bj_out(b,"Object.fromEntries([");
   for(unsigned i=0;i<bj_len(items);i+=2){if(i)bj_out(b,",");bj_out(b,"[");bj_core_ref(b,"jcore-string");bj_out(b,"(");bj_expr(b,bj_at(items,i));bj_out(b,"),");bj_expr(b,bj_at(items,i+1));bj_out(b,"]");}bj_out(b,"])");return;}
  bj_fail(b,"unsupported block type");
 }
 if(bj_is(kind,"and")||bj_is(kind,"or")){bj_logic(b,rb_get(n,"args"),0,bj_is(kind,"or"));return;}
 if(bj_is(kind,"throw")){bj_out(b,"(()=>{throw ");bj_expr(b,rb_get(n,"expr"));bj_out(b,";})()");return;}
 if(bj_is(kind,"try")){bj_out(b,"(()=>{\n");bj_try(b,n,NULL);bj_out(b,"})()");return;}
 if(!bj_is(kind,"sexpr"))bj_fail(b,"unsupported IR kind");
 const char*raw=bj_text(rb_get(n,"op"));FLValue args=rb_get(n,"args");char op[128];bj_op(rb_get(n,"op"),op);
 if(!strcmp(raw,"fl_register_named_fn")){bj_out(b,"null");return;}
 if(!strcmp(op,"define")||!strcmp(op,"set!")){if(bj_len(args)!=2)bj_fail(b,"assignment arity");bj_out(b,"(");bj_ident(b,rb_name(bj_at(args,0)));bj_out(b,"=");bj_expr(b,bj_at(args,1));bj_out(b,")");return;}
 if(!strcmp(op,"defn")){bj_out(b,"(");bj_ident(b,rb_name(bj_at(args,0)));bj_out(b,"=");bj_function(b,args,1);bj_out(b,")");return;}
 if(!strcmp(op,"fn")){bj_out(b,"(");bj_function(b,args,0);bj_out(b,")");return;}
 if(!strcmp(op,"do")||!strcmp(op,"if")||!strcmp(op,"let")){bj_out(b,"(()=>{\n");bj_tail(b,n,NULL);bj_out(b,"})()");return;}
 if(!strcmp(op,"loop")){BJLoop loop={fl_vec_new(),NULL};bj_out(b,"(()=>{\n");bj_bindings(b,bj_at(args,0),&loop.names);bj_out(b,"while(true){\n");bj_tail(b,bj_body(args,1),&loop);bj_out(b,"}\n})()");return;}
 if(!strcmp(op,"recur"))bj_fail(b,"recur outside tail loop");
 if(!strcmp(op,"and")||!strcmp(op,"or")){bj_logic(b,args,0,!strcmp(op,"or"));return;}
 if(bj_prefix(raw,"$")){bj_out(b,"(");bj_ident(b,fl_str_val(raw+1));bj_out(b,")(");bj_args(b,args,0);bj_out(b,")");return;}
 if(!strcmp(op,"--apply--")||!strcmp(raw,"__apply__")){if(!bj_len(args))bj_fail(b,"empty apply");bj_out(b,"(");bj_expr(b,bj_at(args,0));bj_out(b,")(");bj_args(b,args,1);bj_out(b,")");return;}
 if(bj_host(b,op,args))return;
 const char*core=bj_builtin(op);if(core){bj_core_ref(b,core);bj_out(b,"(");bj_array(b,args);bj_out(b,")");return;}
 bj_fail(b,"unsupported operation");
}
FLValue fl_compiler_browser_generate(FLValue user,FLValue parse_source){
 BJ b={0};b.names=fl_map_new();
 if(user.tag!=FL_VECTOR)bj_fail(&b,"program must be a node vector");
 if(rb_externs)bj_fail(&b,"C externs are not available in browser target");
 bj_validate_user(&b,user,0);
 if(parse_source.tag!=FL_FN)bj_fail(&b,"parser callback required");
 char source[sizeof fl_browser_core+1];memcpy(source,fl_browser_core,sizeof fl_browser_core);source[sizeof fl_browser_core]=0;
 FLValue core_source=fl_str_val(source);
 FLValue all=fl_fn_call(parse_source,1,&core_source);if(all.tag!=FL_VECTOR)bj_fail(&b,"support source parse failed");
 char island_source[sizeof fl_browser_island_core+1];memcpy(island_source,fl_browser_island_core,sizeof fl_browser_island_core);island_source[sizeof fl_browser_island_core]=0;
 FLValue island_text=fl_str_val(island_source),island_nodes=fl_fn_call(parse_source,1,&island_text);
 if(island_nodes.tag!=FL_VECTOR)bj_fail(&b,"Island support source parse failed");
 for(unsigned i=0;i<bj_len(island_nodes);i++)all=fl_vec_push(all,bj_at(island_nodes,i));
 unsigned island_serial=0;user=bi_lower(&b,user,0,&island_serial);
 for(unsigned i=0;i<bj_len(user);i++)all=fl_vec_push(all,bj_at(user,i));
 FLValue nodes=fl_compiler_bindings(all),globals=fl_map_new();
 for(unsigned i=0;i<bj_len(nodes);i++){
  FLValue node=bj_at(nodes,i),args=rb_get(node,"args"),op=rb_get(node,"op");
  if(bj_is(op,"define")||bj_is(op,"defn"))globals=fl_map_set(globals,rb_name(bj_at(args,0)),fl_bool(true));
  if(bj_is(op,"fl_register_named_fn")){FLValue name=rb_get(bj_at(args,0),"value"),target=rb_name(bj_at(args,1));b.names=fl_map_set(b.names,name,target);}
 }
 bj_out(&b,"// FreeLang browser core SHA-256: ");bj_out(&b,fl_browser_core_sha256);
 bj_out(&b,"\n// FreeLang Island core SHA-256: ");bj_out(&b,fl_browser_island_core_sha256);
 bj_out(&b,"\n\"use strict\";\n(()=>{\n");
 FLValue keys=fl_map_keys(globals);for(unsigned i=0;i<bj_len(keys);i++){bj_out(&b,"let ");bj_ident(&b,bj_at(keys,i));bj_out(&b,"=null;\n");}
 for(unsigned i=0;i<bj_len(nodes);i++){bj_expr(&b,bj_at(nodes,i));bj_out(&b,";\n");}
 bj_out(&b,"})();\n");FLValue result=fl_str_val(b.code);free(b.code);return result;
}
