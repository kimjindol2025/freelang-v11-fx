/*
 * sqlite.c — FreeLang 네이티브 SQLite 바인딩
 * libsqlite3가 설치되어 있으면 헤더 포함, 아니면 dlopen 방식
 *
 * 지원 함수 (FL에서 호출):
 *   (fxb_sqlite_open  path)            → "db:N" 핸들
 *   (fxb_sqlite_query db sql)          → 벡터 of 맵
 *   (fxb_sqlite_exec  db sql)          → {"affected": N, "last_id": M}
 *   (fxb_sqlite_one   db sql)          → 단일 맵 or nil
 *   (fxb_sqlite_close db)              → nil
 */

#include "runtime.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/stat.h>
#include <errno.h>
#include <ctype.h>

/* SQLite3 헤더 직접 포함 (설치되어 있음) */
#include <sqlite3.h>

/* ── 연결 풀 (path-based 재사용, Connection-Per-Query 방지) ── */
#define MAX_SQLITE_CONN 32

typedef struct {
    char      id[32];
    char      path[512];  /* DB 경로 키 */
    sqlite3*  db;
    pthread_mutex_t lock;
    int       used;
} FLSQLiteConn;

static FLSQLiteConn sq_pool[MAX_SQLITE_CONN];
static int sq_count = 0;
static pthread_mutex_t sq_pool_lock = PTHREAD_MUTEX_INITIALIZER;

/*
 * (db "name")의 경로 해석기.
 *
 * 자동 연결은 프로젝트 로컬만 허용한다. 이름은 파일 경로가 아니라
 * 논리 DB 이름이며, 경로 분리 문자·절대 경로·숨은 traversal을 받지 않는다.
 * FL_DB_DIR을 지정하면 테스트/배포에서 프로젝트 로컬 디렉터리를 명시할 수
 * 있지만, 여전히 이름은 안전한 단일 component여야 한다.
 */
static int db_name_valid(const char* name) {
    size_t n;
    if (!name || !*name) return 0;
    n = strlen(name);
    if (n > 96) return 0;
    for (size_t i = 0; i < n; i++) {
        unsigned char c = (unsigned char)name[i];
        if (!(isalnum(c) || c == '_' || c == '-')) return 0;
    }
    return 1;
}

static int mkdir_if_missing(const char* path) {
    if (mkdir(path, 0700) == 0 || errno == EEXIST) return 0;
    return -1;
}

/* (db "name") → fxb_sqlite_open(<base>/<name>.db)
 * C symbol is fl_rt_db (not bare db) so user `(define db …)` can compile.
 * cgc maps FL op "db" → fl_rt_db(...). runtime.h may #define db fl_rt_db for C callers. */
FLValue fl_rt_db(FLValue name_v) {
    const char* name = (name_v.tag == FL_STRING)
        ? ((FLString*)name_v.obj)->data : NULL;
    if (!db_name_valid(name))
        return fl_str_val("[DB_NAME_INVALID] db name must be 1-96 ASCII letters, digits, '_' or '-'");

    const char* base = getenv("FL_DB_DIR");
    if (!base || !*base) base = ".freelang/db";
    if (base[0] == '/' || strstr(base, "..") != NULL)
        return fl_str_val("[DB_PATH_INVALID] FL_DB_DIR must be a local relative path");
    if (strlen(base) > 400)
        return fl_str_val("[DB_PATH_INVALID] FL_DB_DIR is too long");

    /* 기본 계약 경로만 자동 생성한다. 임의의 중첩 디렉터리는 생성하지 않는다. */
    if (strcmp(base, ".freelang/db") == 0) {
        if (mkdir_if_missing(".freelang") != 0)
            return fl_str_val("[DB_PATH_CREATE_FAILED] cannot create .freelang");
    }
    if (mkdir_if_missing(base) != 0)
        return fl_str_val("[DB_PATH_CREATE_FAILED] cannot create FL_DB_DIR");

    char path[512];
    int written = snprintf(path, sizeof(path), "%s/%s.db", base, name);
    if (written < 0 || (size_t)written >= sizeof(path))
        return fl_str_val("[DB_PATH_INVALID] resolved database path is too long");
    return fxb_sqlite_open(fl_str_val(path));
}

/* (db-transaction db [{"sql" ... "params" [...] } ...])
 * BEGIN/COMMIT/ROLLBACK을 하나의 native 경계로 묶는다. */
FLValue db_transaction(FLValue conn_v, FLValue statements_v) {
    if (conn_v.tag != FL_STRING || statements_v.tag != FL_VECTOR) {
        fl_throw(fl_str_val("[DB_TRANSACTION_INVALID] handle and statements are required"));
        return fl_nil();
    }

    FLValue begin = fxb_sqlite_exec(conn_v, fl_str_val("BEGIN"));
    if (begin.tag == FL_STRING) {
        fl_throw(begin);
        return fl_nil();
    }

    FLValue results = fl_vec_new();
    FLVector* statements = (FLVector*)statements_v.obj;
    for (uint32_t i = 0; i < statements->len; i++) {
        FLValue statement = statements->data[i];
        if (statement.tag != FL_MAP) {
            fxb_sqlite_exec(conn_v, fl_str_val("ROLLBACK"));
            fl_throw(fl_str_val("[DB_TRANSACTION_INVALID] each statement must be a map"));
            return fl_nil();
        }
        FLValue sql = fl_map_get(statement, fl_str_val("sql"));
        FLValue params = fl_map_get(statement, fl_str_val("params"));
        if (sql.tag != FL_STRING) {
            fxb_sqlite_exec(conn_v, fl_str_val("ROLLBACK"));
            fl_throw(fl_str_val("[DB_TRANSACTION_INVALID] each statement needs sql"));
            return fl_nil();
        }
        if (params.tag != FL_VECTOR) params = fl_vec_new();
        FLValue result = fxb_sqlite_exec_p(conn_v, sql, params);
        if (result.tag == FL_STRING) {
            fxb_sqlite_exec(conn_v, fl_str_val("ROLLBACK"));
            fl_throw(result);
            return fl_nil();
        }
        results = fl_vec_push(results, result);
    }

    FLValue commit = fxb_sqlite_exec(conn_v, fl_str_val("COMMIT"));
    if (commit.tag == FL_STRING) {
        fxb_sqlite_exec(conn_v, fl_str_val("ROLLBACK"));
        fl_throw(commit);
        return fl_nil();
    }
    return results;
}

/* 자동 연결 DB API는 SQLite 오류를 값으로 삼키지 않고 언어 예외로 전파한다.
 * 기존 fxb_sqlite_* API의 호환 동작(오류 문자열 반환)은 유지한다. */
FLValue db_query(FLValue conn_v, FLValue sql_v, FLValue params_v) {
    FLValue result = fxb_sqlite_query_p(conn_v, sql_v, params_v);
    if (result.tag == FL_STRING) {
        fl_throw(result);
        return fl_nil();
    }
    return result;
}

FLValue db_exec(FLValue conn_v, FLValue sql_v, FLValue params_v) {
    FLValue result = fxb_sqlite_exec_p(conn_v, sql_v, params_v);
    if (result.tag == FL_STRING) {
        fl_throw(result);
        return fl_nil();
    }
    return result;
}

static FLSQLiteConn* sq_find(const char* id) {
    for (int i = 0; i < sq_count; i++)
        if (strcmp(sq_pool[i].id, id) == 0) return &sq_pool[i];
    return NULL;
}

/* 동일 경로로 이미 열린 DB 존재 → 재사용 */
static FLSQLiteConn* sq_find_by_path(const char* path) {
    for (int i = 0; i < sq_count; i++)
        if (sq_pool[i].used && strcmp(sq_pool[i].path, path) == 0) return &sq_pool[i];
    return NULL;
}

/* ── FL 인터페이스 ── */

/* (fxb_sqlite_open "/path/to/db.sqlite") → "db:N"
 * 동일 경로로 재호출 시 기존 연결 ID 반환 (연결 재사용) */
FLValue fxb_sqlite_open(FLValue path_v) {
    const char* path = (path_v.tag == FL_STRING)
        ? ((FLString*)path_v.obj)->data
        : ":memory:";

    pthread_mutex_lock(&sq_pool_lock);
    FLSQLiteConn* existing = sq_find_by_path(path);
    if (existing) {
        char id_copy[32];
        strncpy(id_copy, existing->id, 32);
        pthread_mutex_unlock(&sq_pool_lock);
        return fl_str_val(id_copy);
    }
    pthread_mutex_unlock(&sq_pool_lock);

    sqlite3* db = NULL;
    int rc = sqlite3_open(path, &db);
    if (rc != SQLITE_OK) {
        char buf[512];
        snprintf(buf, sizeof(buf), "[sqlite] 열기 실패: %s", sqlite3_errmsg(db));
        sqlite3_close(db);
        return fl_str_val(buf);
    }

    /* WAL 모드와 bounded busy wait로 다중 프로세스 writer 충돌을 완화한다. */
    sqlite3_busy_timeout(db, 5000);
    sqlite3_exec(db, "PRAGMA journal_mode=WAL", NULL, NULL, NULL);
    sqlite3_exec(db, "PRAGMA foreign_keys=ON",  NULL, NULL, NULL);

    pthread_mutex_lock(&sq_pool_lock);
    /* double-check */
    existing = sq_find_by_path(path);
    if (existing) {
        char id_copy[32];
        strncpy(id_copy, existing->id, 32);
        pthread_mutex_unlock(&sq_pool_lock);
        sqlite3_close(db);
        return fl_str_val(id_copy);
    }
    if (sq_count >= MAX_SQLITE_CONN) {
        pthread_mutex_unlock(&sq_pool_lock);
        sqlite3_close(db);
        return fl_str_val("[sqlite] 연결 풀 가득");
    }
    int idx = sq_count++;
    snprintf(sq_pool[idx].id, sizeof(sq_pool[idx].id), "db:%d", idx);
    strncpy(sq_pool[idx].path, path, sizeof(sq_pool[idx].path));
    sq_pool[idx].db   = db;
    sq_pool[idx].used = 1;
    pthread_mutex_init(&sq_pool[idx].lock, NULL);
    char id_copy[32];
    strncpy(id_copy, sq_pool[idx].id, 32);
    pthread_mutex_unlock(&sq_pool_lock);

    return fl_str_val(id_copy);
}

/* 내부: rows → FLValue */
static FLValue sq_fetch_rows(sqlite3_stmt* stmt) {
    int ncols = sqlite3_column_count(stmt);

    /* O(n) 수집: C 동적 배열 → fl_vec_from 단일 호출 */
    size_t cap = 16, len = 0;
    FLValue* arr = (FLValue*)malloc(sizeof(FLValue) * cap);
    if (!arr) return fl_vec_new();

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        FLValue row = fl_map_new();
        for (int i = 0; i < ncols; i++) {
            const char* col_name = sqlite3_column_name(stmt, i);
            FLValue key = fl_str_val(col_name);
            FLValue val;
            int col_type = sqlite3_column_type(stmt, i);
            if (col_type == SQLITE_NULL) {
                val = fl_nil();
            } else if (col_type == SQLITE_INTEGER) {
                val = fl_int(sqlite3_column_int64(stmt, i));
            } else if (col_type == SQLITE_FLOAT) {
                val = fl_float(sqlite3_column_double(stmt, i));
            } else {
                const char* text = (const char*)sqlite3_column_text(stmt, i);
                val = fl_str_val(text ? text : "");
            }
            row = fl_map_set(row, key, val);
        }
        if (len >= cap) {
            cap *= 2;
            FLValue* tmp = (FLValue*)realloc(arr, sizeof(FLValue) * cap);
            if (!tmp) { free(arr); return fl_vec_new(); }
            arr = tmp;
        }
        arr[len++] = row;
    }

    FLValue result = fl_vec_from(arr, (uint32_t)len);
    free(arr);
    return result;
}

/* (fxb_sqlite_query db sql) → 벡터 of 맵 */
FLValue fxb_sqlite_query(FLValue conn_v, FLValue sql_v) {
    if (conn_v.tag != FL_STRING || sql_v.tag != FL_STRING)
        return fl_vec_new();

    const char* id  = ((FLString*)conn_v.obj)->data;
    const char* sql = ((FLString*)sql_v.obj)->data;

    FLSQLiteConn* c = sq_find(id);
    if (!c) return fl_str_val("[sqlite] 연결 없음");

    pthread_mutex_lock(&c->lock);

    sqlite3_stmt* stmt = NULL;
    int rc = sqlite3_prepare_v2(c->db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        char buf[512];
        snprintf(buf, sizeof(buf), "[sqlite] 준비 실패: %s", sqlite3_errmsg(c->db));
        pthread_mutex_unlock(&c->lock);
        return fl_str_val(buf);
    }

    FLValue rows = sq_fetch_rows(stmt);
    sqlite3_finalize(stmt);
    pthread_mutex_unlock(&c->lock);
    return rows;
}

/* (fxb_sqlite_exec db sql) → {"affected": N, "last_id": M} */
FLValue fxb_sqlite_exec(FLValue conn_v, FLValue sql_v) {
    if (conn_v.tag != FL_STRING || sql_v.tag != FL_STRING)
        return fl_nil();

    const char* id  = ((FLString*)conn_v.obj)->data;
    const char* sql = ((FLString*)sql_v.obj)->data;

    FLSQLiteConn* c = sq_find(id);
    if (!c) return fl_str_val("[sqlite] 연결 없음");

    pthread_mutex_lock(&c->lock);
    char* errmsg = NULL;
    int rc = sqlite3_exec(c->db, sql, NULL, NULL, &errmsg);
    if (rc != SQLITE_OK) {
        char buf[512];
        snprintf(buf, sizeof(buf), "[sqlite] 실행 오류: %s", errmsg ? errmsg : "?");
        if (errmsg) sqlite3_free(errmsg);
        pthread_mutex_unlock(&c->lock);
        return fl_str_val(buf);
    }
    int64_t affected = (int64_t)sqlite3_changes(c->db);
    int64_t last_id  = (int64_t)sqlite3_last_insert_rowid(c->db);
    pthread_mutex_unlock(&c->lock);

    FLValue map = fl_map_new();
    map = fl_map_set(map, fl_str_val("affected"), fl_int(affected));
    map = fl_map_set(map, fl_str_val("last_id"),  fl_int(last_id));
    return map;
}

/* (fxb_sqlite_one db sql) → 단일 맵 or nil */
FLValue fxb_sqlite_one(FLValue conn_v, FLValue sql_v) {
    FLValue rows = fxb_sqlite_query(conn_v, sql_v);
    if (rows.tag != FL_VECTOR) return fl_nil();
    FLVector* vec = (FLVector*)rows.obj;
    return (vec->len > 0) ? vec->data[0] : fl_nil();
}

/* ── 파라미터 바인딩 헬퍼 ──────────────────────────────────
   FLVector → sqlite3_stmt 바인딩
   지원 타입: nil, bool, int, float, string
   SQL injection 방어: 값은 절대 SQL에 직접 삽입 안 함
*/
static int sq_bind_params(sqlite3_stmt* stmt, FLValue params) {
    if (params.tag != FL_VECTOR) return SQLITE_OK;
    FLVector* vec = (FLVector*)params.obj;
    for (uint32_t i = 0; i < vec->len; i++) {
        int idx = (int)i + 1;   /* SQLite 바인딩은 1-based */
        FLValue v = vec->data[i];
        int rc;
        switch (v.tag) {
            case FL_NIL:
                rc = sqlite3_bind_null(stmt, idx);
                break;
            case FL_BOOL:
                rc = sqlite3_bind_int(stmt, idx, v.b ? 1 : 0);
                break;
            case FL_INT:
                rc = sqlite3_bind_int64(stmt, idx, v.i);
                break;
            case FL_FLOAT:
                rc = sqlite3_bind_double(stmt, idx, v.f);
                break;
            case FL_STRING:
                rc = sqlite3_bind_text(stmt, idx,
                    ((FLString*)v.obj)->data, -1, SQLITE_TRANSIENT);
                break;
            default:
                rc = sqlite3_bind_null(stmt, idx);
        }
        if (rc != SQLITE_OK) return rc;
    }
    return SQLITE_OK;
}

/* (fxb_sqlite_query_p db sql params) → 벡터 of 맵
   예: (fxb_sqlite_query_p db "SELECT * FROM t WHERE id=? AND name=?"
                          (list 42 "kim"))
*/
FLValue fxb_sqlite_query_p(FLValue conn_v, FLValue sql_v, FLValue params) {
    if (conn_v.tag != FL_STRING || sql_v.tag != FL_STRING)
        return fl_vec_new();

    const char* id  = ((FLString*)conn_v.obj)->data;
    const char* sql = ((FLString*)sql_v.obj)->data;

    FLSQLiteConn* c = sq_find(id);
    if (!c) return fl_str_val("[sqlite] 연결 없음");

    pthread_mutex_lock(&c->lock);
    fl_log_sql("sqlite", sql);

    sqlite3_stmt* stmt = NULL;
    int rc = sqlite3_prepare_v2(c->db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        char buf[512];
        snprintf(buf, sizeof(buf), "[sqlite] 준비 실패: %s", sqlite3_errmsg(c->db));
        pthread_mutex_unlock(&c->lock);
        return fl_str_val(buf);
    }

    rc = sq_bind_params(stmt, params);
    if (rc != SQLITE_OK) {
        char buf[512];
        snprintf(buf, sizeof(buf), "[sqlite] 바인딩 실패: %s", sqlite3_errmsg(c->db));
        sqlite3_finalize(stmt);
        pthread_mutex_unlock(&c->lock);
        return fl_str_val(buf);
    }

    FLValue rows = sq_fetch_rows(stmt);
    sqlite3_finalize(stmt);
    pthread_mutex_unlock(&c->lock);
    return rows;
}

/* (fxb_sqlite_exec_p db sql params) → {"affected": N, "last_id": M}
   예: (fxb_sqlite_exec_p db "INSERT INTO links (code, url) VALUES (?, ?)"
                         (list $code $url))
*/
FLValue fxb_sqlite_exec_p(FLValue conn_v, FLValue sql_v, FLValue params) {
    if (conn_v.tag != FL_STRING || sql_v.tag != FL_STRING)
        return fl_nil();

    const char* id  = ((FLString*)conn_v.obj)->data;
    const char* sql = ((FLString*)sql_v.obj)->data;

    FLSQLiteConn* c = sq_find(id);
    if (!c) return fl_str_val("[sqlite] 연결 없음");

    pthread_mutex_lock(&c->lock);
    fl_log_sql("sqlite", sql);

    sqlite3_stmt* stmt = NULL;
    int rc = sqlite3_prepare_v2(c->db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        char buf[512];
        snprintf(buf, sizeof(buf), "[sqlite] 준비 실패: %s", sqlite3_errmsg(c->db));
        pthread_mutex_unlock(&c->lock);
        return fl_str_val(buf);
    }

    rc = sq_bind_params(stmt, params);
    if (rc != SQLITE_OK) {
        char buf[512];
        snprintf(buf, sizeof(buf), "[sqlite] 바인딩 실패: %s", sqlite3_errmsg(c->db));
        sqlite3_finalize(stmt);
        pthread_mutex_unlock(&c->lock);
        return fl_str_val(buf);
    }

    rc = sqlite3_step(stmt);
    int64_t affected = 0, last_id = 0;
    if (rc == SQLITE_DONE) {
        affected = (int64_t)sqlite3_changes(c->db);
        last_id  = (int64_t)sqlite3_last_insert_rowid(c->db);
    } else {
        char buf[512];
        snprintf(buf, sizeof(buf), "[sqlite] 실행 오류: %s", sqlite3_errmsg(c->db));
        sqlite3_finalize(stmt);
        pthread_mutex_unlock(&c->lock);
        return fl_str_val(buf);
    }

    sqlite3_finalize(stmt);
    pthread_mutex_unlock(&c->lock);

    FLValue map = fl_map_new();
    map = fl_map_set(map, fl_str_val("affected"), fl_int(affected));
    map = fl_map_set(map, fl_str_val("last_id"),  fl_int(last_id));
    return map;
}

/* (fxb_sqlite_one_p db sql params) → 단일 맵 or nil */
FLValue fxb_sqlite_one_p(FLValue conn_v, FLValue sql_v, FLValue params) {
    FLValue rows = fxb_sqlite_query_p(conn_v, sql_v, params);
    if (rows.tag != FL_VECTOR) return fl_nil();
    FLVector* vec = (FLVector*)rows.obj;
    return (vec->len > 0) ? vec->data[0] : fl_nil();
}

/* (fxb_sqlite_close db) → nil */
FLValue fxb_sqlite_close(FLValue conn_v) {
    if (conn_v.tag != FL_STRING) return fl_nil();
    const char* id = ((FLString*)conn_v.obj)->data;
    FLSQLiteConn* c = sq_find(id);
    if (!c) return fl_nil();
    pthread_mutex_lock(&c->lock);
    if (c->db) {
        sqlite3_close(c->db);
        c->db   = NULL;
        c->used = 0;
    }
    pthread_mutex_unlock(&c->lock);
    return fl_nil();
}
