#ifndef FREELANG_RUNTIME_INTERNAL_H
#define FREELANG_RUNTIME_INTERNAL_H

#include "runtime.h"
#include <pthread.h>
typedef struct FLAtom {
    FLVector vector;
    pthread_mutex_t mutex;
    struct FLAtom *gc_next,*gc_previous;
    int gc_candidate;
} FLAtom;
int fl_arena_active(void);
void fl_gc_lock(void);
void fl_gc_unlock(void);
void fl_gc_track_atom(FLAtom *atom);
void fl_gc_collect_atoms(void);

/* io.c에서 정의, process.c·error.c가 공유 */
const char* _strval(FLValue v);
FLValue fl_file_write_reserved_result(int reserved, FLValue content);

/* core.c에서 정의, io.c의 str_join이 사용 */
const char* fl_to_str(FLValue v, char* buf, size_t sz);

#endif /* FREELANG_RUNTIME_INTERNAL_H */
