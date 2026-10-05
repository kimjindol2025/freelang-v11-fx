#include "runtime.h"
#include <stdio.h>

int main(int argc, char **argv) {
    if (argc != 2) {
        fputs("usage: test-http-tls-verification URL\n", stderr);
        return 64;
    }

    FLValue response = http_get(fl_str_val(argv[1]));
    FLValue status = http_status(response);
    FLValue error = fl_map_get(response, fl_str_val("error"));
    if (status.tag == FL_INT && status.i == 0 &&
        error.tag == FL_STRING && error.obj &&
        ((FLString *)error.obj)->len > 0) {
        printf("TLS_VERIFY_REJECTED: %s\n", ((FLString *)error.obj)->data);
        return 0;
    }

    fprintf(stderr, "FAIL untrusted TLS response accepted (status tag=%d)\n", status.tag);
    return 1;
}
