/*
 * gc.c — FreeLang 요청 범위 Arena 할당자
 *
 * 설계:
 *   - 전역 GC가 아닌 "요청당 아레나" 방식
 *   - 각 요청 처리 시작 시 arena 생성
 *   - 요청 완료 시 arena 전체 해제 (단일 free)
 *   - 장기 객체(DB 연결 등)는 별도 영속 풀
 *
 * 왜 전역 GC 아닌가:
 *   - HTTP 서버 = 요청 단위가 자연스러운 생명주기
 *   - STW(Stop-The-World) 없음 → 지연 없음
 *   - 구현 단순 → 버그 적음
 */

#define _GNU_SOURCE
#include "runtime.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <pthread.h>

/* ── Arena 구조 ────────────────────────────────────────
   단일 연결 리스트로 청크를 이어붙임
   청크 크기: 64KB (L2 캐시에 맞게)
*/
#define ARENA_CHUNK_SIZE (64 * 1024)

typedef struct ArenaChunk {
    struct ArenaChunk* next;
    size_t used;
    size_t cap;
    char   data[];    /* 유연 배열 */
} ArenaChunk;

typedef struct {
    ArenaChunk* head;       /* 현재 청크 */
    ArenaChunk* first;      /* 첫 청크 (재사용용) */
    size_t      total_alloc; /* 디버그: 총 할당량 */
    size_t      total_free;  /* 디버그: 총 해제량 */
} Arena;

/* ── per-thread 아레나 ── */
static __thread Arena* g_arena = NULL;

static ArenaChunk* arena_new_chunk(size_t min_size) {
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

void fl_arena_begin(void) {
    if (!g_arena) {
        g_arena = malloc(sizeof(Arena));
        g_arena->head  = arena_new_chunk(0);
        g_arena->first = g_arena->head;
        g_arena->total_alloc = 0;
        g_arena->total_free  = 0;
    } else {
        /* 아레나 재사용: 첫 청크로 리셋 */
        g_arena->head = g_arena->first;
        ArenaChunk* c = g_arena->first;
        while (c) { c->used = 0; c = c->next; }
    }
}

void* fl_arena_alloc(size_t size) {
    if (!g_arena || !g_arena->head) return malloc(size);  /* 폴백 */

    /* 8바이트 정렬 */
    size = (size + 7) & ~(size_t)7;

    ArenaChunk* c = g_arena->head;
    if (c->used + size <= c->cap) {
        void* ptr = c->data + c->used;
        c->used += size;
        g_arena->total_alloc += size;
        return ptr;
    }

    /* 새 청크 필요 */
    ArenaChunk* nc = c->next ? c->next : arena_new_chunk(size);
    if (!nc) return malloc(size);  /* OOM 폴백 */

    if (!c->next) c->next = nc;
    nc->used = 0;
    g_arena->head = nc;

    void* ptr = nc->data;
    nc->used = size;
    g_arena->total_alloc += size;
    return ptr;
}

void fl_arena_end(void) {
    /* 아레나 리셋만 (메모리 유지 — 다음 요청에 재사용) */
    if (!g_arena) return;
    ArenaChunk* c = g_arena->first;
    while (c) { c->used = 0; c = c->next; }
    g_arena->head = g_arena->first;
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
*/
#define PERM_POOL_MAX 256
static void*  g_perm_ptrs[PERM_POOL_MAX];
static size_t g_perm_count = 0;
static pthread_mutex_t g_perm_lock = PTHREAD_MUTEX_INITIALIZER;

void* fl_perm_alloc(size_t size) {
    void* ptr = malloc(size);
    if (!ptr) return NULL;
    pthread_mutex_lock(&g_perm_lock);
    if (g_perm_count < PERM_POOL_MAX)
        g_perm_ptrs[g_perm_count++] = ptr;
    pthread_mutex_unlock(&g_perm_lock);
    return ptr;
}

/* 프로세스 종료 시 전체 해제 (ASAN 호환) */
void fl_perm_cleanup(void) {
    pthread_mutex_lock(&g_perm_lock);
    for (size_t i = 0; i < g_perm_count; i++) free(g_perm_ptrs[i]);
    g_perm_count = 0;
    pthread_mutex_unlock(&g_perm_lock);
}

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
