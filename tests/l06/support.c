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
static atomic_int seen;
FLValue l06_seen(void) {
    return fl_int(atomic_fetch_add(&seen, 1)+1);
}
FLValue l06_count(void) { return fl_int(atomic_load(&seen)); }
FLValue l06_hold(void) {
    const char *path=getenv("L06_HOLD_FILE");
    if (path) { FILE *f=fopen(path,"w"); if (f) { fputs("READY\n",f); fclose(f); } }
    sleep(20);
    return fl_nil();
}
#include <sys/stat.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/random.h>
static __thread int fault;
static void arm_fault(const char *p,size_t n) {
    static const char *names[]={"WRITE","SHORT","FLUSH","CLOSE","ALLOC","UNLINK","RANDOM","WRITE_SECOND"};
    for (int i=0;i<8;i++) {
        char needle[64]; snprintf(needle,sizeof needle,"X-L06-Fault: %s\r\n",names[i]);
        if (memmem(p,n,needle,strlen(needle))) fault=i+1;
    }
}
ssize_t __real_recv(int,void*,size_t,int);
ssize_t __wrap_recv(int fd,void *p,size_t n,int flags) {
    ssize_t k=__real_recv(fd,p,n,flags); if (k>0) arm_fault(p,(size_t)k); return k;
}
int __real_SSL_read(void*,void*,int);
int __wrap_SSL_read(void *ssl,void *p,int n) {
    int k=__real_SSL_read(ssl,p,n); if (k>0) arm_fault(p,(size_t)k); return k;
}
size_t __real_fwrite(const void*,size_t,size_t,FILE*);
size_t __wrap_fwrite(const void *p,size_t a,size_t b,FILE *f) {
    if (fault==8 && a*b) { fault=1; return __real_fwrite(p,a,b,f); }
    if (fault==1 && a*b) { fault=0; errno=EIO; return 0; }
    if (fault==2 && a==1 && b>1) { fault=0; return __real_fwrite(p,a,b/2,f); }
    return __real_fwrite(p,a,b,f);
}
int __real_fflush(FILE*);
int __wrap_fflush(FILE *f) {
    if (fault==3) { fault=0; __real_fflush(f); errno=EIO; return EOF; }
    return __real_fflush(f);
}
int __real_fclose(FILE*);
int __wrap_fclose(FILE *f) {
    int rc=__real_fclose(f);
    if (fault==4) { fault=0; errno=EIO; return EOF; }
    return rc;
}
void *__real_fl_arena_alloc(size_t);
void *__wrap_fl_arena_alloc(size_t n) {
    if (fault==5 && n>4096) { fault=0; return NULL; }
    return __real_fl_arena_alloc(n);
}
int __real_unlinkat(int,const char*,int);
int __wrap_unlinkat(int fd,const char *name,int flags) {
    if (fault==6) { fault=0; errno=EIO; return -1; }
    return __real_unlinkat(fd,name,flags);
}
ssize_t __real_getrandom(void*,size_t,unsigned);
ssize_t __wrap_getrandom(void *p,size_t n,unsigned flags) {
    if (fault==7) { fault=0; memset(p,0,n); return (ssize_t)n; }
    return __real_getrandom(p,n,flags);
}
FLValue l06_meta(FLValue meta) {
    FLValue n=fl_map_get(meta,fl_str_val("filename")), p=fl_map_get(meta,fl_str_val("path"));
    if (n.tag!=FL_STRING || p.tag!=FL_STRING) return fl_bool(0);
    const char *name=((FLString*)n.obj)->data, *path=((FLString*)p.obj)->data;
    if (strlen(name)!=16) return fl_bool(0);
    for (int i=0;i<16;i++) if (!strchr("0123456789abcdef",name[i])) return fl_bool(0);
    char expected[4096]; snprintf(expected,sizeof expected,"%s/tmp/fl-uploads/%s",getenv("FL_FILE_BASE"),name);
    struct stat st;
    return fl_bool(!strcmp(path,expected) && !lstat(path,&st) && S_ISREG(st.st_mode) && (st.st_mode&0777)==0600);
}
FLValue l06_pattern(FLValue data) {
    if (data.tag!=FL_STRING) return fl_bool(0);
    FLString *s=(FLString *)data.obj;
    for (uint32_t i=0;i<s->len;i++) if ((unsigned char)s->data[i]!=(unsigned char)i) return fl_bool(0);
    return fl_bool(1);
}
FLValue l06_sse_probe(FLValue conn) {
    l06_seen();
    sse_send(conn,fl_str_val("SSE_PROBE"));
    return fl_nil();
}
