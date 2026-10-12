#include "runtime.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <setjmp.h>

/* 스레드마다 독립된 try 스택 */
__thread FLTryFrame fl_try_stack[FL_TRY_MAX];
__thread int fl_try_top = 0;

/* FL 소스 라인 추적 (cgc-main이 throw 직전에 설정) */
int __fl_throw_line = 0;

/* ── 콜스택 추적 ── */
#define FL_CALLSTACK_MAX 64
__thread const char* __fl_callstack[FL_CALLSTACK_MAX];
__thread int         __fl_callstack_line[FL_CALLSTACK_MAX];
__thread int __fl_callstack_depth = 0;

void fl_push_frame(const char* fn) {
    if (__fl_callstack_depth < FL_CALLSTACK_MAX) {
        __fl_callstack[__fl_callstack_depth]      = fn;
        __fl_callstack_line[__fl_callstack_depth] = 0;
        __fl_callstack_depth++;
    }
}
void fl_push_frame_ln(const char* fn, int line) {
    if (__fl_callstack_depth < FL_CALLSTACK_MAX) {
        __fl_callstack[__fl_callstack_depth]      = fn;
        __fl_callstack_line[__fl_callstack_depth] = line;
        __fl_callstack_depth++;
        if (line > 0) __fl_throw_line = line;
    }
}
void fl_pop_frame(void) {
    if (__fl_callstack_depth > 0) __fl_callstack_depth--;
}

/* ERROR-SCHEMA.md: 파일:줄 | 오류종류 | 받은 값 | 이유 | 고치는 법 */
static void sanitize_field(char* dst, size_t n, const char* src) {
    if (!src) src = "";
    size_t j = 0;
    for (size_t i = 0; src[i] && j + 1 < n; i++) {
        char c = src[i];
        if (c == '|' || c == '\n' || c == '\r') c = '/';
        dst[j++] = c;
    }
    dst[j] = '\0';
}

/* 자주 나는 오류 5종 — 메시지 휴리스틱으로 고치는 법 채움 */
static const char* fix_hint_for(const char* msg) {
    if (!msg) return "try/catch로 감싸거나 Result/*-safe로 실패를 반환";
    if (strstr(msg, "0으로 나눌") || strstr(msg, "ArithmeticError"))
        return "나눗셈 전에 분모가 0인지 확인";
    if (strstr(msg, "nil에 key") || strstr(msg, "(get nil"))
        return "nil 체크 후 get 하거나 기본값/get-in 사용";
    if (strstr(msg, "TypeError"))
        return "인자 타입을 맞추거나 type_of로 확인";
    if (strstr(msg, "[json]") || strstr(msg, "파싱 실패"))
        return "JSON 문자열 형식을 확인";
    if (strstr(msg, "[DB_") || strstr(msg, "[sqlite]"))
        return "SQL·연결 핸들을 확인하고 *-safe/Result로 실패를 받기";
    return "try/catch로 감싸거나 Result/*-safe로 실패를 반환";
}

static const char* reason_hint_for(const char* msg) {
    if (!msg) return "throw가 catch 없이 전파됨";
    if (strstr(msg, "0으로 나눌")) return "정수/실수 나눗셈 분모가 0";
    if (strstr(msg, "nil에 key")) return "nil 맵/값에 get 호출";
    if (strstr(msg, "TypeError")) return "연산자·함수가 기대 타입과 불일치";
    if (strstr(msg, "[json]")) return "JSON 파싱 실패";
    if (strstr(msg, "[DB_") || strstr(msg, "[sqlite]")) return "DB 연산 실패";
    return "throw가 catch 없이 전파됨";
}

static void print_schema_line(const char* kind, const char* value,
                              const char* reason, const char* fix) {
    int lineno = __fl_throw_line;
    char linebuf[32];
    if (lineno > 0) snprintf(linebuf, sizeof(linebuf), "%d", lineno);
    else snprintf(linebuf, sizeof(linebuf), "—");

    char vbuf[256], rbuf[256], fbuf[256], kbuf[64];
    sanitize_field(kbuf, sizeof(kbuf), kind ? kind : "uncaught");
    sanitize_field(vbuf, sizeof(vbuf), value ? value : "");
    sanitize_field(rbuf, sizeof(rbuf), reason ? reason : "");
    sanitize_field(fbuf, sizeof(fbuf), fix ? fix : "");

    /* 파일 경로는 아직 throw 지점에 없음 → -:줄 (ERROR-SCHEMA §2.1) */
    fprintf(stderr, "-:%s | %s | %s | %s | %s\n",
            linebuf, kbuf, vbuf, rbuf, fbuf);
}

static const char* map_str_field(FLValue m, const char* key) {
    if (m.tag != FL_MAP) return NULL;
    FLValue v = fl_map_get(m, fl_str_val(key));
    if (v.tag == FL_STRING && v.obj) return ((FLString*)v.obj)->data;
    return NULL;
}

static void print_uncaught(FLValue err) {
    int lineno = __fl_throw_line;
    char locbuf[32] = "";
    if (lineno > 0) snprintf(locbuf, sizeof(locbuf), " (line %d)", lineno);

    const char* kind = "uncaught";
    const char* value = "";
    const char* reason = "throw가 catch 없이 전파됨";
    const char* fix = "try/catch로 감싸거나 Result/*-safe로 실패를 반환";
    const char* legacy_msg = NULL;

    if (err.tag == FL_STRING && err.obj) {
        legacy_msg = ((FLString*)err.obj)->data;
        value = legacy_msg;
        kind = "uncaught";
        if (strstr(legacy_msg, "TypeError")) kind = "builtin";
        else if (strstr(legacy_msg, "ArithmeticError")) kind = "builtin";
        else if (strstr(legacy_msg, "(get nil")) kind = "builtin";
        else if (strstr(legacy_msg, "[json]")) kind = "builtin";
        else if (strstr(legacy_msg, "[DB_") || strstr(legacy_msg, "[sqlite]"))
            kind = "builtin";
        reason = reason_hint_for(legacy_msg);
        fix = fix_hint_for(legacy_msg);
    } else if (err.tag == FL_MAP) {
        const char* t = map_str_field(err, "type");
        const char* msg = map_str_field(err, "message");
        const char* val = map_str_field(err, "value");
        const char* fx = map_str_field(err, "fix");
        if (t && *t) kind = t;
        legacy_msg = msg ? msg : "";
        value = (val && *val) ? val : (msg ? msg : "");
        reason = msg ? reason_hint_for(msg) : reason;
        if (fx && *fx) fix = fx;
        else if (msg) fix = fix_hint_for(msg);
    }

    /* ① 스키마 한 줄을 먼저 */
    print_schema_line(kind, value, reason, fix);

    /* ② 기존 Uncaught 줄 + 스택은 그 아래 유지 */
    if (err.tag == FL_STRING && err.obj)
        fprintf(stderr, "Uncaught error%s: %s\n", locbuf, ((FLString*)err.obj)->data);
    else if (err.tag == FL_MAP) {
        FLValue msg = fl_map_get(err, fl_str_val("message"));
        if (msg.tag == FL_STRING && msg.obj)
            fprintf(stderr, "Uncaught error%s: %s\n", locbuf, ((FLString*)msg.obj)->data);
        else fprintf(stderr, "Uncaught error%s\n", locbuf);
    } else fprintf(stderr, "Uncaught error%s\n", locbuf);

    if (__fl_callstack_depth > 0) {
        fprintf(stderr, "Stack trace:\n");
        for (int i = __fl_callstack_depth - 1; i >= 0; i--) {
            int ln = __fl_callstack_line[i];
            if (ln > 0)
                fprintf(stderr, "  at %s (line %d)\n", __fl_callstack[i], ln);
            else
                fprintf(stderr, "  at %s\n", __fl_callstack[i]);
        }
    }
}

void fl_throw(FLValue err) {
    if (fl_try_top <= 0) {
        print_uncaught(err);
        exit(1);
    }
    fl_try_stack[fl_try_top - 1].err = err;
    longjmp(fl_try_stack[fl_try_top - 1].buf, 1);
}

FLValue fl_make_error(const char* type, const char* msg,
                      const char* value, const char* fix) {
    FLValue m = fl_map_new();
    m = fl_map_set(m, fl_str_val("type"),    fl_str_val(type ? type : "uncaught"));
    m = fl_map_set(m, fl_str_val("message"), fl_str_val(msg ? msg : ""));
    m = fl_map_set(m, fl_str_val("value"),   fl_str_val(value ? value : ""));
    m = fl_map_set(m, fl_str_val("fix"),     fl_str_val(fix ? fix : ""));
    return m;
}

/* Result: [값 nil] / [nil 메시지] — fx-std result-ok / result-fail 과 동일 형태 */
FLValue fl_result_ok(FLValue v) {
    FLValue items[2];
    items[0] = v;
    items[1] = fl_nil();
    return fl_vec_from(items, 2);
}

FLValue fl_result_fail(FLValue msg) {
    FLValue items[2];
    items[0] = fl_nil();
    items[1] = msg;
    return fl_vec_from(items, 2);
}
