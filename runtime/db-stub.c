/* db-stub.c — --bundle without db: sqlite/mariadb nil stubs */
#include "runtime.h"

FLValue fl_rt_db(FLValue name_v) {
    (void)name_v;
    return fl_str_val("[db-stub] db bundle not linked");
}
FLValue db_transaction(FLValue c, FLValue s) { (void)c; (void)s; return fl_nil(); }
FLValue db_query(FLValue c, FLValue sql, FLValue p) { (void)c; (void)sql; (void)p; return fl_nil(); }
FLValue db_exec(FLValue c, FLValue sql, FLValue p) { (void)c; (void)sql; (void)p; return fl_nil(); }

FLValue fxb_sqlite_open(FLValue path) { (void)path; return fl_nil(); }
FLValue fxb_sqlite_query(FLValue c, FLValue sql) { (void)c; (void)sql; return fl_nil(); }
FLValue fxb_sqlite_exec(FLValue c, FLValue sql) { (void)c; (void)sql; return fl_nil(); }
FLValue fxb_sqlite_one(FLValue c, FLValue sql) { (void)c; (void)sql; return fl_nil(); }
FLValue fxb_sqlite_query_p(FLValue c, FLValue sql, FLValue p) { (void)c; (void)sql; (void)p; return fl_nil(); }
FLValue fxb_sqlite_exec_p(FLValue c, FLValue sql, FLValue p) { (void)c; (void)sql; (void)p; return fl_nil(); }
FLValue fxb_sqlite_one_p(FLValue c, FLValue sql, FLValue p) { (void)c; (void)sql; (void)p; return fl_nil(); }
FLValue fxb_sqlite_close(FLValue c) { (void)c; return fl_nil(); }

FLValue mariadb_connect(FLValue h, FLValue port, FLValue u, FLValue pw, FLValue db) {
    (void)h; (void)port; (void)u; (void)pw; (void)db;
    return fl_str_val("[db-stub] mariadb bundle not linked");
}
FLValue mariadb_query(FLValue c, FLValue sql) { (void)c; (void)sql; return fl_nil(); }
FLValue mariadb_exec(FLValue c, FLValue sql) { (void)c; (void)sql; return fl_nil(); }
FLValue mariadb_one(FLValue c, FLValue sql) { (void)c; (void)sql; return fl_nil(); }
FLValue mariadb_close(FLValue c) { (void)c; return fl_nil(); }
FLValue mariadb_query_p(FLValue c, FLValue sql, FLValue p) { (void)c; (void)sql; (void)p; return fl_nil(); }
FLValue mariadb_exec_p(FLValue c, FLValue sql, FLValue p) { (void)c; (void)sql; (void)p; return fl_nil(); }
FLValue mariadb_one_p(FLValue c, FLValue sql, FLValue p) { (void)c; (void)sql; (void)p; return fl_nil(); }
