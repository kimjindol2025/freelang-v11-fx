#ifndef FREELANG_RUNTIME_H
#define FREELANG_RUNTIME_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>
#include <setjmp.h>

/* ── ABI 헌법 (self/ABI.md) ── */

typedef enum {
    FL_INT,
    FL_FLOAT,
    FL_BOOL,
    FL_NIL,
    FL_STRING,
    FL_VECTOR,
    FL_MAP,
    FL_FN
} FLTag;

typedef struct FLObject FLObject;

typedef struct {
    FLTag tag;
    union {
        int64_t  i;
        double   f;
        bool     b;
        FLObject* obj;
    };
} FLValue;

struct FLObject {
    FLTag    type;
    uint32_t rc;
};

typedef struct {
    FLObject base;
    uint32_t len;
    char     data[];
} FLString;

/* ── S5: Heap Object System ── */

typedef struct {
    FLValue key;
    FLValue val;
} FLMapEntry;

typedef struct {
    FLObject  base;   /* type = FL_VECTOR */
    uint32_t  len;
    uint32_t  cap;
    FLValue*  data;
} FLVector;

typedef struct {
    FLObject    base;  /* type = FL_MAP */
    uint32_t    len;
    uint32_t    cap;
    FLMapEntry* entries;
} FLMap;

/* ── 값 생성 ── */
FLValue fl_int(int64_t v);
FLValue fl_float(double v);
FLValue fl_str_val(const char* s);
FLValue fl_bool(bool v);
FLValue fl_nil(void);

/* ── 조건 판별 ── */
bool fl_truthy(FLValue v);

/* ── 산술 ── */
FLValue fl_add(FLValue a, FLValue b);
FLValue fl_sub(FLValue a, FLValue b);
FLValue fl_mul(FLValue a, FLValue b);
FLValue fl_div(FLValue a, FLValue b);
FLValue fl_mod(FLValue a, FLValue b);

/* ── 비교 ── */
FLValue fl_eq(FLValue a, FLValue b);
FLValue fl_lt(FLValue a, FLValue b);
FLValue fl_gt(FLValue a, FLValue b);
FLValue fl_lte(FLValue a, FLValue b);
FLValue fl_gte(FLValue a, FLValue b);
FLValue fl_neq(FLValue a, FLValue b);

/* ── 논리 ── */
FLValue fl_not(FLValue a);
FLValue fl_and(FLValue a, FLValue b);
FLValue fl_or(FLValue a, FLValue b);

/* ── 문자열 ── */
FLValue fl_str_n(int count, ...);

/* ── I/O ── */
FLValue fl_println(FLValue v);
FLValue fl_print(FLValue v);

/* ── 파일 I/O ── */
FLValue fl_file_read(FLValue path);
FLValue fl_file_write(FLValue path, FLValue content);

/* ── Vector ── */
FLValue fl_vec_new(void);
FLValue fl_vec_from(FLValue* items, uint32_t n);
FLValue fl_vec_get(FLValue vec, FLValue idx);
FLValue fl_vec_set(FLValue vec, FLValue idx, FLValue val);
FLValue fl_vec_push(FLValue vec, FLValue val);
FLValue fl_vec_len(FLValue vec);

/* ── Map ── */
FLValue fl_map_new(void);
FLValue fl_map_from_pairs(FLValue* kv, uint32_t n); /* n = 쌍의 수 */
FLValue fl_map_get(FLValue map, FLValue key);
FLValue fl_map_set(FLValue map, FLValue key, FLValue val);
FLValue fl_map_len(FLValue map);

/* ── S7: Closure ── */
typedef struct FLClosure {
    FLObject base;   /* type = FL_FN */
    FLValue (*call)(struct FLClosure* self, int argc, FLValue* argv);
    uint32_t nenv;
    FLValue  env[];  /* flexible array — captured values */
} FLClosure;

FLValue fl_fn_new(FLValue (*call)(FLClosure*, int, FLValue*),
                  uint32_t nenv, FLValue* env);
FLValue fl_fn_call(FLValue fn, int argc, FLValue* argv);

/* ── S8: 고차함수 ── */
FLValue fl_map_fn(FLValue fn, FLValue vec);
FLValue fl_filter_fn(FLValue fn, FLValue vec);
FLValue fl_reduce_fn(FLValue fn, FLValue init, FLValue vec);

/* ── S9: 맵 accessor ── */
FLValue fl_map_keys(FLValue map);
FLValue fl_map_vals(FLValue map);
FLValue fl_map_entries(FLValue map);

/* ── S22: argv ── */
void    fl_init_argv(int argc, char** argv);
FLValue fl_get_argv(void);

/* ── S12: bridge builtins (FL generic) ── */
FLValue null_p(FLValue v);
FLValue get(FLValue obj, FLValue key);
FLValue length(FLValue obj);
FLValue char_at(FLValue str, FLValue idx);

/* ── S15: stdlib bridge ── */
FLValue fl_floor(FLValue x);
FLValue fl_ceil(FLValue x);
FLValue fl_abs(FLValue x);
FLValue fl_math_sqrt(FLValue x);
FLValue fl_now(void);
FLValue fl_now_ms(void);
FLValue string_p(FLValue v);
FLValue array_p(FLValue v);
FLValue list_p(FLValue v);
FLValue map_p(FLValue v);
FLValue fn_p(FLValue v);
FLValue type_of(FLValue v);
FLValue str_replace(FLValue s, FLValue from, FLValue to);
FLValue split(FLValue s, FLValue sep);
FLValue join(FLValue vec, FLValue sep);
FLValue range(FLValue start, FLValue end);
FLValue char_code_at(FLValue s, FLValue idx);
FLValue substring(FLValue s, FLValue start, FLValue end);
FLValue trim(FLValue s);
FLValue index_of(FLValue vec, FLValue val);
FLValue str_index_of(FLValue s, FLValue sub);

/* ── S26: atom (mutable cell) ── */
FLValue fl_atom_new(FLValue init);
FLValue fl_atom_deref(FLValue atom);
FLValue fl_atom_reset(FLValue atom, FLValue val);

/* ── S26: stdlib bridge ── */
FLValue fl_includes_item(FLValue vec, FLValue item);
FLValue fl_str_includes(FLValue s, FLValue sub);
FLValue fl_string_p(FLValue v);
FLValue fl_vec_slice(FLValue vec, FLValue start, FLValue end);
FLValue fl_vec_last(FLValue vec);
FLValue fl_map_del(FLValue map, FLValue key);
FLValue fl_map_merge(FLValue a, FLValue b);
FLValue fl_concat(FLValue a, FLValue b);

/* ── S27: FL 소스 → AST (cgc-bridge.c + parser.c에서 제공) ── */
FLValue fl_parse(FLValue src);

/* ── JSON ── */
FLValue fl_json_parse(FLValue src);
FLValue fl_json_stringify(FLValue val);
/* cgc-bin이 생성하는 짧은 이름 별칭 */
FLValue json_parse(FLValue src);
FLValue json_stringify(FLValue val);

/* ── 비트 연산 ── */
FLValue fl_bit_xor(FLValue a, FLValue b);
FLValue fl_bit_and(FLValue a, FLValue b);
FLValue fl_bit_or(FLValue a, FLValue b);
FLValue fl_bit_shl(FLValue a, FLValue b);
FLValue fl_bit_shr(FLValue a, FLValue b);

/* ── _fl_process_* ── */
FLValue _fl_process_getcwd(void);
FLValue _fl_process_chdir(FLValue path);
FLValue _fl_process_pid(void);
FLValue _fl_process_ppid(void);
FLValue _fl_process_kill(FLValue pid);
FLValue _fl_process_exists(FLValue pid);
FLValue _fl_process_wait(FLValue pid);
FLValue _fl_process_run(FLValue cmd);
FLValue _fl_process_run_args(FLValue cmd, FLValue args);
FLValue _fl_run_inherit(FLValue cmd);
FLValue _fl_process_exec(FLValue cmd);
FLValue _fl_process_exec_args(FLValue cmd, FLValue args);
FLValue _fl_process_spawn(FLValue cmd, FLValue args);

/* ── _fl_file_* / _fl_env_* / str_join ── */
FLValue _fl_file_append(FLValue path, FLValue content);
FLValue file_exists(FLValue path);
FLValue _fl_file_delete(FLValue path);
FLValue _fl_file_copy(FLValue src, FLValue dst);
FLValue _fl_file_rename(FLValue old, FLValue nw);
FLValue _fl_file_size(FLValue path);
FLValue _fl_file_modified(FLValue path);
FLValue _fl_file_mkdir(FLValue path);
FLValue _fl_file_rmdir(FLValue path);
FLValue _fl_file_list(FLValue path);
FLValue _fl_file_is_file(FLValue path);
FLValue _fl_file_is_dir(FLValue path);
FLValue _fl_env_get(FLValue key);
FLValue _fl_env_set(FLValue key, FLValue val);
FLValue _fl_env_all(void);
FLValue str_join(FLValue sep, FLValue vec);

/* ── HTTP 서버 (http.c) ── */
FLValue server_get(FLValue path, FLValue handler);
FLValue server_post(FLValue path, FLValue handler);
FLValue server_put(FLValue path, FLValue handler);
FLValue server_patch(FLValue path, FLValue handler);
FLValue server_delete(FLValue path, FLValue handler);
FLValue server_html(FLValue html);
FLValue server_text(FLValue text);
FLValue server_json(FLValue json_str);
FLValue server_status(FLValue code, FLValue body);
FLValue server_redirect(FLValue url);
FLValue server_req_param(FLValue req, FLValue name);
FLValue server_req_query(FLValue req, FLValue name);
FLValue server_req_body(FLValue req);
FLValue server_req_header(FLValue req, FLValue name);
FLValue server_req_method(FLValue req);
FLValue server_req_path(FLValue req);
FLValue server_start(FLValue port);
/* kebab-case 앱 호환 alias */
FLValue fl_http_route(FLValue method, FLValue path, FLValue handler);
FLValue fl_http_start(FLValue port);
/* kebab-case response alias */
FLValue fl_resp_html(FLValue html);
FLValue fl_resp_json(FLValue json);
FLValue fl_resp_text(FLValue text);
FLValue fl_resp_status(FLValue code, FLValue body);
FLValue fl_resp_redirect(FLValue url);

/* ── aliases.c — cgc-bin 생성 함수 bridge ── */
/* 환경 */
FLValue env_get(FLValue key);
FLValue env_set(FLValue k, FLValue v);
/* 수학 */
FLValue math_floor(FLValue x);
FLValue math_ceil(FLValue x);
FLValue math_abs(FLValue x);
FLValue math_sqrt(FLValue x);
FLValue math_round(FLValue x);
FLValue math_max(FLValue a, FLValue b);
FLValue math_min(FLValue a, FLValue b);
FLValue math_pow(FLValue base, FLValue exp);
FLValue math_random(void);
/* 문자열 */
FLValue str_trim(FLValue s);
FLValue str_upper(FLValue s);
FLValue str_lower(FLValue s);
FLValue str_starts_with(FLValue s, FLValue prefix);
FLValue str_ends_with(FLValue s, FLValue suffix);
FLValue str_split(FLValue s, FLValue sep);
FLValue str_pad_left(FLValue s, FLValue width, FLValue ch);
FLValue str_pad_right(FLValue s, FLValue width, FLValue ch);
FLValue str_repeat(FLValue s, FLValue n);
FLValue str_contains(FLValue s, FLValue sub);
FLValue parse_int(FLValue s);
FLValue parse_float(FLValue s);
FLValue to_string(FLValue v);
FLValue to_int(FLValue v);
FLValue to_float(FLValue v);
FLValue str_to_num(FLValue v);
/* 컬렉션 */
FLValue first(FLValue vec);
FLValue last(FLValue vec);
FLValue rest(FLValue vec);
FLValue nth(FLValue vec, FLValue idx);
FLValue count(FLValue v);
FLValue take(FLValue n, FLValue vec);
FLValue drop(FLValue n, FLValue vec);
FLValue flatten(FLValue vec);
FLValue sort(FLValue vec);
FLValue reverse(FLValue vec);
FLValue zip(FLValue a, FLValue b);
FLValue some(FLValue fn, FLValue vec);
FLValue every(FLValue fn, FLValue vec);
FLValue vec_max(FLValue vec);
FLValue vec_min(FLValue vec);
/* 맵 */
FLValue merge(FLValue a, FLValue b);
FLValue dissoc(FLValue m, FLValue k);
FLValue vals(FLValue m);
FLValue keys(FLValue m);
FLValue assoc(FLValue m, FLValue k, FLValue v);
/* atom */
FLValue atom(FLValue v);
FLValue deref(FLValue a);
FLValue swap_bang(FLValue a, FLValue fn);
FLValue reset_bang(FLValue a, FLValue v);
/* 타입 */
/* 환경/.env */
FLValue env_load(FLValue path);
FLValue num(FLValue v);
/* fl_or: 93줄에 이미 선언됨 */
FLValue server_rate_limit(FLValue max_reqs, FLValue window_ms);
FLValue mariadb_connect4(FLValue host, FLValue user, FLValue pw, FLValue db);
FLValue int_p(FLValue v);
FLValue float_p(FLValue v);
FLValue bool_p(FLValue v);
FLValue nil_p(FLValue v);
FLValue number_p(FLValue v);
FLValue identity(FLValue v);
FLValue not_p(FLValue v);
FLValue even_p(FLValue v);
FLValue odd_p(FLValue v);
FLValue inc(FLValue v);
FLValue dec(FLValue v);
/* fl_vec_* aliases (cgc-bin 내부 이름) */
FLValue fl_vec_first(FLValue vec);
FLValue fl_vec_last(FLValue vec);
FLValue fl_vec_rest(FLValue vec);

/* ── SQLite (sqlite.c — libsqlite3 직접 링크) ── */
FLValue sqlite_open(FLValue path);
FLValue sqlite_query(FLValue conn, FLValue sql);
FLValue sqlite_exec(FLValue conn, FLValue sql);
FLValue sqlite_one(FLValue conn, FLValue sql);
FLValue sqlite_close(FLValue conn);

/* ── MariaDB (mariadb.c — dlopen 방식, 헤더 불필요) ── */
FLValue mariadb_connect(FLValue host, FLValue port, FLValue user, FLValue pw, FLValue db);
FLValue mariadb_query(FLValue conn, FLValue sql);
FLValue mariadb_exec(FLValue conn, FLValue sql);
FLValue mariadb_one(FLValue conn, FLValue sql);
FLValue mariadb_close(FLValue conn);

/* ── try/catch 인프라 ── */
#define FL_TRY_MAX 64
typedef struct { jmp_buf buf; FLValue err; } FLTryFrame;
extern FLTryFrame fl_try_stack[FL_TRY_MAX];
extern int fl_try_top;
void fl_throw(FLValue err);
FLValue fl_make_error(const char* type, const char* msg);

#endif /* FREELANG_RUNTIME_H */
