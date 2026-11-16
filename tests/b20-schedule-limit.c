#include "runtime.h"
#include <stdio.h>
#include <string.h>
#include <time.h>

static int64_t fake_wall_ms=2000000;
static int64_t fake_mono_ms=100000;

int __wrap_clock_gettime(clockid_t id,struct timespec *out){
    int64_t ms=id==CLOCK_REALTIME?fake_wall_ms:id==CLOCK_MONOTONIC?fake_mono_ms:-1;
    if(ms<0)return -1;
    out->tv_sec=(time_t)(ms/1000);out->tv_nsec=(long)(ms%1000)*1000000L;return 0;
}

static FLValue callback(FLClosure *self, int argc, FLValue *argv) {
    (void)self; (void)argc; (void)argv;
    return fl_int(1);
}

int main(void) {
    fl_arena_begin();
    FLValue fn = fl_fn_new(callback, 0, NULL);
    int64_t due = fl_now_ms().i + 600000;
    for (int i = 4095; i >= 0; i--) {
        fake_mono_ms--;
        FLValue args[] = {fl_int(due), fn};
        (void)fl_v11_timer(fl_str_val("schedule-at"), fl_vec_from(args, 2));
    }
    int saved = fl_try_top;
    FLTryFrame *frame = &fl_try_stack[fl_try_top++];
    if (setjmp(frame->buf) == 0) {
        fake_mono_ms--;
        FLValue args[] = {fl_int(due), fn};
        (void)fl_v11_timer(fl_str_val("schedule-at"), fl_vec_from(args, 2));
        return 1;
    }
    fl_try_top = saved;
    FLValue message = fl_map_get(frame->err, fl_str_val("message"));
    if (message.tag != FL_STRING ||
        strcmp(((FLString*)message.obj)->data, "timer unavailable or pending limit exceeded")) return 2;
    puts("SCHEDULE_4096_ACCEPTED=PASS");
    puts("SCHEDULE_4097_CLEAR_ERROR=PASS");
    fl_schedule_worker_cleanup();
    fl_arena_end();
    return 0;
}
