/* Server-Sent Events. HTTP owns the socket; SSE owns the handler lifetime. */
#define _GNU_SOURCE
#include "runtime.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/socket.h>
#include <dlfcn.h>
#include <errno.h>
#include <limits.h>

typedef struct SSEConn {
    int fd, closed;
    int64_t id;
    unsigned refs;
    pthread_mutex_t mutex;
    struct SSEConn* next;
} SSEConn;

/* The language-visible integer is an opaque lifetime ID, never a pointer. */
static pthread_mutex_t connections_mutex = PTHREAD_MUTEX_INITIALIZER;
static SSEConn* connections;
static int64_t next_connection_id;
static int connections_stopping;

void sse_server_start(void) {
    pthread_mutex_lock(&connections_mutex);
    connections_stopping = 0;
    pthread_mutex_unlock(&connections_mutex);
}

void sse_server_shutdown(void) {
    pthread_mutex_lock(&connections_mutex);
    connections_stopping = 1;
    for (SSEConn* c = connections; c; c = c->next) {
        /* Wake a pending writer before waiting for its per-connection lock. */
        shutdown(c->fd, SHUT_RDWR);
        pthread_mutex_lock(&c->mutex);
        c->closed = 1;
        pthread_mutex_unlock(&c->mutex);
    }
    pthread_mutex_unlock(&connections_mutex);
}

#define MAX_SSE_ROUTES 16
typedef FLValue (*SSEHandlerFn)(FLValue conn);
typedef struct {
    char path[256];
    FLValue name, closure;
    SSEHandlerFn fn;
} SSERoute;
static SSERoute g_sse_routes[MAX_SSE_ROUTES];
static int g_n_sse_routes;
static pthread_mutex_t routes_mutex = PTHREAD_MUTEX_INITIALIZER;

static const char* sse_strval(FLValue v) {
    return v.tag == FL_STRING && v.obj ? ((FLString*)v.obj)->data : "";
}

static SSEConn* sse_connection_new(int fd) {
    SSEConn* c = calloc(1, sizeof(*c));
    if (!c) return NULL;
    if (pthread_mutex_init(&c->mutex, NULL)) { free(c); return NULL; }
    c->fd = fd;
    c->refs = 1; /* Handler owner. */
    pthread_mutex_lock(&connections_mutex);
    if (connections_stopping || next_connection_id == INT64_MAX) {
        pthread_mutex_unlock(&connections_mutex);
        pthread_mutex_destroy(&c->mutex); free(c); return NULL;
    }
    c->id = ++next_connection_id;
    c->next = connections;
    connections = c;
    pthread_mutex_unlock(&connections_mutex);
    return c;
}

static SSEConn* sse_connection_acquire(FLValue value) {
    if (value.tag != FL_MAP) return NULL;
    FLMap* map = (FLMap*)value.obj;
    FLValue id = fl_nil();
    for (uint32_t i = 0; i < map->len; i++) {
        FLValue key = map->entries[i].key;
        if (key.tag == FL_STRING && key.obj
            && ((FLString*)key.obj)->len == 7
            && !memcmp(((FLString*)key.obj)->data, "__sse__", 7)) {
            id = map->entries[i].val; break;
        }
    }
    if (id.tag != FL_INT) return NULL;
    pthread_mutex_lock(&connections_mutex);
    SSEConn* c = connections;
    while (c && c->id != id.i) c = c->next;
    if (c) c->refs++;
    pthread_mutex_unlock(&connections_mutex);
    return c;
}

static void sse_connection_release(SSEConn* c) {
    pthread_mutex_lock(&connections_mutex);
    int destroy = --c->refs == 0;
    pthread_mutex_unlock(&connections_mutex);
    if (destroy) { pthread_mutex_destroy(&c->mutex); free(c); }
}

static void sse_connection_finish(SSEConn* c) {
    pthread_mutex_lock(&connections_mutex);
    SSEConn** slot = &connections;
    while (*slot && *slot != c) slot = &(*slot)->next;
    if (*slot) *slot = c->next;
    pthread_mutex_unlock(&connections_mutex);
    /* Finish any acquired writer before HTTP closes or reuses the socket. */
    pthread_mutex_lock(&c->mutex);
    c->closed = 1;
    pthread_mutex_unlock(&c->mutex);
    sse_connection_release(c);
}

static int sse_send_all(int fd, const char* bytes, size_t length) {
    while (length) {
        ssize_t sent = send(fd, bytes, length, MSG_NOSIGNAL);
        if (sent < 0 && errno == EINTR) continue;
        if (sent <= 0) return 0;
        bytes += sent;
        length -= (size_t)sent;
    }
    return 1;
}

static int sse_do_open(int fd) {
    static const char header[] =
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/event-stream\r\n"
        "Cache-Control: no-cache\r\n"
        "Connection: keep-alive\r\n"
        "X-Accel-Buffering: no\r\n"
        "Access-Control-Allow-Origin: *\r\n\r\n";
    return sse_send_all(fd, header, sizeof(header) - 1);
}

static void sse_do_error(int fd) {
    static const char error[] = "HTTP/1.1 500 Error\r\nContent-Length: 0\r\nConnection: close\r\n\r\n";
    sse_send_all(fd, error, sizeof(error) - 1);
}

/* Each complete call emits one event, including empty and multiline data. */
FLValue sse_send(FLValue conn_v, FLValue data_v) {
    SSEConn* c = sse_connection_acquire(conn_v);
    if (!c) return fl_bool(0);
    const char* data = sse_strval(data_v);
    size_t length = data_v.tag == FL_STRING && data_v.obj
        ? ((FLString*)data_v.obj)->len : 0;
    pthread_mutex_lock(&c->mutex);
    int ok = !c->closed;
    size_t start = 0;
    while (ok) {
        size_t end = start;
        while (end < length && data[end] != '\r' && data[end] != '\n') end++;
        ok = sse_send_all(c->fd, "data: ", 6)
            && sse_send_all(c->fd, data + start, end - start)
            && sse_send_all(c->fd, "\n", 1);
        if (end == length) break;
        start = end + 1;
        if (data[end] == '\r' && start < length && data[start] == '\n') start++;
    }
    if (ok) ok = sse_send_all(c->fd, "\n", 1);
    if (!ok) c->closed = 1;
    pthread_mutex_unlock(&c->mutex);
    sse_connection_release(c);
    return fl_bool(ok);
}

FLValue sse_close(FLValue conn_v) {
    SSEConn* c = sse_connection_acquire(conn_v);
    if (c) {
        pthread_mutex_lock(&c->mutex);
        c->closed = 1;
        pthread_mutex_unlock(&c->mutex);
        sse_connection_release(c);
    }
    return fl_nil();
}

FLValue sse_alive(FLValue conn_v) {
    SSEConn* c = sse_connection_acquire(conn_v);
    int alive = 0;
    if (c) {
        pthread_mutex_lock(&c->mutex);
        if (!c->closed) {
            char byte;
            ssize_t available = recv(c->fd, &byte, 1, MSG_PEEK | MSG_DONTWAIT);
            if (available == 0 || (available < 0 && errno != EAGAIN
                && errno != EWOULDBLOCK && errno != EINTR)) c->closed = 1;
        }
        alive = !c->closed;
        pthread_mutex_unlock(&c->mutex);
        sse_connection_release(c);
    }
    return fl_bool(alive);
}

/* Match ordinary HTTP: resolve language names, then legacy C symbols. */
FLValue server_sse(FLValue path, FLValue handler) {
    if (path.tag != FL_STRING || !path.obj || !((FLString*)path.obj)->len
        || ((FLString*)path.obj)->len >= sizeof(g_sse_routes[0].path)
        || strlen(sse_strval(path)) != ((FLString*)path.obj)->len) {
        fl_throw(fl_str_val("SSE route requires a nonempty path shorter than 256 bytes"));
        return fl_nil();
    }
    if (handler.tag != FL_FN && handler.tag != FL_STRING) {
        fl_throw(fl_str_val("SSE handler requires a function or function name"));
        return fl_nil();
    }
    SSERoute prepared = {0};
    SSERoute* r = &prepared;
    r->name = fl_nil(); r->closure = fl_nil(); r->fn = NULL;
    if (handler.tag == FL_FN) r->closure = fl_heap_copy(handler);
    else {
        r->name = fl_heap_copy(handler);
        char cname[256];
        const char* name = sse_strval(handler);
        size_t i = 0;
        while (i < sizeof(cname) - 1 && name[i]) {
            cname[i] = name[i] == '-' ? '_' : name[i]; i++;
        }
        cname[i] = '\0';
        r->fn = (SSEHandlerFn)dlsym(RTLD_DEFAULT, cname);
    }
    strcpy(r->path, sse_strval(path));
    pthread_mutex_lock(&routes_mutex);
    if (g_n_sse_routes >= MAX_SSE_ROUTES) {
        pthread_mutex_unlock(&routes_mutex);
        fl_heap_release(r->name);
        fl_heap_release(r->closure);
        fl_throw(fl_str_val("SSE route limit exceeded")); return fl_nil();
    }
    g_sse_routes[g_n_sse_routes++] = prepared;
    pthread_mutex_unlock(&routes_mutex);
    return fl_nil();
}

int sse_is_sse_request(const char* raw) {
    return strcasestr(raw, "text/event-stream") != NULL;
}

int sse_handle_request(int fd, const char* raw, int raw_len) {
    const char* end = raw + raw_len;
    const char* p = raw;
    while (p < end && *p != ' ') p++;
    while (p < end && *p == ' ') p++;
    const char* start = p;
    while (p < end && *p != ' ' && *p != '?' && *p != '\r' && *p != '\n') p++;
    SSERoute* matched = NULL;
    pthread_mutex_lock(&routes_mutex);
    for (int i = 0; i < g_n_sse_routes; i++) {
        size_t length = strlen(g_sse_routes[i].path);
        if (length == (size_t)(p - start) && !memcmp(start, g_sse_routes[i].path, length)) {
            matched = &g_sse_routes[i]; break;
        }
    }
    pthread_mutex_unlock(&routes_mutex);
    if (!matched) return 0;
    SSEConn* c = sse_connection_new(fd);
    if (!c) {
        sse_do_error(fd); return 1;
    }
    fl_arena_begin();
    volatile int header_attempted = 0;
    if (fl_try_top < FL_TRY_MAX) {
        FLTryFrame* frame = &fl_try_stack[fl_try_top++];
        if (setjmp(frame->buf) == 0) {
            FLValue fn = matched->closure;
            if (matched->name.tag == FL_STRING) fn = fl_named_fn(matched->name);
            if (fn.tag != FL_FN && !matched->fn)
                fl_throw(fl_str_val("named SSE handler is not a function"));
            FLValue conn = fl_map_set(fl_map_new(), fl_str_val("__sse__"), fl_int(c->id));
            conn = fl_map_set(conn, fl_str_val("type"), fl_str_val("sse"));
            header_attempted = 1;
            if (sse_do_open(fd)) {
                if (fn.tag == FL_FN) fl_fn_call(fn, 1, &conn);
                else matched->fn(conn);
            }
            fl_try_top--;
        } else {
            fl_try_top--;
            if (!header_attempted) sse_do_error(fd);
            fprintf(stderr, "[sse] handler failed\n");
        }
    } else {
        sse_do_error(fd);
        fprintf(stderr, "[sse] handler try stack exhausted\n");
    }
    sse_connection_finish(c);
    fl_arena_end();
    /* handle_connection closes fd exactly once on its ordinary exit path. */
    return 1;
}
