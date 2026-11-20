#ifndef FREELANG_PROCESS_INPUT_H
#define FREELANG_PROCESS_INPUT_H
#include <stddef.h>
#include <stdint.h>

/* Internal, literal process boundary. Inputs are anonymous stdin bytes; argv
 * must select an absolute executable. Captured streams share the selected byte
 * limit. No shell, application protocol or filesystem policy lives here. */
typedef struct {
    char *data;
    size_t size;
} FLProcessBytes;
typedef struct {
    FLProcessBytes out, err;
    int exit_code, signal_number, spawn_error, io_error;
    int timed_out, overflow, cancelled;
} FLProcessInputResult;
int fl_process_input(const char *const argv[], const void *input, size_t size,
                     int64_t timeout_ms, size_t output_limit,
                     FLProcessInputResult *result);
void fl_process_input_release(FLProcessInputResult *result);
#endif
