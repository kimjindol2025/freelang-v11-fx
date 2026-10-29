/*
 * collection.c — FreeLang Vector / Map / Closure 구현
 *
 * 메모리 전략:
 *   - 요청 처리 중 (fl_arena_begin ~ fl_arena_end): Arena에 할당
 *   - 요청 외부 (전역 define, startup): malloc 폴백 (fl_arena_alloc 내부)
 *   - 클로저(fl_fn_new): 장기 생존 가능 → 항상 malloc
 *   - 빌더(fl_vec_builder_new): heap malloc, rc=0xFE 표시, freeze로 arena 변환
 *
 * copy semantics(불변 스타일)이므로 개별 free 불필요.
 * 요청 단위로 fl_arena_end()가 전부 일괄 해제.
 * 예외: rc==0xFE 빌더는 fl_vec_builder_freeze 시 heap free.
 */

#include "runtime.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "native-symbols.h"

/* ── Arena 헬퍼 매크로 ──────────────────────────────────────────────
   fl_arena_alloc: arena 활성 시 bump 할당, 비활성 시 malloc 폴백
*/
#define A(size) fl_arena_alloc(size)

/* ── Vector ── */

FLValue fl_vec_new(void) {
    FLVector* v = A(sizeof(FLVector));
    v->base.type = FL_VECTOR; v->base.rc = 1;
    v->len = 0; v->cap = 0; v->data = NULL;
    FLValue r; r.tag = FL_VECTOR; r.obj = (FLObject*)v; return r;
}

FLValue fl_vec_from(FLValue* items, uint32_t n) {
    FLVector* v = A(sizeof(FLVector));
    v->base.type = FL_VECTOR; v->base.rc = 1;
    v->len = n; v->cap = n;
    v->data = n ? A(sizeof(FLValue) * n) : NULL;
    if (n) memcpy(v->data, items, sizeof(FLValue) * n);
    FLValue r; r.tag = FL_VECTOR; r.obj = (FLObject*)v; return r;
}

FLValue fl_vec_get(FLValue vec, FLValue idx) {
    if (vec.tag != FL_VECTOR) return fl_nil();
    FLVector* v = (FLVector*)vec.obj;
    int64_t i = (idx.tag == FL_FLOAT) ? (int64_t)idx.f : idx.i;
    if (i < 0 || (uint32_t)i >= v->len) return fl_nil();
    return v->data[i];
}

FLValue fl_vec_len(FLValue vec) {
    if (vec.tag != FL_VECTOR) return fl_int(0);
    return fl_int((int64_t)((FLVector*)vec.obj)->len);
}

/* ── 가변 빌더 (heap-allocated, rc=0xFE) ─────────────────────────────
 * fl_vec_builder_new  → heap FLVector, cap=16, rc=0xFE
 * fl_vec_builder_push → O(1) amortized (realloc+doubling)
 * fl_vec_builder_freeze → arena FLVector (불변) 반환 후 heap free
 *
 * 사용 패턴:
 *   FLValue b = fl_vec_builder_new();
 *   for (...) fl_vec_builder_push(b, elem);
 *   FLValue result = fl_vec_builder_freeze(b);
 */
FLValue fl_vec_builder_new(void) {
    FLVector* v = (FLVector*)malloc(sizeof(FLVector));
    v->base.type = FL_VECTOR;
    v->base.rc   = 0xFE;   /* mutable builder 표시 */
    v->len = 0; v->cap = 16;
    v->data = (FLValue*)malloc(sizeof(FLValue) * 16);
    FLValue r; r.tag = FL_VECTOR; r.obj = (FLObject*)v; return r;
}

void fl_vec_builder_push(FLValue b, FLValue val) {
    if (b.tag != FL_VECTOR) return;
    FLVector* v = (FLVector*)b.obj;
    if (v->base.rc != 0xFE) return;
    if (v->len >= v->cap) {
        v->cap = v->cap ? v->cap * 2 : 16;
        v->data = (FLValue*)realloc(v->data, sizeof(FLValue) * v->cap);
    }
    v->data[v->len++] = val;
}

FLValue fl_vec_builder_freeze(FLValue b) {
    if (b.tag != FL_VECTOR) return fl_vec_new();
    FLVector* src = (FLVector*)b.obj;
    if (src->base.rc != 0xFE) return b;   /* 이미 불변 */
    FLValue result = fl_vec_from(src->data, src->len);
    free(src->data);
    free(src);
    return result;
}

/* copy semantics: 새 vector 반환. 구 vector는 Arena에 남아 일괄 해제됨.
 * 빌더(rc==0xFE) 입력이면 O(1) amortized 경로로 mutate. */
FLValue fl_vec_push(FLValue vec, FLValue val) {
    FLVector* src = (vec.tag == FL_VECTOR) ? (FLVector*)vec.obj : NULL;

    /* 빌더 fast-path: O(1) amortized */
    if (src && src->base.rc == 0xFE) {
        fl_vec_builder_push(vec, val);
        return vec;
    }

    /* 불변 copy-semantics */
    uint32_t n = src ? src->len : 0;
    FLVector* v = A(sizeof(FLVector));
    v->base.type = FL_VECTOR; v->base.rc = 1;
    v->len = n + 1; v->cap = n + 1;
    v->data = A(sizeof(FLValue) * (n + 1));
    if (n && src->data) memcpy(v->data, src->data, sizeof(FLValue) * n);
    v->data[n] = val;
    FLValue r; r.tag = FL_VECTOR; r.obj = (FLObject*)v; return r;
}

FLValue fl_vec_set(FLValue vec, FLValue idx, FLValue val) {
    if (vec.tag != FL_VECTOR) return fl_vec_new();
    FLVector* src = (FLVector*)vec.obj;
    int64_t i = (idx.tag == FL_FLOAT) ? (int64_t)idx.f : idx.i;
    if (i < 0 || (uint32_t)i >= src->len) return vec;
    FLVector* v = A(sizeof(FLVector));
    v->base.type = FL_VECTOR; v->base.rc = 1;
    v->len = src->len; v->cap = src->len;
    v->data = A(sizeof(FLValue) * src->len);
    memcpy(v->data, src->data, sizeof(FLValue) * src->len);
    v->data[i] = val;
    FLValue r; r.tag = FL_VECTOR; r.obj = (FLObject*)v; return r;
}

/* ── Map ── */

FLValue fl_map_new(void) {
    FLMap* m = A(sizeof(FLMap));
    m->base.type = FL_MAP; m->base.rc = 1;
    m->len = 0; m->cap = 0; m->entries = NULL;
    FLValue r; r.tag = FL_MAP; r.obj = (FLObject*)m; return r;
}

/* kv: [k0,v0, k1,v1, ...], n = 쌍의 수 */
FLValue fl_map_from_pairs(FLValue* kv, uint32_t n) {
    FLMap* m = A(sizeof(FLMap));
    m->base.type = FL_MAP; m->base.rc = 1;
    m->len = n; m->cap = n;
    m->entries = n ? A(sizeof(FLMapEntry) * n) : NULL;
    for (uint32_t i = 0; i < n; i++) {
        m->entries[i].key = kv[i * 2];
        m->entries[i].val = kv[i * 2 + 1];
    }
    FLValue r; r.tag = FL_MAP; r.obj = (FLObject*)m; return r;
}

FLValue fl_map_get(FLValue map, FLValue key) {
    if (map.tag != FL_MAP) return fl_nil();
    FLMap* m = (FLMap*)map.obj;
    for (uint32_t i = 0; i < m->len; i++) {
        if (fl_truthy(fl_eq(m->entries[i].key, key)))
            return m->entries[i].val;
    }
    return fl_nil();
}

FLValue fl_map_len(FLValue map) {
    if (map.tag != FL_MAP) return fl_int(0);
    return fl_int((int64_t)((FLMap*)map.obj)->len);
}

/* copy semantics: upsert 후 새 map 반환. 구 map은 Arena에서 일괄 해제됨 */
FLValue fl_map_set(FLValue map, FLValue key, FLValue val) {
    FLMap* src = (map.tag == FL_MAP) ? (FLMap*)map.obj : NULL;
    uint32_t n = src ? src->len : 0;
    /* 기존 키 탐색 */
    for (uint32_t i = 0; i < n; i++) {
        if (fl_truthy(fl_eq(src->entries[i].key, key))) {
            FLMap* m = A(sizeof(FLMap));
            m->base.type = FL_MAP; m->base.rc = 1;
            m->len = n; m->cap = n;
            m->entries = A(sizeof(FLMapEntry) * n);
            memcpy(m->entries, src->entries, sizeof(FLMapEntry) * n);
            m->entries[i].val = val;
            FLValue r; r.tag = FL_MAP; r.obj = (FLObject*)m; return r;
        }
    }
    /* 새 키 추가 */
    FLMap* m = A(sizeof(FLMap));
    m->base.type = FL_MAP; m->base.rc = 1;
    m->len = n + 1; m->cap = n + 1;
    m->entries = A(sizeof(FLMapEntry) * (n + 1));
    if (n && src->entries) memcpy(m->entries, src->entries, sizeof(FLMapEntry) * n);
    m->entries[n].key = key; m->entries[n].val = val;
    FLValue r; r.tag = FL_MAP; r.obj = (FLObject*)m; return r;
}

/* ── S7: Closure ──
   클로저는 전역 핸들러로 등록될 수 있어 요청 수명보다 길게 생존.
   반드시 malloc 사용 (Arena 비사용).
*/
static void closure_cleanup(void*pointer){FLValue value={.tag=FL_FN,.obj=pointer};fl_heap_release(value);}
FLValue fl_fn_new(FLValue (*call)(FLClosure*, int, FLValue*),
                  uint32_t nenv, FLValue* env) {
    FLClosure* cl = malloc(sizeof(FLClosure) + sizeof(FLValue) * nenv);
    if(!cl){fl_throw(fl_str_val("closure allocation failed"));return fl_nil();}
    cl->base.type = FL_FN; cl->base.rc = 1;
    cl->call = call; cl->nenv = nenv; cl->hot_count = 0;
    cl->user_global=0;
    for (uint32_t i = 0; i < nenv; i++) cl->env[i] = env[i];
    if(fl_arena_on_reset(closure_cleanup,cl)<0){free(cl);fl_throw(fl_str_val("closure cleanup allocation failed"));return fl_nil();}
    FLValue r; r.tag = FL_FN; r.obj = (FLObject*)cl; return r;
}

FLValue fl_fn_global(FLValue (*call)(FLClosure*,int,FLValue*),uint32_t nenv,FLValue*env){
    FLValue fn=fl_fn_new(call,nenv,env);if(fn.tag==FL_FN)((FLClosure*)fn.obj)->user_global=1;return fn;
}

static int is_tail(FLValue value){return value.tag==FL_MAP&&value.obj&&value.obj->type==FL_NATIVE_TAIL;}
static FLValue run_tail(FLValue request,uint32_t limit){
    if(fl_try_top>=FL_TRY_MAX){fl_throw(fl_str_val("try stack exhausted"));return fl_nil();}
    volatile FLValue held=fl_heap_copy(request),answer=fl_nil();
    void*volatile previous=NULL;volatile int active=0;
    int saved=fl_try_top;FLTryFrame*frame=&fl_try_stack[fl_try_top++];
    if(setjmp(frame->buf)==0){
        previous=fl_arena_scope_enter();active=1;
        for(uint32_t iteration=1;;iteration++){
            if(iteration>=limit)fl_throw(fl_str_val("Maximum tail-call depth exceeded"));
            FLValue target=fl_map_get(held,fl_str_val("fn")),args=fl_map_get(held,fl_str_val("args"));
            if(target.tag!=FL_FN||!target.obj||args.tag!=FL_VECTOR)fl_throw(fl_str_val("invalid tail call"));
            FLClosure*cl=(FLClosure*)target.obj;
            FLCallScope scope;fl_v11_function_enter(&scope,cl->user_global);
            void*mark=fl_borrow_mark();
            FLValue result=cl->call(cl,(int)((FLVector*)args.obj)->len,((FLVector*)args.obj)->data);
            result=fl_borrow_finish(mark,result);fl_v11_scope_leave(&scope);
            FLValue next=fl_heap_copy(result);
            fl_heap_release(held);held=next;
            if(!is_tail(next))break;
            fl_arena_end();fl_arena_begin();
        }
        fl_try_top=saved;fl_arena_scope_leave(previous);active=0;
        answer=fl_arena_copy(held);fl_heap_release(held);
        return answer;
    }
    answer=fl_heap_copy(frame->err);fl_try_top=saved;
    if(active)fl_arena_scope_leave(previous);
    fl_heap_release(held);
    FLValue error=fl_arena_copy(answer);fl_heap_release(answer);
    fl_throw(error);return fl_nil();
}

FLValue fl_fn_call(FLValue fn, int argc, FLValue* argv) {
    if (fn.tag != FL_FN || !fn.obj) { fl_throw(fl_str_val("not a function")); return fl_nil(); }
    FLClosure* cl = (FLClosure*)fn.obj;
    FLCallScope scope;fl_v11_function_enter(&scope,cl->user_global);
    void*mark=fl_borrow_mark();FLValue result=cl->call(cl, argc, argv);
    result=fl_borrow_finish(mark,result);
    if(is_tail(result))result=run_tail(result,cl->user_global?2000000:1000000);
    fl_v11_scope_leave(&scope);return result;
}

/* ── S8: 고차함수 ── */

FLValue fl_map_fn(FLValue fn, FLValue vec) {
    if (vec.tag != FL_VECTOR) return fl_vec_new();
    FLVector* v = (FLVector*)vec.obj;
    if (v->len == 0) return fl_vec_new();
    /* O(n): 결과 크기 확정(입력 동일) → 단일 arena 배열 */
    FLValue* tmp = (FLValue*)malloc(sizeof(FLValue) * v->len);
    if (!tmp) return fl_vec_new();
    for (uint32_t i = 0; i < v->len; i++) {
        FLValue elem = v->data[i];
        tmp[i] = fl_fn_call(fn, 1, &elem);
    }
    FLValue r = fl_vec_from(tmp, v->len);
    free(tmp);
    return r;
}

FLValue fl_filter_fn(FLValue fn, FLValue vec) {
    if (vec.tag != FL_VECTOR) return fl_vec_new();
    FLVector* v = (FLVector*)vec.obj;
    if (v->len == 0) return fl_vec_new();
    /* O(n): 최대 n개 → C 배열 수집 후 fl_vec_from */
    FLValue* tmp = (FLValue*)malloc(sizeof(FLValue) * v->len);
    if (!tmp) return fl_vec_new();
    uint32_t len = 0;
    for (uint32_t i = 0; i < v->len; i++) {
        FLValue elem = v->data[i];
        if (fl_truthy(fl_fn_call(fn, 1, &elem))) tmp[len++] = elem;
    }
    FLValue r = fl_vec_from(tmp, len);
    free(tmp);
    return r;
}

/* for-each: 부작용 전용 반복, nil 반환 */
void fl_for_each(FLValue fn, FLValue vec) {
    if (vec.tag != FL_VECTOR) return;
    FLVector* v = (FLVector*)vec.obj;
    for (uint32_t i = 0; i < v->len; i++) {
        FLValue elem = v->data[i];
        fl_fn_call(fn, 1, &elem);
    }
}

/* distinct: 중복 제거 (순서 유지, O(n²)) */
FLValue distinct(FLValue vec) {
    if (vec.tag != FL_VECTOR) return fl_vec_new();
    FLVector* v = (FLVector*)vec.obj;
    if (v->len == 0) return fl_vec_new();
    FLValue* tmp = (FLValue*)malloc(sizeof(FLValue) * v->len);
    if (!tmp) return fl_vec_new();
    uint32_t len = 0;
    for (uint32_t i = 0; i < v->len; i++) {
        int found = 0;
        for (uint32_t j = 0; j < len; j++) {
            if (fl_truthy(fl_eq(v->data[i], tmp[j]))) { found = 1; break; }
        }
        if (!found) tmp[len++] = v->data[i];
    }
    FLValue r = fl_vec_from(tmp, len);
    free(tmp);
    return r;
}

FLValue fl_reduce_fn(FLValue fn, FLValue init, FLValue vec) {
    if (vec.tag != FL_VECTOR) return init;
    FLVector* v = (FLVector*)vec.obj;
    FLValue acc = init;
    for (uint32_t i = 0; i < v->len; i++) {
        FLValue args[2] = { acc, v->data[i] };
        acc = fl_fn_call(fn, 2, args);
    }
    return acc;
}

/* ── S9: 맵 accessor ── */

FLValue fl_map_keys(FLValue map) {
    if (map.tag != FL_MAP) return fl_vec_new();
    FLMap* m = (FLMap*)map.obj;
    if (m->len == 0) return fl_vec_new();
    FLValue* tmp = (FLValue*)malloc(sizeof(FLValue) * m->len);
    if (!tmp) return fl_vec_new();
    for (uint32_t i = 0; i < m->len; i++) tmp[i] = m->entries[i].key;
    FLValue r = fl_vec_from(tmp, m->len); free(tmp); return r;
}

FLValue fl_map_vals(FLValue map) {
    if (map.tag != FL_MAP) return fl_vec_new();
    FLMap* m = (FLMap*)map.obj;
    if (m->len == 0) return fl_vec_new();
    FLValue* tmp = (FLValue*)malloc(sizeof(FLValue) * m->len);
    if (!tmp) return fl_vec_new();
    for (uint32_t i = 0; i < m->len; i++) tmp[i] = m->entries[i].val;
    FLValue r = fl_vec_from(tmp, m->len); free(tmp); return r;
}

FLValue fl_map_entries(FLValue map) {
    if (map.tag != FL_MAP) return fl_vec_new();
    FLMap* m = (FLMap*)map.obj;
    if (m->len == 0) return fl_vec_new();
    FLValue* tmp = (FLValue*)malloc(sizeof(FLValue) * m->len);
    if (!tmp) return fl_vec_new();
    for (uint32_t i = 0; i < m->len; i++) {
        FLValue pair[2] = { m->entries[i].key, m->entries[i].val };
        tmp[i] = fl_vec_from(pair, 2);
    }
    FLValue r = fl_vec_from(tmp, m->len); free(tmp); return r;
}

/* ── S12: bridge builtins ── */

FLValue null_p(FLValue v) {
    return fl_bool(v.tag == FL_NIL);
}

FLValue get(FLValue obj, FLValue key) {
    if (obj.tag == FL_NIL) {
        const char* ks = (key.tag == FL_STRING) ? ((FLString*)key.obj)->data : "?";
        char msg[256];
        snprintf(msg, sizeof(msg), "(get nil \"%s\") — nil에 key 접근 불가. nil 체크 먼저.", ks);
        fl_throw(fl_str_val(msg));
        return fl_nil();
    }
    if (obj.tag == FL_MAP) return fl_map_get(obj, key);
    if (obj.tag == FL_VECTOR) {
        if (key.tag != FL_INT) return fl_nil();
        return fl_vec_get(obj, key);
    }
    if (obj.tag == FL_STRING) {
        if (key.tag != FL_INT) return fl_nil();
        int64_t idx = key.i;
        const char* s = ((FLString*)obj.obj)->data;
        int64_t len = (int64_t)strlen(s);
        if (idx < 0 || idx >= len) return fl_nil();
        char buf[2] = { s[idx], '\0' };
        return fl_str_val(buf);
    }
    return fl_nil();
}

FLValue length(FLValue obj) {
    if (obj.tag == FL_VECTOR) return fl_vec_len(obj);
    if (obj.tag == FL_MAP)    return fl_map_len(obj);
    if (obj.tag == FL_STRING) return fl_int((int64_t)strlen(((FLString*)obj.obj)->data));
    return fl_int(0);
}

FLValue char_at(FLValue str, FLValue idx) {
    return get(str, idx);
}

/* ── S15: stdlib bridge ── */

FLValue fl_vec_slice(FLValue vec, FLValue start, FLValue end) {
    if (vec.tag != FL_VECTOR) return fl_vec_new();
    FLVector* v = (FLVector*)vec.obj;
    int64_t s = start.tag == FL_INT ? start.i : 0;
    int64_t e = end.tag == FL_INT ? (end.i < 0 ? (int64_t)v->len + end.i + 1 : end.i) : (int64_t)v->len;
    if (s < 0) s = 0;
    if (e > (int64_t)v->len) e = (int64_t)v->len;
    if (s >= e) return fl_vec_new();
    /* O(n): fl_vec_from으로 단일 alloc */
    return fl_vec_from(v->data + s, (uint32_t)(e - s));
}

FLValue fl_vec_last(FLValue vec) {
    if (vec.tag != FL_VECTOR) return fl_nil();
    FLVector* v = (FLVector*)vec.obj;
    return v->len > 0 ? v->data[v->len-1] : fl_nil();
}

FLValue fl_map_del(FLValue map, FLValue key) {
    if (map.tag != FL_MAP) return map;
    FLMap* m = (FLMap*)map.obj;
    FLValue r = fl_map_new();
    for (uint32_t i = 0; i < m->len; i++)
        if (!fl_truthy(fl_eq(m->entries[i].key, key)))
            r = fl_map_set(r, m->entries[i].key, m->entries[i].val);
    return r;
}

FLValue fl_map_merge(FLValue a, FLValue b) {
    if (a.tag != FL_MAP) return b;
    if (b.tag != FL_MAP) return a;
    FLValue r = a;
    FLMap* mb = (FLMap*)b.obj;
    for (uint32_t i = 0; i < mb->len; i++)
        r = fl_map_set(r, mb->entries[i].key, mb->entries[i].val);
    return r;
}

FLValue append(FLValue v1, FLValue v2) {
    if (v1.tag != FL_VECTOR) return v2;
    if (v2.tag != FL_VECTOR) return v1;
    FLVector* va = (FLVector*)v1.obj;
    FLVector* vb = (FLVector*)v2.obj;
    FLValue r = fl_vec_new();
    for (uint32_t i = 0; i < va->len; i++)
        r = fl_vec_push(r, va->data[i]);
    for (uint32_t i = 0; i < vb->len; i++)
        r = fl_vec_push(r, vb->data[i]);
    return r;
}

/* C03: distinguish an absent key from a stored nil. */
FLValue fl_map_has(FLValue map, FLValue key) {
    if (map.tag!=FL_MAP) return fl_bool(false);
    FLMap *m=(FLMap*)map.obj;
    for(uint32_t i=0;i<m->len;i++)
        if(fl_truthy(fl_eq(m->entries[i].key,key))) return fl_bool(true);
    return fl_bool(false);
}
static FLValue compat_key(FLValue key,int nil_empty) {
    if(key.tag==FL_STRING) {
        key=fl_v11_key(key);FLString *s=(FLString*)key.obj;
        return s->len&&s->data[0]==':'?fl_str_val(s->data+1):key;
    }
    if(key.tag==FL_NIL)return fl_str_val(nil_empty?"":"null");
    if(key.tag==FL_BOOL)return fl_str_val(key.b?"true":"false");
    char b[64];
    if(key.tag==FL_INT||key.tag==FL_FLOAT)return fl_v11_number(key);
    else snprintf(b,sizeof(b),"[object Object]");
    return fl_str_val(b);
}
FLValue fl_has_key(FLValue map,FLValue key) {
    return fl_map_has(map,compat_key(key,1));
}
FLValue fl_get_default(FLValue obj, FLValue key, FLValue fallback) {
    if(obj.tag==FL_NIL) {
        const char *strict=getenv("FL_STRICT");
        if(strict && !strcmp(strict,"1")) {
            FLValue printed=compat_key(key,0);
            if(key.tag==FL_STRING)printed=fl_str_n(3,fl_str_val("\""),key,fl_str_val("\""));
            FLValue message=fl_str_n(3,fl_str_val("(get nil "),printed,
                fl_str_val(") — cannot access key on nil. Use (get-or coll key default)."));
            FLValue error=fl_map_new();
            error=fl_map_set(error,fl_str_val("message"),message);
            error=fl_map_set(error,fl_str_val("code"),fl_str_val("E_TYPE_NIL"));
            error=fl_map_set(error,fl_str_val("type"),fl_str_val("TypeError"));
            error=fl_map_set(error,fl_str_val("hint"),fl_str_val("값이 nil인지 (nil? x) 또는 (get-or x :key default) 로 먼저 확인하세요."));
            error.obj->type=FL_NATIVE_ERROR;
            fl_throw(error);
        }
        return fallback;
    }
    if(obj.tag==FL_MAP) {
        FLValue normalized=compat_key(key,0);
        if(fl_truthy(fl_map_has(obj,normalized))) return fl_map_get(obj,normalized);
        return fl_truthy(fl_map_has(obj,key))?fl_map_get(obj,key):fallback;
    }
    int64_t index;
    if(key.tag==FL_INT) index=key.i;
    else if(key.tag==FL_FLOAT && key.f>=0 && key.f<INT64_MAX && key.f==(int64_t)key.f) index=(int64_t)key.f;
    else return fallback;
    if(index<0) return fallback;
    if(obj.tag==FL_VECTOR) {
        FLVector *v=(FLVector*)obj.obj;
        if((uint64_t)index>=v->len || v->data[index].tag==FL_NIL) return fallback;
        return v->data[index];
    }
    if(obj.tag==FL_STRING) return fl_v11_text_get(obj,key,fallback);
    return fallback;
}

/* Exact source names are retained for string callbacks and redefinition. */
#include <pthread.h>
typedef struct NamedFn { char *name; FLValue fn; struct NamedFn *next; } NamedFn;
static NamedFn *named_fns;
static pthread_mutex_t named_fn_lock=PTHREAD_MUTEX_INITIALIZER;
FLValue fl_register_named_fn(FLValue name,FLValue fn) {
    if(name.tag!=FL_STRING) fl_throw(fl_str_val("invalid named function"));
    const char *s=((FLString*)name.obj)->data;
    pthread_mutex_lock(&named_fn_lock);
    NamedFn *p=named_fns;
    for(;p;p=p->next) if(!strcmp(p->name,s)) break;
    if(!p) {
        p=calloc(1,sizeof(*p));
        if(p) p->name=strdup(s);
        if(!p||!p->name) {free(p);pthread_mutex_unlock(&named_fn_lock);fl_throw(fl_str_val("named function allocation failed"));return fl_nil();}
        p->next=named_fns;named_fns=p;
    }
    p->fn=fn;pthread_mutex_unlock(&named_fn_lock);return fn;
}
FLValue fl_named_fn(FLValue name) {
    if(name.tag!=FL_STRING) return fl_nil();
    pthread_mutex_lock(&named_fn_lock);FLValue fn=fl_nil();
    for(NamedFn *p=named_fns;p;p=p->next) if(!strcmp(p->name,((FLString*)name.obj)->data)){fn=p->fn;break;}
    if(fn.tag==FL_NIL) {
        FLValue alias=fl_function_alias(name);
        if(alias.tag==FL_STRING)for(NamedFn *p=named_fns;p;p=p->next)
            if(p->fn.tag==FL_FN&&!strcmp(p->name,((FLString*)alias.obj)->data)){fn=p->fn;break;}
    }
    pthread_mutex_unlock(&named_fn_lock);return fn;
}
FLValue fl_function_alias(FLValue name) {
    if(name.tag!=FL_STRING)return fl_nil();
    FLString *s=(FLString*)name.obj;
    int underscore=memchr(s->data,'_',s->len)!=NULL;
    if(!underscore&&!memchr(s->data,'-',s->len))return fl_nil();
    /* Match the complete source namespace before trying a spelling alias. */
    for(size_t i=0;fl_native_names[i];i++)if(!strcmp(s->data,fl_native_names[i]))return fl_nil();
    char *copy=malloc(s->len+1);
    if(!copy)return fl_nil();
    memcpy(copy,s->data,s->len+1);
    for(uint32_t i=0;i<s->len;i++)if(copy[i]==(underscore?'_':'-'))copy[i]=underscore?'-':'_';
    FLValue result=fl_str_val(copy);free(copy);return result;
}
/* swap! uses the same callable path for named functions and lambdas. */
FLValue fl_swap_args(FLValue atom,FLValue fn,FLValue extra) {
    FLVector *v=(FLVector*)extra.obj;
    FLValue *args=malloc(sizeof(*args)*(v->len+1));
    if(!args) fl_throw(fl_str_val("swap!: allocation failed"));
    args[0]=fl_atom_deref(atom);memcpy(args+1,v->data,v->len*sizeof(*args));
    return fl_atom_reset(atom,fl_call_owned_argv(fn,v->len+1,args));
}
FLValue fl_call_owned_argv(FLValue fn,int argc,FLValue *args) {
    /* Preserve native catch state while releasing temporary argv on throw. */
    if(fl_try_top>=FL_TRY_MAX){free(args);fl_throw(fl_str_val("swap!: try stack overflow"));return fl_nil();}
    FLTryFrame *frame=&fl_try_stack[fl_try_top++];
    if(setjmp(frame->buf)!=0){FLValue error=frame->err;fl_try_top--;free(args);fl_throw(error);return fl_nil();}
    FLValue value=fl_fn_call(fn,argc,args);
    fl_try_top--;free(args);return value;
}
