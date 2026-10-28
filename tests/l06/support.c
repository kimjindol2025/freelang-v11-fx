#define _GNU_SOURCE
#include "support.h"
#include <openssl/evp.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdatomic.h>
/* Test-only port publication after an atomic kernel port-0 bind. */
int __real_listen(int, int);
int __wrap_listen(int fd, int backlog) {
    int rc = __real_listen(fd, backlog);
    const char *path = getenv("L06_PORT_FILE");
    if (!rc && path) {
        struct sockaddr_in a; socklen_t n = sizeof a;
        if (!getsockname(fd, (struct sockaddr *)&a, &n)) {
            FILE *f = fopen(path, "w");
            if (f) { fprintf(f, "%u\n", ntohs(a.sin_port)); fclose(f); }
        }
    }
    return rc;
}
FLValue l06_digest(FLValue data) {
    if (data.tag != FL_STRING) return fl_str_val("NOT_BYTES");
    FLString *s = (FLString *)data.obj;
    unsigned char hash[32]; unsigned n = 0; char hex[65];
    EVP_Digest(s->data, s->len, hash, &n, EVP_sha256(), NULL);
    for (unsigned i=0; i<n; i++) sprintf(hex+2*i, "%02x", hash[i]);
    return fl_str_val(hex);
}
FLValue l06_seen(void) {
    static atomic_int seen;
    return fl_int(atomic_fetch_add(&seen, 1)+1);
}
