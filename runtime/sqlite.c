/*
 * sqlite.c — FreeLang 네이티브 SQLite 바인딩
 * libsqlite3가 설치되어 있으면 헤더 포함, 아니면 dlopen 방식
 *
 * 지원 함수 (FL에서 호출):
 *   (sqlite_open  path)            → "db:N" 핸들
 *   (sqlite_query db sql)          → 벡터 of 맵
 *   (sqlite_exec  db sql)          → {"affected": N, "last_id": M}
 *   (sqlite_one   db sql)          → 단일 맵 or nil
 *   (sqlite_close db)              → nil
 */

#include "runtime.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

/* SQLite3 헤더 직접 포함 (설치되어 있음) */
#include <sqlite3.h>

/* ── 연결 풀 ── */
#define MAX_SQLITE_CONN 32

typedef struct {
    char      id[32];
    sqlite3*  db;
    pthread_mutex_t lock;
    int       used;
} FLSQLiteConn;

static FLSQLiteConn sq_pool[MAX_SQLITE_CONN];
static int sq_count = 0;
static pthread_mutex_t sq_pool_lock = PTHREAD_MUTEX_INITIALIZER;

static FLSQLiteConn* sq_find(const char* id) {
    for (int i = 0; i < sq_count; i++)
        if (strcmp(sq_pool[i].id, id) == 0) return &sq_pool[i];
    return NULL;
}

/* ── FL 인터페이스 ── */

/* (sqlite_open "/path/to/db.sqlite") → "db:0" */
FLValue sqlite_open(FLValue path_v) {
    const char* path = (path_v.tag == FL_STRING)
        ? ((FLString*)path_v.obj)->data
        : ":memory:";

    sqlite3* db = NULL;
    int rc = sqlite3_open(path, &db);
    if (rc != SQLITE_OK) {
        char buf[512];
        snprintf(buf, sizeof(buf), "[sqlite] 열기 실패: %s", sqlite3_errmsg(db));
        sqlite3_close(db);
        return fl_str_val(buf);
    }

    /* WAL 모드 활성화 (동시성 향상) */
    sqlite3_exec(db, "PRAGMA journal_mode=WAL", NULL, NULL, NULL);
    sqlite3_exec(db, "PRAGMA foreign_keys=ON",  NULL, NULL, NULL);

    pthread_mutex_lock(&sq_pool_lock);
    if (sq_count >= MAX_SQLITE_CONN) {
        pthread_mutex_unlock(&sq_pool_lock);
        sqlite3_close(db);
        return fl_str_val("[sqlite] 연결 풀 가득");
    }
    int idx = sq_count++;
    snprintf(sq_pool[idx].id, sizeof(sq_pool[idx].id), "db:%d", idx);
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
    FLValue rows = fl_vec_new();
    int ncols = sqlite3_column_count(stmt);
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
        rows = fl_vec_push(rows, row);
    }
    return rows;
}

/* (sqlite_query db sql) → 벡터 of 맵 */
FLValue sqlite_query(FLValue conn_v, FLValue sql_v) {
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

/* (sqlite_exec db sql) → {"affected": N, "last_id": M} */
FLValue sqlite_exec(FLValue conn_v, FLValue sql_v) {
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

/* (sqlite_one db sql) → 단일 맵 or nil */
FLValue sqlite_one(FLValue conn_v, FLValue sql_v) {
    FLValue rows = sqlite_query(conn_v, sql_v);
    if (rows.tag != FL_VECTOR) return fl_nil();
    FLVector* vec = (FLVector*)rows.obj;
    return (vec->len > 0) ? vec->data[0] : fl_nil();
}

/* (sqlite_close db) → nil */
FLValue sqlite_close(FLValue conn_v) {
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
