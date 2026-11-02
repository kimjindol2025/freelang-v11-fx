#include "support.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>

static int passed, failed;
static char fault[32];
static FILE* read_error;
extern size_t __real_fread(void*, size_t, size_t, FILE*);
extern size_t __real_fwrite(const void*, size_t, size_t, FILE*);
extern int __real_ferror(FILE*);
extern int __real_fflush(FILE*);
extern int __real_fclose(FILE*);
extern void* __real_malloc(size_t);
extern void* __real_fl_arena_alloc(size_t);
extern int __real_fstat(int, struct stat*);
extern int __real_ftruncate(int, off_t);
extern FILE* __real_fdopen(int, const char*);

static const char* text(FLValue v) { return v.tag == FL_STRING && v.obj ? ((FLString*)v.obj)->data : ""; }
static int consume(const char* name) { if (strcmp(fault, name)) return 0; fault[0] = 0; return 1; }
size_t __wrap_fread(void* p, size_t s, size_t n, FILE* f) {
    if (consume("read-fail")) { read_error = f; errno = EIO; return 0; }
    if (s == 1 && n > 2 && consume("read-short")) n = 2;
    return __real_fread(p, s, n, f);
}
size_t __wrap_fwrite(const void* p, size_t s, size_t n, FILE* f) {
    if (consume("write-fail")) { errno = EIO; return 0; }
    if (s == 1 && n > 2 && consume("write-short")) n = 2;
    return __real_fwrite(p, s, n, f);
}
int __wrap_ferror(FILE* f) { if (f == read_error) return 1; return __real_ferror(f); }
int __wrap_fflush(FILE* f) {
    int result = __real_fflush(f);
    if (consume("flush-fail")) { errno = EIO; return EOF; }
    return result;
}
int __wrap_fclose(FILE* f) {
    if (read_error == f) read_error = NULL;
    int result = __real_fclose(f);
    if (consume("close-fail")) { errno = EIO; return EOF; }
    return result;
}
void* __wrap_malloc(size_t n) {
    if (n == sizeof(FLString) + 4 && consume("allocation-fail")) { errno = ENOMEM; return NULL; }
    return __real_malloc(n);
}
/* The public read allocates through the arena, including cached CLI storage. */
void* __wrap_fl_arena_alloc(size_t n) {
    if (n == sizeof(FLString) + 4 && consume("allocation-fail")) { errno = ENOMEM; return NULL; }
    return __real_fl_arena_alloc(n);
}
int __wrap_fstat(int fd, struct stat* st) {
    if (consume("stat-fail")) { errno = EIO; return -1; }
    return __real_fstat(fd, st);
}
int __wrap_ftruncate(int fd, off_t n) {
    if (consume("truncate-fail")) { errno = EIO; return -1; }
    return __real_ftruncate(fd, n);
}
FILE* __wrap_fdopen(int fd, const char* mode) {
    if (consume("stream-fail")) { errno = ENOMEM; return NULL; }
    return __real_fdopen(fd, mode);
}

FLValue l07_check(FLValue condition, FLValue label) {
    int ok = fl_truthy(condition);
    printf("%s %s\n", ok ? "PASS" : "FAIL", text(label));
    if (ok) passed++; else failed++;
    return fl_bool(ok);
}
FLValue l07_error(FLValue result, FLValue error, FLValue label) {
    FLValue ok = fl_map_get(result, fl_str_val("ok"));
    FLValue code = fl_map_get(result, fl_str_val("error"));
    FLValue number = fl_map_get(result, fl_str_val("errno"));
    return l07_check(fl_bool(ok.tag == FL_BOOL && !ok.b && !strcmp(text(code), text(error))
        && number.tag == FL_INT && number.i > 0), label);
}
static int raw_open(FLValue name) {
    char path[8192]; snprintf(path, sizeof(path), "%s/%s", getenv("FL_FILE_BASE"), text(name));
    return open(path, O_RDONLY | O_CLOEXEC);
}
static int compare_files(FLValue source, FLValue destination, int prefix) {
    int a = raw_open(source), b = raw_open(destination), ok = a >= 0 && b >= 0;
    if (ok && prefix) { char c; ok = read(b, &c, 1) == 1 && c == 'P'; }
    unsigned char x[8192], y[8192];
    while (ok) {
        ssize_t n = read(a, x, sizeof(x)), m = read(b, y, sizeof(y));
        if (n < 0 || n != m || (n > 0 && memcmp(x, y, n))) { ok = 0; break; }
        if (!n) break;
    }
    if (a >= 0) close(a); if (b >= 0) close(b);
    return ok;
}
FLValue l07_same(FLValue a, FLValue b) { return fl_bool(compare_files(a, b, 0)); }
FLValue l07_append_matches(FLValue a, FLValue b) { return fl_bool(compare_files(a, b, 1)); }
FLValue l07_raw_size(FLValue path) {
    int fd = raw_open(path); struct stat st;
    int ok = fd >= 0 && __real_fstat(fd, &st) == 0;
    if (fd >= 0) close(fd); return fl_int(ok ? st.st_size : -1);
}
FLValue l07_fd_count(void) {
    DIR* d = opendir("/proc/self/fd"); if (!d) return fl_int(-1);
    int n = 0; struct dirent* entry;
    while ((entry = readdir(d))) if (strcmp(entry->d_name, ".") && strcmp(entry->d_name, "..")) n++;
    closedir(d); return fl_int(n);
}
FLValue l07_arm(FLValue name) {
    if (fault[0]) { printf("FAIL unconsumed-fault-%s\n", fault); failed++; }
    snprintf(fault, sizeof(fault), "%s", text(name)); read_error = NULL; return fl_nil();
}
FLValue l07_bad_value(FLValue kind) {
    const char* k = text(kind);
    if (!strcmp(k, "oversize")) {
        FLString* s = __real_malloc(sizeof(FLString) + 1);
        s->base.type = FL_STRING; s->base.rc = 1; s->len = 64u * 1024u * 1024u + 1; s->data[0] = 0;
        return (FLValue){.tag = FL_STRING, .obj = (FLObject*)s};
    }
    char buffer[8192]; size_t n;
    if (!strcmp(k, "nul-path")) { memcpy(buffer, "sentinel\0suffix", 15); n = 15; }
    else if (!strcmp(k, "long-path")) { memset(buffer, 'x', 4096); n = 4096; }
    else if (!strcmp(k, "absolute")) { snprintf(buffer, sizeof(buffer), "%s/text", getenv("FL_FILE_BASE")); n = strlen(buffer); }
    else if (!strcmp(k, "outside-direct")) {
        snprintf(buffer, sizeof(buffer), "%s", getenv("FL_FILE_BASE"));
        char* slash = strrchr(buffer, '/'); if (!slash) abort();
        strcpy(slash + 1, "outside"); n = strlen(buffer);
    }
    else { snprintf(buffer, sizeof(buffer), "%s/../outside", getenv("FL_FILE_BASE")); n = strlen(buffer); }
    FLString* s = __real_malloc(sizeof(FLString) + n + 1);
    s->base.type = FL_STRING; s->base.rc = 1; s->len = (uint32_t)n;
    memcpy(s->data, buffer, n); s->data[n] = 0;
    return (FLValue){.tag = FL_STRING, .obj = (FLObject*)s};
}
FLValue l07_finish(void) {
    if (fault[0]) { printf("FAIL unconsumed-fault-%s\n", fault); failed++; }
    printf("L07_SELF_PASS=%d\nL07_SELF_FAIL=%d\n", passed, failed);
    if (failed) exit(1); return fl_nil();
}
