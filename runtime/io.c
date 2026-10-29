#include "runtime.h"
#include "internal.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>

FLValue fl_bit_xor(FLValue a, FLValue b) { return fl_int(a.i ^ b.i); }
FLValue fl_bit_and(FLValue a, FLValue b) { return fl_int(a.i & b.i); }
FLValue fl_bit_or(FLValue a, FLValue b)  { return fl_int(a.i | b.i); }
FLValue fl_bit_shl(FLValue a, FLValue b) { return fl_int(a.i << b.i); }
FLValue fl_bit_shr(FLValue a, FLValue b) { return fl_int(a.i >> b.i); }

/* ── _fl_file_* — all.fl stdlib 래퍼용 ── */
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>

const char* _strval(FLValue v) {
    if (v.tag == FL_STRING && v.obj) return ((FLString*)v.obj)->data;
    return "";
}

typedef struct { const char* code; int number; size_t bytes; } FileError;

static void file_error(FileError* e, const char* code, int number) {
    if (!e->code) { e->code = code; e->number = number ? number : EIO; }
}

static FLValue file_result(FileError* e, FLValue data) {
    FLValue result = fl_map_new();
    result = fl_map_set(result, fl_str_val("ok"), fl_bool(!e->code));
    result = fl_map_set(result, fl_str_val("bytes"), fl_int((int64_t)e->bytes));
    result = fl_map_set(result, fl_str_val("error"), fl_str_val(e->code ? e->code : ""));
    result = fl_map_set(result, fl_str_val("errno"), fl_int(e->code ? e->number : 0));
    return fl_map_set(result, fl_str_val("data"), data);
}

static int file_open(FLValue path, int flags, FileError* e) {
    if (path.tag != FL_STRING || !path.obj) {
        file_error(e, "INVALID_PATH", EINVAL); return -1;
    }
    FLString* value = (FLString*)path.obj;
    if (!value->len || value->len >= PATH_MAX || memchr(value->data, 0, value->len)) {
        file_error(e, "INVALID_PATH", EINVAL); return -1;
    }
    const char* base = getenv("FL_FILE_BASE");
    int fd;
    if (!base || !*base) {
        fd = open(value->data, flags | O_CLOEXEC, 0666);
    } else {
        char root[PATH_MAX], relative[PATH_MAX];
        if (!realpath(base, root)) { file_error(e, "OPEN_FAILED", errno); return -1; }
        const char* name = value->data;
        if (*name == '/') {
            size_t n = strlen(root);
            if (n == 1 && root[0] == '/') name++;
            else if (!strncmp(name, root, n) && name[n] == '/') name += n + 1;
            else { file_error(e, "PATH_OUTSIDE_BASE", EACCES); return -1; }
        }
        strcpy(relative, name);
        /* Reject traversal before creating anything; walk by fd to prevent symlink races. */
        char validation[PATH_MAX]; strcpy(validation, relative);
        char* save = NULL;
        for (char* part = strtok_r(validation, "/", &save); part; part = strtok_r(NULL, "/", &save)) {
            if (!strcmp(part, "..")) { file_error(e, "PATH_OUTSIDE_BASE", EACCES); return -1; }
        }
        int directory = open(root, O_RDONLY | O_DIRECTORY | O_CLOEXEC | O_NOFOLLOW);
        if (directory < 0) { file_error(e, "OPEN_FAILED", errno); return -1; }
        save = NULL;
        char* part = strtok_r(relative, "/", &save);
        if (!part) { close(directory); file_error(e, "INVALID_PATH", EINVAL); return -1; }
        for (;;) {
            char* next = strtok_r(NULL, "/", &save);
            if (!next) {
                fd = openat(directory, part, flags | O_CLOEXEC | O_NOFOLLOW, 0666);
                int number = errno; close(directory); errno = number;
                break;
            }
            int child = openat(directory, part, O_RDONLY | O_DIRECTORY | O_CLOEXEC | O_NOFOLLOW);
            int number = errno; close(directory);
            if (child < 0) { file_error(e, "OPEN_FAILED", number); return -1; }
            directory = child; part = next;
        }
    }
    if (fd < 0) file_error(e, "OPEN_FAILED", errno);
    return fd;
}

static int file_stat(int fd, struct stat* st, int limit, FileError* e) {
    if (fstat(fd, st)) { file_error(e, "OPEN_FAILED", errno); return 0; }
    if (!S_ISREG(st->st_mode)) { file_error(e, "UNSUPPORTED_FILE", EINVAL); return 0; }
    if (limit && (st->st_size < 0 || (uint64_t)st->st_size > FL_FILE_IO_MAX_BYTES)) {
        file_error(e, "SIZE_LIMIT", EFBIG); return 0;
    }
    return 1;
}

static FILE* file_stream(int fd, const char* mode, FileError* e) {
    FILE* stream = fdopen(fd, mode);
    if (!stream) { int number = errno; close(fd); file_error(e, "OPEN_FAILED", number); }
    return stream;
}

static void file_close(FILE* stream, FileError* e) {
    errno = 0;
    if (fclose(stream)) file_error(e, "CLOSE_FAILED", errno);
}

static void file_write_all(FILE* stream, const char* data, size_t length, FileError* e) {
    size_t offset = 0;
    while (offset < length && !e->code) {
        errno = 0;
        size_t n = fwrite(data + offset, 1, length - offset, stream);
        offset += n; e->bytes += n;
        if (!n || ferror(stream)) file_error(e, "WRITE_FAILED", errno);
    }
}

FLValue fl_file_read_result(FLValue path) {
    FileError e = {0}; struct stat st;
    int fd = file_open(path, O_RDONLY | O_NONBLOCK, &e);
    if (fd < 0) return file_result(&e, fl_nil());
    if (!file_stat(fd, &st, 1, &e)) { close(fd); return file_result(&e, fl_nil()); }
    FILE* stream = file_stream(fd, "rb", &e);
    if (!stream) return file_result(&e, fl_nil());
    size_t length = (size_t)st.st_size;
    FLString* data = fl_arena_alloc(sizeof(FLString) + length + 1);
    if (!data) { file_error(&e, "ALLOCATION_FAILED", ENOMEM); file_close(stream, &e); return file_result(&e, fl_nil()); }
    data->base.type = FL_STRING; data->base.rc = 1; data->len = (uint32_t)length;
    while (e.bytes < length && !e.code) {
        errno = 0;
        size_t n = fread(data->data + e.bytes, 1, length - e.bytes, stream);
        e.bytes += n;
        if (!n || ferror(stream)) file_error(&e, "READ_FAILED", errno);
    }
    if (!e.code) {
        errno = 0;
        if (fgetc(stream) != EOF) file_error(&e, "FILE_CHANGED", EAGAIN);
        else if (ferror(stream)) file_error(&e, "READ_FAILED", errno);
    }
    file_close(stream, &e);
    if (e.code) return file_result(&e, fl_nil());
    data->data[length] = 0;
    FLValue value = {.tag = FL_STRING, .obj = (FLObject*)data};
    return file_result(&e, value);
}

static FLValue file_write_result(FLValue path, FLValue content, int append) {
    FileError e = {0}; struct stat st;
    if (content.tag != FL_STRING || !content.obj) {
        file_error(&e, "INVALID_DATA", EINVAL); return file_result(&e, fl_nil());
    }
    FLString* data = (FLString*)content.obj;
    if (data->len > FL_FILE_IO_MAX_BYTES) { file_error(&e, "SIZE_LIMIT", EFBIG); return file_result(&e, fl_nil()); }
    int fd = file_open(path, O_WRONLY | O_CREAT | O_NONBLOCK | (append ? O_APPEND : 0), &e);
    if (fd < 0) return file_result(&e, fl_nil());
    if (!file_stat(fd, &st, append, &e)) { close(fd); return file_result(&e, fl_nil()); }
    if (append && (uint64_t)st.st_size + data->len > FL_FILE_IO_MAX_BYTES) {
        close(fd); file_error(&e, "SIZE_LIMIT", EFBIG); return file_result(&e, fl_nil());
    }
    if (!append && ftruncate(fd, 0)) {
        int number = errno; close(fd); file_error(&e, "WRITE_FAILED", number); return file_result(&e, fl_nil());
    }
    FILE* stream = file_stream(fd, append ? "ab" : "wb", &e);
    if (!stream) return file_result(&e, fl_nil());
    file_write_all(stream, data->data, data->len, &e);
    errno = 0;
    if (fflush(stream)) file_error(&e, "WRITE_FAILED", errno);
    file_close(stream, &e);
    return file_result(&e, fl_nil());
}

FLValue fl_file_write_result(FLValue path, FLValue content) { return file_write_result(path, content, 0); }
FLValue fl_file_append_result(FLValue path, FLValue content) { return file_write_result(path, content, 1); }

/* HTTP owns an exclusively reserved inode. Reuse L07's checked byte writer
 * without reopening a pathname between reservation and writing. */
FLValue fl_file_write_reserved_result(int reserved, FLValue content) {
    FileError e = {0}; struct stat st;
    if (content.tag != FL_STRING || !content.obj) {
        file_error(&e, "INVALID_DATA", EINVAL); return file_result(&e, fl_nil());
    }
    FLString* data = (FLString*)content.obj;
    if (data->len > FL_FILE_IO_MAX_BYTES) {
        file_error(&e, "SIZE_LIMIT", EFBIG); return file_result(&e, fl_nil());
    }
    int fd = dup(reserved);
    if (fd < 0) { file_error(&e, "OPEN_FAILED", errno); return file_result(&e, fl_nil()); }
    if (!file_stat(fd, &st, 1, &e)) { close(fd); return file_result(&e, fl_nil()); }
    FILE* stream = file_stream(fd, "wb", &e);
    if (!stream) return file_result(&e, fl_nil());
    file_write_all(stream, data->data, data->len, &e);
    errno = 0;
    if (fflush(stream)) file_error(&e, "WRITE_FAILED", errno);
    file_close(stream, &e);
    return file_result(&e, fl_nil());
}

FLValue fl_file_copy_result(FLValue source, FLValue destination) {
    FileError e = {0}; struct stat src, dst;
    int in_fd = file_open(source, O_RDONLY | O_NONBLOCK, &e);
    if (in_fd < 0) return file_result(&e, fl_nil());
    if (!file_stat(in_fd, &src, 1, &e)) { close(in_fd); return file_result(&e, fl_nil()); }
    int out_fd = file_open(destination, O_WRONLY | O_CREAT | O_NONBLOCK, &e);
    if (out_fd < 0) { close(in_fd); return file_result(&e, fl_nil()); }
    if (!file_stat(out_fd, &dst, 0, &e)) { close(in_fd); close(out_fd); return file_result(&e, fl_nil()); }
    if (src.st_dev == dst.st_dev && src.st_ino == dst.st_ino) {
        close(in_fd); close(out_fd); file_error(&e, "SAME_FILE", EINVAL); return file_result(&e, fl_nil());
    }
    if (ftruncate(out_fd, 0)) {
        int number = errno; close(in_fd); close(out_fd); file_error(&e, "WRITE_FAILED", number); return file_result(&e, fl_nil());
    }
    FILE* input = file_stream(in_fd, "rb", &e);
    if (!input) { close(out_fd); return file_result(&e, fl_nil()); }
    FILE* output = file_stream(out_fd, "wb", &e);
    if (!output) { file_close(input, &e); return file_result(&e, fl_nil()); }
    char buffer[8192];
    while (!e.code) {
        errno = 0;
        size_t n = fread(buffer, 1, sizeof(buffer), input);
        if (ferror(input)) { file_error(&e, "READ_FAILED", errno); break; }
        if (!n) break;
        if (e.bytes + n > FL_FILE_IO_MAX_BYTES) file_error(&e, "SIZE_LIMIT", EFBIG);
        else file_write_all(output, buffer, n, &e);
    }
    if (!e.code && e.bytes != (size_t)src.st_size) file_error(&e, "FILE_CHANGED", EAGAIN);
    errno = 0;
    if (fflush(output)) file_error(&e, "WRITE_FAILED", errno);
    file_close(input, &e); file_close(output, &e);
    return file_result(&e, fl_nil());
}

FLValue _fl_file_append(FLValue path, FLValue content) {
    return fl_map_get(fl_file_append_result(path, content), fl_str_val("ok"));
}
FLValue file_exists(FLValue path) {
    struct stat st; return fl_bool(stat(_strval(path), &st) == 0);
}
FLValue _fl_file_delete(FLValue path) { return fl_bool(remove(_strval(path)) == 0); }
FLValue _fl_file_copy(FLValue src, FLValue dst) {
    return fl_map_get(fl_file_copy_result(src, dst), fl_str_val("ok"));
}
FLValue _fl_file_rename(FLValue old, FLValue nw) { return fl_bool(rename(_strval(old), _strval(nw)) == 0); }
FLValue _fl_file_size(FLValue path) {
    struct stat st; if (stat(_strval(path), &st) != 0) return fl_int(-1);
    return fl_int((int64_t)st.st_size);
}
FLValue _fl_file_modified(FLValue path) {
    struct stat st; if (stat(_strval(path), &st) != 0) return fl_int(0);
    return fl_int((int64_t)st.st_mtime * 1000);
}
FLValue _fl_file_mkdir(FLValue path) { return fl_bool(mkdir(_strval(path), 0755) == 0); }
FLValue _fl_file_rmdir(FLValue path) { return fl_bool(rmdir(_strval(path)) == 0); }
FLValue _fl_file_list(FLValue path) {
    DIR* d = opendir(_strval(path));
    FLValue r = fl_vec_new();
    if (!d) return r;
    struct dirent* e;
    while ((e = readdir(d))) {
        if (strcmp(e->d_name,".") && strcmp(e->d_name,".."))
            r = fl_vec_push(r, fl_str_val(e->d_name));
    }
    closedir(d); return r;
}
FLValue _fl_file_is_file(FLValue path) {
    struct stat st; if (stat(_strval(path), &st) != 0) return fl_bool(false);
    return fl_bool(S_ISREG(st.st_mode));
}
FLValue _fl_file_is_dir(FLValue path) {
    struct stat st; if (stat(_strval(path), &st) != 0) return fl_bool(false);
    return fl_bool(S_ISDIR(st.st_mode));
}

/* ── _fl_env_* ── */
#include <stdlib.h>
extern char** environ;

FLValue _fl_env_get(FLValue key) {
    const char* v = getenv(_strval(key));
    return v ? fl_str_val(v) : fl_nil();
}
FLValue _fl_env_set(FLValue key, FLValue val) {
    return fl_bool(setenv(_strval(key), _strval(val), 1) == 0);
}
FLValue _fl_env_all(void) {
    FLValue r = fl_map_new();
    if (!environ) return r;
    for (int i = 0; environ[i]; i++) {
        const char* e = environ[i];
        const char* eq = strchr(e, '=');
        if (!eq) continue;
        char key[256]; int klen = (int)(eq - e);
        if (klen >= 256) klen = 255;
        strncpy(key, e, klen); key[klen] = '\0';
        r = fl_map_set(r, fl_str_val(key), fl_str_val(eq + 1));
    }
    return r;
}

/* ── str_join ── */
FLValue str_join(FLValue sep, FLValue vec) {
    if (vec.tag != FL_VECTOR) return fl_str_val("");
    FLVector* v = (FLVector*)vec.obj;
    const char* s = (sep.tag == FL_STRING && sep.obj) ? ((FLString*)sep.obj)->data : "";
    size_t slen = strlen(s);
    /* 먼저 총 길이 계산 */
    size_t total = 0;
    char tmp[64];
    for (uint32_t i = 0; i < v->len; i++) {
        total += strlen(fl_to_str(v->data[i], tmp, sizeof(tmp)));
        if (i + 1 < v->len) total += slen;
    }
    char* buf = malloc(total + 1); buf[0] = '\0';
    for (uint32_t i = 0; i < v->len; i++) {
        strcat(buf, fl_to_str(v->data[i], tmp, sizeof(tmp)));
        if (i + 1 < v->len) strcat(buf, s);
    }
    FLValue r = fl_str_val(buf); free(buf); return r;
}

/* ── _fl_process_* ── */
#include <signal.h>
#include <sys/wait.h>

#include "v11-file.h"
#include "v11-fd.h"
