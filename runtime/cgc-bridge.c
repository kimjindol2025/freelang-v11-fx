/* Native compiler helpers. Parser functions use the same named binding registry
 * as application callbacks; no dependency on mangled user C symbols. */
#include "runtime.h"
#include "compiler-bindings.h"
FLValue fl_parse(FLValue src) {
    FLValue tokens=fl_fn_call(fl_named_fn(fl_str_val("lex")),1,&src);
    return fl_fn_call(fl_named_fn(fl_str_val("parse")),1,&tokens);
}
#include "browser-target.h"
