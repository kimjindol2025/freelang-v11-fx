#include "runtime.h"
#include <stdio.h>
#include <string.h>

int main(int argc, char **argv) {
    fl_init_argv(argc, argv);

    if (argc > 1 && strcmp(argv[1], "--exit-probe") == 0) {
        fl_println(fl_str_val("before-exit"));
        exit(fl_int(42));
    }
    if (argc > 1 && strcmp(argv[1], "--invalid-exit-probe") == 0) {
        exit(fl_str_val("bad"));
    }

    FLValue first = get_arg(fl_int(0));
    FLValue args = sys(fl_str_val("args"));

    if (first.tag != FL_STRING || strcmp(((FLString*)first.obj)->data, "alpha") != 0) {
        fputs("FAIL get_arg\n", stderr);
        return 1;
    }
    if (args.tag != FL_VECTOR || fl_vec_len(args).i != 2 ||
        strcmp(((FLString*)fl_vec_get(args, fl_int(1)).obj)->data, "beta") != 0) {
        fputs("FAIL sys:args\n", stderr);
        return 1;
    }
    if (sys(fl_str_val("unknown")).tag != FL_NIL) {
        fputs("FAIL sys unknown key\n", stderr);
        return 1;
    }

    puts("PASS script-abi argv");
    return 0;
}
