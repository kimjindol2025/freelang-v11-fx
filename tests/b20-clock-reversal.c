#define _GNU_SOURCE
#include "runtime.h"
#include <stdio.h>
#include <time.h>

static int64_t fake_wall_ms = 2000000;
static int64_t fake_mono_ms = 100000;
static int calls;

int __wrap_clock_gettime(clockid_t id, struct timespec *out) {
    int64_t ms;
    if (id == CLOCK_REALTIME) ms = fake_wall_ms;
    else if (id == CLOCK_MONOTONIC) ms = fake_mono_ms;
    else return -1;
    out->tv_sec = (time_t)(ms / 1000);
    out->tv_nsec = (long)(ms % 1000) * 1000000L;
    return 0;
}

static FLValue callback(FLClosure *self, int argc, FLValue *argv) {
    (void)self; (void)argc; (void)argv;
    calls++;
    return fl_int(calls);
}

int main(void) {
    fl_arena_begin();
    FLValue fn = fl_fn_new(callback, 0, NULL);
    const int64_t at = fake_wall_ms + 3000;
    FLValue args[] = {fl_int(at), fn};
    FLValue promise = fl_v11_timer(fl_str_val("schedule-at"), fl_vec_from(args, 2));
    fake_wall_ms -= 60000; /* wall clock moved backward after registration */
    for (int i = 0; i < 8; i++) {
        fake_mono_ms += 1100;
        fl_schedule_tick(0);
        if (calls) return 1;
    }
    fake_wall_ms = at;
    fake_mono_ms += 1100;
    fl_schedule_tick(0);
    if (calls != 1 || fl_v11_timer(fl_str_val("await-result"), fl_vec_from(&promise, 1)).i != 1) return 2;
    fake_mono_ms += 10000;
    fl_schedule_tick(0);
    if (calls != 1) return 3;
    puts("SCHEDULE_CLOCK_BACKWARD_NO_EARLY_RUN=PASS");
    puts("SCHEDULE_CLOCK_BACKWARD_EVENTUAL_ONCE=PASS");
    fl_arena_end();
    return 0;
}
