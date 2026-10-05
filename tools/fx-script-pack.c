/* Deterministic, compile-time-only FreeLang Script module packer. */
#define _XOPEN_SOURCE 700
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <openssl/sha.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

typedef struct { char *s; size_t n, cap; } Buf;
typedef struct { char *path, *body; char hash[65]; size_t order; int state; } Module;
typedef struct { Module *v; size_t n, cap, completed; } Modules;
typedef struct { size_t start, end; char *request; } Import;
typedef struct { Import *v; size_t n, cap; } Imports;

static void die(const char *f, ...) { va_list a; fputs("fx-script-pack: ", stderr); va_start(a, f); vfprintf(stderr, f, a); va_end(a); fputc(10, stderr); exit(2); }
static void *mem(size_t n) { void *p = malloc(n ? n : 1); if (!p) die("out of memory"); return p; }
static void *grow(void *p, size_t n) { void *q = realloc(p, n ? n : 1); if (!q) die("out of memory"); return q; }
static char *dup_n(const char *s, size_t n) { char *p = mem(n + 1); memcpy(p, s, n); p[n] = 0; return p; }
static char *dup_s(const char *s) { return dup_n(s, strlen(s)); }
static void put_n(Buf *b, const char *s, size_t n) { if (b->n + n + 1 > b->cap) { b->cap = b->cap ? b->cap * 2 : 1024; while (b->n + n + 1 > b->cap) b->cap *= 2; b->s = grow(b->s, b->cap); } memcpy(b->s + b->n, s, n); b->n += n; b->s[b->n] = 0; }
static void put(Buf *b, const char *s) { put_n(b, s, strlen(s)); }
static void put_f(Buf *b, const char *f, ...) { va_list a, c; va_start(a, f); va_copy(c, a); int n = vsnprintf(NULL, 0, f, c); va_end(c); if (n < 0) die("format error"); if (b->n + (size_t)n + 1 > b->cap) { b->cap = b->cap ? b->cap * 2 : 1024; while (b->n + (size_t)n + 1 > b->cap) b->cap *= 2; b->s = grow(b->s, b->cap); } vsnprintf(b->s + b->n, b->cap - b->n, f, a); va_end(a); b->n += (size_t)n; }

static char *read_file(const char *path, size_t *n) { FILE *f = fopen(path, "rb"); if (!f) die("E_IMPORT_NOT_FOUND cannot read %s: %s", path, strerror(errno)); if (fseek(f, 0, SEEK_END)) die("cannot seek %s", path); long z = ftell(f); if (z < 0) die("cannot size %s", path); rewind(f); char *s = mem((size_t)z + 1); if ((size_t)z && fread(s, 1, (size_t)z, f) != (size_t)z) die("cannot read %s", path); fclose(f); s[z] = 0; *n = (size_t)z; return s; }
static void write_file(const char *path, const char *s, size_t n) { FILE *f = fopen(path, "wb"); if (!f) die("cannot write %s: %s", path, strerror(errno)); if (n && fwrite(s, 1, n, f) != n) die("cannot write %s", path); if (fclose(f)) die("cannot close %s", path); }
static void hash(const char *s, size_t n, char out[65]) { unsigned char d[SHA256_DIGEST_LENGTH]; SHA256((const unsigned char *)s, n, d); for (size_t i = 0; i < sizeof d; i++) sprintf(out + 2 * i, "%02x", d[i]); out[64] = 0; }
static bool in_root(const char *root, const char *p) { size_t n = strlen(root); return !strncmp(root, p, n) && (p[n] == '/' || p[n] == 0); }
static bool has_suffix(const char *s, const char *x) { size_t a = strlen(s), b = strlen(x); return a >= b && !strcmp(s + a - b, x); }
static size_t skip_ws(const char *s, size_t n, size_t i) { while (i < n) { if (isspace((unsigned char)s[i])) { i++; continue; } if (s[i] == ';') { while (i < n && s[i] != 10) i++; continue; } break; } return i; }
static size_t scan_str(const char *s, size_t n, size_t i) { for (i++; i < n;) { if (s[i] == '\\' && i + 1 < n) { i += 2; continue; } if (s[i++] == '"') return i; } die("E_IMPORT_SYNTAX unterminated string"); return n; }
static bool word(const char *s, size_t n, size_t i, const char *w) { size_t z = strlen(w); return i + z <= n && !memcmp(s + i, w, z) && (i + z == n || isspace((unsigned char)s[i + z]) || s[i + z] == ')' || s[i + z] == '"'); }
static void import_push(Imports *x, Import v) { if (x->n == x->cap) { x->cap = x->cap ? 2 * x->cap : 8; x->v = grow(x->v, x->cap * sizeof *x->v); } x->v[x->n++] = v; }

/* Import is intentionally constrained to a top-level (import "./x.fls") form. */
static Imports imports_of(const char *s, size_t n) {
  Imports r = {0}; size_t i = 0; int depth = 0;
  while (i < n) {
    if (s[i] == ';') { while (i < n && s[i] != 10) i++; continue; }
    if (s[i] == '"') { i = scan_str(s, n, i); continue; }
    if (s[i] != '(') { if (s[i] == ')') depth--; i++; continue; }
    size_t start = i, p = skip_ws(s, n, i + 1);
    if (!word(s, n, p, "import")) { depth++; i++; continue; }
    if (depth) die("E_IMPORT_SCOPE import is only allowed at top level");
    p = skip_ws(s, n, p + 6); if (p == n || s[p] != '"') die("E_IMPORT_SYNTAX import requires one quoted relative path");
    size_t vstart = ++p; while (p < n && s[p] != '"') { if (s[p] == '\\') die("E_IMPORT_SYNTAX escaped paths are not supported"); p++; }
    if (p == n) die("E_IMPORT_SYNTAX unterminated import path");
    char *req = dup_n(s + vstart, p - vstart);
    p = skip_ws(s, n, p + 1);
    if (p == n || s[p] != ')') die("E_IMPORT_SYNTAX import accepts only one path in P1-B");
    import_push(&r, (Import){start, p + 1, req}); i = p + 1;
  }
  return r;
}
static Module *module_of(Modules *m, const char *path) { for (size_t i = 0; i < m->n; i++) if (!strcmp(m->v[i].path, path)) return &m->v[i]; if (m->n == m->cap) { m->cap = m->cap ? 2 * m->cap : 8; m->v = grow(m->v, m->cap * sizeof *m->v); } Module *x = &m->v[m->n++]; memset(x, 0, sizeof *x); x->path = dup_s(path); return x; }
static char *resolve(const char *root, const char *from, const char *req) {
  if (strncmp(req, "./", 2) || strstr(req, "..")) die("E_IMPORT_ESCAPE import path must begin with ./ and may not contain ..: %s", req);
  const char *slash = strrchr(from, '/'); char candidate[PATH_MAX], real[PATH_MAX];
  int z = snprintf(candidate, sizeof candidate, "%.*s/%s%s", (int)(slash - from), from, req, has_suffix(req, ".fls") ? "" : ".fls");
  if (z < 0 || (size_t)z >= sizeof candidate || !realpath(candidate, real)) die("E_IMPORT_NOT_FOUND import %s from %s", req, from);
  struct stat st; if (!in_root(root, real)) die("E_IMPORT_ESCAPE resolved module escapes project root: %s", req); if (stat(real, &st) || !S_ISREG(st.st_mode)) die("E_IMPORT_NOT_FOUND not a regular module: %s", req); return dup_s(real);
}
static void pack(Modules *mods, const char *root, Module *m) {
  size_t self = (size_t)(m - mods->v);
  if (m->state == 2) return;
  if (m->state == 1) die("E_CIRCULAR_IMPORT circular import reaches %s", m->path);
  m->state = 1;
  size_t n; char *src = read_file(m->path, &n); hash(src, n, m->hash); Imports im = imports_of(src, n); Buf body = {0}; size_t at = 0;
  for (size_t i = 0; i < im.n; i++) {
    m = &mods->v[self];
    char *path = resolve(root, m->path, im.v[i].request); Module *child = module_of(mods, path); free(path); pack(mods, root, child);
    put_n(&body, src + at, im.v[i].start - at); for (size_t k = im.v[i].start; k < im.v[i].end; k++) if (src[k] == 10) put(&body, "\\n"); at = im.v[i].end;
  }
  m = &mods->v[self]; put_n(&body, src + at, n - at); m->body = body.s; m->state = 2; m->order = mods->completed++;
  for (size_t i = 0; i < im.n; i++) free(im.v[i].request);
  free(im.v);
  free(src);
}
static const char *rel(const char *root, const char *p) { size_t n = strlen(root); return p + n + (p[n] == '/'); }
static void jstr(Buf *b, const char *s) { put(b, "\""); for (; *s; s++) { if (*s == '\\' || *s == '"') put_f(b, "\\%c", *s); else if ((unsigned char)*s < 32) put_f(b, "\\u%04x", (unsigned char)*s); else put_n(b, s, 1); } put(b, "\""); }
static void lock_write(const char *path, const char *root, const char *entry, Modules *m) { Buf b = {0}; put(&b, "{\n  \"schema\": \"freelang-script/fx-lock/2\",\n  \"compiler\": \"fx-script-pack/1\",\n  \"entry\": "); jstr(&b, rel(root, entry)); put_f(&b, ",\n  \"moduleCount\": %zu,\n  \"modules\": [\n", m->n); for (size_t i = 0; i < m->n; i++) { put(&b, "    { \"path\": "); jstr(&b, rel(root, m->v[i].path)); put_f(&b, ", \"sha256\": \"%s\" }%s\n", m->v[i].hash, i + 1 == m->n ? "" : ","); } put(&b, "  ]\n}\n"); write_file(path, b.s, b.n); free(b.s); }
static void lock_check(const char *path, const char *root, const char *entry, Modules *m) { if (access(path, R_OK) != 0) die("E_LOCKFILE_MISMATCH lockfile not found: %s", path); size_t n; char *s = read_file(path, &n); (void)n; if (!strstr(s, "\"schema\": \"freelang-script/fx-lock/2\"")) die("E_LOCKFILE_MISMATCH unsupported or missing lockfile schema"); char p[PATH_MAX + 160]; snprintf(p, sizeof p, "\"entry\": \"%s\"", rel(root, entry)); if (!strstr(s, p)) die("E_LOCKFILE_MISMATCH entry differs from lockfile"); snprintf(p, sizeof p, "\"moduleCount\": %zu", m->n); if (!strstr(s, p)) die("E_LOCKFILE_MISMATCH module graph differs from lockfile"); for (size_t i = 0; i < m->n; i++) { snprintf(p, sizeof p, "\"path\": \"%s\", \"sha256\": \"%s\"", rel(root, m->v[i].path), m->v[i].hash); if (!strstr(s, p)) die("E_LOCKFILE_MISMATCH changed or missing module: %s", rel(root, m->v[i].path)); } free(s); }
static void free_mods(Modules *m) { for (size_t i = 0; i < m->n; i++) { free(m->v[i].path); free(m->v[i].body); } free(m->v); }

int main(int argc, char **argv) {
  bool update = false, verify = false; const char *root_arg = NULL, *lock = NULL, *entry_arg = NULL, *out = NULL;
  for (int i = 1; i < argc; i++) { if (!strcmp(argv[i], "--root") && i + 1 < argc) root_arg = argv[++i]; else if (!strcmp(argv[i], "--lock") && i + 1 < argc) lock = argv[++i]; else if (!strcmp(argv[i], "--update-lock")) update = true; else if (!strcmp(argv[i], "--verify-lock")) verify = true; else if (!entry_arg) entry_arg = argv[i]; else if (!out) out = argv[i]; else die("usage: fx-script-pack --root ROOT --lock LOCK (--update-lock|--verify-lock) ENTRY OUTPUT"); }
  if (!root_arg || !lock || !entry_arg || !out || update == verify) die("usage: fx-script-pack --root ROOT --lock LOCK (--update-lock|--verify-lock) ENTRY OUTPUT");
  char root[PATH_MAX], entry[PATH_MAX]; if (!realpath(root_arg, root)) die("invalid project root: %s", root_arg); if (!realpath(entry_arg, entry)) die("invalid entry: %s", entry_arg); if (!in_root(root, entry)) die("E_IMPORT_ESCAPE entry escapes project root");
  Modules mods = {0}; Module *main = module_of(&mods, entry); pack(&mods, root, main); if (update) lock_write(lock, root, entry, &mods); else lock_check(lock, root, entry, &mods);
  Buf packed = {0};
  for (size_t order = 0; order < mods.n; order++) for (size_t i = 0; i < mods.n; i++) if (mods.v[i].order == order) {
    put_f(&packed, "\n;; fx-module: %s\n", rel(root, mods.v[i].path)); put(&packed, mods.v[i].body); put(&packed, "\n");
  }
  write_file(out, packed.s, packed.n); fprintf(stdout, "fx-script-pack: modules=%zu lock=%s mode=%s\n", mods.n, lock, update ? "update" : "verify"); free(packed.s); free_mods(&mods); return 0;
}
