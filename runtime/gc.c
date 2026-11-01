/*
 * gc.c — FreeLang Hybrid GC
 *
 * 두 가지 GC 전략을 혼합:
 *
 * 1. Arena GC (요청 스코프 — 기존 유지)
 *    - HTTP 요청마다 arena 생성 → 완료 시 bulk free
 *    - STW 없음, 극도로 빠름
 *    - 단점: atom/global이 보유한 값은 요청 완료 후 dangling!
 *
 * 2. RC-Heap GC (atom 보유값 — 신규)
 *    - atom.data[0]이 가리키는 값은 반드시 heap에 있어야 함
 *    - fl_heap_copy(v): arena 값은 heap으로 복사, 소유된 불변 값은 참조 유지
 *    - fl_heap_release(v): RC--, 0이면 재귀 free
 *    - swap!: 새값 heap_copy → 이전값 heap_release
 *
 * 왜 이 설계인가:
 *   - atom이 보유하는 값(예: fl-kv의 store 맵)은 요청 사이에 살아있어야 함
 *   - 기존 코드: atom.data[0] = arena 객체 → arena 리셋 후 dangling (UB!)
 *   - RC-Heap: atom 전용 heap 영역 → 명시적 생명주기 관리
 *   - 전체 GC (mark-sweep, tri-color)는 오버킬 — RC로 충분
 */

#define _GNU_SOURCE
#include "runtime.h"
#include "internal.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <pthread.h>

/* ── Arena 구조 ────────────────────────────────────────
   단일 연결 리스트로 청크를 이어붙임
   청크 크기: 64KB (L2 캐시에 맞게)
*/
#define ARENA_CHUNK_SIZE (64 * 1024)
#define FL_HEAP_RC UINT32_C(0x80000000)
static pthread_mutex_t gc_mutex;
static pthread_once_t gc_once=PTHREAD_ONCE_INIT;
static int gc_init_error;
static FLAtom *gc_atoms;
static int gc_pending;
static void gc_init(void){
    pthread_mutexattr_t attr;gc_init_error=pthread_mutexattr_init(&attr);
    if(gc_init_error)return;
    gc_init_error=pthread_mutexattr_settype(&attr,PTHREAD_MUTEX_RECURSIVE);
    if(!gc_init_error)gc_init_error=pthread_mutex_init(&gc_mutex,&attr);
    pthread_mutexattr_destroy(&attr);
}
void fl_gc_lock(void){pthread_once(&gc_once,gc_init);if(gc_init_error||pthread_mutex_lock(&gc_mutex))abort();}
void fl_gc_unlock(void){pthread_mutex_unlock(&gc_mutex);}
void fl_gc_track_atom(FLAtom *atom){
    fl_gc_lock();atom->gc_candidate=0;atom->gc_previous=NULL;atom->gc_next=gc_atoms;
    if(gc_atoms)gc_atoms->gc_previous=atom;gc_atoms=atom;fl_gc_unlock();
}
static void gc_untrack_atom(FLAtom *atom){
    if(atom->gc_previous)atom->gc_previous->gc_next=atom->gc_next;else gc_atoms=atom->gc_next;
    if(atom->gc_next)atom->gc_next->gc_previous=atom->gc_previous;
}
typedef struct BorrowPin {FLValue value;struct BorrowPin*next;} BorrowPin;
static __thread BorrowPin*borrow_pins;
void*fl_borrow_mark(void){return borrow_pins;}
void fl_borrow_release_to(void*mark){while(borrow_pins&&(void*)borrow_pins!=mark){BorrowPin*pin=borrow_pins;borrow_pins=pin->next;fl_heap_release(pin->value);free(pin);}}
/* Takes the reference retained while the atom lock was held. */
FLValue fl_borrow_take(FLValue value){
    if(value.tag<FL_STRING||!value.obj)return value;
    if(value.tag==FL_VECTOR&&value.obj->type!=FL_MANAGED_ATOM&&__atomic_load_n(&value.obj->rc,__ATOMIC_RELAXED)==0xFF)return value;
    BorrowPin*pin=malloc(sizeof(*pin));
    if(!pin){fl_heap_release(value);fl_throw(fl_str_val("snapshot pin allocation failed"));return fl_nil();}
    pin->value=value;pin->next=borrow_pins;borrow_pins=pin;return value;
}

typedef struct ArenaChunk {
    struct ArenaChunk* next;
    size_t used;
    size_t cap;
    char   data[];    /* 유연 배열 */
} ArenaChunk;

typedef struct Arena {
    ArenaChunk* head;       /* 현재 청크 */
    ArenaChunk* first;      /* 첫 청크 (재사용용) */
    size_t      total_alloc; /* 디버그: 총 할당량 */
    size_t      total_free;  /* 디버그: 총 해제량 */
    struct ArenaCleanup* cleanups;
    void* borrow_mark;
    struct Arena* previous;
} Arena;
typedef struct ArenaCleanup {void (*call)(void*);void*value;struct ArenaCleanup*next;} ArenaCleanup;

/* ── per-thread 아레나 ── */
static __thread Arena* g_arena = NULL;
static __thread Arena* cli_arena;
static __thread int thread_watched,thread_cleaning;
static pthread_key_t thread_key;
static pthread_once_t thread_once=PTHREAD_ONCE_INIT;
static int thread_key_error;
static void thread_cleanup(void*unused){
    (void)unused;thread_cleaning=1;thread_watched=0;
    pthread_setspecific(thread_key,NULL);
    fl_schedule_worker_cleanup();
    while(g_arena)fl_arena_scope_leave(g_arena->previous);
    fl_borrow_release_to(NULL);cli_arena=NULL;thread_cleaning=0;
}
static void process_thread_cleanup(void){thread_cleanup(NULL);}
static void thread_key_init(void){
    thread_key_error=pthread_key_create(&thread_key,thread_cleanup);
    if(!thread_key_error&&atexit(process_thread_cleanup))thread_key_error=1;
}
int fl_thread_lifetime_begin(void){
    if(thread_watched||thread_cleaning)return 1;
    pthread_once(&thread_once,thread_key_init);
    if(thread_key_error||pthread_setspecific(thread_key,(void*)1))return 0;
    thread_watched=1;return 1;
}
int fl_arena_active(void){return g_arena!=NULL;}
static void arena_clean(Arena*arena){
    if(arena)fl_borrow_release_to(arena->borrow_mark);
    while(arena&&arena->cleanups){ArenaCleanup*entry=arena->cleanups;arena->cleanups=entry->next;entry->call(entry->value);free(entry);}
    fl_gc_collect_atoms();
}
int fl_arena_on_reset(void (*cleanup)(void*),void*value){
    if(!g_arena)return 0;
    ArenaCleanup*entry=malloc(sizeof(*entry));if(!entry)return -1;
    *entry=(ArenaCleanup){cleanup,value,g_arena->cleanups};g_arena->cleanups=entry;return 1;
}

static ArenaChunk* arena_new_chunk(size_t min_size) {
    if(min_size>SIZE_MAX-2*sizeof(ArenaChunk))return NULL;
    size_t cap = ARENA_CHUNK_SIZE;
    if (min_size + sizeof(ArenaChunk) > cap)
        cap = min_size + sizeof(ArenaChunk);
    ArenaChunk* c = malloc(sizeof(ArenaChunk) + cap);
    if (!c) return NULL;
    c->next = NULL;
    c->used = 0;
    c->cap  = cap;
    return c;
}
static void arena_rewind(Arena *arena){
    /* Keep the small common-request cache, not every prior large upload. */
    ArenaChunk *chunk=arena->first->next;arena->first->next=NULL;
    while(chunk){ArenaChunk *next=chunk->next;free(chunk);chunk=next;}
    arena->first->used=0;arena->head=arena->first;
}

void fl_arena_begin(void) {
    if(!fl_thread_lifetime_begin()){fl_throw(fl_str_val("thread lifecycle initialization failed"));return;}
    if (!g_arena) {
        g_arena = calloc(1,sizeof(Arena));
        if(!g_arena){fl_throw(fl_str_val("arena allocation failed"));return;}
        g_arena->head  = arena_new_chunk(0);
        if(!g_arena->head){free(g_arena);g_arena=NULL;fl_throw(fl_str_val("arena allocation failed"));return;}
        g_arena->first = g_arena->head;
        g_arena->total_alloc = 0;
        g_arena->total_free  = 0;
        g_arena->borrow_mark=fl_borrow_mark();
    } else {
        arena_clean(g_arena);
        g_arena->borrow_mark=fl_borrow_mark();
        arena_rewind(g_arena);
    }
}

void* fl_arena_alloc(size_t size) {
    if(size>SIZE_MAX-7)return NULL;
    if (!g_arena || !g_arena->head) return malloc(size);  /* 폴백 */

    /* 8바이트 정렬 */
    size = (size + 7) & ~(size_t)7;

    ArenaChunk* c = g_arena->head;
    if (size <= c->cap-c->used) {
        void* ptr = c->data + c->used;
        c->used += size;
        g_arena->total_alloc += size;
        return ptr;
    }

    /* 새 청크 필요 */
    ArenaChunk* nc = c->next;
    /* A later request may need a larger upload than the retained next chunk. */
    if (!nc || nc->cap < size) {
        nc = arena_new_chunk(size);
        if (nc) { nc->next = c->next; c->next = nc; }
    }
    if (!nc) {
        void *fallback=malloc(size);
        if(fallback&&fl_arena_on_reset(free,fallback)!=1){free(fallback);return NULL;}
        return fallback;
    }

    nc->used = 0;
    g_arena->head = nc;

    void* ptr = nc->data;
    nc->used = size;
    g_arena->total_alloc += size;
    return ptr;
}

void fl_arena_end(void) {
    /* Preserve one small cache after request-owned values have been released. */
    if (!g_arena) return;
    arena_clean(g_arena);
    arena_rewind(g_arena);
}
void*fl_arena_scope_enter(void){
    if(!fl_thread_lifetime_begin()){fl_throw(fl_str_val("thread lifecycle initialization failed"));return NULL;}
    Arena*previous=g_arena;Arena*current=calloc(1,sizeof(*current));
    if(!current){fl_throw(fl_str_val("arena allocation failed"));return NULL;}
    current->first=current->head=arena_new_chunk(0);
    if(!current->head){free(current);fl_throw(fl_str_val("arena allocation failed"));return NULL;}
    current->borrow_mark=fl_borrow_mark();current->previous=previous;g_arena=current;return previous;
}
void fl_arena_scope_leave(void*previous){
    Arena*current=g_arena;arena_clean(current);
    if(current){ArenaChunk*chunk=current->first;while(chunk){ArenaChunk*next=chunk->next;free(chunk);chunk=next;}free(current);}
    g_arena=(Arena*)previous;
}

/* Register before timer initialization: its atexit drain must run first. */
static void cli_cleanup(void){
    if(!cli_arena)return;
    fl_schedule_worker_cleanup();
    Arena*previous=cli_arena->previous;
    while(g_arena&&g_arena!=previous)fl_arena_scope_leave(g_arena->previous);
    cli_arena=NULL;
}
void fl_cli_runtime_begin(void){
    if(cli_arena)return;
    if(!fl_thread_lifetime_begin()){fputs("FreeLang CLI initialization failed\n",stderr);exit(1);}
    Arena*current=calloc(1,sizeof(*current));
    if(current)current->first=current->head=arena_new_chunk(0);
    if(!current||!current->head){
        free(current);fputs("FreeLang CLI initialization failed\n",stderr);exit(1);
    }
    current->previous=g_arena;current->borrow_mark=fl_borrow_mark();
    if(atexit(cli_cleanup)){
        free(current->first);free(current);
        fputs("FreeLang CLI initialization failed\n",stderr);exit(1);
    }
    cli_arena=g_arena=current;
}

/* 누적 통계 (debug level 2+) */
void fl_arena_stats(void) {
    if (!g_arena) return;
    size_t n_chunks = 0;
    ArenaChunk* c = g_arena->first;
    while (c) { n_chunks++; c = c->next; }
    fl_log(2, "gc", "아레나 청크=%zu, 총할당=%zuKB",
           n_chunks, g_arena->total_alloc / 1024);
}

/* ── 영속 풀 (DB 연결, 설정 등) ──────────────────────
   요청 생명주기 밖의 객체는 여기서 관리
   동적 확장 (기존 256 고정 제한 제거)
*/
typedef struct PermNode {
    struct PermNode* next;
    void*            ptr;
} PermNode;

static PermNode*       g_perm_head  = NULL;
static size_t          g_perm_count = 0;
static pthread_mutex_t g_perm_lock  = PTHREAD_MUTEX_INITIALIZER;

void* fl_perm_alloc(size_t size) {
    void* ptr = malloc(size);
    if (!ptr) return NULL;
    PermNode* node = (PermNode*)malloc(sizeof(PermNode));
    if (!node) { free(ptr); return NULL; }
    node->ptr = ptr;
    pthread_mutex_lock(&g_perm_lock);
    node->next = g_perm_head;
    g_perm_head = node;
    g_perm_count++;
    pthread_mutex_unlock(&g_perm_lock);
    return ptr;
}

/* 프로세스 종료 시 전체 해제 (ASAN 호환) */
void fl_perm_cleanup(void) {
    pthread_mutex_lock(&g_perm_lock);
    PermNode* n = g_perm_head;
    while (n) {
        PermNode* nx = n->next;
        free(n->ptr);
        free(n);
        n = nx;
    }
    g_perm_head  = NULL;
    g_perm_count = 0;
    pthread_mutex_unlock(&g_perm_lock);
}

/* ── RC-Heap GC — atom 보유값 전용 ──────────────────
 *
 * atom.data[0]이 가리키는 FLValue는 반드시 RC-Heap에 있어야 함.
 * Arena 리셋 후에도 살아있어야 하기 때문.
 *
 * FLObject.rc 필드를 활용:
 *   rc == 0    → heap에 없음 (arena 객체 또는 임시값)
 *   rc high bit → heap snapshot, lower bits count references
 *   rc == 1..N → legacy owned closure 또는 arena 객체
 *   rc == 0xFF → 고정 (절대 해제 안 함, DB 연결 등)
 */

/* Copy arena values to the heap; already owned immutable values retain RC. */
static FLValue heap_copy_locked(FLValue v) {
    if(v.tag==FL_MAP&&v.obj&&v.obj->type==FL_NATIVE_PROMISE){fl_heap_retain(v);return v;}
    /* Ordinary owned strings/containers are immutable; retain their shared
       subtrees instead of cloning a full token stream at every tail step. */
    if((v.tag==FL_STRING||v.tag==FL_VECTOR||v.tag==FL_MAP)&&v.obj&&
       (__atomic_load_n(&v.obj->rc,__ATOMIC_RELAXED)&FL_HEAP_RC)){
        fl_heap_retain(v);return v;
    }
    switch (v.tag) {
    case FL_INT: case FL_FLOAT: case FL_BOOL: case FL_NIL:
        return v;  /* 스칼라: 값 복사로 충분 */

    case FL_STRING: {
        FLString* src = (FLString*)v.obj;
        FLString* dst = (FLString*)malloc(sizeof(FLString) + src->len + 1);
        if (!dst) return fl_nil();
        dst->base.type = src->base.type;
        dst->base.rc   = FL_HEAP_RC|1;
        dst->len = src->len;
        memcpy(dst->data, src->data, src->len + 1);
        FLValue r; r.tag = FL_STRING; r.obj = (FLObject*)dst;
        return r;
    }

    case FL_VECTOR: {
        if(v.obj->type==FL_MANAGED_ATOM){fl_heap_retain(v);return v;}
        if(__atomic_load_n(&v.obj->rc,__ATOMIC_RELAXED)==0xFF)return v; /* Atoms keep their shared identity. */
        FLVector* src = (FLVector*)v.obj;
        FLVector* dst = (FLVector*)malloc(sizeof(FLVector));
        if (!dst) return fl_nil();
        dst->base.type = src->base.type;
        dst->base.rc   = FL_HEAP_RC|1;
        dst->len = src->len;
        dst->cap = src->len;  /* 딱 맞게 */
        dst->data = src->len > 0
            ? (FLValue*)malloc(src->len * sizeof(FLValue))
            : NULL;
        for (uint32_t i = 0; i < src->len; i++)
            dst->data[i] = fl_heap_copy(src->data[i]);
        FLValue r; r.tag = FL_VECTOR; r.obj = (FLObject*)dst;
        return r;
    }

    case FL_MAP: {
        FLMap* src = (FLMap*)v.obj;
        FLMap* dst = (FLMap*)malloc(sizeof(FLMap));
        if (!dst) return fl_nil();
        dst->base.type = src->base.type;
        dst->base.rc   = FL_HEAP_RC|1;
        dst->len = src->len;
        dst->cap = src->len;
        dst->entries = src->len > 0
            ? (FLMapEntry*)malloc(src->len * sizeof(FLMapEntry))
            : NULL;
        for (uint32_t i = 0; i < src->len; i++) {
            dst->entries[i].key = fl_heap_copy(src->entries[i].key);
            dst->entries[i].val = fl_heap_copy(src->entries[i].val);
        }
        FLValue r; r.tag = FL_MAP; r.obj = (FLObject*)dst;
        return r;
    }

    case FL_FN: {
        if(v.obj->type==FL_OWNED_FN){fl_heap_retain(v);return v;}
        FLClosure*src=(FLClosure*)v.obj;
        FLClosure*dst=malloc(sizeof(*dst)+src->nenv*sizeof(FLValue));if(!dst)return fl_nil();
        dst->base.type=FL_OWNED_FN;dst->base.rc=1;dst->call=src->call;dst->nenv=src->nenv;dst->hot_count=0;dst->user_global=src->user_global;
        for(uint32_t i=0;i<src->nenv;i++)dst->env[i]=fl_heap_copy(src->env[i]);
        FLValue result={.tag=FL_FN,.obj=(FLObject*)dst};return result;
    }
    }
    return fl_nil();
}
FLValue fl_heap_copy(FLValue v){fl_gc_lock();FLValue result=heap_copy_locked(v);fl_gc_unlock();return result;}

/* RC++ (heap 객체에 대한 추가 참조) */
static void heap_retain_locked(FLValue v) {
    if (v.tag < FL_STRING || !v.obj) return;
    if(v.obj->type==FL_OWNED_FN||v.obj->type==FL_NATIVE_PROMISE||v.obj->type==FL_MANAGED_ATOM){__atomic_add_fetch(&v.obj->rc,1,__ATOMIC_RELAXED);return;}
    uint32_t rc=__atomic_load_n(&v.obj->rc,__ATOMIC_RELAXED);
    if(rc==0||rc==0xFF)return;
    __atomic_add_fetch(&v.obj->rc,1,__ATOMIC_RELAXED);
}
void fl_heap_retain(FLValue v){fl_gc_lock();heap_retain_locked(v);fl_gc_unlock();}

/* RC-- → 0이면 재귀 해제 */
static void heap_release_locked(FLValue v) {
    if (v.tag < FL_STRING || !v.obj) return;
    if(v.obj->type==FL_MANAGED_ATOM){
        uint32_t rc=__atomic_load_n(&v.obj->rc,__ATOMIC_RELAXED);if(!rc)return;
        FLAtom *atom=(FLAtom*)v.obj;
        if(__atomic_fetch_sub(&v.obj->rc,1,__ATOMIC_ACQ_REL)!=1){atom->gc_candidate=1;gc_pending=1;return;}
        gc_untrack_atom(atom);fl_heap_release(atom->vector.data[0]);
        if(pthread_mutex_destroy(&atom->mutex)){fputs("atom release encountered a locked object\n",stderr);abort();}
        free(atom->vector.data);free(atom);return;
    }
    if(v.obj->type==FL_OWNED_FN||v.obj->type==FL_NATIVE_PROMISE){
        if(__atomic_fetch_sub(&v.obj->rc,1,__ATOMIC_ACQ_REL)!=1)return;
        if(v.obj->type==FL_NATIVE_PROMISE){fl_schedule_promise_destroy(v);return;}
        FLClosure*cl=(FLClosure*)v.obj;for(uint32_t i=0;i<cl->nenv;i++)fl_heap_release(cl->env[i]);free(cl);return;
    }
    uint32_t rc=__atomic_load_n(&v.obj->rc,__ATOMIC_RELAXED);
    if(rc==0||rc==0xFF)return;
    uint32_t before=__atomic_fetch_sub(&v.obj->rc,1,__ATOMIC_ACQ_REL);
    if(before!=(rc&FL_HEAP_RC?FL_HEAP_RC|1:1))return;

    /* RC = 0 → 해제 */
    switch (v.tag) {
    case FL_STRING:
        free(v.obj);
        break;
    case FL_VECTOR: {
        FLVector* vp = (FLVector*)v.obj;
        for (uint32_t i = 0; i < vp->len; i++)
            fl_heap_release(vp->data[i]);
        free(vp->data);
        free(vp);
        break;
    }
    case FL_MAP: {
        FLMap* mp = (FLMap*)v.obj;
        for (uint32_t i = 0; i < mp->len; i++) {
            fl_heap_release(mp->entries[i].key);
            fl_heap_release(mp->entries[i].val);
        }
        free(mp->entries);
        free(mp);
        break;
    }
    case FL_FN:
        free(v.obj);
        break;
    default: break;
    }
}
void fl_heap_release(FLValue v){fl_gc_lock();heap_release_locked(v);fl_gc_unlock();}

#include "atom-gc.h"

static void snapshot_owned_cleanup(void*pointer) {
    FLObject*object=pointer;
    FLValue value={.tag=object->type==FL_MANAGED_ATOM?FL_VECTOR:object->type==FL_OWNED_FN?FL_FN:
                       object->type==FL_NATIVE_PROMISE||object->type==FL_NATIVE_TAIL?FL_MAP:object->type,.obj=object};
    fl_heap_release(value);
}

/* A returned child may outlive the pinned atom root that contains it. */
static FLValue snapshot_copy(FLValue value) {
    if(value.tag<FL_STRING||!value.obj)return value;
    if(value.tag==FL_VECTOR&&value.obj->type!=FL_MANAGED_ATOM&&__atomic_load_n(&value.obj->rc,__ATOMIC_RELAXED)==0xFF)return value;
    if(value.tag==FL_FN||value.obj->type==FL_NATIVE_PROMISE||value.obj->type==FL_MANAGED_ATOM||
       (__atomic_load_n(&value.obj->rc,__ATOMIC_RELAXED)&FL_HEAP_RC)){
        FLValue owned=fl_heap_copy(value);
        if(!owned.obj){fl_throw(fl_str_val("snapshot allocation failed"));return fl_nil();}
        if(fl_arena_on_reset(snapshot_owned_cleanup,owned.obj)<0){
            fl_heap_release(owned);fl_throw(fl_str_val("snapshot cleanup allocation failed"));return fl_nil();
        }
        return owned;
    }
    if(value.tag==FL_STRING){
        FLString*src=(FLString*)value.obj;
        FLString*dst=fl_arena_alloc(sizeof(*dst)+src->len+1);
        if(!dst){fl_throw(fl_str_val("snapshot allocation failed"));return fl_nil();}
        dst->base=(FLObject){.type=src->base.type,.rc=0};dst->len=src->len;
        memcpy(dst->data,src->data,src->len+1);
        return (FLValue){.tag=FL_STRING,.obj=(FLObject*)dst};
    }
    if(value.tag==FL_VECTOR){
        FLVector*src=(FLVector*)value.obj;
        FLVector*dst=fl_arena_alloc(sizeof(*dst));
        FLValue*data=src->len?fl_arena_alloc(src->len*sizeof(*data)):NULL;
        if(!dst||(src->len&&!data)){fl_throw(fl_str_val("snapshot allocation failed"));return fl_nil();}
        dst->base=(FLObject){.type=src->base.type,.rc=0};dst->len=dst->cap=src->len;dst->data=data;
        for(uint32_t i=0;i<src->len;i++)data[i]=snapshot_copy(src->data[i]);
        return (FLValue){.tag=FL_VECTOR,.obj=(FLObject*)dst};
    }
    if(value.tag==FL_MAP){
        FLMap*src=(FLMap*)value.obj;
        FLMap*dst=fl_arena_alloc(sizeof(*dst));
        FLMapEntry*entries=src->len?fl_arena_alloc(src->len*sizeof(*entries)):NULL;
        if(!dst||(src->len&&!entries)){fl_throw(fl_str_val("snapshot allocation failed"));return fl_nil();}
        dst->base=(FLObject){.type=src->base.type,.rc=0};dst->len=dst->cap=src->len;dst->entries=entries;
        for(uint32_t i=0;i<src->len;i++){
            entries[i].key=snapshot_copy(src->entries[i].key);
            entries[i].val=snapshot_copy(src->entries[i].val);
        }
        return (FLValue){.tag=FL_MAP,.obj=(FLObject*)dst};
    }
    return value;
}

FLValue fl_borrow_finish(void*mark,FLValue result) {
    if(borrow_pins!=(BorrowPin*)mark)result=snapshot_copy(result);
    fl_borrow_release_to(mark);
    return result;
}
FLValue fl_arena_copy(FLValue value){return snapshot_copy(value);}

/* ── 메모리 사용량 통계 ── */
FLValue fl_memory_stats(void) {
    FLValue m = fl_map_new();

    /* /proc/self/status에서 VmRSS 읽기 */
    FILE* f = fopen("/proc/self/status", "r");
    if (f) {
        char line[256];
        while (fgets(line, sizeof(line), f)) {
            long kb = 0;
            if (sscanf(line, "VmRSS: %ld kB", &kb) == 1)
                m = fl_map_set(m, fl_str_val("rss_kb"), fl_int(kb));
            if (sscanf(line, "VmPeak: %ld kB", &kb) == 1)
                m = fl_map_set(m, fl_str_val("peak_kb"), fl_int(kb));
            if (sscanf(line, "VmSize: %ld kB", &kb) == 1)
                m = fl_map_set(m, fl_str_val("virt_kb"), fl_int(kb));
        }
        fclose(f);
    }

    if (g_arena) {
        size_t n = 0; ArenaChunk* c = g_arena->first;
        while (c) { n += c->used; c = c->next; }
        m = fl_map_set(m, fl_str_val("arena_kb"), fl_int((int64_t)(n / 1024)));
    }
    return m;
}
