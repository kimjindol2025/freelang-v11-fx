/* v11 HTTP contracts, sharing the native listener, framing and upload parser. */
#include <sys/stat.h>
#include <limits.h>
#include <ctype.h>
static Route vh_middlewares[MAX_ROUTES];
static int vh_nmiddlewares;
static FLValue vh_get(FLValue v,const char*k){return fl_map_get(v,fl_str_val(k));}
static FLValue vh_set(FLValue v,const char*k,FLValue x){return fl_map_set(v,fl_str_val(k),x);}
static FLValue vh_error(const char*why){fl_throw(fl_make_error("TypeError",why));return fl_nil();}
static FLValue vh_string(FLValue v){FLValue a[1]={v};return fl_v11_builtin(fl_str_val("num-to-str"),fl_vec_from(a,1));}
static FLValue vh_trim(FLValue v){FLValue a[1]={v};return fl_v11_builtin(fl_str_val("str-trim"),fl_vec_from(a,1));}
static FLValue vh_lower(FLValue v){FLValue a[1]={v};return fl_v11_builtin(fl_str_val("str-lower"),fl_vec_from(a,1));}
static FLValue vh_join(FLValue a,FLValue b){FLValue v[2]={a,b};return fl_str_n(2,v[0],v[1]);}
static FLValue vh_response(int status,const char*type,FLValue body){
 FLValue out=fl_map_new();out=vh_set(out,"__fl_response",fl_bool(true));out=vh_set(out,"status",fl_int(status));out=vh_set(out,"contentType",fl_str_val(type));return vh_set(out,"body",body);
}
static FLValue vh_json_response(int status,FLValue body){return vh_response(status,"application/json",body);}
static int vh_number(FLValue value,int fallback){
 value=fl_v11_to_number(value);
 if(value.tag==FL_FLOAT&&isfinite(value.f)&&value.f>=INT_MIN&&value.f<=INT_MAX)return(int)value.f;
 return fallback;
}
static FLValue vh_static(Route*route,FLValue req){
 const char*path=strval(vh_get(req,"path"));size_t prefix=strlen(route->path)-1;
 const char*relative=strlen(path)>=prefix?path+prefix:"";
 if(!*relative)relative="index.html";
 char filename[PATH_MAX];if(snprintf(filename,sizeof(filename),"%s/%s",strval(route->directory),relative)>=(int)sizeof(filename))return vh_response(403,"text/plain",fl_str_val("Forbidden"));
 FLValue args[1]={fl_str_val(filename)},data=fl_nil();int failed=0;FLValue error=fl_nil();
 if(fl_try_top>=FL_TRY_MAX)return vh_json_response(500,fl_str_val("Internal error"));
 FLTryFrame*frame=&fl_try_stack[fl_try_top++];
 if(setjmp(frame->buf)==0){data=fl_v11_file(fl_str_val("file_read"),fl_vec_from(args,1));fl_try_top--;}
 else{fl_try_top--;failed=1;error=frame->err;}
 if(failed){
  FLValue message=fl_v11_builtin(fl_str_val("error-message"),fl_vec_from(&error,1));
  int denied=strstr(strval(message),"PATH_OR_ACCESS_DENIED")!=NULL;
  return vh_response(denied?403:404,"text/plain",fl_str_val(denied?"Forbidden":"Not found"));
 }
 const char*ext=strrchr(relative,'.');const char*type="application/octet-stream";
 static const char*types[][2]={{".png","image/png"},{".jpg","image/jpeg"},{".jpeg","image/jpeg"},{".gif","image/gif"},{".svg","image/svg+xml"},{".html","text/html; charset=utf-8"},{".htm","text/html; charset=utf-8"},{".css","text/css; charset=utf-8"},{".js","application/javascript; charset=utf-8"},{".mjs","application/javascript; charset=utf-8"},{".json","application/json; charset=utf-8"},{".ico","image/x-icon"},{".woff","font/woff"},{".woff2","font/woff2"},{".ttf","font/ttf"},{".txt","text/plain; charset=utf-8"},{".pdf","application/pdf"}};
 for(size_t i=0;ext&&i<sizeof(types)/sizeof(types[0]);i++)if(!strcasecmp(ext,types[i][0])){type=types[i][1];break;}
 FLValue out=vh_response(200,type,data),headers=fl_map_new();headers=vh_set(headers,"Cache-Control",fl_str_val("public, max-age=3600"));return vh_set(out,"headers",headers);
}
FLValue fl_v11_http(FLValue name,FLValue argv){
 const char*op=strval(name);FLVector*v=(FLVector*)argv.obj;size_t n=v->len;
 FLValue a=n?v->data[0]:fl_nil(),b=n>1?v->data[1]:fl_nil(),c=n>2?v->data[2]:fl_nil();
 if(!strcmp(op,"server-use")){
  if(a.tag!=FL_STRING||((FLString*)a.obj)->len!=strlen(strval(a))||((FLString*)a.obj)->len>=sizeof(vh_middlewares[0].path)||vh_nmiddlewares>=MAX_ROUTES)return vh_error("invalid middleware path or limit");
  if(b.tag!=FL_STRING&&b.tag!=FL_FN)return vh_error("middleware handler must be a name or function");
  Route*r=&vh_middlewares[vh_nmiddlewares++];memset(r,0,sizeof(*r));strcpy(r->path,strval(a));strcpy(r->method,"*");r->name=b.tag==FL_STRING?fl_heap_copy(b):fl_nil();r->closure=b.tag==FL_FN?fl_heap_copy(b):fl_nil();r->directory=fl_nil();return fl_nil();
 }
 if(!strcmp(op,"server-static")){
  if(a.tag!=FL_STRING||(n>1&&b.tag!=FL_STRING)||g_nroutes>=MAX_ROUTES)return vh_error("invalid static directory, prefix or limit");
  if(((FLString*)a.obj)->len!=strlen(strval(a))||(n>1&&((FLString*)b.obj)->len!=strlen(strval(b))))return vh_error("invalid static path");
  const char*prefix=n>1?strval(b):"/";size_t len=strlen(prefix);char path[512];
  if(snprintf(path,sizeof(path),"%s%s*",prefix,len&&prefix[len-1]=='/'?"":"/")>=(int)sizeof(path))return vh_error("static prefix too long");
  Route*r=&g_routes[g_nroutes++];memset(r,0,sizeof(*r));strcpy(r->method,"GET");strcpy(r->path,path);r->directory=fl_heap_copy(((FLString*)a.obj)->len?a:fl_str_val("."));r->is_static=1;r->name=fl_nil();r->closure=fl_nil();return fl_nil();
 }
 if(!strcmp(op,"server-body-limit")){
  FLValue value=fl_v11_to_number(a);double limit=value.f;if(isnan(limit))return vh_error("invalid body limit");
  if(limit<1)limit=1;if(limit>MAX_BODY_SIZE)limit=MAX_BODY_SIZE;g_body_limit=(size_t)floor(limit);return fl_nil();
 }
 if(!strcmp(op,"server-header")){
  if(a.tag!=FL_MAP)return vh_error("server_header: response map required");
  FLValue headers=vh_get(a,"headers");headers=fl_map_set(headers,vh_string(b),c);return vh_set(a,"headers",headers);
 }
 if(!strcmp(op,"server-set-cookie")){
  FLValue enc[1]={vh_string(a)};FLValue out=fl_v11_builtin(fl_str_val("str-encode-uri"),fl_vec_from(enc,1));out=vh_join(out,fl_str_val("="));enc[0]=vh_string(b);out=vh_join(out,fl_v11_builtin(fl_str_val("str-encode-uri"),fl_vec_from(enc,1)));
  if(fl_truthy(fl_map_has(c,fl_str_val("max_age")))){out=vh_join(out,fl_str_val("; Max-Age="));out=vh_join(out,vh_string(vh_get(c,"max_age")));}
  FLValue path=vh_get(c,"path");out=vh_join(out,fl_str_val("; Path="));out=vh_join(out,fl_v11_truthy(path)?vh_string(path):fl_str_val("/"));
  FLValue domain=vh_get(c,"domain");if(fl_v11_truthy(domain)){
   domain=vh_string(domain);const unsigned char*p=(const unsigned char*)strval(domain);size_t len=((FLString*)domain.obj)->len;
   for(size_t i=0;i<len;i++)if(!((p[i]>='a'&&p[i]<='z')||(p[i]>='A'&&p[i]<='Z')||(p[i]>='0'&&p[i]<='9')||p[i]=='.'||p[i]=='-'))return vh_error("invalid cookie domain");
   out=vh_join(out,fl_str_val("; Domain="));out=vh_join(out,domain);
  }
  FLValue flag=vh_get(c,"http_only");if(flag.tag!=FL_BOOL||flag.b)out=vh_join(out,fl_str_val("; HttpOnly"));
  flag=vh_get(c,"secure");if(flag.tag!=FL_BOOL||flag.b)out=vh_join(out,fl_str_val("; Secure"));
  FLValue same=vh_get(c,"same_site");out=vh_join(out,fl_str_val("; SameSite="));return vh_join(out,fl_v11_truthy(same)?vh_string(same):fl_str_val("Strict"));
 }
 if(!strcmp(op,"server-redirect")||!strcmp(op,"server-redirect-cookie")){
  FLValue location=vh_string(a);
  if(!strcmp(op,"server-redirect")){
   FLString*s=(FLString*)location.obj;char*clean=fl_arena_alloc(s->len+1);if(!clean)return vh_error("redirect allocation failed");size_t used=0;for(size_t i=0;i<s->len;i++)if(s->data[i]!='\r'&&s->data[i]!='\n')clean[used++]=s->data[i];clean[used]=0;location=form_bytes(clean,used);
  }
  FLValue headers=vh_set(fl_map_new(),"Location",location);if(!strcmp(op,"server-redirect-cookie"))headers=vh_set(headers,"Set-Cookie",b);
  return vh_set(vh_response(302,"text/plain",fl_str_val("")),"headers",headers);
 }
 if(!strcmp(op,"server-req-cookie")){
  FLValue header=vh_get(vh_get(a,"headers"),"cookie");if(header.tag!=FL_STRING)return fl_nil();FLString*s=(FLString*)header.obj;size_t at=0;
  while(at<=s->len){size_t end=at;while(end<s->len&&s->data[end]!=';')end++;size_t equal=at;while(equal<end&&s->data[equal]!='=')equal++;
   FLValue key=vh_trim(form_bytes(s->data+at,equal-at));if(fl_truthy(fl_eq(key,b)))return vh_trim(form_bytes(s->data+(equal<end?equal+1:end),equal<end?end-equal-1:0));
   if(end==s->len)break;at=end+1;
  }return fl_nil();
 }
 if(!strcmp(op,"server-req-json")){
  FLValue body=vh_get(a,"body");if(body.tag==FL_NIL||body.tag==FL_MAP||body.tag==FL_VECTOR)return body;
  FLValue parsed=fl_v11_json_try_parse(body);return fl_truthy(fl_vec_get(parsed,fl_int(0)))?fl_vec_get(parsed,fl_int(1)):fl_nil();
 }
 if(!strcmp(op,"server-req-header")){
  FLValue value=fl_map_get(vh_get(a,"headers"),vh_lower(vh_string(b)));return value.tag==FL_VECTOR?fl_vec_get(value,fl_int(0)):value;
 }
 if(!strcmp(op,"server-req-query")){
  FLValue query=vh_get(a,"query");if(n<2)return query;FLValue value=fl_map_get(query,b);return value.tag==FL_VECTOR?fl_vec_get(value,fl_int(0)):value;
 }
 if(!strcmp(op,"server-json")){
  int code=vh_number(a,0);int is_status=(a.tag==FL_INT||a.tag==FL_FLOAT)&&code>=100&&code<600;return vh_json_response(is_status?code:200,is_status?b:a);
 }
 if(!strcmp(op,"server-html"))return vh_response(n>1&&(b.tag==FL_INT||b.tag==FL_FLOAT)?vh_number(b,0):200,"text/html; charset=utf-8",a);
 if(!strcmp(op,"server-text"))return vh_response(200,"text/plain",a);
 if(!strcmp(op,"server-status")){
  if(a.tag==FL_STRING&&(b.tag==FL_INT||b.tag==FL_FLOAT)){
   const char*strict=getenv("FL_V12");if(strict&&!strcmp(strict,"1"))return vh_error("server_status: reversed arguments");
   fprintf(stderr,"server_status: reversed arguments\n");return vh_json_response(vh_number(b,0),a);
  }
  return vh_json_response(vh_number(a,0),b);
 }
 return vh_error("unknown HTTP operation");
}

typedef struct {FLValue body;char*header;size_t header_len;int status;} VHttpPrepared;
static int vh_hop(const char*key){
 static const char*names[]={"connection","keep-alive","transfer-encoding","te","trailer","proxy-authorization","proxy-authenticate","upgrade","content-encoding","content-length"};
 for(size_t i=0;i<sizeof(names)/sizeof(names[0]);i++)if(!strcasecmp(key,names[i]))return 1;return 0;
}
static void vh_append_header(char*buffer,size_t*used,const char*bytes,size_t n){
 if(n>MAX_HEADER_TOTAL-*used)vh_error("response header limit exceeded");memcpy(buffer+*used,bytes,n);*used+=n;
}
static void vh_prepare_response(FLValue input,int keep_alive,VHttpPrepared*out){
 FLValue response=input;
 if(input.tag!=FL_MAP||(!fl_v11_truthy(vh_get(input,"__fl_response"))&&
    !(fl_truthy(fl_map_has(input,fl_str_val("status")))&&fl_truthy(fl_map_has(input,fl_str_val("body"))))))response=vh_json_response(200,input);
 FLValue status=vh_get(response,"status");out->status=status.tag==FL_NIL?200:vh_number(status,0);
 if(out->status<100||out->status>999)vh_error("invalid HTTP response status");
 FLValue headers=vh_get(response,"headers"),routing_type=vh_get(headers,"content-type");
 if(routing_type.tag==FL_NIL)routing_type=vh_get(response,"contentType");
 if(routing_type.tag==FL_NIL){routing_type=vh_get(headers,"Content-Type");if(routing_type.tag==FL_NIL)routing_type=fl_str_val("application/json");}
 routing_type=vh_string(routing_type);
 FLValue body=vh_get(response,"body");
 if(body.tag==FL_NIL)body=fl_str_val("");
 else if(body.tag==FL_STRING){if(body.obj->type==FL_TEXT)body=fl_v11_builtin(fl_str_val("str"),fl_vec_from(&body,1));}
 else if((body.tag==FL_MAP||body.tag==FL_VECTOR)&&strstr(strval(routing_type),"json"))body=fl_v11_json_encode(body);
 else if(body.tag==FL_FN)vh_error("unsupported HTTP response body");
 else body=vh_string(body);
 out->body=body;FLValue wire_type=fl_v11_header_value(routing_type);
 char*extra=fl_arena_alloc(MAX_HEADER_TOTAL);if(!extra)vh_error("header allocation failed");size_t used=0;
 if(headers.tag==FL_MAP){
  FLMap*m=(FLMap*)headers.obj;
  for(uint32_t i=0;i<m->len;i++){
   FLValue key=vh_string(m->entries[i].key);FLString*k=(FLString*)key.obj;
   if(!k->len)vh_error("empty HTTP header name");
   for(size_t j=0;j<k->len;j++)if(!token_char((unsigned char)k->data[j]))vh_error("invalid HTTP header name");
   if(vh_hop(k->data))continue;
   FLValue value=m->entries[i].val;uint32_t count=value.tag==FL_VECTOR?((FLVector*)value.obj)->len:1;
   if(!strcasecmp(k->data,"content-type")&&count!=1)vh_error("ambiguous Content-Type");
   for(uint32_t j=0;j<count;j++){
    FLValue item=value.tag==FL_VECTOR?((FLVector*)value.obj)->data[j]:value;FLValue encoded=fl_v11_header_value(item);FLString*v=(FLString*)encoded.obj;
    if(!strcasecmp(k->data,"content-type")){wire_type=encoded;continue;}
    vh_append_header(extra,&used,k->data,k->len);vh_append_header(extra,&used,": ",2);vh_append_header(extra,&used,v->data,v->len);vh_append_header(extra,&used,"\r\n",2);
   }
  }
 }
 out->header=fl_arena_alloc(MAX_HEADER_TOTAL);if(!out->header)vh_error("header allocation failed");out->header_len=0;
 char line[256];int length=snprintf(line,sizeof(line),"HTTP/1.1 %d %s\r\nContent-Type: ",out->status,status_text(out->status));
 vh_append_header(out->header,&out->header_len,line,(size_t)length);
 FLString*type=(FLString*)wire_type.obj;vh_append_header(out->header,&out->header_len,type->data,type->len);
 length=snprintf(line,sizeof(line),"\r\nContent-Length: %u\r\nConnection: %s\r\n",((FLString*)body.obj)->len,keep_alive?"keep-alive":"close");
 vh_append_header(out->header,&out->header_len,line,(size_t)length);vh_append_header(out->header,&out->header_len,extra,used);vh_append_header(out->header,&out->header_len,"\r\n",2);
}
