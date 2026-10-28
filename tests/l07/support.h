#ifndef L07_TEST_SUPPORT_H
#define L07_TEST_SUPPORT_H
#include "runtime.h"
FLValue l07_check(FLValue condition, FLValue label);
FLValue l07_error(FLValue result, FLValue error, FLValue label);
FLValue l07_same(FLValue source, FLValue destination);
FLValue l07_append_matches(FLValue source, FLValue destination);
FLValue l07_raw_size(FLValue path);
FLValue l07_fd_count(void);
FLValue l07_arm(FLValue fault);
FLValue l07_bad_value(FLValue kind);
FLValue l07_finish(void);
#endif
