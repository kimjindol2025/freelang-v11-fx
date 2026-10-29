#include "runtime.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static FLValue shared;
static pthread_barrier_t barrier;
static void require(int ok,const char*why){
    if(!ok){fprintf(stderr,"SNAPSHOT_MEMORY_FAIL %s\n",why);exit(1);}
}
static FLValue record(int number){
    return fl_map_set(fl_map_set(fl_map_new(),fl_str_val("number"),fl_int(number)),
                      fl_str_val("text"),fl_str_val("snapshot-한글-😀"));
}
static void*reader(void*unused){
    (void)unused;
    for(int i=0;i<1000;i++){
        fl_arena_begin();
        FLValue before=fl_atom_deref(shared);
        pthread_barrier_wait(&barrier);
        pthread_barrier_wait(&barrier);
        FLValue n=fl_map_get(before,fl_str_val("number"));
        FLValue text=fl_map_get(before,fl_str_val("text"));
        require(n.tag==FL_INT&&n.i==i,"concurrent old number");
        require(text.tag==FL_STRING&&!strcmp(((FLString*)text.obj)->data,"snapshot-한글-😀"),"concurrent old text");
        fl_arena_end();
        pthread_barrier_wait(&barrier);
    }
    return NULL;
}
static FLValue return_snapshot(FLClosure*closure,int argc,FLValue*argv){
    (void)closure;(void)argc;(void)argv;
    return fl_atom_deref(shared);
}
int main(void){
    fl_arena_begin();shared=fl_atom_new(record(0));
    require(!pthread_barrier_init(&barrier,NULL,5),"barrier");
    pthread_t threads[4];
    for(int i=0;i<4;i++)require(!pthread_create(&threads[i],NULL,reader,NULL),"thread");
    for(int i=0;i<1000;i++){
        pthread_barrier_wait(&barrier);
        fl_atom_reset(shared,record(i+1));
        fl_arena_end();fl_arena_begin();
        pthread_barrier_wait(&barrier);
        pthread_barrier_wait(&barrier);
    }
    for(int i=0;i<4;i++)require(!pthread_join(threads[i],NULL),"join");
    pthread_barrier_destroy(&barrier);
    puts("CONCURRENT_SNAPSHOTS_4X1000=PASS");
    FLValue old=fl_atom_deref(shared);
    void*outer=fl_arena_scope_enter();
    (void)fl_atom_deref(shared);fl_atom_reset(shared,record(2000));
    fl_arena_scope_leave(outer);
    require(fl_map_get(old,fl_str_val("number")).i==1000,"nested arena pin boundary");
    puts("NESTED_ARENA_PINS=PASS");
    FLValue fn=fl_fn_new(return_snapshot,0,NULL);
    FLValue returned=fl_fn_call(fn,0,NULL);
    fl_atom_reset(shared,record(3000));
    require(fl_map_get(returned,fl_str_val("number")).i==2000,"returned snapshot");
    fl_arena_end();
    puts("RETURNED_ARENA_SNAPSHOT=PASS");
    return 0;
}
