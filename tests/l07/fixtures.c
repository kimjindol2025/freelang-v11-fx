#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

static void fixture(const char* root, const char* name, size_t n, int kind) {
    char path[4096]; snprintf(path, sizeof(path), "%s/%s", root, name);
    FILE* f = fopen(path, "wb"); if (!f) exit(2);
    for (size_t i = 0; i < n; i++) {
        unsigned char c = (unsigned char)(i * 37 + 17);
        if (kind == 1) c = (unsigned char)"abc"[i];
        if (kind == 2) c = (unsigned char[]){0, 'A', 'B'}[i];
        if (kind == 3) c = (unsigned char[]){'A', 0, 'B'}[i];
        if (kind == 4) c = (unsigned char[]){'A', 'B', 0}[i];
        if (kind == 5) c = (unsigned char)i;
        if (kind == 6) c = (unsigned char[]){0xff, 0xfe, 0xc0, 0x80}[i];
        if (fputc(c, f) == EOF) exit(2);
    }
    if (fclose(f)) exit(2);
}
int main(int argc, char** argv) {
    if (argc != 2) return 2;
    const char* root = argv[1];
    fixture(root, "empty", 0, 0); fixture(root, "text", 3, 1);
    fixture(root, "nul-first", 3, 2); fixture(root, "nul-middle", 3, 3);
    fixture(root, "nul-last", 3, 4); fixture(root, "all-bytes", 256, 5);
    fixture(root, "non-utf8", 4, 6); fixture(root, "boundary-8191", 8191, 0);
    fixture(root, "boundary-8192", 8192, 0); fixture(root, "boundary-8193", 8193, 0);
    fixture(root, "multi-block", 20000, 0); fixture(root, "sentinel", 3, 1);
    char path[4096], other[4096];
    snprintf(path, sizeof(path), "%s/readonly", root); fixture(root, "readonly", 3, 1); chmod(path, 0400);
    snprintf(path, sizeof(path), "%s/text", root);
    snprintf(other, sizeof(other), "%s/hardlink", root); if (link(path, other)) return 2;
    snprintf(other, sizeof(other), "%s/symlink", root); if (symlink("../outside", other)) return 2;
    snprintf(other, sizeof(other), "%s/symlink-dir", root); if (symlink("..", other)) return 2;
    snprintf(path, sizeof(path), "%s/fifo", root); if (mkfifo(path, 0600)) return 2;
    const char* names[] = {"limit", "too-large"};
    for (int i = 0; i < 2; i++) {
        snprintf(path, sizeof(path), "%s/%s", root, names[i]);
        int fd = open(path, O_WRONLY | O_CREAT | O_TRUNC, 0600);
        if (fd < 0 || ftruncate(fd, 64u * 1024u * 1024u + i) || close(fd)) return 2;
    }
    snprintf(path, sizeof(path), "%s/../outside", root);
    FILE* f = fopen(path, "wb"); if (!f) return 2;
    fputs("outside-sentinel", f); return fclose(f) ? 2 : 0;
}
