/*
 * http.c — FreeLang C 런타임 HTTP 서버
 *
 * 구현 함수:
 *   server_get / server_post / server_put / server_patch / server_delete
 *   server_html / server_text / server_status / server_json
 *   server_start
 *   server_req_param / server_req_query / server_req_body / server_req_header
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
#include <arpa/inet.h>

/* ───────────────────────────────────────────
   내부 타입
─────────────────────────────────────────── */

#define MAX_ROUTES      256
#define MAX_HEADERS     64
#define RECV_BUF        65536
#define SEND_BUF        65536

typedef FLValue (*HandlerFn)(FLValue req);

typedef struct {
    char   method[8];      /* GET POST PUT PATCH DELETE */
    char   path[512];      /* /api/users/:id */
    HandlerFn fn;
} Route;

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

static void register_route(const char* method, FLValue path, FLValue handler_name) {
    if (g_nroutes >= MAX_ROUTES) {
        fprintf(stderr, "[http] 라우트 한도 초과\n");
        return;
    }
    const char* p = strval(path);
    const char* h = strval(handler_name);

    /* dlsym으로 핸들러 함수 룩업 */
    char cname[256];
    fn_name_to_c(h, cname, sizeof(cname));
    HandlerFn fn = (HandlerFn)dlsym(RTLD_DEFAULT, cname);
    if (!fn) {
        fprintf(stderr, "[http] 핸들러 '%s' (%s) 를 찾을 수 없음\n", h, cname);
        return;
    }

    Route* r = &g_routes[g_nroutes++];
    strncpy(r->method, method, sizeof(r->method) - 1);
    strncpy(r->path,   p,      sizeof(r->path) - 1);
    r->fn = fn;
    fprintf(stderr, "[http] 라우트 등록: %s %s → %s\n", method, p, cname);
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
    return make_response(200, "application/json; charset=utf-8", strval(json_str));
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

FLValue server_req_body(FLValue req) {
    return fl_map_get(req, fl_str_val("body"));
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
    char path[1024];
    char query_str[1024];
    char headers[MAX_HEADERS][2][512];  /* [i][0]=name [i][1]=value */
    int  nheaders;
    char body[RECV_BUF];
    int  body_len;
    int  content_length;
} HttpRequest;

/* URL 디코드 (%XX → char) */
static void url_decode(const char* src, char* dst, size_t max) {
    size_t i = 0;
    while (*src && i < max - 1) {
        if (*src == '%' && src[1] && src[2]) {
            char hex[3] = { src[1], src[2], 0 };
            dst[i++] = (char)strtol(hex, NULL, 16);
            src += 3;
        } else if (*src == '+') {
            dst[i++] = ' '; src++;
        } else {
            dst[i++] = *src++;
        }
    }
    dst[i] = '\0';
}

/* query string 파싱 → FLMap */
static FLValue parse_query(const char* qs) {
    FLValue map = fl_map_new();
    if (!qs || !*qs) return map;
    char buf[1024];
    strncpy(buf, qs, sizeof(buf) - 1);
    char* pair = strtok(buf, "&");
    while (pair) {
        char* eq = strchr(pair, '=');
        if (eq) {
            *eq = '\0';
            char k[512], v[512];
            url_decode(pair, k, sizeof(k));
            url_decode(eq + 1, v, sizeof(v));
            map = fl_map_set(map, fl_str_val(k), fl_str_val(v));
        }
        pair = strtok(NULL, "&");
    }
    return map;
}

/* HTTP 요청 파싱 */
static int parse_http_request(const char* raw, int raw_len, HttpRequest* req) {
    (void)raw_len;
    memset(req, 0, sizeof(*req));

    /* 요청 라인 파싱 */
    char line[2048];
    const char* p = raw;
    const char* eol = strstr(p, "\r\n");
    if (!eol) return -1;
    int llen = (int)(eol - p);
    if (llen >= (int)sizeof(line)) return -1;
    strncpy(line, p, llen); line[llen] = '\0';

    char path_and_query[1024];
    if (sscanf(line, "%7s %1023s", req->method, path_and_query) != 2) return -1;

    /* path / query 분리 */
    char* q = strchr(path_and_query, '?');
    if (q) {
        *q = '\0';
        strncpy(req->query_str, q + 1, sizeof(req->query_str) - 1);
    }
    url_decode(path_and_query, req->path, sizeof(req->path));

    /* 헤더 파싱 */
    p = eol + 2;
    while (*p && !(p[0] == '\r' && p[1] == '\n')) {
        eol = strstr(p, "\r\n");
        if (!eol) break;
        llen = (int)(eol - p);
        if (llen >= (int)sizeof(line)) { p = eol + 2; continue; }
        strncpy(line, p, llen); line[llen] = '\0';
        char* colon = strchr(line, ':');
        if (colon && req->nheaders < MAX_HEADERS) {
            *colon = '\0';
            const char* val = colon + 1;
            while (*val == ' ') val++;
            strncpy(req->headers[req->nheaders][0], line, 511);
            strncpy(req->headers[req->nheaders][1], val,  511);
            if (strcasecmp(line, "Content-Length") == 0)
                req->content_length = atoi(val);
            req->nheaders++;
        }
        p = eol + 2;
    }
    p += 2; /* 빈 줄 건너뜀 */

    /* 바디 */
    if (req->content_length > 0) {
        int bl = req->content_length;
        if (bl >= RECV_BUF) bl = RECV_BUF - 1;
        memcpy(req->body, p, bl);
        req->body[bl] = '\0';
        req->body_len = bl;
    }
    return 0;
}

/* ───────────────────────────────────────────
   라우트 매칭 + 파라미터 추출
─────────────────────────────────────────── */

/* /users/:id  vs  /users/42  → params["id"] = "42" */
static int match_route(const Route* r, const char* method, const char* path, FLValue* params) {
    if (strcmp(r->method, method) != 0) return 0;

    const char* rp = r->path;
    const char* pp = path;
    *params = fl_map_new();

    while (*rp && *pp) {
        if (*rp == ':') {
            /* 파라미터 세그먼트 */
            rp++;
            char pname[128]; int pi = 0;
            while (*rp && *rp != '/') pname[pi++] = *rp++;
            pname[pi] = '\0';
            char pval[512]; int vi = 0;
            while (*pp && *pp != '/') pval[vi++] = *pp++;
            pval[vi] = '\0';
            *params = fl_map_set(*params, fl_str_val(pname), fl_str_val(pval));
        } else if (*rp == *pp) {
            rp++; pp++;
        } else {
            return 0;
        }
    }
    /* 양쪽 끝에 도달하거나 라우트가 / 로 끝나는 경우 */
    return (*rp == '\0' && (*pp == '\0' || *pp == '?'));
}

/* ───────────────────────────────────────────
   HTTP 응답 전송
─────────────────────────────────────────── */

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

static void send_response(int client_fd, FLValue resp) {
    int    status  = 200;
    const char* body    = "";
    const char* ctype   = "text/plain";
    char   extra_headers[2048] = "";

    if (resp.tag == FL_MAP) {
        FLValue sv = fl_map_get(resp, fl_str_val(K_STATUS));
        if (sv.tag == FL_INT) status = (int)sv.i;

        FLValue bv = fl_map_get(resp, fl_str_val(K_BODY));
        if (bv.tag == FL_STRING) body = strval(bv);

        FLValue hv = fl_map_get(resp, fl_str_val(K_HEADERS));
        if (hv.tag == FL_MAP) {
            FLMap* hm = (FLMap*)hv.obj;
            for (uint32_t i = 0; i < hm->len; i++) {
                const char* k = strval(hm->entries[i].key);
                const char* v = strval(hm->entries[i].val);
                if (strcasecmp(k, "Content-Type") == 0) {
                    ctype = v;
                } else {
                    char hbuf[512];
                    snprintf(hbuf, sizeof(hbuf), "%s: %s\r\n", k, v);
                    strncat(extra_headers, hbuf, sizeof(extra_headers) - strlen(extra_headers) - 1);
                }
            }
        }
    } else if (resp.tag == FL_STRING) {
        body  = strval(resp);
        ctype = "text/html; charset=utf-8";
    }

    size_t blen = strlen(body);
    char header_buf[4096];
    snprintf(header_buf, sizeof(header_buf),
        "HTTP/1.1 %d %s\r\n"
        "Content-Type: %s\r\n"
        "Content-Length: %zu\r\n"
        "Connection: close\r\n"
        "%s"
        "\r\n",
        status, status_text(status),
        ctype,
        blen,
        extra_headers
    );

    send(client_fd, header_buf, strlen(header_buf), 0);
    if (blen > 0) send(client_fd, body, blen, 0);
}

/* ───────────────────────────────────────────
   요청 → FLMap 변환
─────────────────────────────────────────── */

static FLValue make_req_map(HttpRequest* hr, FLValue params) {
    /* 헤더 맵 */
    FLValue headers = fl_map_new();
    for (int i = 0; i < hr->nheaders; i++) {
        headers = fl_map_set(headers,
            fl_str_val(hr->headers[i][0]),
            fl_str_val(hr->headers[i][1]));
    }

    /* body — JSON이면 파싱 시도 */
    FLValue body;
    const char* ct_header = "";
    for (int i = 0; i < hr->nheaders; i++) {
        if (strcasecmp(hr->headers[i][0], "Content-Type") == 0) {
            ct_header = hr->headers[i][1]; break;
        }
    }
    if (hr->body_len > 0 && strstr(ct_header, "application/json")) {
        body = fl_json_parse(fl_str_val(hr->body));
    } else {
        body = fl_str_val(hr->body);
    }

    FLValue req = fl_map_new();
    req = fl_map_set(req, fl_str_val("method"),  fl_str_val(hr->method));
    req = fl_map_set(req, fl_str_val("path"),    fl_str_val(hr->path));
    req = fl_map_set(req, fl_str_val("query"),   parse_query(hr->query_str));
    req = fl_map_set(req, fl_str_val("params"),  params);
    req = fl_map_set(req, fl_str_val("headers"), headers);
    req = fl_map_set(req, fl_str_val("body"),    body);
    return req;
}

/* ───────────────────────────────────────────
   연결 처리 (단일 요청)
─────────────────────────────────────────── */

typedef struct { int fd; } ConnArg;

static void* handle_connection(void* arg) {
    ConnArg* ca = (ConnArg*)arg;
    int client_fd = ca->fd;
    free(ca);

    char* raw = malloc(RECV_BUF);
    if (!raw) { close(client_fd); return NULL; }

    int total = 0;
    int n;
    while ((n = recv(client_fd, raw + total, RECV_BUF - total - 1, 0)) > 0) {
        total += n;
        /* 헤더 끝 감지 */
        raw[total] = '\0';
        if (strstr(raw, "\r\n\r\n")) {
            /* Content-Length 체크 후 바디까지 받기 */
            char* cl_str = strcasestr(raw, "Content-Length:");
            if (cl_str) {
                int cl = atoi(cl_str + 15);
                char* body_start = strstr(raw, "\r\n\r\n");
                if (body_start) {
                    int body_recv = total - (int)(body_start + 4 - raw);
                    if (body_recv >= cl) break;
                }
            } else {
                break;
            }
        }
        if (total >= RECV_BUF - 1) break;
    }
    raw[total] = '\0';

    if (total == 0) { free(raw); close(client_fd); return NULL; }

    HttpRequest hr;
    if (parse_http_request(raw, total, &hr) < 0) {
        const char* err = "HTTP/1.1 400 Bad Request\r\nContent-Length: 0\r\n\r\n";
        send(client_fd, err, strlen(err), 0);
        free(raw); close(client_fd); return NULL;
    }
    free(raw);

    /* 라우트 매칭 */
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

    if (!matched) {
        /* 404 */
        char body[256];
        snprintf(body, sizeof(body),
            "{\"error\":\"Not Found\",\"path\":\"%s\"}", hr.path);
        FLValue resp = make_response(404, "application/json", body);
        send_response(client_fd, resp);
        close(client_fd); return NULL;
    }

    /* 핸들러 호출 */
    FLValue req  = make_req_map(&hr, params);
    FLValue resp = fl_nil();
    /* try/catch */
    if (fl_try_top < FL_TRY_MAX) {
        FLTryFrame* frame = &fl_try_stack[fl_try_top++];
        if (setjmp(frame->buf) == 0) {
            resp = matched->fn(req);
            fl_try_top--;
        } else {
            fl_try_top--;
            char errbuf[512];
            const char* emsg = (frame->err.tag == FL_STRING)
                ? strval(frame->err) : "Internal error";
            snprintf(errbuf, sizeof(errbuf),
                "{\"error\":\"%s\"}", emsg);
            resp = make_response(500, "application/json", errbuf);
        }
    } else {
        resp = matched->fn(req);
    }

    send_response(client_fd, resp);
    close(client_fd);
    return NULL;
}

/* ───────────────────────────────────────────
   server_start — 메인 루프
─────────────────────────────────────────── */

FLValue server_start(FLValue port_val) {
    int port = (port_val.tag == FL_INT) ? (int)port_val.i : 8080;

    /* SIGPIPE 무시 */
    signal(SIGPIPE, SIG_IGN);

    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        fprintf(stderr, "[http] socket() 실패: %s\n", strerror(errno));
        return fl_nil();
    }

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

    listen(server_fd, 128);
    fprintf(stderr, "[http] 서버 시작: http://0.0.0.0:%d\n", port);

    while (1) {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        int client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_len);
        if (client_fd < 0) {
            if (errno == EINTR) continue;
            break;
        }

        /* 각 연결을 스레드로 처리 */
        ConnArg* arg = malloc(sizeof(ConnArg));
        arg->fd = client_fd;
        pthread_t tid;
        if (pthread_create(&tid, NULL, handle_connection, arg) != 0) {
            /* 스레드 생성 실패 시 직접 처리 */
            handle_connection(arg);
        } else {
            pthread_detach(tid);
        }
    }

    close(server_fd);
    return fl_nil();
}
