/* mail-stub.c — --bundle without mail: smtp nil stubs */
#include "runtime.h"

FLValue fl_smtp_send(FLValue from, FLValue to, FLValue subject, FLValue body) {
    (void)from; (void)to; (void)subject; (void)body;
    return fl_str_val("[mail-stub] mail bundle not linked");
}
FLValue fl_smtp_send_html(FLValue from, FLValue to, FLValue subject, FLValue html) {
    (void)from; (void)to; (void)subject; (void)html;
    return fl_str_val("[mail-stub] mail bundle not linked");
}
FLValue fl_smtp_test(FLValue to) {
    (void)to;
    return fl_str_val("[mail-stub] mail bundle not linked");
}
