/*
 * aliases.c — cgc-bin이 생성하는 함수명 ↔ 런타임 구현 매핑
 *
 * cgc-bin은 FreeLang stdlib 함수를 특정 C 이름으로 emit.
 * 런타임 구현은 fl_ 접두사를 쓰므로 여기서 bridge.
 */

#include "runtime.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <time.h>
#include <math.h>

/* ── 환경 ── */
FLValue env_get(FLValue key)        { return _fl_env_get(key); }
FLValue env_set(FLValue k, FLValue v) { return _fl_env_set(k, v); }

/* ── 수학 ── */
FLValue math_floor(FLValue x)  { return fl_floor(x); }
FLValue math_ceil(FLValue x)   { return fl_ceil(x); }
FLValue math_abs(FLValue x)    { return fl_abs(x); }
FLValue math_sqrt(FLValue x)   { return fl_math_sqrt(x); }

FLValue math_round(FLValue x) {
    if (x.tag == FL_INT)   return x;
    if (x.tag == FL_FLOAT) return fl_int((int64_t)round(x.f));
    return fl_nil();
}
FLValue math_max(FLValue a, FLValue b) {
    double va = (a.tag == FL_INT) ? (double)a.i : a.f;
    double vb = (b.tag == FL_INT) ? (double)b.i : b.f;
    return (va >= vb) ? a : b;
}
FLValue math_min(FLValue a, FLValue b) {
    double va = (a.tag == FL_INT) ? (double)a.i : a.f;
    double vb = (b.tag == FL_INT) ? (double)b.i : b.f;
    return (va <= vb) ? a : b;
}
FLValue math_pow(FLValue base, FLValue exp) {
    double b = (base.tag == FL_INT) ? (double)base.i : base.f;
    double e = (exp.tag  == FL_INT) ? (double)exp.i  : exp.f;
    return fl_float(pow(b, e));
}
FLValue math_random(void) {
    static int seeded = 0;
    if (!seeded) { srand((unsigned)time(NULL)); seeded = 1; }
    return fl_float((double)rand() / ((double)RAND_MAX + 1.0));
}

/* ── 문자열 ── */

/* 공통: FL_STRING인지 확인 */
static inline const char* fl_cstr(FLValue v) {
    if (v.tag != FL_STRING) return "";
    return ((FLString*)v.obj)->data;
}

FLValue str_trim(FLValue s) {
    if (s.tag != FL_STRING) return s;
    const char* p = fl_cstr(s);
    while (*p && isspace((unsigned char)*p)) p++;
    size_t len = strlen(p);
    while (len > 0 && isspace((unsigned char)p[len-1])) len--;
    char* buf = (char*)malloc(len + 1);
    memcpy(buf, p, len); buf[len] = '\0';
    FLValue r = fl_str_val(buf); free(buf);
    return r;
}

FLValue str_upper(FLValue s) {
    if (s.tag != FL_STRING) return s;
    const char* p = fl_cstr(s);
    size_t len = strlen(p);
    char* buf = (char*)malloc(len + 1);
    for (size_t i = 0; i < len; i++) buf[i] = (char)toupper((unsigned char)p[i]);
    buf[len] = '\0';
    FLValue r = fl_str_val(buf); free(buf);
    return r;
}

FLValue str_lower(FLValue s) {
    if (s.tag != FL_STRING) return s;
    const char* p = fl_cstr(s);
    size_t len = strlen(p);
    char* buf = (char*)malloc(len + 1);
    for (size_t i = 0; i < len; i++) buf[i] = (char)tolower((unsigned char)p[i]);
    buf[len] = '\0';
    FLValue r = fl_str_val(buf); free(buf);
    return r;
}

FLValue str_starts_with(FLValue s, FLValue prefix) {
    const char* sp = fl_cstr(s);
    const char* pp = fl_cstr(prefix);
    return fl_bool(strncmp(sp, pp, strlen(pp)) == 0);
}

FLValue str_ends_with(FLValue s, FLValue suffix) {
    const char* sp = fl_cstr(s);
    const char* su = fl_cstr(suffix);
    size_t slen = strlen(sp), sulen = strlen(su);
    if (sulen > slen) return fl_bool(false);
    return fl_bool(strcmp(sp + slen - sulen, su) == 0);
}

FLValue str_split(FLValue s, FLValue sep) {
    /* alias — split already exists in runtime.h (cgc-bridge.c) */
    return split(s, sep);
}

FLValue str_pad_left(FLValue s, FLValue width_v, FLValue ch_v) {
    const char* sp  = fl_cstr(s);
    int width = (width_v.tag == FL_INT) ? (int)width_v.i : 0;
    const char* ch  = fl_cstr(ch_v);
    char pad = (ch && ch[0]) ? ch[0] : ' ';
    int slen = (int)strlen(sp);
    if (slen >= width) return s;
    int pads = width - slen;
    char* buf = (char*)malloc((size_t)(width + 1));
    for (int i = 0; i < pads; i++) buf[i] = pad;
    memcpy(buf + pads, sp, (size_t)slen + 1);
    FLValue r = fl_str_val(buf); free(buf);
    return r;
}

FLValue str_pad_right(FLValue s, FLValue width_v, FLValue ch_v) {
    const char* sp  = fl_cstr(s);
    int width = (width_v.tag == FL_INT) ? (int)width_v.i : 0;
    const char* ch  = fl_cstr(ch_v);
    char pad = (ch && ch[0]) ? ch[0] : ' ';
    int slen = (int)strlen(sp);
    if (slen >= width) return s;
    int pads = width - slen;
    char* buf = (char*)malloc((size_t)(width + 1));
    memcpy(buf, sp, (size_t)slen);
    for (int i = 0; i < pads; i++) buf[slen + i] = pad;
    buf[width] = '\0';
    FLValue r = fl_str_val(buf); free(buf);
    return r;
}

FLValue str_repeat(FLValue s, FLValue n_v) {
    const char* sp = fl_cstr(s);
    int n = (n_v.tag == FL_INT) ? (int)n_v.i : 0;
    size_t slen = strlen(sp);
    if (n <= 0 || slen == 0) return fl_str_val("");
    size_t total = slen * (size_t)n;
    char* buf = (char*)malloc(total + 1);
    for (int i = 0; i < n; i++) memcpy(buf + i * slen, sp, slen);
    buf[total] = '\0';
    FLValue r = fl_str_val(buf); free(buf);
    return r;
}

FLValue str_contains(FLValue s, FLValue sub) { return fl_str_includes(s, sub); }

/* parse number */
FLValue parse_int(FLValue s) {
    const char* p = fl_cstr(s);
    return fl_int((int64_t)strtoll(p, NULL, 10));
}
FLValue parse_float(FLValue s) {
    const char* p = fl_cstr(s);
    return fl_float(strtod(p, NULL));
}
FLValue to_string(FLValue v) {
    char buf[64];
    if (v.tag == FL_INT)    { snprintf(buf, sizeof(buf), "%lld", (long long)v.i); return fl_str_val(buf); }
    if (v.tag == FL_FLOAT)  { snprintf(buf, sizeof(buf), "%g", v.f); return fl_str_val(buf); }
    if (v.tag == FL_BOOL)   return fl_str_val(v.b ? "true" : "false");
    if (v.tag == FL_NIL)    return fl_str_val("nil");
    if (v.tag == FL_STRING) return v;
    return fl_json_stringify(v);
}
FLValue to_int(FLValue v) {
    if (v.tag == FL_INT)   return v;
    if (v.tag == FL_FLOAT) return fl_int((int64_t)v.f);
    if (v.tag == FL_STRING) return parse_int(v);
    return fl_nil();
}
FLValue to_float(FLValue v) {
    if (v.tag == FL_FLOAT) return v;
    if (v.tag == FL_INT)   return fl_float((double)v.i);
    if (v.tag == FL_STRING) return parse_float(v);
    return fl_nil();
}
/* str_to_num alias (FreeLang v11 stdlib) */
FLValue str_to_num(FLValue v) { return parse_float(v); }

/* ── 컬렉션 ── */

/* first / last / rest / nth */
FLValue first(FLValue vec) {
    if (vec.tag != FL_VECTOR) return fl_nil();
    FLVector* v = (FLVector*)vec.obj;
    if (v->len == 0) return fl_nil();
    return v->data[0];
}
FLValue last(FLValue vec) {
    if (vec.tag != FL_VECTOR) return fl_nil();
    FLVector* v = (FLVector*)vec.obj;
    if (v->len == 0) return fl_nil();
    return v->data[v->len - 1];
}
FLValue rest(FLValue vec) {
    if (vec.tag != FL_VECTOR) return fl_vec_new();
    FLVector* vp = (FLVector*)vec.obj;
    if (vp->len <= 1) return fl_vec_new();
    return fl_vec_slice(vec, fl_int(1), fl_int((int64_t)vp->len));
}
FLValue nth(FLValue vec, FLValue idx) { return fl_vec_get(vec, idx); }
FLValue count(FLValue v)              { return length(v); }

/* take / drop */
FLValue take(FLValue n_v, FLValue vec) {
    if (vec.tag != FL_VECTOR) return fl_vec_new();
    FLVector* vp = (FLVector*)vec.obj;
    int64_t n = (n_v.tag == FL_INT) ? n_v.i : 0;
    if (n < 0) n = 0;
    if (n > (int64_t)vp->len) n = (int64_t)vp->len;
    return fl_vec_slice(vec, fl_int(0), fl_int(n));
}
FLValue drop(FLValue n_v, FLValue vec) {
    if (vec.tag != FL_VECTOR) return fl_vec_new();
    FLVector* vp = (FLVector*)vec.obj;
    int64_t n = (n_v.tag == FL_INT) ? n_v.i : 0;
    if (n < 0) n = 0;
    if (n > (int64_t)vp->len) n = (int64_t)vp->len;
    return fl_vec_slice(vec, fl_int(n), fl_int((int64_t)vp->len));
}

/* flatten (1단계) */
FLValue flatten(FLValue vec) {
    if (vec.tag != FL_VECTOR) return vec;
    FLVector* vp = (FLVector*)vec.obj;
    FLValue result = fl_vec_new();
    for (uint32_t i = 0; i < vp->len; i++) {
        FLValue item = vp->data[i];
        if (item.tag == FL_VECTOR) {
            FLVector* inner = (FLVector*)item.obj;
            for (uint32_t j = 0; j < inner->len; j++)
                result = fl_vec_push(result, inner->data[j]);
        } else {
            result = fl_vec_push(result, item);
        }
    }
    return result;
}

/* sort (숫자/문자열 오름차순) */
static int fl_compare(const void* a, const void* b) {
    const FLValue* va = (const FLValue*)a;
    const FLValue* vb = (const FLValue*)b;
    if (va->tag == FL_INT && vb->tag == FL_INT)
        return (va->i < vb->i) ? -1 : (va->i > vb->i) ? 1 : 0;
    if ((va->tag == FL_FLOAT || va->tag == FL_INT) &&
        (vb->tag == FL_FLOAT || vb->tag == FL_INT)) {
        double da = va->tag == FL_INT ? (double)va->i : va->f;
        double db = vb->tag == FL_INT ? (double)vb->i : vb->f;
        return (da < db) ? -1 : (da > db) ? 1 : 0;
    }
    if (va->tag == FL_STRING && vb->tag == FL_STRING)
        return strcmp(((FLString*)va->obj)->data, ((FLString*)vb->obj)->data);
    return 0;
}
FLValue sort(FLValue vec) {
    if (vec.tag != FL_VECTOR) return vec;
    FLVector* vp = (FLVector*)vec.obj;
    FLValue* copy = (FLValue*)malloc(vp->len * sizeof(FLValue));
    memcpy(copy, vp->data, vp->len * sizeof(FLValue));
    qsort(copy, vp->len, sizeof(FLValue), fl_compare);
    FLValue r = fl_vec_from(copy, vp->len);
    free(copy);
    return r;
}

/* reverse */
FLValue reverse(FLValue vec) {
    if (vec.tag != FL_VECTOR) return vec;
    FLVector* vp = (FLVector*)vec.obj;
    FLValue* copy = (FLValue*)malloc(vp->len * sizeof(FLValue));
    for (uint32_t i = 0; i < vp->len; i++)
        copy[i] = vp->data[vp->len - 1 - i];
    FLValue r = fl_vec_from(copy, vp->len);
    free(copy);
    return r;
}

/* zip / uniq */
FLValue zip(FLValue a, FLValue b) {
    if (a.tag != FL_VECTOR || b.tag != FL_VECTOR) return fl_vec_new();
    FLVector* va = (FLVector*)a.obj;
    FLVector* vb = (FLVector*)b.obj;
    uint32_t n = va->len < vb->len ? va->len : vb->len;
    FLValue result = fl_vec_new();
    for (uint32_t i = 0; i < n; i++) {
        FLValue pair = fl_vec_new();
        pair = fl_vec_push(pair, va->data[i]);
        pair = fl_vec_push(pair, vb->data[i]);
        result = fl_vec_push(result, pair);
    }
    return result;
}

/* map 관련 */
FLValue merge(FLValue a, FLValue b) { return fl_map_merge(a, b); }
FLValue dissoc(FLValue m, FLValue k) { return fl_map_del(m, k); }
FLValue vals(FLValue m)  { return fl_map_vals(m); }
FLValue keys(FLValue m)  { return fl_map_keys(m); }

/* assoc (set alias) */
FLValue assoc(FLValue m, FLValue k, FLValue v) { return fl_map_set(m, k, v); }

/* some / every / any */
FLValue some(FLValue fn, FLValue vec) {
    if (vec.tag != FL_VECTOR) return fl_bool(false);
    FLVector* vp = (FLVector*)vec.obj;
    for (uint32_t i = 0; i < vp->len; i++) {
        FLValue r = fl_fn_call(fn, 1, &vp->data[i]);
        if (fl_truthy(r)) return fl_bool(true);
    }
    return fl_bool(false);
}
FLValue every(FLValue fn, FLValue vec) {
    if (vec.tag != FL_VECTOR) return fl_bool(true);
    FLVector* vp = (FLVector*)vec.obj;
    for (uint32_t i = 0; i < vp->len; i++) {
        FLValue r = fl_fn_call(fn, 1, &vp->data[i]);
        if (!fl_truthy(r)) return fl_bool(false);
    }
    return fl_bool(true);
}

/* max / min on vector */
FLValue vec_max(FLValue vec) {
    if (vec.tag != FL_VECTOR) return fl_nil();
    FLVector* vp = (FLVector*)vec.obj;
    if (vp->len == 0) return fl_nil();
    FLValue m = vp->data[0];
    for (uint32_t i = 1; i < vp->len; i++) {
        if (fl_truthy(fl_gt(vp->data[i], m))) m = vp->data[i];
    }
    return m;
}
FLValue vec_min(FLValue vec) {
    if (vec.tag != FL_VECTOR) return fl_nil();
    FLVector* vp = (FLVector*)vec.obj;
    if (vp->len == 0) return fl_nil();
    FLValue m = vp->data[0];
    for (uint32_t i = 1; i < vp->len; i++) {
        if (fl_truthy(fl_lt(vp->data[i], m))) m = vp->data[i];
    }
    return m;
}

/* atom aliases */
FLValue atom(FLValue v)             { return fl_atom_new(v); }
FLValue deref(FLValue a)            { return fl_atom_deref(a); }
FLValue swap_bang(FLValue a, FLValue fn) {
    FLValue cur = fl_atom_deref(a);
    FLValue next = fl_fn_call(fn, 1, &cur);
    return fl_atom_reset(a, next);
}
FLValue reset_bang(FLValue a, FLValue v) { return fl_atom_reset(a, v); }

/* 타입 변환 */
FLValue int_p(FLValue v)    { return fl_bool(v.tag == FL_INT); }
FLValue float_p(FLValue v)  { return fl_bool(v.tag == FL_FLOAT); }
FLValue bool_p(FLValue v)   { return fl_bool(v.tag == FL_BOOL); }
FLValue nil_p(FLValue v)    { return fl_bool(v.tag == FL_NIL); }
FLValue number_p(FLValue v) { return fl_bool(v.tag == FL_INT || v.tag == FL_FLOAT); }

/* 기타 */
FLValue identity(FLValue v) { return v; }
FLValue not_p(FLValue v)    { return fl_not(v); }
FLValue even_p(FLValue v)   { return fl_bool(v.tag == FL_INT && v.i % 2 == 0); }
FLValue odd_p(FLValue v)    { return fl_bool(v.tag == FL_INT && v.i % 2 != 0); }
FLValue inc(FLValue v)      { return fl_add(v, fl_int(1)); }
FLValue dec(FLValue v)      { return fl_sub(v, fl_int(1)); }

/* list = vec 생성 (cgc-bin이 list를 fl_vec_from으로 emit하므로 alias 불필요할 수 있음) */
/* 하지만 혹시 필요할 경우를 위해 */
FLValue list_new0(void)                    { return fl_vec_new(); }

/* fl_vec_* aliases (cgc-bin이 emit하는 내부 이름) */
FLValue fl_vec_first(FLValue vec) { return first(vec); }
/* fl_vec_last: collection.c에 이미 구현됨 */
FLValue fl_vec_rest(FLValue vec)  { return rest(vec);  }

/* fl_concat — 벡터/문자열 연결 */
FLValue fl_concat(FLValue a, FLValue b) {
    if (a.tag == FL_VECTOR && b.tag == FL_VECTOR) {
        FLVector* va = (FLVector*)a.obj;
        FLVector* vb = (FLVector*)b.obj;
        FLValue result = fl_vec_new();
        for (uint32_t i = 0; i < va->len; i++) result = fl_vec_push(result, va->data[i]);
        for (uint32_t i = 0; i < vb->len; i++) result = fl_vec_push(result, vb->data[i]);
        return result;
    }
    /* 문자열 연결 */
    if (a.tag == FL_STRING && b.tag == FL_STRING) {
        const char* sa = fl_cstr(a);
        const char* sb = fl_cstr(b);
        size_t la = strlen(sa), lb = strlen(sb);
        char* buf = (char*)malloc(la + lb + 1);
        memcpy(buf, sa, la); memcpy(buf + la, sb, lb); buf[la + lb] = '\0';
        FLValue r = fl_str_val(buf); free(buf);
        return r;
    }
    return fl_nil();
}

/* concat alias (짧은 이름) */
FLValue concat(FLValue a, FLValue b) { return fl_concat(a, b); }

/* ── 환경 변수 / .env ── */
/* env-load ".env" → env_load(".env") */
FLValue env_load(FLValue path_v) {
    if (path_v.tag != FL_STRING) return fl_nil();
    const char* path = ((FLString*)path_v.obj)->data;
    FILE* f = fopen(path, "r");
    if (!f) return fl_nil();  /* .env 없으면 조용히 무시 */
    char line[512];
    while (fgets(line, sizeof(line), f)) {
        /* 앞뒤 공백 제거 */
        char* p = line;
        while (*p == ' ' || *p == '\t') p++;
        /* 주석/빈 줄 무시 */
        if (*p == '#' || *p == '\n' || *p == '\0') continue;
        /* KEY=VALUE 파싱 */
        char* eq = strchr(p, '=');
        if (!eq) continue;
        *eq = '\0';
        char* key = p;
        char* val = eq + 1;
        /* 값 끝의 개행 제거 */
        size_t vlen = strlen(val);
        while (vlen > 0 && (val[vlen-1] == '\n' || val[vlen-1] == '\r' || val[vlen-1] == '"' || val[vlen-1] == '\''))
            val[--vlen] = '\0';
        /* 값 앞의 따옴표 제거 */
        if (*val == '"' || *val == '\'') val++;
        setenv(key, val, 1);
    }
    fclose(f);
    return fl_nil();
}

/* num "42" → 42 or 42.0 */
FLValue num(FLValue v) {
    if (v.tag == FL_INT || v.tag == FL_FLOAT) return v;
    if (v.tag == FL_STRING) {
        const char* p = ((FLString*)v.obj)->data;
        /* 소수점 있으면 float */
        if (strchr(p, '.')) return fl_float(strtod(p, NULL));
        return fl_int((int64_t)strtoll(p, NULL, 10));
    }
    return fl_nil();
}

/* fl_or: core.c에 이미 구현됨 */

/* server-rate-limit — 현재 no-op (향후 구현 가능) */
FLValue server_rate_limit(FLValue max_reqs, FLValue window_ms) {
    (void)max_reqs; (void)window_ms;
    return fl_nil();
}

/* str_slice — 문자열 슬라이스 (str-slice s start end) */
FLValue str_slice(FLValue s, FLValue start_v, FLValue end_v) {
    /* FL_FLOAT이면 정수로 변환 */
    int64_t st = (start_v.tag == FL_FLOAT) ? (int64_t)start_v.f :
                 (start_v.tag == FL_INT)   ? start_v.i : 0;
    int64_t en = (end_v.tag == FL_FLOAT)   ? (int64_t)end_v.f :
                 (end_v.tag == FL_INT)      ? end_v.i : 0;
    return substring(s, fl_int(st), fl_int(en));
}

/* mariadb_connect 4인자 버전 (host, user, pass, db → port=3306) */
FLValue mariadb_connect4(FLValue host, FLValue user, FLValue pw, FLValue db) {
    return mariadb_connect(host, fl_int(3306), user, pw, db);
}

/* ── URL 디코딩 + 폼 파싱 ── */

/* %XX 디코딩 + '+' → 공백 변환 */
static int hex_val(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    return 0;
}

/* url_decode "hello%20world" → "hello world" */
FLValue url_decode(FLValue s) {
    if (s.tag != FL_STRING) return s;
    const char* src = ((FLString*)s.obj)->data;
    size_t slen = strlen(src);
    char* out = (char*)malloc(slen + 1);
    size_t j = 0;
    for (size_t i = 0; i < slen; i++) {
        if (src[i] == '+') {
            out[j++] = ' ';
        } else if (src[i] == '%' && i + 2 < slen &&
                   isxdigit((unsigned char)src[i+1]) &&
                   isxdigit((unsigned char)src[i+2])) {
            out[j++] = (char)((hex_val(src[i+1]) << 4) | hex_val(src[i+2]));
            i += 2;
        } else {
            out[j++] = src[i];
        }
    }
    out[j] = '\0';
    FLValue r = fl_str_val(out);
    free(out);
    return r;
}

/* ── P0: 이름 불일치 alias ── */

/* fl_str_to_num, str_to_upper/lower (runtime.h에 없는 것만) */
FLValue fl_str_to_num(FLValue s)  { return str_to_num(s); }
FLValue str_to_upper(FLValue s)   { return str_upper(s); }
FLValue str_to_lower(FLValue s)   { return str_lower(s); }

/* any?/every? fl_ alias */
FLValue fl_any_p(FLValue fn, FLValue vec)   { return some(fn, vec); }
FLValue fl_every_p(FLValue fn, FLValue vec) { return every(fn, vec); }

/* ── P0: 미구현 기본 함수 ── */

/* sleep */
FLValue fl_sleep_ms(FLValue ms_v) {
    int64_t ms = (ms_v.tag == FL_INT) ? ms_v.i
               : (ms_v.tag == FL_FLOAT) ? (int64_t)ms_v.f : 0;
    if (ms > 0) {
        struct timespec ts = { ms / 1000, (ms % 1000) * 1000000L };
        nanosleep(&ts, NULL);
    }
    return fl_nil();
}

/* fl_empty_p, fl_nil_or_empty_p, fl_not_empty_p → runtime.h에 static inline으로 이미 정의됨 */

/* none? — 하나도 true 없어야 */
FLValue fl_none_p(FLValue fn, FLValue vec) {
    return fl_not(some(fn, vec));
}

/* count-if */
FLValue fl_count_if(FLValue fn, FLValue vec) {
    if (vec.tag != FL_VECTOR) return fl_int(0);
    FLVector* vp = (FLVector*)vec.obj;
    int64_t cnt = 0;
    for (uint32_t i = 0; i < vp->len; i++) {
        if (fl_truthy(fl_fn_call(fn, 1, &vp->data[i]))) cnt++;
    }
    return fl_int(cnt);
}

/* find-first */
FLValue fl_find_first(FLValue fn, FLValue vec) {
    if (vec.tag != FL_VECTOR) return fl_nil();
    FLVector* vp = (FLVector*)vec.obj;
    for (uint32_t i = 0; i < vp->len; i++) {
        if (fl_truthy(fl_fn_call(fn, 1, &vp->data[i]))) return vp->data[i];
    }
    return fl_nil();
}

/* ── P1: 컬렉션 함수 ── */

/* entries: 맵 → [[k v] ...] */
FLValue entries(FLValue m) {
    if (m.tag != FL_MAP) return fl_vec_new();
    FLValue ks = fl_map_keys(m);
    FLVector* kv = (FLVector*)ks.obj;
    FLValue result = fl_vec_new();
    for (uint32_t i = 0; i < kv->len; i++) {
        FLValue k = kv->data[i];
        FLValue v = fl_map_get(m, k);
        FLValue pair = fl_vec_new();
        pair = fl_vec_push(pair, k);
        pair = fl_vec_push(pair, v);
        result = fl_vec_push(result, pair);
    }
    return result;
}

/* select-keys: 맵에서 특정 키만 추출 */
FLValue select_keys(FLValue m, FLValue ks) {
    if (m.tag != FL_MAP || ks.tag != FL_VECTOR) return fl_map_new();
    FLVector* kv = (FLVector*)ks.obj;
    FLValue result = fl_map_new();
    for (uint32_t i = 0; i < kv->len; i++) {
        FLValue v = fl_map_get(m, kv->data[i]);
        if (v.tag != FL_NIL) result = fl_map_set(result, kv->data[i], v);
    }
    return result;
}

/* get-in: 중첩 맵 접근 (get-in m ["a" "b"]) */
FLValue fl_get_in(FLValue m, FLValue path) {
    if (path.tag != FL_VECTOR) return fl_nil();
    FLVector* pv = (FLVector*)path.obj;
    FLValue cur = m;
    for (uint32_t i = 0; i < pv->len; i++) {
        if (cur.tag != FL_MAP) return fl_nil();
        cur = fl_map_get(cur, pv->data[i]);
    }
    return cur;
}

/* obj-omit: 맵에서 키 제거 */
FLValue fl_obj_omit(FLValue m, FLValue ks) {
    if (m.tag != FL_MAP) return m;
    if (ks.tag != FL_VECTOR) return m;
    FLVector* kv = (FLVector*)ks.obj;
    FLValue result = m;
    for (uint32_t i = 0; i < kv->len; i++)
        result = fl_map_del(result, kv->data[i]);
    return result;
}

/* repeat: n번 값 반복 → 벡터 */
FLValue fl_repeat(FLValue n_v, FLValue val) {
    int64_t n = (n_v.tag == FL_INT) ? n_v.i : 0;
    FLValue result = fl_vec_new();
    for (int64_t i = 0; i < n; i++) result = fl_vec_push(result, val);
    return result;
}

/* sort-by: keyfn으로 정렬 */
static FLValue _sort_by_fn;
static int _sort_by_cmp(const void* a, const void* b) {
    FLValue ka = fl_fn_call(_sort_by_fn, 1, (FLValue*)a);
    FLValue kb = fl_fn_call(_sort_by_fn, 1, (FLValue*)b);
    if (ka.tag == FL_INT && kb.tag == FL_INT) return (ka.i < kb.i) ? -1 : (ka.i > kb.i) ? 1 : 0;
    if (ka.tag == FL_STRING && kb.tag == FL_STRING)
        return strcmp(((FLString*)ka.obj)->data, ((FLString*)kb.obj)->data);
    double da = (ka.tag == FL_FLOAT) ? ka.f : (double)ka.i;
    double db = (kb.tag == FL_FLOAT) ? kb.f : (double)kb.i;
    return (da < db) ? -1 : (da > db) ? 1 : 0;
}
FLValue fl_sort_by(FLValue fn, FLValue vec) {
    if (vec.tag != FL_VECTOR) return vec;
    FLVector* vp = (FLVector*)vec.obj;
    FLValue* copy = (FLValue*)malloc(vp->len * sizeof(FLValue));
    memcpy(copy, vp->data, vp->len * sizeof(FLValue));
    _sort_by_fn = fn;
    qsort(copy, vp->len, sizeof(FLValue), _sort_by_cmp);
    FLValue r = fl_vec_from(copy, vp->len);
    free(copy);
    return r;
}

/* keep: fn 결과가 nil이 아닌 것만 모음 */
FLValue fl_keep(FLValue fn, FLValue vec) {
    if (vec.tag != FL_VECTOR) return fl_vec_new();
    FLVector* vp = (FLVector*)vec.obj;
    FLValue result = fl_vec_new();
    for (uint32_t i = 0; i < vp->len; i++) {
        FLValue r = fl_fn_call(fn, 1, &vp->data[i]);
        if (r.tag != FL_NIL) result = fl_vec_push(result, r);
    }
    return result;
}

/* map-indexed: (fn index elem) */
FLValue fl_map_indexed(FLValue fn, FLValue vec) {
    if (vec.tag != FL_VECTOR) return fl_vec_new();
    FLVector* vp = (FLVector*)vec.obj;
    FLValue result = fl_vec_new();
    for (uint32_t i = 0; i < vp->len; i++) {
        FLValue args[2] = { fl_int((int64_t)i), vp->data[i] };
        result = fl_vec_push(result, fl_fn_call(fn, 2, args));
    }
    return result;
}

/* mapcat: map + flatten 1단계 */
FLValue fl_mapcat(FLValue fn, FLValue vec) {
    return flatten(fl_map_fn(fn, vec));
}

/* into: coll에 항목 추가 */
FLValue fl_into(FLValue target, FLValue src) {
    if (src.tag != FL_VECTOR) return target;
    FLVector* sv = (FLVector*)src.obj;
    FLValue result = target;
    if (result.tag != FL_VECTOR) result = fl_vec_new();
    for (uint32_t i = 0; i < sv->len; i++) result = fl_vec_push(result, sv->data[i]);
    return result;
}

/* conj: 컬렉션에 항목 추가 */
FLValue fl_conj(FLValue coll, FLValue item) {
    if (coll.tag == FL_VECTOR) return fl_vec_push(coll, item);
    return fl_vec_push(fl_vec_new(), item);
}

/* comp: 함수 합성 (comp f g) → (fn [x] (f (g x))) */
/* comp은 고차함수라 FL 클로저로 래핑이 필요하나 C에서 단순 구현 */
/* 현재는 2-인자 직접 호출만 지원 */
FLValue fl_comp(FLValue f, FLValue g) {
    /* fl_comp(f, g) 자체는 잘 안 쓰임 — 대신 cgc가 직접 emit */
    /* 여기서는 fl_fn_call로 compose 함수 반환은 불가, 대신 apply */
    (void)f; (void)g;
    return fl_nil();  /* TODO: closure 지원 시 개선 */
}

/* map-vals: 맵의 모든 값에 fn 적용 */
FLValue fl_map_vals_fn(FLValue fn, FLValue m) {
    if (m.tag != FL_MAP) return m;
    FLValue ks = fl_map_keys(m);
    FLVector* kv = (FLVector*)ks.obj;
    FLValue result = fl_map_new();
    for (uint32_t i = 0; i < kv->len; i++) {
        FLValue k = kv->data[i];
        FLValue v = fl_map_get(m, k);
        FLValue nv = fl_fn_call(fn, 1, &v);
        result = fl_map_set(result, k, nv);
    }
    return result;
}

/* frequencies: 빈도 집계 */
FLValue frequencies(FLValue vec) {
    if (vec.tag != FL_VECTOR) return fl_map_new();
    FLVector* vp = (FLVector*)vec.obj;
    FLValue m = fl_map_new();
    for (uint32_t i = 0; i < vp->len; i++) {
        FLValue k = vp->data[i];
        FLValue cur = fl_map_get(m, k);
        int64_t cnt = (cur.tag == FL_INT) ? cur.i : 0;
        m = fl_map_set(m, k, fl_int(cnt + 1));
    }
    return m;
}

/* group-by: 키 함수로 그룹핑 */
FLValue group_by(FLValue fn, FLValue vec) {
    if (vec.tag != FL_VECTOR) return fl_map_new();
    FLVector* vp = (FLVector*)vec.obj;
    FLValue m = fl_map_new();
    for (uint32_t i = 0; i < vp->len; i++) {
        FLValue k = fl_fn_call(fn, 1, &vp->data[i]);
        FLValue cur = fl_map_get(m, k);
        if (cur.tag != FL_VECTOR) cur = fl_vec_new();
        cur = fl_vec_push(cur, vp->data[i]);
        m = fl_map_set(m, k, cur);
    }
    return m;
}

/* ── P2: 응답 쿠키 ── */
FLValue fl_resp_set_cookie(FLValue name, FLValue val, FLValue opts) {
    /* opts: {"path" "/" "max-age" 3600 "httponly" true} */
    const char* n = (name.tag == FL_STRING) ? ((FLString*)name.obj)->data : "";
    const char* v = (val.tag  == FL_STRING) ? ((FLString*)val.obj)->data  : "";
    char buf[512];
    int pos = snprintf(buf, sizeof(buf), "%s=%s", n, v);
    /* path */
    FLValue path = (opts.tag == FL_MAP) ? fl_map_get(opts, fl_str_val("path")) : fl_nil();
    if (path.tag == FL_STRING)
        pos += snprintf(buf+pos, sizeof(buf)-pos, "; Path=%s", ((FLString*)path.obj)->data);
    else
        pos += snprintf(buf+pos, sizeof(buf)-pos, "; Path=/");
    /* max-age */
    FLValue max_age = (opts.tag == FL_MAP) ? fl_map_get(opts, fl_str_val("max-age")) : fl_nil();
    if (max_age.tag == FL_INT)
        pos += snprintf(buf+pos, sizeof(buf)-pos, "; Max-Age=%lld", (long long)max_age.i);
    /* HttpOnly */
    FLValue httponly = (opts.tag == FL_MAP) ? fl_map_get(opts, fl_str_val("httponly")) : fl_nil();
    if (fl_truthy(httponly))
        pos += snprintf(buf+pos, sizeof(buf)-pos, "; HttpOnly");
    (void)pos;
    return fl_str_val(buf);
}
FLValue fl_resp_html_cookie(FLValue html, FLValue cookie_hdr) {
    /* cookie_hdr는 fl_resp_set_cookie가 반환한 헤더 문자열 */
    /* 현재 server.c는 직접 Set-Cookie 헤더를 넣는 방법이 없음 */
    /* 임시: html 그대로 반환 (향후 server.c 개선 시 수정) */
    (void)cookie_hdr;
    return server_html(html);
}

/* form_parse "title=hello&content=world" → {"title":"hello","content":"world"} */
FLValue form_parse(FLValue body) {
    if (body.tag != FL_STRING) return fl_map_new();
    const char* src = ((FLString*)body.obj)->data;
    FLValue map = fl_map_new();
    if (!src || src[0] == '\0') return map;

    /* 복사본으로 작업 */
    char* buf = strdup(src);
    char* pair = buf;
    while (pair && *pair) {
        char* next = strchr(pair, '&');
        if (next) *next = '\0';

        char* eq = strchr(pair, '=');
        FLValue key, val;
        if (eq) {
            *eq = '\0';
            key = url_decode(fl_str_val(pair));
            val = url_decode(fl_str_val(eq + 1));
        } else {
            key = url_decode(fl_str_val(pair));
            val = fl_str_val("");
        }
        map = fl_map_set(map, key, val);

        pair = next ? next + 1 : NULL;
    }
    free(buf);
    return map;
}
