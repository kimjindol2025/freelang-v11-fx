#ifndef FREELANG_TCP_REQUEST_FRAME_H
#define FREELANG_TCP_REQUEST_FRAME_H
#include <stddef.h>
#include <stdint.h>
#define FL_TCP_FRAME_SIZE 28u
#define FL_TCP_FRAME_MAGIC "FLTCP01\0"
static inline void fl_tcp_frame_u32(unsigned char *p, uint32_t v) {
    for (int i = 3; i >= 0; --i) { p[i] = (unsigned char)v; v >>= 8; }
}
static inline uint32_t fl_tcp_frame_read_u32(const unsigned char *p) {
    uint32_t v = 0;
    for (int i = 0; i < 4; ++i) v = (v << 8) | p[i];
    return v;
}
static inline void fl_tcp_frame_u64(unsigned char *p, uint64_t v) {
    for (int i = 7; i >= 0; --i) { p[i] = (unsigned char)v; v >>= 8; }
}
static inline uint64_t fl_tcp_frame_read_u64(const unsigned char *p) {
    uint64_t v = 0;
    for (int i = 0; i < 8; ++i) v = (v << 8) | p[i];
    return v;
}
#endif
