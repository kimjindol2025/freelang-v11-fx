static int expected_timer_exception(void);
#define L01_EXPECTED_EXCEPTIONS 1
#define L01_PROOF_EXTRA() expected_timer_exception()
#include "browser-proof.c"
static int expected_timer_exception(void){
 FLValue exception=field(last_browser_exception,"exception");
 int ok=exceptions==1&&strstr(text(field(exception,"value")),"L01_TIMER_EXPECTED_EXCEPTION")!=NULL;
 printf("J_TIMER_EXCEPTION_OBSERVED=%s\n",ok?"PASS":"FAIL");return ok;
}
