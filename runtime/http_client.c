/* FreeLang HTTP client backed by the native, certificate-verifying transport. */
#define _GNU_SOURCE
#include "runtime.h"
#include <arpa/inet.h>
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <math.h>
#include <netdb.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "http_transport.inc"

typedef struct {
    char host[256];
    char authority[320];
    char path[2048];
    int64_t port;
    int tls;
} FXHTTPURL;

static FLValue http_error(const char *message) {
    FLValue result = fl_map_new();
    result = fl_map_set(result, fl_str_val("status"), fl_int(0));
    result = fl_map_set(result, fl_str_val("body"), fl_str_val(""));
    return fl_map_set(result, fl_str_val("error"), fl_str_val(message));
}

static int http_parse_url(const char *url, FXHTTPURL *out) {
    const char *begin;
    if (strncmp(url, "https://", 8) == 0) {
        out->tls = 1; out->port = 443; begin = url + 8;
    } else if (strncmp(url, "http://", 7) == 0) {
        out->tls = 0; out->port = 80; begin = url + 7;
    } else return -1;

    const char *end = begin;
    while (*end && *end != '/' && *end != '?') {
        unsigned char c = (unsigned char)*end;
        if (c <= 32 || c == '#' || c == '@') return -1;
        end++;
    }
    size_t authority_len = (size_t)(end - begin);
    if (!authority_len || authority_len >= sizeof out->authority ||
        memchr(begin, '[', authority_len) || memchr(begin, ']', authority_len)) return -1;
    memcpy(out->authority, begin, authority_len); out->authority[authority_len] = 0;

    const char *colon = memchr(begin, ':', authority_len);
    size_t host_len = colon ? (size_t)(colon - begin) : authority_len;
    if (!host_len || host_len >= sizeof out->host ||
        (colon && memchr(colon + 1, ':', (size_t)(end - colon - 1)))) return -1;
    memcpy(out->host, begin, host_len); out->host[host_len] = 0;
    if (colon) {
        const char *p = colon + 1;
        if (p == end) return -1;
        int64_t port = 0;
        for (; p < end; p++) {
            if (*p < '0' || *p > '9' || port > (65535 - (*p - '0')) / 10) return -1;
            port = port * 10 + (*p - '0');
        }
        if (port < 1) return -1;
        out->port = port;
    }

    int n;
    if (*end == '?') n = snprintf(out->path, sizeof out->path, "/%s", end);
    else n = snprintf(out->path, sizeof out->path, "%s", *end ? end : "/");
    if (n < 0 || (size_t)n >= sizeof out->path) return -1;
    for (const unsigned char *p = (const unsigned char *)out->path; *p; p++)
        if (*p <= 32 || *p == 127 || *p == '#') return -1;
    return 0;
}

static int http_token(const char *s, size_t n) {
    if (!n) return 0;
    for (size_t i = 0; i < n; i++) {
        unsigned char c = (unsigned char)s[i];
        if (!((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') ||
              (c >= '0' && c <= '9') || strchr("!#$%&'*+-.^_`|~", c))) return 0;
    }
    return 1;
}

static int http_is_framing_header(const FLString *key) {
    return (key->len == 14 && strncasecmp(key->data, "content-length", 14) == 0) ||
           (key->len == 17 && strncasecmp(key->data, "transfer-encoding", 17) == 0);
}

static int http_append(char *dst, size_t cap, size_t *used, const char *src, size_t n) {
    if (*used > cap || n > cap - *used) return -1;
    memcpy(dst + *used, src, n); *used += n; return 0;
}

static FLValue http_response_headers(const char *raw, const char *split) {
    FLValue headers = fl_map_new();
    const char *p = strstr(raw, "\r\n");
    if (!p || p >= split) return headers;
    p += 2;
    while (p < split) {
        const char *end = strstr(p, "\r\n");
        if (!end || end > split) break;
        const char *colon = memchr(p, ':', (size_t)(end - p));
        if (!colon || colon == p) { p = end + 2; continue; }
        size_t key_len = (size_t)(colon - p), value_len = (size_t)(end - colon - 1);
        if (key_len >= 256) { p = end + 2; continue; }
        char key[256]; memcpy(key, p, key_len); key[key_len] = 0;
        for (size_t i = 0; i < key_len; i++) key[i] = (char)tolower((unsigned char)key[i]);
        const char *v = colon + 1;
        while (value_len && (*v == ' ' || *v == '\t')) { v++; value_len--; }
        while (value_len && (v[value_len - 1] == ' ' || v[value_len - 1] == '\t')) value_len--;
        char *value = malloc(value_len + 1);
        if (!value) return headers;
        memcpy(value, v, value_len); value[value_len] = 0;
        headers = fl_map_set(headers, fl_str_val(key), fl_str_val(value));
        free(value); p = end + 2;
    }
    return headers;
}

static char *http_decode_chunks(const char *src, size_t len, size_t *out_len) {
    char *out = malloc(len + 1);
    if (!out) return NULL;
    size_t pos = 0, used = 0;
    while (pos < len) {
        const char *line_end = memmem(src + pos, len - pos, "\r\n", 2);
        if (!line_end || line_end == src + pos || (size_t)(line_end - (src + pos)) > 16) goto bad;
        size_t line_len = (size_t)(line_end - (src + pos)), chunk = 0;
        const char *extension = memchr(src + pos, ';', line_len);
        size_t size_len = extension ? (size_t)(extension - (src + pos)) : line_len;
        if (!size_len) goto bad;
        for (size_t i = 0; i < size_len; i++) {
            unsigned char c = (unsigned char)src[pos + i]; unsigned digit;
            if (c >= '0' && c <= '9') digit = c - '0';
            else if (c >= 'a' && c <= 'f') digit = c - 'a' + 10;
            else if (c >= 'A' && c <= 'F') digit = c - 'A' + 10;
            else goto bad;
            if (chunk > (len - digit) / 16) goto bad;
            chunk = chunk * 16 + digit;
        }
        pos += line_len + 2;
        if (!chunk) {
            if (pos + 2 == len) { *out_len = used; out[used] = 0; return out; }
            if (len - pos < 2 || src[pos] != '\r' || src[pos + 1] != '\n') goto bad;
            if (pos + 2 != len) {
                const char *trail = memmem(src + pos, len - pos, "\r\n\r\n", 4);
                if (!trail || trail + 4 != src + len) goto bad;
            }
            *out_len = used; out[used] = 0; return out;
        }
        if (chunk > len - pos || len - pos - chunk < 2 ||
            src[pos + chunk] != '\r' || src[pos + chunk + 1] != '\n') goto bad;
        memcpy(out + used, src + pos, chunk); used += chunk; pos += chunk + 2;
    }
bad:
    free(out); return NULL;
}

static FLValue http_parse_response(FLValue exchange) {
    if (!fl_truthy(fl_map_get(exchange, fl_str_val("ok")))) {
        FLValue error = fl_map_get(exchange, fl_str_val("error"));
        FLValue message = fl_map_get(error, fl_str_val("message"));
        return http_error(message.tag == FL_STRING ? ((FLString *)message.obj)->data : "HTTP transport failed");
    }
    FLValue raw_v = fl_map_get(exchange, fl_str_val("value"));
    if (raw_v.tag != FL_STRING || !raw_v.obj) return http_error("HTTP_PROTOCOL: missing response data");
    FLString *raw = (FLString *)raw_v.obj;
    const char *split = strstr(raw->data, "\r\n\r\n");
    int status = 0;
    if (!split || (size_t)(split - raw->data) > 65536 ||
        sscanf(raw->data, "HTTP/%*u.%*u %d", &status) != 1 || status < 100 || status > 599)
        return http_error("HTTP_PROTOCOL: malformed response");
    FLValue headers = http_response_headers(raw->data, split);
    const char *body = split + 4;
    size_t body_len = raw->len - (size_t)(body - raw->data);
    FLValue transfer = fl_map_get(headers, fl_str_val("transfer-encoding"));
    if (transfer.tag == FL_STRING && strcasecmp(((FLString *)transfer.obj)->data, "chunked") == 0) {
        size_t decoded_len = 0;
        char *decoded = http_decode_chunks(body, body_len, &decoded_len);
        if (!decoded) return http_error("HTTP_PROTOCOL: invalid chunked response");
        FLValue body_v = fl_str_val(decoded); free(decoded);
        FLValue result = fl_map_new();
        result = fl_map_set(result, fl_str_val("status"), fl_int(status));
        result = fl_map_set(result, fl_str_val("body"), body_v);
        return fl_map_set(result, fl_str_val("headers"), headers);
    }
    FLValue result = fl_map_new();
    result = fl_map_set(result, fl_str_val("status"), fl_int(status));
    result = fl_map_set(result, fl_str_val("body"), fl_str_val(body));
    return fl_map_set(result, fl_str_val("headers"), headers);
}

static FLValue fx_http_wire(const char *method, const char *url, const char *body,
                            size_t body_len, FLValue req_headers, long timeout_ms, int v11) {
    FXHTTPURL parsed = {0};
    if (http_parse_url(url, &parsed) != 0) return http_error("invalid HTTP URL");
    if (timeout_ms < 1) timeout_ms = 1;
    if (timeout_ms > 120000) timeout_ms = 120000;
    if (body_len > 67108864) return http_error("HTTP request body too large");
    size_t capacity = strlen(parsed.path) + strlen(parsed.authority) + body_len + 1024;
    FLMap *map = req_headers.tag == FL_MAP && req_headers.obj ? (FLMap *)req_headers.obj : NULL;
    if (map) {
        if (map->len > 64) return http_error("HTTP header limit exceeded");
        for (uint32_t i = 0; i < map->len; i++) {
            if (map->entries[i].key.tag != FL_STRING || map->entries[i].val.tag != FL_STRING ||
                !map->entries[i].key.obj || !map->entries[i].val.obj) return http_error("invalid HTTP header");
            FLString *k = (FLString *)map->entries[i].key.obj, *v = (FLString *)map->entries[i].val.obj;
            if (!http_token(k->data, k->len) || k->len > 255 || v->len > 32768 ||
                (size_t)k->len + (size_t)v->len + 4 > SIZE_MAX - capacity) return http_error("invalid HTTP header");
            for (uint32_t j = 0; j < v->len; j++) if (v->data[j] == '\r' || v->data[j] == '\n') return http_error("invalid HTTP header value");
            if (v11 && http_is_framing_header(k)) continue;
            capacity += (size_t)k->len + (size_t)v->len + 4;
        }
    }
    if (capacity > 2097152) return http_error("HTTP request headers too large");
    char *wire = malloc(capacity + 1);
    if (!wire) return http_error("HTTP allocation failed");
    size_t used = 0; char line[512];
    int n = snprintf(line, sizeof line, "%s %s HTTP/1.1\r\nHost: %s\r\nConnection: close\r\n",
                     method, parsed.path, parsed.authority);
    if (n < 0 || (size_t)n >= sizeof line || http_append(wire, capacity, &used, line, (size_t)n) != 0) { free(wire); return http_error("HTTP request too large"); }
    if (!v11 && http_append(wire, capacity, &used, "User-Agent: fx-http-client/1.0\r\n", 32) != 0) { free(wire); return http_error("HTTP request too large"); }
    if (http_append(wire, capacity, &used, "Expect:\r\n", 9) != 0) { free(wire); return http_error("HTTP request too large"); }
    if (body || !strcmp(method, "POST")) {
        n = snprintf(line, sizeof line, "Content-Length: %zu\r\n", body_len);
        if (n < 0 || (size_t)n >= sizeof line || http_append(wire, capacity, &used, line, (size_t)n) != 0) { free(wire); return http_error("HTTP request too large"); }
    }
    if (map) for (uint32_t i = 0; i < map->len; i++) {
        FLString *k = (FLString *)map->entries[i].key.obj, *v = (FLString *)map->entries[i].val.obj;
        if (v11 && http_is_framing_header(k)) continue;
        if (http_append(wire, capacity, &used, k->data, k->len) != 0 ||
            http_append(wire, capacity, &used, ": ", 2) != 0 ||
            http_append(wire, capacity, &used, v->data, v->len) != 0 ||
            http_append(wire, capacity, &used, "\r\n", 2) != 0) { free(wire); return http_error("HTTP request too large"); }
    }
    if (http_append(wire, capacity, &used, "\r\n", 2) != 0 ||
        (body_len && http_append(wire, capacity, &used, body, body_len) != 0)) { free(wire); return http_error("HTTP request too large"); }
    wire[used] = 0;
    FLValue exchange = transport_exchange(fl_str_val(parsed.host), fl_int(parsed.port), fl_str_val(wire),
                                          fl_int(timeout_ms), fl_int(67108864), fl_nil(), parsed.tls);
    free(wire);
    return http_parse_response(exchange);
}

static FLValue fx_http_exec(const char *method, const char *url, const char *body, FLValue headers) {
    return fx_http_wire(method, url, body, body ? strlen(body) : 0, headers, 120000, 0);
}

FLValue http_get(FLValue url) { return url.tag == FL_STRING && url.obj ? fx_http_exec("GET", ((FLString *)url.obj)->data, NULL, fl_nil()) : http_error("url must be string"); }
FLValue http_get_h(FLValue url, FLValue headers) { return url.tag == FL_STRING && url.obj ? fx_http_exec("GET", ((FLString *)url.obj)->data, NULL, headers) : http_error("url must be string"); }
FLValue http_post(FLValue url, FLValue body) { return url.tag == FL_STRING && url.obj ? fx_http_exec("POST", ((FLString *)url.obj)->data, body.tag == FL_STRING && body.obj ? ((FLString *)body.obj)->data : "", fl_nil()) : http_error("url must be string"); }
FLValue http_post_h(FLValue url, FLValue body, FLValue headers) { return url.tag == FL_STRING && url.obj ? fx_http_wire("POST", ((FLString *)url.obj)->data, body.tag == FL_STRING && body.obj ? ((FLString *)body.obj)->data : "", body.tag == FL_STRING && body.obj ? ((FLString *)body.obj)->len : 0, headers, 120000, 0) : http_error("url must be string"); }
FLValue http_put(FLValue url, FLValue body) { return url.tag == FL_STRING && url.obj ? fx_http_exec("PUT", ((FLString *)url.obj)->data, body.tag == FL_STRING && body.obj ? ((FLString *)body.obj)->data : "", fl_nil()) : http_error("url must be string"); }
FLValue http_del(FLValue url) { return url.tag == FL_STRING && url.obj ? fx_http_exec("DELETE", ((FLString *)url.obj)->data, NULL, fl_nil()) : http_error("url must be string"); }
FLValue http_patch(FLValue url, FLValue body) { return url.tag == FL_STRING && url.obj ? fx_http_exec("PATCH", ((FLString *)url.obj)->data, body.tag == FL_STRING && body.obj ? ((FLString *)body.obj)->data : "", fl_nil()) : http_error("url must be string"); }
FLValue http_req(FLValue method, FLValue url, FLValue body, FLValue headers) {
    if (method.tag != FL_STRING || !method.obj || url.tag != FL_STRING || !url.obj) return http_error("method and url must be strings");
    const char *payload = body.tag == FL_STRING && body.obj ? ((FLString *)body.obj)->data : "";
    size_t payload_len = body.tag == FL_STRING && body.obj ? ((FLString *)body.obj)->len : 0;
    return fx_http_wire(((FLString *)method.obj)->data, ((FLString *)url.obj)->data, payload, payload_len, headers, 120000, 0);
}
FLValue http_body(FLValue result) { return result.tag == FL_MAP ? fl_map_get(result, fl_str_val("body")) : fl_str_val(""); }
FLValue http_status(FLValue result) { return result.tag == FL_MAP ? fl_map_get(result, fl_str_val("status")) : fl_int(0); }
FLValue http_ok_p(FLValue result) {
    FLValue status = http_status(result);
    double code = status.tag == FL_INT ? (double)status.i : status.tag == FL_FLOAT ? status.f : 0.0;
    return fl_bool(code >= 200 && code < 300);
}

static FLValue vc_error(const char *message) {
    FLValue out = fl_map_new(); out = fl_map_set(out, fl_str_val("status"), fl_int(0));
    out = fl_map_set(out, fl_str_val("body"), fl_str_val(""));
    return fl_map_set(out, fl_str_val("error"), fl_str_val(message));
}
static int vc_token(unsigned char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') ||
           (c && strchr("!#$%&'*+-.^_`|~", c) != NULL);
}
static FLValue vc_request(FLValue name, FLValue argv) {
    const char *op = ((FLString *)name.obj)->data;
    FLVector *v = (FLVector *)argv.obj; size_t n = v->len;
    FLValue a = n ? v->data[0] : fl_nil(), b = n > 1 ? v->data[1] : fl_nil();
    FLValue headers = n > 2 ? v->data[2] : fl_nil(), body = n > 3 ? v->data[3] : fl_nil();
    int get = !strcmp(op, "http-get") || !strcmp(op, "http-with-timeout");
    FLValue method = get ? fl_str_val("GET") : fl_v11_utf8_string(fl_v11_truthy(a) ? a : fl_str_val("GET"));
    FLValue url = fl_v11_utf8_string(get ? a : b);
    FLString *m = (FLString *)method.obj, *u = (FLString *)url.obj;
    if (!m->len || m->len >= 128 || m->len != strlen(m->data) || u->len != strlen(u->data)) return vc_error("invalid HTTP request");
    char upper[128];
    for (size_t i = 0; i < m->len; i++) { unsigned char c = (unsigned char)m->data[i]; if (!vc_token(c)) return vc_error("invalid HTTP method"); upper[i] = (char)toupper(c); }
    upper[m->len] = 0;
    FXHTTPURL parsed = {0}; if (http_parse_url(u->data, &parsed) != 0) return vc_error("invalid HTTP URL");
    if (get) { headers = fl_nil(); body = fl_nil(); }
    FLValue normalized = fl_map_new();
    if (headers.tag == FL_MAP && headers.obj) {
        FLMap *map = (FLMap *)headers.obj; if (map->len > 64) return vc_error("HTTP header limit exceeded");
        for (uint32_t i = 0; i < map->len; i++) {
            FLValue key = fl_v11_utf8_string(map->entries[i].key); FLString *k = (FLString *)key.obj;
            if (!k->len || k->len > 32768) return vc_error("invalid HTTP header name");
            char *lower = malloc(k->len + 1); if (!lower) return vc_error("HTTP allocation failed");
            for (uint32_t j = 0; j < k->len; j++) { if (!vc_token((unsigned char)k->data[j])) { free(lower); return vc_error("invalid HTTP header name"); } lower[j] = (char)tolower((unsigned char)k->data[j]); }
            lower[k->len] = 0;
            if (body.tag != FL_NIL && (!strcmp(lower, "content-length") || !strcmp(lower, "transfer-encoding"))) { free(lower); continue; }
            FLValue value = fl_v11_header_value(map->entries[i].val);
            if (!value.obj || value.tag != FL_STRING || ((FLString *)value.obj)->len > 32768) { free(lower); return vc_error("invalid HTTP header value"); }
            normalized = fl_map_set(normalized, fl_str_val(lower), value); free(lower);
        }
    }
    long timeout_ms = 10000;
    if (!strcmp(op, "http-request-timeout") || !strcmp(op, "http-with-timeout")) {
        FLValue value = !strcmp(op, "http-with-timeout") ? b : (n > 4 ? v->data[4] : fl_nil());
        value = fl_v11_to_number(value); double ms = value.f;
        if (!strcmp(op, "http-with-timeout")) ms = (b.tag == FL_INT || b.tag == FL_FLOAT) && ms > 0 ? ms : 10000;
        else if (isnan(ms) || ms == 0) ms = 5000;
        if (!isfinite(ms) || ms > 120000 || ms < 1) ms = 1;
        timeout_ms = (long)ms;
    }
    FLValue bytes = body.tag == FL_NIL ? fl_nil() : fl_v11_utf8_string(body);
    FLString *s = bytes.tag == FL_STRING && bytes.obj ? (FLString *)bytes.obj : NULL;
    return fx_http_wire(upper, u->data, s ? s->data : NULL, s ? s->len : 0, normalized, timeout_ms, 1);
}
FLValue fl_v11_http_client(FLValue name, FLValue argv) {
    if (fl_try_top >= FL_TRY_MAX) return vc_error("HTTP exception frame limit exceeded");
    FLTryFrame *frame = &fl_try_stack[fl_try_top++];
    if (setjmp(frame->buf) == 0) { FLValue out = vc_request(name, argv); fl_try_top--; return out; }
    fl_try_top--; return vc_error("invalid HTTP request");
}
