#include "runtime.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <pthread.h>
typedef struct {FLVector vector;pthread_mutex_t mutex;} FLAtom;
static pthread_mutex_t atom_locks[64];
static pthread_once_t atom_locks_once=PTHREAD_ONCE_INIT;
static int atom_lock_error;
static int atom_mutex_init(pthread_mutex_t *mutex){
    pthread_mutexattr_t attr;
    int rc=pthread_mutexattr_init(&attr);
    if(rc)return rc;
    rc=pthread_mutexattr_settype(&attr,PTHREAD_MUTEX_RECURSIVE);
    if(!rc)rc=pthread_mutex_init(mutex,&attr);
    pthread_mutexattr_destroy(&attr);return rc;
}
static void init_atom_locks(void){
    for(int i=0;i<64;i++)if((atom_lock_error=atom_mutex_init(&atom_locks[i])))return;
}
static pthread_mutex_t*atom_lock(FLValue atom){
    /* Only atom_new creates immortal vectors; each keeps its own update lock. */
    if(__atomic_load_n(&atom.obj->rc,__ATOMIC_RELAXED)==0xFF)return &((FLAtom*)atom.obj)->mutex;
    pthread_once(&atom_locks_once,init_atom_locks);
    if(atom_lock_error)fl_throw(fl_str_val("atom: lock initialization failed"));
    return &atom_locks[((uintptr_t)atom.obj>>4)%64];
}

FLValue fl_floor(FLValue x) { return x.tag==FL_FLOAT ? fl_float(floor(x.f)) : x; }
FLValue fl_ceil(FLValue x)  { return x.tag==FL_FLOAT ? fl_float(ceil(x.f))  : x; }
FLValue fl_abs(FLValue x) {
    if (x.tag==FL_INT)   return fl_int(x.i<0 ? -x.i : x.i);
    if (x.tag==FL_FLOAT) return fl_float(x.f<0 ? -x.f : x.f);
    return x;
}
FLValue fl_math_sqrt(FLValue x) {
    return fl_float(sqrt(x.tag==FL_INT ? (double)x.i : x.f));
}
FLValue fl_now(void)    { return fl_int((int64_t)time(NULL)); }
FLValue fl_now_ms(void) {
    struct timespec ts; clock_gettime(CLOCK_REALTIME, &ts);
    return fl_int(ts.tv_sec*1000LL + ts.tv_nsec/1000000LL);
}
FLValue string_p(FLValue v) { return fl_bool(v.tag==FL_STRING); }
FLValue array_p(FLValue v)  { return fl_bool(v.tag==FL_VECTOR); }
FLValue list_p(FLValue v)   { return fl_bool(v.tag==FL_VECTOR); }
FLValue map_p(FLValue v)    { return fl_bool(v.tag==FL_MAP); }
FLValue fn_p(FLValue v)     { return fl_bool(v.tag==FL_FN); }
FLValue type_of(FLValue v) {
    switch(v.tag) {
        case FL_NIL:    return fl_str_val("nil");
        case FL_INT:    return fl_str_val("number");
        case FL_FLOAT:  return fl_str_val("number");
        case FL_BOOL:   return fl_str_val("boolean");
        case FL_STRING: return fl_str_val("string");
        case FL_VECTOR: return fl_str_val("array");
        case FL_MAP:    return fl_str_val("map");
        case FL_FN:     return fl_str_val("function");
        default:        return fl_str_val("unknown");
    }
}

/* B-3: String ops */
FLValue str_replace(FLValue s, FLValue from, FLValue to) {
    if (s.tag!=FL_STRING||from.tag!=FL_STRING||to.tag!=FL_STRING) return s;
    const char *src=((FLString*)s.obj)->data;
    const char *f=((FLString*)from.obj)->data;
    const char *t=((FLString*)to.obj)->data;
    size_t flen=strlen(f);
    if (flen==0) return s;
    size_t tlen=strlen(t);
    /* global replace: 모든 occurrence 치환 */
    size_t cap=strlen(src)*2+64; char *buf=(char*)malloc(cap+1);
    size_t wi=0; const char *cur=src; const char *pos;
    while ((pos=strstr(cur,f))) {
        size_t plen=(size_t)(pos-cur);
        if (wi+plen+tlen>cap) { cap=(wi+plen+tlen)*2; buf=(char*)realloc(buf,cap+1); }
        memcpy(buf+wi,cur,plen); wi+=plen;
        memcpy(buf+wi,t,tlen); wi+=tlen;
        cur=pos+flen;
    }
    size_t rest=strlen(cur);
    if (wi+rest>cap) buf=(char*)realloc(buf,wi+rest+1);
    memcpy(buf+wi,cur,rest); buf[wi+rest]='\0';
    FLValue r=fl_str_val(buf); free(buf); return r;
}
FLValue split(FLValue s, FLValue sep) {
    if (s.tag!=FL_STRING||sep.tag!=FL_STRING) return fl_vec_new();
    const char *src=((FLString*)s.obj)->data;
    const char *d=((FLString*)sep.obj)->data;
    size_t dlen=strlen(d); FLValue vec=fl_vec_new();
    const char *p=src, *q;
    while ((q=strstr(p,d))) {
        size_t n=(size_t)(q-p); char *buf=(char*)malloc(n+1);
        memcpy(buf,p,n); buf[n]='\0';
        vec=fl_vec_push(vec,fl_str_val(buf)); free(buf); p=q+dlen;
    }
    return fl_vec_push(vec,fl_str_val(p));
}
FLValue join(FLValue vec, FLValue sep) {
    if (vec.tag!=FL_VECTOR) return fl_str_val("");
    FLVector *v=(FLVector*)vec.obj;
    const char *d=sep.tag==FL_STRING?((FLString*)sep.obj)->data:"";
    size_t dlen=strlen(d), total=0;
    for (uint32_t i=0;i<v->len;i++) {
        if (v->data[i].tag==FL_STRING) total+=strlen(((FLString*)v->data[i].obj)->data);
        if (i+1<v->len) total+=dlen;
    }
    char *buf=(char*)malloc(total+1); buf[0]='\0';
    for (uint32_t i=0;i<v->len;i++) {
        if (v->data[i].tag==FL_STRING) strcat(buf,((FLString*)v->data[i].obj)->data);
        if (i+1<v->len) strcat(buf,d);
    }
    FLValue r=fl_str_val(buf); free(buf); return r;
}

/* B-4: Range / String utilities */
/* cgc-dispatch는 (range N) → range(N) 1인자 호출 */
FLValue range(FLValue n_or_start) {
    int64_t e = n_or_start.tag==FL_INT ? n_or_start.i : 0;
    FLValue vec = fl_vec_new();
    for (int64_t i = 0; i < e; i++) vec = fl_vec_push(vec, fl_int(i));
    return vec;
}
/* 2인자 버전 (직접 C에서 호출 시) */
FLValue range2(FLValue start, FLValue end) {
    int64_t s=start.tag==FL_INT?start.i:0, e=end.tag==FL_INT?end.i:0;
    FLValue vec=fl_vec_new();
    for (int64_t i=s;i<e;i++) vec=fl_vec_push(vec,fl_int(i));
    return vec;
}
FLValue char_code_at(FLValue s, FLValue idx) {
    if (s.tag!=FL_STRING||idx.tag!=FL_INT) return fl_nil();
    const char *p=((FLString*)s.obj)->data;
    int64_t i=idx.i, len=(int64_t)strlen(p);
    if (i<0||i>=len) return fl_nil();
    return fl_int((unsigned char)p[i]);
}
FLValue substring(FLValue s, FLValue start, FLValue end) {
    /* 벡터 슬라이스 지원: (slice vec i j) → substring(vec, i, j) */
    if (s.tag==FL_VECTOR) {
        FLVector* vec=(FLVector*)s.obj;
        int64_t len=(int64_t)vec->len;
        int64_t a=start.tag==FL_INT?start.i:0, b=end.tag==FL_INT?end.i:len;
        if (a<0) a=0;
        if (b>len) b=len;
        if (a>b) a=b;
        FLValue r=fl_vec_new();
        for (int64_t i=a; i<b; i++) r=fl_vec_push(r, vec->data[i]);
        return r;
    }
    if (s.tag!=FL_STRING) return fl_str_val("");
    const char *p=((FLString*)s.obj)->data;
    int64_t len=(int64_t)strlen(p);
    /* FL_FLOAT 인덱스 지원 (math_floor 결과) */
    int64_t a = (start.tag==FL_INT) ? start.i :
                (start.tag==FL_FLOAT) ? (int64_t)start.f : 0;
    int64_t b = (end.tag==FL_INT)   ? end.i   :
                (end.tag==FL_FLOAT)   ? (int64_t)end.f   : len;
    if (a<0) a=0;
    if (b>len) b=len;
    if (a>b) a=b;
    size_t n=(size_t)(b-a); char *buf=(char*)malloc(n+1);
    memcpy(buf,p+a,n); buf[n]='\0';
    FLValue r=fl_str_val(buf); free(buf); return r;
}
FLValue trim(FLValue s) {
    if (s.tag!=FL_STRING) return s;
    const char *p=((FLString*)s.obj)->data;
    while (*p==' '||*p=='\t'||*p=='\n'||*p=='\r') p++;
    size_t len=strlen(p);
    while (len>0&&(p[len-1]==' '||p[len-1]=='\t'||p[len-1]=='\n'||p[len-1]=='\r')) len--;
    char *buf=(char*)malloc(len+1); memcpy(buf,p,len); buf[len]='\0';
    FLValue r=fl_str_val(buf); free(buf); return r;
}

/* B-5: index-of */
FLValue index_of(FLValue vec, FLValue val) {
    if (vec.tag!=FL_VECTOR) return fl_int(-1);
    FLVector *v=(FLVector*)vec.obj;
    for (uint32_t i=0;i<v->len;i++)
        if (fl_truthy(fl_eq(v->data[i],val))) return fl_int((int64_t)i);
    return fl_int(-1);
}
FLValue str_index_of(FLValue s, FLValue sub) {
    if (s.tag!=FL_STRING||sub.tag!=FL_STRING) return fl_int(-1);
    const char *p=strstr(((FLString*)s.obj)->data,((FLString*)sub.obj)->data);
    if (!p) return fl_int(-1);
    return fl_int((int64_t)(p-((FLString*)s.obj)->data));
}

/* ── S22: argv ── */
static FLValue _fl_argv;
static bool    _fl_argv_init = false;

void fl_init_argv(int argc, char** argv) {
    FLValue vec = fl_vec_new();
    for (int i = 1; i < argc; i++)   /* argv[0]은 프로그램 이름 — Node slice(2)와 동일 */
        vec = fl_vec_push(vec, fl_str_val(argv[i]));
    _fl_argv = vec;
    _fl_argv_init = true;
}

FLValue fl_get_argv(void) {
    return _fl_argv_init ? _fl_argv : fl_vec_new();
}

/* ── S26: atom ── */
/*
 * atom — RC-Heap 방식으로 안전하게 관리
 *
 * 문제: 기존 fl_vec_from()은 Arena에 할당 → 요청 완료 후 dangling
 * 해결: atom 벡터 + 보유값을 모두 heap_copy로 heap에 올림
 *       swap!할 때 이전 값 heap_release → 메모리 누수 방지
 */
FLValue fl_atom_new(FLValue init) {
    /* atom 벡터를 malloc으로 직접 생성 (arena 우회) */
    FLAtom *shared=malloc(sizeof(*shared));
    if(!shared)return fl_nil();
    if(atom_mutex_init(&shared->mutex)){free(shared);return fl_nil();}
    FLVector* av = &shared->vector;
    av->base.type = FL_VECTOR;
    av->base.rc   = 0xFF;   /* 고정: atom 컨테이너는 절대 해제 안 함 */
    av->len  = 1;
    av->cap  = 1;
    av->data = (FLValue*)malloc(sizeof(FLValue));
    if (!av->data) { pthread_mutex_destroy(&shared->mutex);free(shared);return fl_nil(); }
    av->data[0] = fl_heap_copy(init);  /* 초기값도 heap으로 */
    FLValue r; r.tag = FL_VECTOR; r.obj = (FLObject*)av;
    return r;
}

FLValue fl_atom_deref(FLValue atom) {
    if (atom.tag != FL_VECTOR || !atom.obj || !((FLVector*)atom.obj)->len) return fl_nil();
    pthread_mutex_t*mutex=atom_lock(atom);pthread_mutex_lock(mutex);
    FLValue value=((FLVector*)atom.obj)->data[0];fl_heap_retain(value);pthread_mutex_unlock(mutex);
    return fl_borrow_take(value);
}

FLValue fl_atom_reset(FLValue atom, FLValue val) {
    if (atom.tag != FL_VECTOR || !atom.obj || !((FLVector*)atom.obj)->len) return fl_nil();
    FLVector* av = (FLVector*)atom.obj;
    FLValue replacement=fl_heap_copy(val);
    pthread_mutex_t*mutex=atom_lock(atom);pthread_mutex_lock(mutex);
    FLValue old = av->data[0];
    av->data[0] = replacement;
    pthread_mutex_unlock(mutex);
    fl_heap_release(old);              /* 이전 값 RC-- */
    return val; /* v11 reset! returns the supplied value. */
}

FLValue fl_swap_args(FLValue atom,FLValue fn,FLValue extra) {
    if(atom.tag!=FL_VECTOR||!atom.obj||!((FLVector*)atom.obj)->len)return fl_nil();
    if(extra.tag!=FL_VECTOR||!extra.obj)fl_throw(fl_str_val("swap!: invalid extra arguments"));
    FLVector *v=(FLVector*)extra.obj;
    if(v->len>=INT32_MAX)fl_throw(fl_str_val("swap!: too many arguments"));
    if(fl_try_top>=FL_TRY_MAX)fl_throw(fl_str_val("swap!: try stack overflow"));
    pthread_mutex_t *mutex=atom_lock(atom);
    if(pthread_mutex_lock(mutex))fl_throw(fl_str_val("swap!: lock failed"));
    FLValue *volatile pending=NULL;
    FLTryFrame *frame=&fl_try_stack[fl_try_top++];
    if(setjmp(frame->buf)!=0){
        FLValue error=frame->err;fl_try_top--;
        free((void*)pending);pthread_mutex_unlock(mutex);fl_throw(error);return fl_nil();
    }
    pending=malloc(sizeof(FLValue)*(v->len+1));
    if(!pending)fl_throw(fl_str_val("swap!: allocation failed"));
    pending[0]=fl_atom_deref(atom);
    if(v->len)memcpy(pending+1,v->data,v->len*sizeof(FLValue));
    FLValue *owned=pending;pending=NULL;
    FLValue result=fl_call_owned_argv(fn,v->len+1,owned);
    fl_atom_reset(atom,result);
    fl_try_top--;pthread_mutex_unlock(mutex);return result;
}

FLValue fl_includes_item(FLValue vec, FLValue item) {
    if (vec.tag != FL_VECTOR) return fl_bool(false);
    FLVector* v = (FLVector*)vec.obj;
    for (uint32_t i = 0; i < v->len; i++)
        if (fl_truthy(fl_eq(v->data[i], item))) return fl_bool(true);
    return fl_bool(false);
}

FLValue fl_str_includes(FLValue s, FLValue sub) {
    if (s.tag != FL_STRING || sub.tag != FL_STRING) return fl_bool(false);
    return fl_bool(strstr(((FLString*)s.obj)->data, ((FLString*)sub.obj)->data) != NULL);
}

FLValue fl_string_p(FLValue v) { return string_p(v); }

/* ── stdlib aliases — self/all.fl compatibility ── */

