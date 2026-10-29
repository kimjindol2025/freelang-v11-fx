/*
 * http.c — FreeLang C 런타임 HTTP 서버
 *
 * 구현 함수:
 *   server_get / server_post / server_put / server_patch / server_delete
 *   server_html / server_text / server_status / server_json
 *   server_start
 *   server_req_param / server_req_query / fxb_server_req_body / server_req_header
 *   server_redirect
 *
 * 의존: POSIX sockets + dlsym (외부 라이브러리 없음)
 * 빌드: gcc ... -rdynamic -lpthread
 */

#define _GNU_SOURCE
#include "runtime.h"
#include "internal.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include <dlfcn.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <math.h>

/* ───────────────────────────────────────────
   내부 타입
─────────────────────────────────────────── */

#define MAX_ROUTES      256
#define MAX_HEADERS     64
#define RECV_BUF        65536
#define SEND_BUF        65536

/* ── 보안 한도 ── */
#define MAX_URI_LEN         8192        /* URI 최대 길이 → 414 */
#define MAX_HEADER_TOTAL    32768       /* 헤더 총합 최대 → 431 */
#define MAX_BODY_SIZE       (64*1024*1024) /* Includes multipart framing. */
static size_t g_body_limit=MAX_BODY_SIZE;
#define HEADER_RECV_TIMEOUT 10          /* 헤더 완성 타임아웃 (초) */
#define MAX_CONNECTIONS     512         /* 동시 연결 한도 → 503 */

typedef FLValue (*HandlerFn)(FLValue req);

typedef struct {
    char   method[8];      /* GET POST PUT PATCH DELETE */
    char   path[512];      /* /api/users/:id */
    HandlerFn fn;
    FLValue  closure;      /* FL_FN 클로저 핸들러 (fn이 NULL일 때 사용) */
    FLValue  name;         /* Source names are resolved for each request. */
    FLValue directory;
    int is_static;
} Route;
static FLValue vh_static(Route*,FLValue);
static const char*status_text(int status);

/* ── 응답 FLMap 키 상수 ── */
static const char* K_STATUS  = "status";
static const char* K_BODY    = "body";
static const char* K_HEADERS = "headers";
static const char* K_TYPE    = "type";

/* ── 라우트 테이블 ── */
static Route   g_routes[MAX_ROUTES];
static int     g_nroutes = 0;

/* ── 현재 요청 context (per-thread 확장 가능하나 단순 구현은 전역) ── */
static __thread FLValue g_current_req;

/* ───────────────────────────────────────────
   내부 헬퍼
─────────────────────────────────────────── */

static const char* strval(FLValue v) {
    if (v.tag == FL_STRING && v.obj) return ((FLString*)v.obj)->data;
    return "";
}

/* "handle-index" → "handle_index" (hyphen to underscore) */
static void fn_name_to_c(const char* fl_name, char* out, size_t max) {
    size_t i;
    for (i = 0; i < max - 1 && fl_name[i]; i++) {
        out[i] = (fl_name[i] == '-') ? '_' : fl_name[i];
    }
    out[i] = '\0';
}

/* FLMap 빠른 생성 헬퍼 */
static FLValue make_response(int status, const char* content_type, const char* body) {
    FLValue headers = fl_map_new();
    headers = fl_map_set(headers, fl_str_val("Content-Type"), fl_str_val(content_type));

    FLValue resp = fl_map_new();
    resp = fl_map_set(resp, fl_str_val(K_STATUS),  fl_int(status));
    resp = fl_map_set(resp, fl_str_val(K_BODY),    fl_str_val(body));
    resp = fl_map_set(resp, fl_str_val(K_HEADERS), headers);
    return resp;
}

/* ───────────────────────────────────────────
   라우트 등록 API
─────────────────────────────────────────── */

static void register_route(const char* method, FLValue path, FLValue handler) {
    if (g_nroutes >= MAX_ROUTES) {
        fprintf(stderr, "[http] 라우트 한도 초과\n");
        return;
    }
    const char* p = strval(path);
    Route* r = &g_routes[g_nroutes++];
    strncpy(r->method, method, sizeof(r->method) - 1);
    strncpy(r->path,   p,      sizeof(r->path) - 1);
    r->closure = fl_nil();
    r->name = fl_nil();
    r->fn = NULL;
    r->directory=fl_nil();
    r->is_static=0;

    if (handler.tag == FL_FN) {
        /* FL_FN 클로저 핸들러 */
        r->closure = handler;
        fprintf(stderr, "[http] 라우트 등록: %s %s → <closure>\n", method, p);
    } else {
        /* 문자열 이름 → dlsym 룩업 */
        const char* h = strval(handler);
        char cname[256];
        fn_name_to_c(h, cname, sizeof(cname));
        r->name = fl_heap_copy(handler);
        HandlerFn fn = (HandlerFn)dlsym(RTLD_DEFAULT, cname);
        if (!fn) {
            fprintf(stderr, "[http] named handler: %s\n", h);
            return;
        }
        r->fn = fn;
        fprintf(stderr, "[http] 라우트 등록: %s %s → %s\n", method, p, cname);
    }
}

static FLValue invoke_route(Route* route, FLValue req) {
    if(route->is_static)return vh_static(route,req);
    if (route->name.tag == FL_STRING) {
        FLValue fn = fl_named_fn(route->name);
        if (fn.tag == FL_FN) return fl_fn_call(fn, 1, &req);
    }
    if (route->fn) return route->fn(req);
    if (route->closure.tag == FL_FN) return fl_fn_call(route->closure, 1, &req);
    fl_throw(fl_str_val("named HTTP handler is not a function"));
    return fl_nil();
}

FLValue server_get(FLValue path, FLValue handler) {
    register_route("GET",    path, handler); return fl_nil();
}
FLValue server_post(FLValue path, FLValue handler) {
    register_route("POST",   path, handler); return fl_nil();
}
FLValue server_put(FLValue path, FLValue handler) {
    register_route("PUT",    path, handler); return fl_nil();
}
FLValue server_patch(FLValue path, FLValue handler) {
    register_route("PATCH",  path, handler); return fl_nil();
}
FLValue server_delete(FLValue path, FLValue handler) {
    register_route("DELETE", path, handler); return fl_nil();
}

/* ───────────────────────────────────────────
   응답 생성 API
─────────────────────────────────────────── */

FLValue server_html(FLValue html) {
    return make_response(200, "text/html; charset=utf-8", strval(html));
}

FLValue server_text(FLValue text) {
    return make_response(200, "text/plain; charset=utf-8", strval(text));
}

FLValue server_json(FLValue json_str) {
    const char* body;
    if (json_str.tag == FL_STRING && json_str.obj) {
        body = ((FLString*)json_str.obj)->data;
    } else {
        // 맵/벡터면 자동 stringify
        FLValue s = fl_json_stringify(json_str);
        body = (s.tag == FL_STRING && s.obj) ? ((FLString*)s.obj)->data : "{}";
    }
    return make_response(200, "application/json; charset=utf-8", body);
}

FLValue server_status(FLValue code, FLValue body) {
    int status = (code.tag == FL_INT) ? (int)code.i : 200;
    return make_response(status, "text/plain; charset=utf-8", strval(body));
}

FLValue server_redirect(FLValue url) {
    FLValue headers = fl_map_new();
    headers = fl_map_set(headers, fl_str_val("Location"), url);
    FLValue resp = fl_map_new();
    resp = fl_map_set(resp, fl_str_val(K_STATUS),  fl_int(302));
    resp = fl_map_set(resp, fl_str_val(K_BODY),    fl_str_val(""));
    resp = fl_map_set(resp, fl_str_val(K_HEADERS), headers);
    return resp;
}

/* ───────────────────────────────────────────
   요청 접근자 API
─────────────────────────────────────────── */

FLValue server_req_param(FLValue req, FLValue name) {
    FLValue params = fl_map_get(req, fl_str_val("params"));
    if (params.tag == FL_MAP) return fl_map_get(params, name);
    return fl_nil();
}

FLValue server_req_query(FLValue req, FLValue name) {
    FLValue query = fl_map_get(req, fl_str_val("query"));
    if (query.tag == FL_MAP) return fl_map_get(query, name);
    return fl_nil();
}

FLValue fxb_server_req_body(FLValue req) {
    return fl_map_get(req, fl_str_val("body"));
}

FLValue server_req_fields(FLValue req) {
    FLValue fields = fl_map_get(req, fl_str_val("fields"));
    return fields.tag == FL_MAP ? fields : fl_map_new();
}
FLValue server_req_files(FLValue req) {
    FLValue files = fl_map_get(req, fl_str_val("files"));
    return files.tag == FL_VECTOR ? files : fl_vec_new();
}

FLValue server_req_header(FLValue req, FLValue name) {
    FLValue headers = fl_map_get(req, fl_str_val("headers"));
    if (headers.tag == FL_MAP) return fl_map_get(headers, name);
    return fl_nil();
}

FLValue server_req_method(FLValue req) {
    return fl_map_get(req, fl_str_val("method"));
}

FLValue server_req_path(FLValue req) {
    return fl_map_get(req, fl_str_val("path"));
}

/* ───────────────────────────────────────────
   HTTP 파서
─────────────────────────────────────────── */

typedef struct {
    char method[8];
    char path[MAX_URI_LEN+1];
    char query_str[MAX_URI_LEN+1];
    char headers[MAX_HEADERS][2][512];  /* [i][0]=name [i][1]=value */
    int  nheaders;
    const char* body;
    int  body_len;
    int  content_length;
    char content_type[256];
} HttpRequest;

#include "http-form.h"

/* Node querystring semantics: raw path, forgiving UTF-8 query decoding. */
static int query_hex(unsigned char c){return c>='0'&&c<='9'?c-'0':c>='A'&&c<='F'?c-'A'+10:c>='a'&&c<='f'?c-'a'+10:-1;}
static FLValue query_part(const char*src,size_t n){
    char*out=fl_arena_alloc(n+1);if(!out)fl_throw(fl_str_val("query allocation failed"));size_t used=0;
    for(size_t i=0;i<n;i++){
        int a,b;
        if(src[i]=='%'&&i+2<n&&(a=query_hex(src[i+1]))>=0&&(b=query_hex(src[i+2]))>=0){out[used++]=(char)((a<<4)|b);i+=2;}
        else out[used++]=src[i]=='+'?' ':src[i];
    }
    FLValue value=form_bytes(out,used);return fl_v11_builtin(fl_str_val("str"),fl_vec_from(&value,1));
}
static FLValue parse_query(const char*qs){
    FLValue result=fl_map_new();size_t at=0,n=strlen(qs);unsigned pairs=0;
    while(at<n&&pairs++<1000){
        size_t end=at;while(end<n&&qs[end]!='&')end++;
        if(end>at){
            size_t equal=at;while(equal<end&&qs[equal]!='=')equal++;
            FLValue key=query_part(qs+at,equal-at),value=query_part(qs+(equal<end?equal+1:end),equal<end?end-equal-1:0);
            if(fl_truthy(fl_map_has(result,key))){
                FLValue previous=fl_map_get(result,key);
                if(previous.tag!=FL_VECTOR){FLValue two[2]={previous,value};value=fl_vec_from(two,2);}
                else value=fl_vec_push(previous,value);
            }
            result=fl_map_set(result,key,value);
        }
        at=end+1;
    }
    return result;
}

/* HTTP 요청 파싱 */
static int parse_http_request(const char* raw, int raw_len, HttpRequest* req) {
    size_t offset;
    int rc = form_headers((char*)raw, (size_t)raw_len, req, &offset);
    if (rc) return rc;
    return req->body_len == req->content_length ? 0 : form_fail(400, "INCOMPLETE_BODY");
}

/* ───────────────────────────────────────────
   라우트 매칭 + 파라미터 추출
─────────────────────────────────────────── */

/* /users/:id  vs  /users/42  → params["id"] = "42" */
static int match_path(const char*pattern,const char*path,FLValue*params,unsigned*budget){
    if(!*budget)return 0;(*budget)--;
    while(*pattern){
        if(*pattern=='*'){
            pattern++;if(!*pattern)return 1;
            size_t n=strlen(path);do{FLValue trial=*params;if(match_path(pattern,path+n,&trial,budget)){*params=trial;return 1;}if(!*budget)return 0;}while(n--);
            return 0;
        }
        if(*pattern==':'){
            const char*start=++pattern;
            while((*pattern>='a'&&*pattern<='z')||(*pattern>='A'&&*pattern<='Z')||(*pattern>='0'&&*pattern<='9')||*pattern=='_')pattern++;
            if(pattern==start)return 0;
            const char*end=path;while(*end&&*end!='/')end++;
            FLValue key=form_bytes(start,(size_t)(pattern-start));
            if((!*pattern||*pattern=='/')&&end>path){FLValue trial=fl_map_set(*params,key,form_bytes(path,(size_t)(end-path)));if(match_path(pattern,end,&trial,budget)){*params=trial;return 1;}return 0;}
            while(end>path){
                FLValue trial=fl_map_set(*params,key,form_bytes(path,(size_t)(end-path)));
                if(match_path(pattern,end,&trial,budget)){*params=trial;return 1;}if(!*budget)return 0;end--;
            }
            return 0;
        }
        if(*pattern!=*path||!*path)return 0;pattern++;path++;
    }
    return !*path;
}
static int match_route(const Route*r,const char*method,const char*path,FLValue*params){
    if(strcmp(r->method,"*")&&strcmp(r->method,method))return 0;
    *params=fl_map_new();unsigned budget=100000;return match_path(r->path,path,params,&budget);
}

#include "v11-http.h"

static const char* status_text(int code) {
    switch (code) {
        case 200: return "OK";
        case 201: return "Created";
        case 204: return "No Content";
        case 301: return "Moved Permanently";
        case 302: return "Found";
        case 400: return "Bad Request";
        case 401: return "Unauthorized";
        case 403: return "Forbidden";
        case 404: return "Not Found";
        case 405: return "Method Not Allowed";
        case 422: return "Unprocessable Entity";
        case 500: return "Internal Server Error";
        default:  return "Unknown";
    }
}

/* keep_alive=1이면 Connection: keep-alive 헤더 포함 */
static int send_response(int client_fd,FLValue response,int keep_alive,int head_only,int*actual_status){
    VHttpPrepared prepared;int failed=0;
    if(fl_try_top>=FL_TRY_MAX)failed=1;
    else{
        FLTryFrame*frame=&fl_try_stack[fl_try_top++];
        if(setjmp(frame->buf)==0){vh_prepare_response(response,keep_alive,&prepared);fl_try_top--;}
        else{fl_try_top--;failed=1;}
    }
    if(failed){
        static const char body[]="{\"error\":\"Internal error\"}";
        char header[256];int n=snprintf(header,sizeof(header),"HTTP/1.1 500 Internal Server Error\r\nContent-Type: application/json\r\nContent-Length: %zu\r\nConnection: close\r\n\r\n",sizeof(body)-1);
        *actual_status=500;transport_send(client_fd,header,(size_t)n);
        if(!head_only)transport_send(client_fd,body,sizeof(body)-1);
        return 0;
    }
    *actual_status=prepared.status;
    if(!transport_send(client_fd,prepared.header,prepared.header_len))return 0;
    FLString*body=(FLString*)prepared.body.obj;
    return head_only||!body->len||transport_send(client_fd,body->data,body->len);
}

/* ───────────────────────────────────────────
   요청 → FLMap 변환
─────────────────────────────────────────── */

static FLValue request_header_value(const char*raw){
    size_t n=strlen(raw);char*utf8=fl_arena_alloc(n*2+1);if(!utf8)fl_throw(fl_str_val("header allocation failed"));size_t used=0;
    for(size_t i=0;i<n;i++){unsigned char c=(unsigned char)raw[i];if(c<128)utf8[used++]=c;else{utf8[used++]=(char)(0xc0|(c>>6));utf8[used++]=(char)(0x80|(c&63));}}
    return form_bytes(utf8,used);
}
static int request_singleton(const char*name){
    static const char*names[]={"authorization","host","user-agent","referer","from","if-modified-since","if-unmodified-since","age","content-length","content-type","etag","expires","last-modified","location","max-forwards","proxy-authorization","retry-after","server"};
    for(size_t i=0;i<sizeof(names)/sizeof(names[0]);i++)if(!strcmp(name,names[i]))return 1;return 0;
}
static FLValue make_req_map(HttpRequest*hr,FLValue params,FormContext*form){
    FLValue headers=fl_map_new();
    for(int i=0;i<hr->nheaders;i++){
        char lower[512];size_t j=0;for(;hr->headers[i][0][j];j++)lower[j]=(char)tolower((unsigned char)hr->headers[i][0][j]);lower[j]=0;
        FLValue key=fl_str_val(lower),value=request_header_value(hr->headers[i][1]);
        int exists=fl_truthy(fl_map_has(headers,key));
        if(exists&&request_singleton(lower))continue;
        if(!strcmp(lower,"set-cookie")){
            FLValue array=exists?fl_map_get(headers,key):fl_vec_new();value=fl_vec_push(array,value);
        }else if(exists)value=fl_str_n(3,fl_map_get(headers,key),fl_str_val(!strcmp(lower,"cookie")?"; ":", "),value);
        headers=fl_map_set(headers,key,value);
    }
    FLValue body=form->fields;
    if(!form_type(hr->content_type)){
        FLValue raw=form_bytes(hr->body_len?hr->body:"",(size_t)hr->body_len);
        raw=fl_v11_builtin(fl_str_val("str"),fl_vec_from(&raw,1));body=raw;
        const char*ct=hr->content_type;
        if(!strncasecmp(ct,"application/json",16)&&(!ct[16]||ct[16]==';'||ct[16]==' '||ct[16]=='\t')){
            FLValue parsed=fl_v11_json_try_parse(raw);
            if(fl_truthy(fl_vec_get(parsed,fl_int(0))))body=fl_vec_get(parsed,fl_int(1));
        }
    }
    FLValue req=fl_map_new();
    req=vh_set(req,"method",fl_str_val(hr->method));req=vh_set(req,"path",fl_str_val(hr->path));req=vh_set(req,"query",parse_query(hr->query_str));
    req=vh_set(req,"params",params);req=vh_set(req,"headers",headers);req=vh_set(req,"body",body);
    req=vh_set(req,"fields",form->fields);req=vh_set(req,"files",form->values);return req;
}

/* ───────────────────────────────────────────
   연결 처리 (단일 요청)
─────────────────────────────────────────── */

typedef struct { int fd; } ConnArg;

/*
 * keep-alive 설정:
 * - HTTP/1.1 기본 keep-alive, HTTP/1.0 기본 close
 * - 클라이언트 Connection 헤더 우선
 * - 최대 100 요청 또는 60초 유휴시간 후 자동 close
 */
#define KEEPALIVE_MAX_REQUESTS 100
#define KEEPALIVE_IDLE_SEC     60

/* 단일 요청 수신
 *
 * 반환값:
 *   >0   : 수신 바이트 수 (정상)
 *   -408 : 헤더 미완성 timeout (Slowloris) → 408
 *   -413 : Content-Length 초과 → 413
 *   -414 : URI 너무 김 → 414
 *   -431 : 헤더 총합 초과 → 431
 *   <=0  : 연결 종료 or 기타 오류 → break
 */

/* Connection 헤더에서 keep-alive 여부 파악
 * HTTP/1.1: 기본 keep-alive (Connection: close 있으면 close)
 * HTTP/1.0: 기본 close (Connection: keep-alive 있으면 keep-alive)
 */
static int check_keep_alive(HttpRequest* hr, const char* raw) {
    /* HTTP 버전 확인 */
    int is_11 = (strstr(raw, "HTTP/1.1") != NULL);
    int want_keepalive = is_11 ? 1 : 0;   /* 1.1: default on, 1.0: default off */

    for (int i = 0; i < hr->nheaders; i++) {
        if (strcasecmp(hr->headers[i][0], "Connection") == 0) {
            if (strcasecmp(hr->headers[i][1], "keep-alive") == 0)
                want_keepalive = 1;
            else if (strcasecmp(hr->headers[i][1], "close") == 0)
                want_keepalive = 0;
            break;
        }
    }
    return want_keepalive;
}

/* websocket.c에서 선언 */
extern int ws_is_upgrade_request(const char* raw);
extern int ws_handle_upgrade(int fd, const char* raw, int raw_len);

/* sse.c에서 선언 */
extern int sse_is_sse_request(const char* raw);
extern int sse_handle_request(int fd, const char* raw, int raw_len);

static void* handle_connection(void* arg) {
    ConnArg* ca = (ConnArg*)arg;
    int client_fd = ca->fd;
    free(ca);

    /* TCP_NODELAY: Nagle 비활성화
     * 헤더+바디를 2번 send() 할 때 Nagle+DelayedACK로 40ms 지연 방지
     * keep-alive 연결에서 특히 중요 */
    int nodelay = 1;
    setsockopt(client_fd, IPPROTO_TCP, TCP_NODELAY, &nodelay, sizeof(nodelay));

    /* keep-alive 유휴 타임아웃 설정 */
    struct timeval tv = { .tv_sec = KEEPALIVE_IDLE_SEC, .tv_usec = 0 };
    setsockopt(client_fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

    char* raw = NULL;

    int req_count = 0;

    while (req_count < KEEPALIVE_MAX_REQUESTS) {

        /* ── 1. 요청 수신 ── */
        free(raw); raw = NULL;
        http_error = NULL;
        int total = form_receive(client_fd, &raw);
        if (total < 0) {
            char body[96], response[512];
            snprintf(body,sizeof body,"{\"error\":\"%s\"}",http_error ? http_error : "UPLOAD_IO_FAILED");
            snprintf(response,sizeof response,"HTTP/1.1 %d Error\r\nContent-Type: application/json\r\nContent-Length: %zu\r\nConnection: close\r\n\r\n%s",-total,strlen(body),body);
            transport_send(client_fd,response,strlen(response));
            break;
        }
        if (!total) break;

        /* Protocol detection must not interpret uploaded bytes as headers. */
        char* body_start = strstr(raw, "\r\n\r\n") + 4;
        char first_body_byte = *body_start;
        *body_start = '\0';
        int can_upgrade = !http_ssl && !strncmp(raw, "GET ", 4);
        int wants_sse = can_upgrade && sse_is_sse_request(raw);
        int wants_ws = can_upgrade && ws_is_upgrade_request(raw);
        *body_start = first_body_byte;

        /* ── 2a. SSE 감지 (WS보다 먼저 체크) ── */
        if (wants_sse) {
            if (!sse_handle_request(client_fd, raw, total))
                goto normal_http;   /* 등록된 라우트 없으면 일반 HTTP */
            break;  /* SSE는 keep-alive 루프 탈출 */
        }

        /* ── 2b. WebSocket 업그레이드 감지 ── */
        if (wants_ws) {
            ws_handle_upgrade(client_fd, raw, total);
            break;  /* WS는 keep-alive 루프 탈출 (핸들러가 fd 관리) */
        }

        normal_http:;

        /* ── 3. 아레나 시작 (요청 단위 메모리 관리) ── */
        fl_arena_begin();

        /* ── 4. 파싱 ── */
        HttpRequest hr;
        if (parse_http_request(raw, total, &hr) < 0) {
            const char* err = "HTTP/1.1 400 Bad Request\r\nContent-Length: 0\r\n\r\n";
            transport_send(client_fd, err, strlen(err));
            fl_arena_end();
            break;
        }

        /* ── 5. keep-alive 결정 ── */
        int keep_alive = !http_ssl && !form_type(hr.content_type) && check_keep_alive(&hr, raw);
        FormContext form;
        int form_result = http_form_parse(&form, &hr);

        /* ── 6. 디버그 로그 ── */
        struct timespec t_start;
        clock_gettime(CLOCK_MONOTONIC, &t_start);

        /* ── 6. 라우트 매칭 ── */
        FLValue params = fl_nil();
        Route* matched = NULL;
        for (int i = 0; i < g_nroutes; i++) {
            FLValue p;
            if (match_route(&g_routes[i], hr.method, hr.path, &p)) {
                matched = &g_routes[i];
                params  = p;
                break;
            }
        }

        /* Dynamic path segments may carry private preview tokens. */
        const char* log_path = matched ? matched->path : "[unmatched]";
        fl_log_request(hr.method, log_path, hr.body, hr.body_len,
                       hr.content_type[0] ? hr.content_type : NULL);
        FLValue resp=fl_nil();
        if(form_result){
            FLValue error=vh_set(fl_map_new(),"error",fl_str_val(http_error));resp=vh_json_response(-form_result,error);keep_alive=0;
        }else{
            if(fl_try_top>=FL_TRY_MAX){resp=vh_json_response(500,fl_str_val("Internal error"));keep_alive=0;}
            else{
                FLTryFrame*frame=&fl_try_stack[fl_try_top++];
                if(setjmp(frame->buf)==0){
                    FLValue req=make_req_map(&hr,fl_map_new(),&form);g_current_req=req;
                    for(int i=0;i<vh_nmiddlewares;i++){
                        FLValue ignored;
                        if(match_route(&vh_middlewares[i],hr.method,hr.path,&ignored)){
                            FLValue value=invoke_route(&vh_middlewares[i],req);
                            if(value.tag!=FL_NIL){resp=value;break;}
                        }
                    }
                    if(resp.tag==FL_NIL){
                        if(matched){req=vh_set(req,"params",params);g_current_req=req;resp=invoke_route(matched,req);}
                        else{
                            FLValue error=vh_set(fl_map_new(),"error",fl_str_val("Not Found"));error=vh_set(error,"path",fl_str_val(hr.path));resp=vh_json_response(404,error);
                        }
                    }
                    fl_try_top--;
                }else{
                    fl_try_top--;resp=vh_json_response(500,vh_set(fl_map_new(),"error",fl_str_val("Internal error")));keep_alive=0;
                    fprintf(stderr,"[FreeLang 500] HTTP handler failed\n");
                }
            }
        }

        if (!form_cleanup(&form)) {
            resp = make_response(500,"application/json","{\"error\":\"UPLOAD_CLEANUP_FAILED\"}");
            keep_alive = 0;
        }

        /* ── 8. 응답 전송 ── */
        struct timespec t_end;
        clock_gettime(CLOCK_MONOTONIC, &t_end);
        long elapsed_ms = (t_end.tv_sec - t_start.tv_sec) * 1000
                        + (t_end.tv_nsec - t_start.tv_nsec) / 1000000;
        int status_code = 200;
        if(!send_response(client_fd,resp,keep_alive,!strcmp(hr.method,"HEAD"),&status_code))keep_alive=0;
        fl_log_response(status_code, log_path, elapsed_ms);
        fl_arena_stats();


        /* ── 9. 아레나 종료 ── */
        g_current_req = fl_nil();
        fl_arena_end();

        req_count++;
        if (!keep_alive) break;

    }   /* while keep_alive */

    free(raw);
    if (http_ssl) SSL_shutdown(http_ssl);
    close(client_fd);
    return NULL;
}

/* ═══════════════════════════════════════════════════════
   스레드 풀 + Graceful Shutdown
   ═══════════════════════════════════════════════════════

   환경변수:
     FL_WORKERS=N    워커 스레드 수 (기본: CPU코어 수, 최대 256)
     FL_QUEUE=N      작업 큐 크기 (기본: 1024)
     FL_CONN_TIMEOUT=N  연결 타임아웃 초 (기본: 30)
     FL_SHUTDOWN_TIMEOUT=N  shutdown 대기 초 (기본: 30)
*/

#define POOL_MAX_WORKERS 256
#define POOL_DEFAULT_WORKERS 16
#define POOL_DEFAULT_QUEUE   1024

/* ── 작업 큐 항목 ── */
typedef struct QueueItem {
    int fd;
    SSL* ssl;
    struct QueueItem* next;
} QueueItem;

/* ── 스레드 풀 상태 ── */
static struct {
    pthread_t*      threads;
    int             n_workers;
    int             conn_timeout;

    QueueItem*      head;
    QueueItem*      tail;
    int             queue_len;
    int             queue_cap;

    pthread_mutex_t lock;
    pthread_cond_t  cond_work;   /* 새 작업 */
    pthread_cond_t  cond_drain;  /* 큐 비어짐 */

    volatile int    shutdown;    /* 1 = graceful shutdown 시작 */
    volatile int    active;      /* 현재 처리 중인 연결 수 */
} g_pool;

static volatile int g_server_fd = -1;  /* SIGTERM에서 close용 */

/* ── 503 즉시 반환 ── */
static void send_503(int fd) {
    const char* r = "HTTP/1.1 503 Service Unavailable\r\n"
                    "Content-Type: application/json\r\n"
                    "Content-Length: 35\r\n"
                    "Retry-After: 1\r\n\r\n"
                    "{\"error\":\"server busy, try again\"}";
    send(fd, r, strlen(r), 0);
    close(fd);
}

/* ── 워커 스레드 루프 ── */
static void* worker_loop(void* arg) {
    (void)arg;
    for (;;) {
        pthread_mutex_lock(&g_pool.lock);
        while (g_pool.head == NULL && !g_pool.shutdown)
            pthread_cond_wait(&g_pool.cond_work, &g_pool.lock);

        if (g_pool.head == NULL && g_pool.shutdown) {
            pthread_mutex_unlock(&g_pool.lock);
            break;
        }

        QueueItem* item = g_pool.head;
        g_pool.head = item->next;
        if (g_pool.head == NULL) g_pool.tail = NULL;
        g_pool.queue_len--;
        g_pool.active++;
        pthread_mutex_unlock(&g_pool.lock);

        /* 연결 처리 */
        int fd = item->fd;
        SSL* ssl = item->ssl;
        free(item);
        http_ssl = ssl;
        struct timeval handshake_timeout = {HEADER_RECV_TIMEOUT, 0};
        if (ssl) {
            setsockopt(fd,SOL_SOCKET,SO_RCVTIMEO,&handshake_timeout,sizeof handshake_timeout);
            setsockopt(fd,SOL_SOCKET,SO_SNDTIMEO,&handshake_timeout,sizeof handshake_timeout);
        }
        ConnArg* ca = NULL;
        if ((!ssl || SSL_accept(ssl) > 0) && (ca = malloc(sizeof *ca))) {
            ca->fd = fd;
            handle_connection(ca);
        } else close(fd);
        if (ssl) SSL_free(ssl);
        http_ssl = NULL;

        pthread_mutex_lock(&g_pool.lock);
        g_pool.active--;
        if (g_pool.active == 0 && g_pool.queue_len == 0)
            pthread_cond_broadcast(&g_pool.cond_drain);
        pthread_mutex_unlock(&g_pool.lock);
    }
    return NULL;
}

/* ── SIGTERM/SIGINT 핸들러 ── */
static void handle_shutdown_signal(int sig) {
    (void)sig;
    fl_log(1, "http", "shutdown 신호 수신 — graceful 종료 시작");
    g_pool.shutdown = 1;
    /* accept 루프 깨우기 */
    if (g_server_fd >= 0) {
        shutdown(g_server_fd, SHUT_RDWR);
        close(g_server_fd);
        g_server_fd = -1;
    }
    /* 워커 깨우기 */
    pthread_cond_broadcast(&g_pool.cond_work);
}

/* ── 스레드 풀 초기화 ── */
static int pool_init(void) {
    /* 워커 수 결정 */
    int n = POOL_DEFAULT_WORKERS;
    const char* env_w = getenv("FL_WORKERS");
    if (env_w) n = atoi(env_w);
    if (n <= 0) n = 1;
    if (n > POOL_MAX_WORKERS) n = POOL_MAX_WORKERS;

    const char* env_q = getenv("FL_QUEUE");
    g_pool.queue_cap = env_q ? atoi(env_q) : POOL_DEFAULT_QUEUE;
    if (g_pool.queue_cap < 8) g_pool.queue_cap = 8;

    const char* env_t = getenv("FL_CONN_TIMEOUT");
    g_pool.conn_timeout = env_t ? atoi(env_t) : 30;

    g_pool.n_workers = n;
    g_pool.threads   = malloc(n * sizeof(pthread_t));
    g_pool.head = g_pool.tail = NULL;
    g_pool.queue_len = 0;
    g_pool.shutdown  = 0;
    g_pool.active    = 0;

    pthread_mutex_init(&g_pool.lock, NULL);
    pthread_cond_init(&g_pool.cond_work, NULL);
    pthread_cond_init(&g_pool.cond_drain, NULL);

    for (int i = 0; i < n; i++) {
        if (pthread_create(&g_pool.threads[i], NULL, worker_loop, NULL) != 0) {
            fprintf(stderr, "[http] 워커 스레드 %d 생성 실패\n", i);
            g_pool.n_workers = i;
            break;
        }
    }

    fprintf(stderr, "[http] 스레드 풀: %d 워커, 큐 %d, 타임아웃 %ds\n",
            g_pool.n_workers, g_pool.queue_cap, g_pool.conn_timeout);
    return 0;
}

/* ── 스레드 풀 종료 (graceful) ── */
static void pool_shutdown(int timeout_sec) {
    fprintf(stderr, "[http] graceful shutdown (최대 %ds 대기)...\n", timeout_sec);

    pthread_mutex_lock(&g_pool.lock);
    g_pool.shutdown = 1;
    pthread_cond_broadcast(&g_pool.cond_work);

    /* 진행 중 요청 완료 대기 */
    if (g_pool.active > 0 || g_pool.queue_len > 0) {
        struct timespec deadline;
        clock_gettime(CLOCK_REALTIME, &deadline);
        deadline.tv_sec += timeout_sec;
        pthread_cond_timedwait(&g_pool.cond_drain, &g_pool.lock, &deadline);
    }
    pthread_mutex_unlock(&g_pool.lock);

    /* 워커 스레드 합류 */
    pthread_cond_broadcast(&g_pool.cond_work);
    for (int i = 0; i < g_pool.n_workers; i++)
        pthread_join(g_pool.threads[i], NULL);

    fprintf(stderr, "[http] 종료 완료 (처리 중 연결: %d)\n", g_pool.active);
}

/* ───────────────────────────────────────────
   server_start — 메인 루프 (스레드 풀)
─────────────────────────────────────────── */

FLValue server_start(FLValue port_val) {
    form_recover_startup();
    /* 수리(2026-06-14): 이전엔 FL_INT가 아니면 조용히 8080으로 떨어져
     * (server_start (str-to-num env))·(server_start (floor x)) 가 무력화됐다(트랩 #9,
     * str-to-num/floor 은 FL_FLOAT 반환). 이제 FLOAT·STRING 도 명시 수용. */
    int port = 8080;
    if      (port_val.tag == FL_INT)    port = (int)port_val.i;
    else if (port_val.tag == FL_FLOAT)  port = (int)port_val.f;
    else if (port_val.tag == FL_STRING) { int p = atoi(strval(port_val)); if (p > 0) port = p; }

    /* 시그널 설정 */
    signal(SIGPIPE, SIG_IGN);
    signal(SIGTERM, handle_shutdown_signal);
    signal(SIGINT,  handle_shutdown_signal);

    /* 스레드 풀 초기화 */
    pool_init();

    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        fprintf(stderr, "[http] socket() 실패: %s\n", strerror(errno));
        return fl_nil();
    }
    g_server_fd = server_fd;

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family      = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port        = htons((uint16_t)port);

    if (bind(server_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        fprintf(stderr, "[http] bind(%d) 실패: %s\n", port, strerror(errno));
        close(server_fd);
        return fl_nil();
    }

    listen(server_fd, 512);   /* backlog 증가 */
    fprintf(stderr, "[http] 서버 시작: http://0.0.0.0:%d\n", port);
    fl_debug_banner("fl-app", port);

    /* ── accept 루프 ── */
    while (!g_pool.shutdown) {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        int client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_len);
        if (client_fd < 0) {
            if (errno == EINTR) continue;
            break;   /* shutdown이 server_fd를 닫으면 여기서 탈출 */
        }

        /* 송신 타임아웃 설정 (SO_RCVTIMEO는 recv_one_request에서 설정) */
        struct timeval tv = { g_pool.conn_timeout, 0 };
        setsockopt(client_fd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));

        /* 큐 엔큐 + 연결 수 한도 통합 체크 */
        pthread_mutex_lock(&g_pool.lock);
        int total_conns = g_pool.queue_len + g_pool.active;
        if (total_conns >= MAX_CONNECTIONS || g_pool.queue_len >= g_pool.queue_cap) {
            pthread_mutex_unlock(&g_pool.lock);
            fl_log_error("http", "연결 한도(%d) 초과(큐=%d,활성=%d) → 503",
                         MAX_CONNECTIONS, g_pool.queue_len, g_pool.active);
            send_503(client_fd);
            continue;
        }
        QueueItem* item = malloc(sizeof(QueueItem));
        if (!item) {
            pthread_mutex_unlock(&g_pool.lock);
            send_503(client_fd);
            continue;
        }
        item->ssl = NULL;
        item->fd   = client_fd;
        item->next = NULL;
        if (g_pool.tail) g_pool.tail->next = item;
        else             g_pool.head = item;
        g_pool.tail = item;
        g_pool.queue_len++;
        pthread_cond_signal(&g_pool.cond_work);
        pthread_mutex_unlock(&g_pool.lock);
    }

    /* ── Graceful shutdown ── */
    if (g_server_fd >= 0) { close(g_server_fd); g_server_fd = -1; }

    const char* env_st = getenv("FL_SHUTDOWN_TIMEOUT");
    int shutdown_timeout = env_st ? atoi(env_st) : 30;
    pool_shutdown(shutdown_timeout);

    return fl_nil();
}

/* ── fl_http_route / fl_http_start — kebab-case 앱 호환 alias ── */
/* (server-get "/path" "handler")  → fl_http_route("GET", "/path", "handler") */
/* (server-post ...)               → fl_http_route("POST", ...) */
/* (server-start PORT)             → fl_http_start(PORT) */

FLValue fl_http_route(FLValue method_v, FLValue path_v, FLValue handler_v) {
    if (method_v.tag != FL_STRING) return fl_nil();
    const char* method = ((FLString*)method_v.obj)->data;
    if      (strcmp(method, "GET")    == 0) return server_get(path_v, handler_v);
    else if (strcmp(method, "POST")   == 0) return server_post(path_v, handler_v);
    else if (strcmp(method, "PUT")    == 0) return server_put(path_v, handler_v);
    else if (strcmp(method, "PATCH")  == 0) return server_patch(path_v, handler_v);
    else if (strcmp(method, "DELETE") == 0) return server_delete(path_v, handler_v);
    return fl_nil();
}

FLValue fl_http_start(FLValue port) { return server_start(port); }

/* server-json (kebab) → server_json (이미 있음, 하지만 alias 명시) */
/* server-text, server-html, server-status, server-redirect도 동일 */
/* cgc-bin이 이미 _ 로 변환하므로 추가 alias 불필요 */

/* ── fl_resp_* alias — kebab-case response 함수 ── */
FLValue fl_resp_html(FLValue html)              { return server_html(html); }
FLValue fl_resp_json(FLValue json)              { return server_json(json); }
FLValue fl_resp_text(FLValue text)              { return server_text(text); }
FLValue fl_resp_status(FLValue code, FLValue b) { return server_status(code, b); }
FLValue fl_resp_redirect(FLValue url)           { return server_redirect(url); }

/* ───────────────────────────────────────────
   HTTP 클라이언트 (outbound)
   fl_http_get(url) → 응답 본문 FL String 반환 (실패 시 nil)
   HTTP only (TLS 미지원). 단순 GET, Connection: close.

   실패 시 nil을 반환하되 *왜* 실패했는지 stderr로 항상 진단한다
   (FL_DEBUG 무관). 침묵하는 nil은 디버깅 지옥이므로.
   끄려면: 환경변수 FL_HTTP_QUIET=1
─────────────────────────────────────────── */
static void httpget_diag(const char* fmt, ...) {
    if (getenv("FL_HTTP_QUIET")) return;
    va_list ap; va_start(ap, fmt);
    fprintf(stderr, "[http-get] ");
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    va_end(ap);
}

FLValue fl_http_get(FLValue url_v) {
    if (url_v.tag != FL_STRING || !url_v.obj) {
        httpget_diag("인자가 문자열이 아님 (URL 필요)");
        return fl_nil();
    }
    const char* url = ((FLString*)url_v.obj)->data;

    /* URL 파싱: http(s)://host[:port][/path] */
    int use_ssl = 0;
    const char* p = url;
    if (strncmp(p, "https://", 8) == 0)      { p += 8; use_ssl = 1; }
    else if (strncmp(p, "http://", 7) == 0)  { p += 7; }
    else { /* scheme 없으면 HTTP */ }

    char host[256]; int port = use_ssl ? 443 : 80; char path[2048] = "/";
    int hi = 0;
    while (*p && *p != ':' && *p != '/' && hi < 255) host[hi++] = *p++;
    host[hi] = 0;
    if (host[0] == 0) {
        httpget_diag("URL에서 host를 파싱하지 못함: %s", url);
        return fl_nil();
    }
    if (*p == ':') {
        p++; port = 0;
        while (*p >= '0' && *p <= '9') port = port * 10 + (*p++ - '0');
    }
    if (*p == '/') { strncpy(path, p, sizeof(path) - 1); path[sizeof(path) - 1] = 0; }

    /* resolve + connect */
    char port_str[16]; snprintf(port_str, sizeof(port_str), "%d", port);
    struct addrinfo hints, *res = NULL;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family   = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    int gai = getaddrinfo(host, port_str, &hints, &res);
    if (gai != 0 || !res) {
        httpget_diag("DNS 해석 실패 %s:%s — %s", host, port_str, gai_strerror(gai));
        return fl_nil();
    }

    int fd = -1, last_errno = 0, tried = 0;
    for (struct addrinfo* ai = res; ai; ai = ai->ai_next) {
        tried++;
        fd = socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol);
        if (fd < 0) { last_errno = errno; continue; }
        if (connect(fd, ai->ai_addr, ai->ai_addrlen) == 0) break;
        last_errno = errno;
        close(fd); fd = -1;
    }
    freeaddrinfo(res);
    if (fd < 0) {
        httpget_diag("연결 실패 %s:%d (후보 %d개 시도) — %s",
               host, port, tried, strerror(last_errno));
        return fl_nil();
    }

    /* ── SSL 핸드셰이크 (HTTPS) ───────────────────────────────────── */
    SSL_CTX* ctx = NULL;
    SSL*     ssl = NULL;
    if (use_ssl) {
        SSL_library_init();
        SSL_load_error_strings();
        ctx = SSL_CTX_new(TLS_client_method());
        if (!ctx) { close(fd); return fl_nil(); }
        SSL_CTX_set_verify(ctx, SSL_VERIFY_NONE, NULL);  /* 인증서 검증 생략 */
        ssl = SSL_new(ctx);
        SSL_set_fd(ssl, fd);
        SSL_set_tlsext_host_name(ssl, host);
        if (SSL_connect(ssl) <= 0) {
            httpget_diag("TLS 핸드셰이크 실패: %s", host);
            SSL_free(ssl); SSL_CTX_free(ctx); close(fd);
            return fl_nil();
        }
    }

    /* send request */
    char req[4096];
    int rn = snprintf(req, sizeof(req),
        "GET %s HTTP/1.1\r\nHost: %s\r\nConnection: close\r\n"
        "User-Agent: fx-http/1.0\r\nAccept: */*\r\n\r\n", path, host);
    ssize_t sent = 0;
    while (sent < rn) {
        ssize_t s = use_ssl
            ? SSL_write(ssl, req + sent, rn - sent)
            : send(fd, req + sent, rn - sent, MSG_NOSIGNAL);
        if (s <= 0) {
            httpget_diag("요청 전송 실패 %s", host);
            if (use_ssl) { SSL_free(ssl); SSL_CTX_free(ctx); }
            close(fd); return fl_nil();
        }
        sent += s;
    }

    /* recv 전체 응답 */
    size_t cap = 8192, len = 0;
    char* buf = malloc(cap);
    if (!buf) {
        if (use_ssl) { SSL_free(ssl); SSL_CTX_free(ctx); }
        close(fd); return fl_nil();
    }
    ssize_t n;
    while ((n = use_ssl
               ? SSL_read(ssl, buf + len, (int)(cap - len - 1))
               : recv(fd, buf + len, cap - len - 1, 0)) > 0) {
        len += (size_t)n;
        if (len + 1 >= cap) {
            cap *= 2;
            char* nb = realloc(buf, cap);
            if (!nb) { free(buf);
                if (use_ssl) { SSL_free(ssl); SSL_CTX_free(ctx); }
                close(fd); return fl_nil(); }
            buf = nb;
        }
    }
    buf[len] = 0;
    if (use_ssl) { SSL_free(ssl); SSL_CTX_free(ctx); }
    close(fd);

    /* 본문만 추출 */
    char* sep = strstr(buf, "\r\n\r\n");
    const char* body = sep ? sep + 4 : buf;

    /* chunked transfer encoding 디코딩 */
    int is_chunked = 0;
    char* hdrs_end = sep ? sep : buf;
    char* hdr_search = buf;
    while (hdr_search < hdrs_end) {
        if (strncasecmp(hdr_search, "Transfer-Encoding: chunked", 26) == 0) {
            is_chunked = 1; break;
        }
        char* nl = memchr(hdr_search, '\n', (size_t)(hdrs_end - hdr_search));
        if (!nl) break;
        hdr_search = nl + 1;
    }
    if (is_chunked) {
        size_t dcap = strlen(body) + 1;
        char* decoded = malloc(dcap);
        size_t dlen = 0;
        const char* p = body;
        while (*p) {
            /* 청크 크기 (hex) */
            char* end;
            long chunk_size = strtol(p, &end, 16);
            if (end == p || chunk_size < 0) break;
            if (chunk_size == 0) break;
            p = end;
            if (*p == '\r') p++;
            if (*p == '\n') p++;
            /* 데이터 복사 */
            if (dlen + (size_t)chunk_size + 1 >= dcap) {
                dcap = dlen + (size_t)chunk_size + 64;
                decoded = realloc(decoded, dcap);
            }
            memcpy(decoded + dlen, p, (size_t)chunk_size);
            dlen += (size_t)chunk_size;
            p += chunk_size;
            if (*p == '\r') p++;
            if (*p == '\n') p++;
        }
        decoded[dlen] = '\0';
        FLValue out = fl_str_val(decoded);
        free(decoded);
        free(buf);
        return out;
    }

    FLValue out = fl_str_val(body);
    free(buf);
    return out;
}

/* ───────────────────────────────────────────
   HTTPS 서버 (server_start_tls)
   server_start_tls(port, "/path/to/cert.pem", "/path/to/key.pem")
   → HTTP와 동일한 라우트 + SSL 래핑
─────────────────────────────────────────── */

/* server_start_tls(port, cert_path, key_path) */
FLValue server_start_tls(FLValue port_val, FLValue cert_val, FLValue key_val) {
    form_recover_startup();
    int port = 8443;
    if      (port_val.tag == FL_INT)   port = (int)port_val.i;
    else if (port_val.tag == FL_FLOAT) port = (int)port_val.f;

    const char* cert_path = strval(cert_val);
    const char* key_path  = strval(key_val);

    /* SSL 초기화 */
    SSL_library_init();
    SSL_load_error_strings();
    SSL_CTX* ctx = SSL_CTX_new(TLS_server_method());
    if (!ctx) {
        fprintf(stderr, "[tls] SSL_CTX_new 실패\n");
        return fl_nil();
    }
    if (SSL_CTX_use_certificate_file(ctx, cert_path, SSL_FILETYPE_PEM) <= 0) {
        fprintf(stderr, "[tls] 인증서 로드 실패: %s\n", cert_path);
        SSL_CTX_free(ctx); return fl_nil();
    }
    if (SSL_CTX_use_PrivateKey_file(ctx, key_path, SSL_FILETYPE_PEM) <= 0) {
        fprintf(stderr, "[tls] 개인키 로드 실패: %s\n", key_path);
        SSL_CTX_free(ctx); return fl_nil();
    }

    signal(SIGPIPE, SIG_IGN);
    signal(SIGTERM, handle_shutdown_signal);
    signal(SIGINT, handle_shutdown_signal);
    pool_init();

    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family      = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port        = htons((uint16_t)port);

    if (bind(server_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        fprintf(stderr, "[tls] bind(%d) 실패: %s\n", port, strerror(errno));
        close(server_fd); SSL_CTX_free(ctx); return fl_nil();
    }
    g_server_fd = server_fd;
    listen(server_fd, 128);
    fprintf(stderr, "[tls] HTTPS 서버 시작: https://0.0.0.0:%d\n", port);

    /* accept 루프 */
    while (!g_pool.shutdown) {
        struct sockaddr_in caddr;
        socklen_t clen = sizeof(caddr);
        int cfd = accept(server_fd, (struct sockaddr*)&caddr, &clen);
        if (cfd < 0) { if (errno == EINTR) continue; break; }

        SSL* ssl = SSL_new(ctx);
        if (!ssl) { close(cfd); continue; }
        SSL_set_fd(ssl, cfd);
        pthread_mutex_lock(&g_pool.lock);
        if (g_pool.queue_len >= g_pool.queue_cap || g_pool.active + g_pool.queue_len >= MAX_CONNECTIONS) {
            pthread_mutex_unlock(&g_pool.lock); SSL_free(ssl); close(cfd); continue;
        }
        QueueItem* item = malloc(sizeof *item);
        if (!item) { pthread_mutex_unlock(&g_pool.lock); SSL_free(ssl); close(cfd); continue; }
        item->fd = cfd; item->ssl = ssl; item->next = NULL;
        if (g_pool.tail) g_pool.tail->next = item; else g_pool.head = item;
        g_pool.tail = item; g_pool.queue_len++;
        pthread_cond_signal(&g_pool.cond_work);
        pthread_mutex_unlock(&g_pool.lock);
    }

    if (g_server_fd >= 0) { close(g_server_fd); g_server_fd = -1; }
    pool_shutdown(30);
    SSL_CTX_free(ctx);
    return fl_nil();
}
