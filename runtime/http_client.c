/* ─────────────────────────────────────────────────────────────────
   fx HTTP 클라이언트 — libcurl 기반
   ─────────────────────────────────────────────────────────────────
   함수 목록:
     http_get  url                    → {status body headers}
     http_post url body               → {status body headers}
     http_put  url body               → {status body headers}
     http_del  url                    → {status body headers}
     http_req  method url body headers → {status body headers}  (풀 제어)

   반환 맵:
     {"status" 200 "body" "..." "headers" {"content-type" "..."}}
     실패 시 {"status" 0 "body" "" "error" "설명"}
 ───────────────────────────────────────────────────────────────── */
#include "runtime.h"
#include <curl/curl.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <limits.h>
#include <math.h>
#include <ctype.h>

/* ── curl 응답 버퍼 ── */
typedef struct {
    char*  data;
    size_t len;
    size_t cap;
} CurlBuf;
static pthread_once_t curl_once=PTHREAD_ONCE_INIT;
static CURLcode curl_init_code=CURLE_FAILED_INIT;
static void init_curl(void){curl_init_code=curl_global_init(CURL_GLOBAL_DEFAULT);}

static size_t write_cb(void* ptr, size_t size, size_t nmemb, void* ud) {
    CurlBuf* b = (CurlBuf*)ud;
    if (size && nmemb > SIZE_MAX / size) return 0;
    size_t n = size * nmemb;
    if (n > (64u*1024*1024) - b->len) return 0;
    if (b->len + n + 1 >= b->cap) {
        size_t cap=(b->len+n+1)*2+4096;
        char* data=realloc(b->data,cap);
        if (!data) return 0;
        b->data=data;b->cap=cap;
    }
    memcpy(b->data + b->len, ptr, n);
    b->len += n;
    b->data[b->len] = '\0';
    return n;
}

/* ── curl 응답 헤더 버퍼 ── */
static size_t header_cb(void* ptr, size_t size, size_t nmemb, void* ud) {
    return write_cb(ptr,size,nmemb,ud);
}

/* ── 헤더 맵 파싱: "Key: Value\r\n..." → FLMap ── */
static FLValue parse_headers(const char* raw) {
    FLValue m = fl_map_new();
    const char* p = raw;
    /* 첫 줄 (HTTP/1.1 200 OK) 건너뜀 */
    const char* nl = strchr(p, '\n');
    if (nl) p = nl + 1;

    while (*p && *p != '\r' && *p != '\n') {
        const char* colon = strchr(p, ':');
        const char* lf    = strchr(p, '\n');
        if (!colon || (lf && colon > lf)) { if (lf) p = lf + 1; else break; continue; }

        /* 키 소문자화 */
        size_t klen = (size_t)(colon - p);
        char* key = malloc(klen + 1);
        for (size_t i = 0; i < klen; i++)
            key[i] = (p[i] >= 'A' && p[i] <= 'Z') ? p[i] + 32 : p[i];
        key[klen] = '\0';

        /* 값 앞 공백 제거 + 뒤 \r\n 제거 */
        const char* vs = colon + 1;
        while (*vs == ' ') vs++;
        size_t vlen = lf ? (size_t)(lf - vs) : strlen(vs);
        if (vlen > 0 && vs[vlen - 1] == '\r') vlen--;
        char* val = malloc(vlen + 1);
        memcpy(val, vs, vlen);
        val[vlen] = '\0';

        m = fl_map_set(m, fl_str_val(key), fl_str_val(val));
        free(key); free(val);
        if (lf) p = lf + 1; else break;
    }
    return m;
}

/* ── FLMap → curl slist 변환 (요청 헤더) ── */
static struct curl_slist* map_to_slist(FLValue headers,int*ok) {
    *ok=1;
    if (headers.tag != FL_MAP || !headers.obj) return NULL;
    struct curl_slist* list = NULL;
    FLMap* m = (FLMap*)headers.obj;
    for (int i = 0; i < m->len; i++) {
        FLMapEntry* e = &m->entries[i];
        if (e->key.tag != FL_STRING || e->val.tag != FL_STRING) continue;
        const char* k = ((FLString*)e->key.obj)->data;
        const char* v = ((FLString*)e->val.obj)->data;
        size_t length=strlen(k)+strlen(v)+3;
        char* line=malloc(length);
        if(!line){*ok=0;curl_slist_free_all(list);return NULL;}
        if(*v)snprintf(line,length,"%s: %s",k,v);else snprintf(line,length,"%s;",k);
        struct curl_slist* next=curl_slist_append(list,line);free(line);
        if(!next){*ok=0;curl_slist_free_all(list);return NULL;}list=next;
    }
    return list;
}

/* ── 코어 실행 ── */
static FLValue fx_http_wire(const char* method, const char* url,
                             const char* body,size_t body_len,FLValue req_headers,long timeout_ms,int v11) {
    pthread_once(&curl_once,init_curl);
    CURL* curl = curl_init_code==CURLE_OK?curl_easy_init():NULL;
    if (!curl) return fl_map_set(fl_map_set(fl_map_set(fl_map_new(),fl_str_val("body"),fl_str_val("")),
        fl_str_val("status"), fl_int(0)),
        fl_str_val("error"), fl_str_val("curl_easy_init 실패"));

    CurlBuf resp = {malloc(4096), 0, 4096};
    CurlBuf hdrs = {malloc(4096), 0, 4096};
    if(!resp.data||!hdrs.data){free(resp.data);free(hdrs.data);curl_easy_cleanup(curl);return fl_map_set(fl_map_set(fl_map_set(fl_map_new(),fl_str_val("status"),fl_int(0)),fl_str_val("body"),fl_str_val("")),fl_str_val("error"),fl_str_val("HTTP allocation failed"));}
    resp.data[0] = '\0'; hdrs.data[0] = '\0';

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_cb);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &resp);
    curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, header_cb);
    curl_easy_setopt(curl, CURLOPT_HEADERDATA, &hdrs);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, timeout_ms);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 10L);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, v11?0L:1L);
    curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 5L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, v11?1L:0L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);
    curl_easy_setopt(curl, CURLOPT_NOSIGNAL,1L);
    if(v11){
        curl_easy_setopt(curl,CURLOPT_PROXY,"");
        curl_easy_setopt(curl,CURLOPT_PROTOCOLS_STR,"http,https");
        const char*ca=getenv("SSL_CERT_FILE");if(ca&&*ca)curl_easy_setopt(curl,CURLOPT_CAINFO,ca);
    }else curl_easy_setopt(curl, CURLOPT_USERAGENT, "fx-http-client/1.0");

    /* 메서드별 설정 */
    if(v11){
        if(body){curl_easy_setopt(curl,CURLOPT_POSTFIELDS,body);curl_easy_setopt(curl,CURLOPT_POSTFIELDSIZE_LARGE,(curl_off_t)body_len);}
        curl_easy_setopt(curl,CURLOPT_CUSTOMREQUEST,method);
        if(!strcmp(method,"HEAD"))curl_easy_setopt(curl,CURLOPT_NOBODY,1L);
    }else if (strcmp(method, "POST") == 0) {
        curl_easy_setopt(curl, CURLOPT_POST, 1L);
        if (body) {
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body);
            curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, (long)strlen(body));
        } else {
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, "");
            curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, 0L);
        }
    } else if (strcmp(method, "PUT") == 0) {
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PUT");
        if (body) {
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body);
            curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, (long)strlen(body));
        }
    } else if (strcmp(method, "DELETE") == 0) {
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "DELETE");
    } else if (strcmp(method, "PATCH") == 0) {
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PATCH");
        if (body) {
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body);
            curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, (long)strlen(body));
        }
    }
    /* GET은 기본값 */

    /* 요청 헤더 */
    int header_ok=1;
    struct curl_slist* slist = map_to_slist(req_headers,&header_ok);
    if(v11){
        struct curl_slist* next=curl_slist_append(slist,"Expect:");if(next)slist=next;else header_ok=0;
        if(fl_map_get(req_headers,fl_str_val("content-type")).tag==FL_NIL){next=curl_slist_append(slist,"Content-Type:");if(next)slist=next;else header_ok=0;}
    }
    if (slist) curl_easy_setopt(curl, CURLOPT_HTTPHEADER, slist);

    CURLcode rc = header_ok?curl_easy_perform(curl):CURLE_OUT_OF_MEMORY;

    long status = 0;
    if (rc == CURLE_OK)
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &status);

    if (slist) curl_slist_free_all(slist);
    curl_easy_cleanup(curl);

    /* 결과 맵 구성 */
    FLValue result = fl_map_new();
    if (rc != CURLE_OK) {
        result = fl_map_set(result, fl_str_val("status"), fl_int(0));
        result = fl_map_set(result, fl_str_val("body"),   fl_str_val(""));
        result = fl_map_set(result, fl_str_val("error"),  fl_str_val(v11&&rc==CURLE_OPERATION_TIMEDOUT?"timeout":curl_easy_strerror(rc)));
    } else {
        result = fl_map_set(result, fl_str_val("status"),  fl_int((int64_t)status));
        FLString*text=fl_arena_alloc(sizeof(*text)+resp.len+1);
        if(!text){free(resp.data);free(hdrs.data);fl_throw(fl_str_val("HTTP allocation failed"));return fl_nil();}
        text->base.type=FL_STRING;text->base.rc=1;text->len=(uint32_t)resp.len;memcpy(text->data,resp.data,resp.len+1);
        FLValue value={.tag=FL_STRING,.obj=(FLObject*)text};
        if(v11)value=fl_v11_builtin(fl_str_val("str"),fl_vec_from(&value,1));
        result = fl_map_set(result, fl_str_val("body"), value);
        if(!v11)result = fl_map_set(result, fl_str_val("headers"), parse_headers(hdrs.data));
    }

    free(resp.data);
    free(hdrs.data);
    return result;
}

static FLValue fx_http_exec(const char*method,const char*url,const char*body,FLValue headers){return fx_http_wire(method,url,body,body?strlen(body):0,headers,120000,0);}

/* ─────────────────────────────────────────────────────────────────
   공개 FL 함수
 ───────────────────────────────────────────────────────────────── */

/* http_get url */
FLValue http_get(FLValue url_v) {
    if (url_v.tag != FL_STRING) return fl_map_set(fl_map_new(),
        fl_str_val("error"), fl_str_val("url must be string"));
    return fx_http_exec("GET", ((FLString*)url_v.obj)->data, NULL, fl_nil());
}

/* http_get_h url headers-map */
FLValue http_get_h(FLValue url_v, FLValue hdrs_v) {
    if (url_v.tag != FL_STRING) return fl_map_set(fl_map_new(),
        fl_str_val("error"), fl_str_val("url must be string"));
    return fx_http_exec("GET", ((FLString*)url_v.obj)->data, NULL, hdrs_v);
}

/* http_post url body */
FLValue http_post(FLValue url_v, FLValue body_v) {
    if (url_v.tag != FL_STRING) return fl_map_set(fl_map_new(),
        fl_str_val("error"), fl_str_val("url must be string"));
    const char* body = (body_v.tag == FL_STRING && body_v.obj)
        ? ((FLString*)body_v.obj)->data : "";
    return fx_http_exec("POST", ((FLString*)url_v.obj)->data, body, fl_nil());
}

/* http_post_h url body headers-map */
FLValue http_post_h(FLValue url_v, FLValue body_v, FLValue hdrs_v) {
    if (url_v.tag != FL_STRING) return fl_map_set(fl_map_new(),
        fl_str_val("error"), fl_str_val("url must be string"));
    const char* body = (body_v.tag == FL_STRING && body_v.obj)
        ? ((FLString*)body_v.obj)->data : "";
    return fx_http_exec("POST", ((FLString*)url_v.obj)->data, body, hdrs_v);
}

/* http_put url body */
FLValue http_put(FLValue url_v, FLValue body_v) {
    if (url_v.tag != FL_STRING) return fl_map_set(fl_map_new(),
        fl_str_val("error"), fl_str_val("url must be string"));
    const char* body = (body_v.tag == FL_STRING && body_v.obj)
        ? ((FLString*)body_v.obj)->data : "";
    return fx_http_exec("PUT", ((FLString*)url_v.obj)->data, body, fl_nil());
}

/* http_del url */
FLValue http_del(FLValue url_v) {
    if (url_v.tag != FL_STRING) return fl_map_set(fl_map_new(),
        fl_str_val("error"), fl_str_val("url must be string"));
    return fx_http_exec("DELETE", ((FLString*)url_v.obj)->data, NULL, fl_nil());
}

/* http_patch url body */
FLValue http_patch(FLValue url_v, FLValue body_v) {
    if (url_v.tag != FL_STRING) return fl_map_set(fl_map_new(),
        fl_str_val("error"), fl_str_val("url must be string"));
    const char* body = (body_v.tag == FL_STRING && body_v.obj)
        ? ((FLString*)body_v.obj)->data : "";
    return fx_http_exec("PATCH", ((FLString*)url_v.obj)->data, body, fl_nil());
}

/* http_req method url body headers */
FLValue http_req(FLValue method_v, FLValue url_v, FLValue body_v, FLValue hdrs_v) {
    if (method_v.tag != FL_STRING || url_v.tag != FL_STRING)
        return fl_map_set(fl_map_new(), fl_str_val("error"),
                          fl_str_val("method and url must be strings"));
    const char* method = ((FLString*)method_v.obj)->data;
    const char* url    = ((FLString*)url_v.obj)->data;
    const char* body   = (body_v.tag == FL_STRING && body_v.obj)
                         ? ((FLString*)body_v.obj)->data : "";
    return fx_http_exec(method, url, *body ? body : NULL, hdrs_v);
}

/* http_body res — 응답 맵에서 body 추출 (편의 함수) */
FLValue http_body(FLValue res_v) {
    if (res_v.tag != FL_MAP) return fl_str_val("");
    return fl_map_get(res_v, fl_str_val("body"));
}

/* http_status res — 응답 맵에서 status 추출 */
FLValue http_status(FLValue res_v) {
    if (res_v.tag != FL_MAP) return fl_int(0);
    return fl_map_get(res_v, fl_str_val("status"));
}

/* http_ok? res — 2xx 여부 */
FLValue http_ok_p(FLValue res_v) {
    FLValue s = http_status(res_v);
    double code = (s.tag == FL_INT) ? (double)s.i
                : (s.tag == FL_FLOAT) ? s.f : 0.0;
    return fl_bool(code >= 200 && code < 300);
}

/* v11 public calls keep their argument order and omit legacy response headers. */
static FLValue vc_error(const char*message){
 FLValue out=fl_map_new();out=fl_map_set(out,fl_str_val("status"),fl_int(0));out=fl_map_set(out,fl_str_val("body"),fl_str_val(""));return fl_map_set(out,fl_str_val("error"),fl_str_val(message));
}
static int vc_token(unsigned char c){return(c>='a'&&c<='z')||(c>='A'&&c<='Z')||(c>='0'&&c<='9')||(c&&strchr("!#$%&'*+-.^_`|~",c)!=NULL);}
static FLValue vc_request(FLValue name,FLValue argv){
 const char*op=((FLString*)name.obj)->data;FLVector*v=(FLVector*)argv.obj;size_t n=v->len;
 FLValue a=n?v->data[0]:fl_nil(),b=n>1?v->data[1]:fl_nil(),headers=n>2?v->data[2]:fl_nil(),body=n>3?v->data[3]:fl_nil();
 int get=!strcmp(op,"http-get")||!strcmp(op,"http-with-timeout");
 FLValue method=get?fl_str_val("GET"):fl_v11_utf8_string(fl_v11_truthy(a)?a:fl_str_val("GET"));
 FLValue url=fl_v11_utf8_string(get?a:b);
 FLString*m=(FLString*)method.obj,*u=(FLString*)url.obj;
 if(!m->len||m->len>=128||m->len!=strlen(m->data)||u->len!=strlen(u->data))return vc_error("invalid HTTP request");
 char upper[128];for(size_t i=0;i<m->len;i++){unsigned char c=m->data[i];if(!vc_token(c))return vc_error("invalid HTTP method");upper[i]=(char)toupper(c);}upper[m->len]=0;
 CURLU*parsed=curl_url();if(!parsed)return vc_error("HTTP allocation failed");
 int valid=curl_url_set(parsed,CURLUPART_URL,u->data,0)==CURLUE_OK;char*scheme=NULL;
 if(valid){valid=curl_url_get(parsed,CURLUPART_SCHEME,&scheme,0)==CURLUE_OK&&(!strcmp(scheme,"http")||!strcmp(scheme,"https"));}curl_free(scheme);curl_url_cleanup(parsed);
 if(!valid)return vc_error("invalid HTTP URL");
 if(get){headers=fl_nil();body=fl_nil();}
 FLValue normalized=fl_map_new();
 if(headers.tag==FL_MAP){FLMap*map=(FLMap*)headers.obj;
  if(map->len>64)return vc_error("HTTP header limit exceeded");
  for(uint32_t i=0;i<map->len;i++){
   FLValue key=fl_v11_utf8_string(map->entries[i].key);FLString*k=(FLString*)key.obj;
   if(!k->len||k->len>32768)return vc_error("invalid HTTP header name");
   char*lower=fl_arena_alloc(k->len+1);if(!lower)return vc_error("HTTP allocation failed");
   for(size_t j=0;j<k->len;j++){if(!vc_token((unsigned char)k->data[j]))return vc_error("invalid HTTP header name");lower[j]=(char)tolower((unsigned char)k->data[j]);}lower[k->len]=0;
   if(body.tag!=FL_NIL&&(!strcmp(lower,"content-length")||!strcmp(lower,"transfer-encoding")))continue;
   FLValue value=fl_v11_header_value(map->entries[i].val);if(((FLString*)value.obj)->len>32768)return vc_error("HTTP header limit exceeded");
   normalized=fl_map_set(normalized,fl_str_val(lower),value);
  }
 }
 long timeout_ms=10000;
 if(!strcmp(op,"http-request-timeout")||!strcmp(op,"http-with-timeout")){
  FLValue value=!strcmp(op,"http-with-timeout")?b:(n>4?v->data[4]:fl_nil());value=fl_v11_to_number(value);double ms=value.f;
  if(!strcmp(op,"http-with-timeout"))ms=(b.tag==FL_INT||b.tag==FL_FLOAT)&&ms>0?ms:10000;
  else if(isnan(ms)||ms==0)ms=5000;
  if(!isfinite(ms)||ms>INT_MAX||ms<1)ms=1;timeout_ms=(long)ms;
 }
 FLValue bytes=body.tag==FL_NIL?fl_nil():fl_v11_utf8_string(body);FLString*s=bytes.tag==FL_STRING?(FLString*)bytes.obj:NULL;
 return fx_http_wire(upper,u->data,s?s->data:NULL,s?s->len:0,normalized,timeout_ms,1);
}
FLValue fl_v11_http_client(FLValue name,FLValue argv){
 if(fl_try_top>=FL_TRY_MAX)return vc_error("HTTP exception frame limit exceeded");
 FLTryFrame*frame=&fl_try_stack[fl_try_top++];
 if(setjmp(frame->buf)==0){FLValue out=vc_request(name,argv);fl_try_top--;return out;}
 fl_try_top--;return vc_error("invalid HTTP request");
}
