#include "runtime.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

/* FL:USER_SECTION:BEGIN */
/* FL:FN:str-indent */
FLValue ufl_str_indent(FLValue s, FLValue n) {
    if (s.tag != FL_STRING || n.tag != FL_INT) return s;
    int64_t cnt = n.i < 0 ? 0 : n.i;
    const char* src = ((FLString*)s.obj)->data;
    size_t slen = strlen(src);
    char* buf = (char*)fl_arena_alloc(cnt + slen + 1);
    if (!buf) return s;
    memset(buf, 32, cnt);
    memcpy(buf + cnt, src, slen);
    buf[cnt + slen] = 0;
    return fl_str_val(buf);
}
/* FL:FN_END */
/* FL:USER_SECTION:END */
