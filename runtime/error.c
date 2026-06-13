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

static void print_uncaught(FLValue err) {
    int lineno = __fl_throw_line;
    const char* loc = lineno > 0 ? "" : "";
    char locbuf[32] = "";
    if (lineno > 0) snprintf(locbuf, sizeof(locbuf), " (line %d)", lineno);
    (void)loc;
    if (err.tag == FL_STRING && err.obj)
        fprintf(stderr, "Uncaught error%s: %s\n", locbuf, ((FLString*)err.obj)->data);
    else if (err.tag == FL_MAP) {
        FLValue msg = fl_map_get(err, fl_str_val("message"));
        if (msg.tag == FL_STRING && msg.obj)
            fprintf(stderr, "Uncaught error%s: %s\n", locbuf, ((FLString*)msg.obj)->data);
        else fprintf(stderr, "Uncaught error%s\n", locbuf);
    } else fprintf(stderr, "Uncaught error%s\n", locbuf);
}

void fl_throw(FLValue err) {
    if (fl_try_top <= 0) {
        print_uncaught(err);
        exit(1);
    }
    fl_try_stack[fl_try_top - 1].err = err;
    longjmp(fl_try_stack[fl_try_top - 1].buf, 1);
}

FLValue fl_make_error(const char* type, const char* msg) {
    FLValue m = fl_map_new();
    m = fl_map_set(m, fl_str_val("type"),    fl_str_val(type));
    m = fl_map_set(m, fl_str_val("message"), fl_str_val(msg));
    return m;
}
