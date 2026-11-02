#ifndef WORKER_CLEANUP_SUPPORT_H
#define WORKER_CLEANUP_SUPPORT_H
#include "runtime.h"
FLValue worker_timer_run(FLValue mode,FLValue setup);
FLValue worker_observe(FLValue text);
FLValue worker_pending(void);
FLValue worker_exit(void);
#endif
