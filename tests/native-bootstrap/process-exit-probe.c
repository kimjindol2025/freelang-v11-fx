#include "runtime.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
static int cleaned;
static void arena_cleanup(void *unused){
    (void)unused;cleaned++;
    puts("ARENA_CLEANUP=PASS");
}
static void observe_cleanup(void){
    if(cleaned!=1){fputs("ATEXIT_CLEANUP=FAIL\n",stderr);_Exit(99);}
    puts("ATEXIT_CLEANUP=PASS");
}
int main(int argc,char **argv){
    assert(argc==2);int code=atoi(argv[1]);
    assert(atexit(observe_cleanup)==0);
    fl_cli_runtime_begin();
    assert(fl_arena_on_reset(arena_cleanup,NULL)==1);
    printf("NATIVE_BEFORE_EXIT=%d|",code);
    process_exit(fl_int(code));
    fputs("PROCESS_EXIT_RETURNED=FAIL\n",stderr);return 99;
}
