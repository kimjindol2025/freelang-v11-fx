/* The same key contracts must hold above the tiny-map path. */
#define main b17_small_main
#include "b17-runtime-safety.c"
#undef main
int main(void){
 fl_arena_begin();
 FLValue p[]={fl_int(1),fl_int(10),fl_str_val("1"),fl_int(20),bytes("a\0x",3),fl_int(30),bytes("a\0y",3),fl_int(40),bytes("a\0x",3),fl_int(50),fl_str_val("한🙂"),fl_int(60),fl_str_val("x"),fl_int(70),fl_str_val("y"),fl_int(80),fl_str_val(""),fl_int(90),fl_bool(true),fl_int(100)};
 FLValue map=fl_v11_map_from_pairs(p,10);FLMap *m=(FLMap*)map.obj;
 CHECK(m->len==8,"INDEX_NORMALIZED_COUNT");
 CHECK(fl_map_get(map,fl_v11_key(fl_str_val("1"))).i==20,"INDEX_LAST_VALUE");
 CHECK(fl_map_get(map,fl_v11_key(bytes("a\0x",3))).i==50&&fl_map_get(map,fl_v11_key(bytes("a\0y",3))).i==40,"INDEX_BINARY_KEYS");
 CHECK(m->entries[0].val.i==20&&m->entries[1].val.i==50&&m->entries[2].val.i==40&&m->entries[3].val.i==60,"INDEX_FIRST_KEY_ORDER");
 CHECK(fl_map_get(map,fl_v11_key(fl_str_val(""))).i==90&&fl_map_get(map,fl_v11_key(fl_bool(true))).i==100,"INDEX_EMPTY_AND_BOOLEAN");
 fl_arena_end();return 0;
}
