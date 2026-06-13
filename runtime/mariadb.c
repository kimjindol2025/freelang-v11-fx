/*
 * mariadb.c — FreeLang 네이티브 MariaDB 바인딩
 * 헤더 없이 dlopen/dlsym으로 런타임 로딩 (libmariadb-dev 불필요)
 *
 * 지원 함수:
 *   (mariadb_connect host port user pw db)  → conn 핸들(문자열)
 *   (mariadb_query conn sql)               → 결과 벡터 (맵의 벡터)
 *   (mariadb_exec conn sql)                → {"affected": N}
 *   (mariadb_one conn sql)                 → 단일 행 맵 or nil
 *   (mariadb_close conn)                   → nil
 */

#include "runtime.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>
#include <pthread.h>
#include <stdint.h>

/* ── 타입 정의 (opaque pointer로 처리) ── */
typedef void MYSQL;
typedef void MYSQL_RES;
typedef char** MYSQL_ROW;

typedef struct {
    char* name;
    uint32_t name_length;
    /* 나머지 필드는 사용 안 함, 바이트 크기 맞추기 위해 padding */
    char _pad[256 - sizeof(char*) - sizeof(uint32_t)];
} MYSQL_FIELD;

/* ── 함수 포인터 테이블 ── */
static void* mariadb_lib = NULL;
static pthread_once_t lib_init_once = PTHREAD_ONCE_INIT;

typedef MYSQL*       (*fn_mysql_init)(MYSQL*);
typedef MYSQL*       (*fn_mysql_real_connect)(MYSQL*, const char*, const char*, const char*, const char*, unsigned int, const char*, unsigned long);
typedef int          (*fn_mysql_real_query)(MYSQL*, const char*, unsigned long);
typedef MYSQL_RES*   (*fn_mysql_store_result)(MYSQL*);
typedef MYSQL_ROW    (*fn_mysql_fetch_row)(MYSQL_RES*);
typedef unsigned long* (*fn_mysql_fetch_lengths)(MYSQL_RES*);
typedef unsigned int (*fn_mysql_num_fields)(MYSQL_RES*);
typedef MYSQL_FIELD* (*fn_mysql_fetch_fields)(MYSQL_RES*);
typedef void         (*fn_mysql_free_result)(MYSQL_RES*);
typedef void         (*fn_mysql_close)(MYSQL*);
typedef const char*  (*fn_mysql_error)(MYSQL*);
typedef uint64_t     (*fn_mysql_affected_rows)(MYSQL*);

static fn_mysql_init           p_mysql_init           = NULL;
static fn_mysql_real_connect   p_mysql_real_connect   = NULL;
static fn_mysql_real_query     p_mysql_real_query     = NULL;
static fn_mysql_store_result   p_mysql_store_result   = NULL;
static fn_mysql_fetch_row      p_mysql_fetch_row      = NULL;
static fn_mysql_fetch_lengths  p_mysql_fetch_lengths  = NULL;
static fn_mysql_num_fields     p_mysql_num_fields     = NULL;
static fn_mysql_fetch_fields   p_mysql_fetch_fields   = NULL;
static fn_mysql_free_result    p_mysql_free_result    = NULL;
static fn_mysql_close          p_mysql_close          = NULL;
static fn_mysql_error          p_mysql_error          = NULL;
static fn_mysql_affected_rows  p_mysql_affected_rows  = NULL;

static void load_mariadb_lib(void) {
    /* libmariadb.so.3 → libmysqlclient.so.21 순서로 시도 */
    const char* libs[] = {
        "libmariadb.so.3",
        "libmariadb.so",
        "libmysqlclient.so.21",
        "libmysqlclient.so",
        NULL
    };
    for (int i = 0; libs[i]; i++) {
        mariadb_lib = dlopen(libs[i], RTLD_LAZY | RTLD_GLOBAL);
        if (mariadb_lib) break;
    }
    if (!mariadb_lib) {
        fprintf(stderr, "[mariadb] 라이브러리 로딩 실패: %s\n", dlerror());
        return;
    }

#define LOAD(fn) p_##fn = (fn_##fn)dlsym(mariadb_lib, #fn); \
    if (!p_##fn) fprintf(stderr, "[mariadb] 함수 없음: " #fn "\n");

    LOAD(mysql_init)
    LOAD(mysql_real_connect)
    LOAD(mysql_real_query)
    LOAD(mysql_store_result)
    LOAD(mysql_fetch_row)
    LOAD(mysql_fetch_lengths)
    LOAD(mysql_num_fields)
    LOAD(mysql_fetch_fields)
    LOAD(mysql_free_result)
    LOAD(mysql_close)
    LOAD(mysql_error)
    LOAD(mysql_affected_rows)
#undef LOAD
}

static int ensure_lib(void) {
    pthread_once(&lib_init_once, load_mariadb_lib);
    return mariadb_lib != NULL;
}

/* ── 연결 풀 (단순 단일 연결, 추후 풀로 확장 가능) ── */
#define MAX_CONNECTIONS 32

typedef struct {
    char     id[32];
    MYSQL*   conn;
    pthread_mutex_t lock;
    int      used;
} FLMariaConn;

static FLMariaConn conn_pool[MAX_CONNECTIONS];
static int conn_count = 0;
static pthread_mutex_t pool_lock = PTHREAD_MUTEX_INITIALIZER;

static FLMariaConn* find_conn(const char* id) {
    for (int i = 0; i < conn_count; i++) {
        if (strcmp(conn_pool[i].id, id) == 0)
            return &conn_pool[i];
    }
    return NULL;
}

/* ── FL 인터페이스 ── */

/* (mariadb_connect "localhost" 3306 "user" "pass" "dbname") → "conn:0" */
FLValue mariadb_connect(FLValue host_v, FLValue port_v, FLValue user_v,
                        FLValue pw_v, FLValue db_v) {
    if (!ensure_lib())
        return fl_str_val("[mariadb] 라이브러리 없음");

    const char* host = (host_v.tag == FL_STRING) ? ((FLString*)host_v.obj)->data : "localhost";
    int         port = (port_v.tag == FL_INT)    ? (int)port_v.i : 3306;
    const char* user = (user_v.tag == FL_STRING) ? ((FLString*)user_v.obj)->data : "root";
    const char* pw   = (pw_v.tag  == FL_STRING)  ? ((FLString*)pw_v.obj)->data  : "";
    const char* db   = (db_v.tag  == FL_STRING)  ? ((FLString*)db_v.obj)->data  : NULL;

    MYSQL* conn = p_mysql_init(NULL);
    if (!conn) return fl_str_val("[mariadb] mysql_init 실패");

    MYSQL* ok = p_mysql_real_connect(conn, host, user, pw, db, (unsigned int)port, NULL, 0);
    if (!ok) {
        const char* err = p_mysql_error(conn);
        char buf[512];
        snprintf(buf, sizeof(buf), "[mariadb] 연결 실패: %s", err);
        p_mysql_close(conn);
        return fl_str_val(buf);
    }

    pthread_mutex_lock(&pool_lock);
    if (conn_count >= MAX_CONNECTIONS) {
        pthread_mutex_unlock(&pool_lock);
        p_mysql_close(conn);
        return fl_str_val("[mariadb] 연결 풀 가득");
    }
    int idx = conn_count++;
    snprintf(conn_pool[idx].id, sizeof(conn_pool[idx].id), "conn:%d", idx);
    conn_pool[idx].conn = conn;
    conn_pool[idx].used = 1;
    pthread_mutex_init(&conn_pool[idx].lock, NULL);
    char id_copy[32];
    strncpy(id_copy, conn_pool[idx].id, 32);
    pthread_mutex_unlock(&pool_lock);

    return fl_str_val(id_copy);
}

/* rows → FLValue vector of maps */
static FLValue fetch_rows(MYSQL_RES* res) {
    FLValue rows = fl_vec_new();
    if (!res) return rows;

    unsigned int nfields = p_mysql_num_fields(res);
    MYSQL_FIELD* fields  = p_mysql_fetch_fields(res);

    MYSQL_ROW row;
    while ((row = p_mysql_fetch_row(res))) {
        unsigned long* lens = p_mysql_fetch_lengths(res);
        FLValue map = fl_map_new();
        for (unsigned int i = 0; i < nfields; i++) {
            FLValue key = fl_str_val(fields[i].name);
            FLValue val;
            if (row[i] == NULL) {
                val = fl_nil();
            } else {
                char* buf = (char*)malloc(lens[i] + 1);
                memcpy(buf, row[i], lens[i]);
                buf[lens[i]] = '\0';
                val = fl_str_val(buf);
                free(buf);
            }
            map = fl_map_set(map, key, val);
        }
        rows = fl_vec_push(rows, map);
    }
    p_mysql_free_result(res);
    return rows;
}

/* (mariadb_query conn_id sql) → vector of maps */
FLValue mariadb_query(FLValue conn_v, FLValue sql_v) {
    if (!ensure_lib()) return fl_vec_new();
    if (conn_v.tag != FL_STRING || sql_v.tag != FL_STRING)
        return fl_vec_new();

    const char* id  = ((FLString*)conn_v.obj)->data;
    const char* sql = ((FLString*)sql_v.obj)->data;

    FLMariaConn* c = find_conn(id);
    if (!c) return fl_str_val("[mariadb] 연결 없음");

    pthread_mutex_lock(&c->lock);
    int rc = p_mysql_real_query(c->conn, sql, (unsigned long)strlen(sql));
    if (rc != 0) {
        const char* err = p_mysql_error(c->conn);
        char buf[512];
        snprintf(buf, sizeof(buf), "[mariadb] 쿼리 오류: %s", err);
        pthread_mutex_unlock(&c->lock);
        return fl_str_val(buf);
    }
    MYSQL_RES* res = p_mysql_store_result(c->conn);
    pthread_mutex_unlock(&c->lock);

    return fetch_rows(res);
}

/* (mariadb_exec conn_id sql) → {"affected": N} */
FLValue mariadb_exec(FLValue conn_v, FLValue sql_v) {
    if (!ensure_lib()) return fl_nil();
    if (conn_v.tag != FL_STRING || sql_v.tag != FL_STRING)
        return fl_nil();

    const char* id  = ((FLString*)conn_v.obj)->data;
    const char* sql = ((FLString*)sql_v.obj)->data;

    FLMariaConn* c = find_conn(id);
    if (!c) return fl_str_val("[mariadb] 연결 없음");

    pthread_mutex_lock(&c->lock);
    int rc = p_mysql_real_query(c->conn, sql, (unsigned long)strlen(sql));
    uint64_t affected = 0;
    if (rc == 0) {
        affected = (uint64_t)p_mysql_affected_rows(c->conn);
    } else {
        const char* err = p_mysql_error(c->conn);
        char buf[512];
        snprintf(buf, sizeof(buf), "[mariadb] 실행 오류: %s", err);
        pthread_mutex_unlock(&c->lock);
        return fl_str_val(buf);
    }
    pthread_mutex_unlock(&c->lock);

    FLValue map = fl_map_new();
    map = fl_map_set(map, fl_str_val("affected"), fl_int((int64_t)affected));
    return map;
}

/* (mariadb_one conn_id sql) → 단일 맵 or nil */
FLValue mariadb_one(FLValue conn_v, FLValue sql_v) {
    FLValue rows = mariadb_query(conn_v, sql_v);
    if (rows.tag != FL_VECTOR) return fl_nil();
    FLVector* vec = (FLVector*)rows.obj;
    if (vec->len == 0) return fl_nil();
    return vec->data[0];
}

/* (mariadb_close conn_id) → nil */
FLValue mariadb_close(FLValue conn_v) {
    if (!ensure_lib()) return fl_nil();
    if (conn_v.tag != FL_STRING) return fl_nil();

    const char* id = ((FLString*)conn_v.obj)->data;
    FLMariaConn* c = find_conn(id);
    if (!c) return fl_nil();

    pthread_mutex_lock(&c->lock);
    if (c->conn) {
        p_mysql_close(c->conn);
        c->conn = NULL;
        c->used = 0;
    }
    pthread_mutex_unlock(&c->lock);
    return fl_nil();
}
