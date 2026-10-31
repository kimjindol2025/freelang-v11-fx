/* Browser AST printer. Runtime algorithms live in embedded FreeLang source.
 * Only fixed grammar tokens and whitelisted primitive host operations are emitted. */
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include "browser-core-data.h"
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
  "browser-after!","jcore-after","browser-cancel!","jcore-cancel",NULL};
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
 const char *call=NULL;unsigned arity=0;
 const char *calls[]={"j-host-array?","Array.isArray","1","j-host-own?","Object.hasOwn","2","j-host-keys","Object.keys","1","j-host-integer?","Number.isInteger","1","j-host-string","String","1","j-host-dom-by-id","document.getElementById","1","j-host-dom-create","document.createElement","1","j-host-array-from","Array.from","1",NULL};
 for(unsigned i=0;calls[i];i+=3)if(!strcmp(op,calls[i])){call=calls[i+1];arity=(unsigned)atoi(calls[i+2]);}
 if(call){if(n!=arity)bj_fail(b,"host call arity");bj_out(b,call);bj_out(b,"(");bj_args(b,args,0);bj_out(b,")");return 1;}
 const char *method=NULL;unsigned argc=0;
 const char *methods[]={"j-host-weak-has","has","1","j-host-weak-add","add","1","j-host-weak-get","get","1","j-host-weak-set!","set","2","j-host-copy","slice","0","j-host-slice","slice","1","j-host-concat","concat","1","j-host-push!","push","1","j-host-substring","substring","2","j-host-dom-on!","addEventListener","2","j-host-dom-off!","removeEventListener","2","j-host-prevent-default!","preventDefault","0","j-host-dom-append!","appendChild","1","j-host-dom-before!","insertBefore","2","j-host-dom-remove!","remove","0","j-host-dom-clear!","replaceChildren","0","j-host-dom-attribute","getAttribute","1","j-host-dom-set-attribute!","setAttribute","2","j-host-dom-focus!","focus","0",NULL};
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
 for(unsigned i=0;i<bj_len(user);i++)all=fl_vec_push(all,bj_at(user,i));
 FLValue nodes=fl_compiler_bindings(all),globals=fl_map_new();
 for(unsigned i=0;i<bj_len(nodes);i++){
  FLValue node=bj_at(nodes,i),args=rb_get(node,"args"),op=rb_get(node,"op");
  if(bj_is(op,"define")||bj_is(op,"defn"))globals=fl_map_set(globals,rb_name(bj_at(args,0)),fl_bool(true));
  if(bj_is(op,"fl_register_named_fn")){FLValue name=rb_get(bj_at(args,0),"value"),target=rb_name(bj_at(args,1));b.names=fl_map_set(b.names,name,target);}
 }
 bj_out(&b,"// FreeLang browser core SHA-256: ");bj_out(&b,fl_browser_core_sha256);
 bj_out(&b,"\n\"use strict\";\n(()=>{\n");
 FLValue keys=fl_map_keys(globals);for(unsigned i=0;i<bj_len(keys);i++){bj_out(&b,"let ");bj_ident(&b,bj_at(keys,i));bj_out(&b,"=null;\n");}
 for(unsigned i=0;i<bj_len(nodes);i++){bj_expr(&b,bj_at(nodes,i));bj_out(&b,";\n");}
 bj_out(&b,"})();\n");FLValue result=fl_str_val(b.code);free(b.code);return result;
}
