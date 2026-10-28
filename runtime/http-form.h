/* Internal HTTP form implementation; included once by http.c. */
#include <ctype.h>
#include <fcntl.h>
#include <limits.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <sys/random.h>
#include <poll.h>

#define FORM_FILE_LIMIT (15u * 1024 * 1024)
#define FORM_FIELD_LIMIT (64u * 1024)
#define FORM_FIELDS_LIMIT (256u * 1024)

static __thread SSL *http_ssl;
static __thread const char *http_error;
static int form_fail(int status, const char *code) { http_error = code; return -status; }
static ssize_t transport_read(int fd, void *p, size_t n) {
    return http_ssl ? SSL_read(http_ssl, p, (int)n) : recv(fd, p, n, 0);
}
static int transport_send(int fd, const void *p, size_t n) {
    const char *s = p;
    while (n) {
        int k = http_ssl ? SSL_write(http_ssl, s, (int)n) : (int)send(fd, s, n, MSG_NOSIGNAL);
        if (k < 0 && errno == EINTR) continue;
        if (k <= 0) return 0;
        s += k; n -= k;
    }
    return 1;
}
static double monotonic_seconds(void) {
    struct timespec t; clock_gettime(CLOCK_MONOTONIC, &t);
    return t.tv_sec + t.tv_nsec / 1e9;
}
static FLValue form_bytes(const char *p, size_t n) {
    FLString *s = fl_arena_alloc(sizeof *s + n + 1);
    if (!s) return fl_nil();
    s->base.type = FL_STRING; s->base.rc = 1; s->len = (uint32_t)n;
    memcpy(s->data, p, n); s->data[n] = 0;
    return (FLValue){.tag=FL_STRING, .obj=(FLObject *)s};
}
static int form_utf8(const unsigned char *p, size_t n) {
    size_t i=0;
    while (i<n) {
        unsigned c=p[i++], v; int k;
        if (!c) return 0;
        if (c<128) continue;
        if (c>=0xc2 && c<=0xdf) { v=c&31; k=1; }
        else if (c>=0xe0 && c<=0xef) { v=c&15; k=2; }
        else if (c>=0xf0 && c<=0xf4) { v=c&7; k=3; }
        else return 0;
        int width=k;
        if (i+(size_t)k>n) return 0;
        while (k--) { unsigned b=p[i++]; if ((b&0xc0)!=0x80) return 0; v=(v<<6)|(b&63); }
        if ((width==2 && v<0x800) || (width==3 && v<0x10000) ||
            v>0x10ffff || (v>=0xd800 && v<=0xdfff)) return 0;
    }
    return 1;
}
static int token_char(unsigned char c) {
    return isalnum(c) || strchr("!#$%&'*+-.^_`|~", c)!=NULL;
}
/* Strict parameter grammar, including quoted values. No substring matching. */
static int form_parameters(const char *s, char *type, size_t cap, FLValue *params) {
    *params=fl_map_new(); size_t n=0;
    while (*s && *s!=';' && *s!=' ' && *s!='\t') {
        if ((unsigned char)*s<=32 || (unsigned char)*s>=127 || n+1>=cap) return 0;
        type[n++]=(char)tolower((unsigned char)*s++);
    }
    type[n]=0; if (!n) return 0;
    while (*s) {
        while (*s==' ' || *s=='\t') s++;
        if (!*s) break;
        if (*s++!=';') return 0;
        while (*s==' ' || *s=='\t') s++;
        char key[128], value[8193]; n=0;
        while (*s && token_char((unsigned char)*s)) {
            if (n+1>=sizeof key) return 0;
            key[n++]=(char)tolower((unsigned char)*s++);
        }
        key[n]=0; if (!n) return 0;
        while (*s==' ' || *s=='\t') s++;
        if (*s++!='=') return 0;
        while (*s==' ' || *s=='\t') s++;
        n=0;
        if (*s=='"') {
            s++;
            while (*s && *s!='"') {
                if (*s=='\\') { s++; if (!*s) return 0; }
                unsigned char c=*s++;
                if (c<32 || c==127 || n+1>=sizeof value) return 0;
                value[n++]=(char)c;
            }
            if (*s++!='"') return 0;
        } else {
            while (*s && token_char((unsigned char)*s)) {
                if (n+1>=sizeof value) return 0;
                value[n++]=*s++;
            }
            if (!n) return 0;
        }
        value[n]=0;
        FLValue k=fl_str_val(key);
        if (fl_map_get(*params,k).tag!=FL_NIL) return 0;
        *params=fl_map_set(*params,k,fl_str_val(value));
    }
    return 1;
}
static int form_type(const char *ct) {
    size_t n=strcspn(ct,"; \t");
    if (n==33 && !strncasecmp(ct,"application/x-www-form-urlencoded",n)) return 1;
    if (n==19 && !strncasecmp(ct,"multipart/form-data",n)) return 2;
    return 0;
}

/* Header parsing uses only bounded, validated header bytes. */
static int form_headers(char *raw, size_t len, HttpRequest *hr, size_t *offset) {
    memset(hr,0,sizeof *hr);
    char *end=memmem(raw,len,"\r\n\r\n",4);
    if (!end) return form_fail(400,"BAD_PART_HEADER");
    *offset=(size_t)(end-raw)+4;
    if (*offset>MAX_HEADER_TOTAL) return form_fail(431,"HEADER_LIMIT");
    if (memchr(raw,0,*offset)) return form_fail(400,"BAD_PART_HEADER");
    char *e=memmem(raw,(size_t)(end-raw)+2,"\r\n",2);
    if (!e) return form_fail(400,"BAD_PART_HEADER");
    char line[MAX_URI_LEN+32]; size_t n=(size_t)(e-raw);
    if (n>=sizeof line) return form_fail(414,"URI_LIMIT");
    memcpy(line,raw,n); line[n]=0;
    char method[8], uri[1024], version[16], extra;
    if (sscanf(line,"%7s %1023s %15s %c",method,uri,version,&extra)!=3 ||
        (strcmp(version,"HTTP/1.1") && strcmp(version,"HTTP/1.0"))) return form_fail(400,"BAD_PART_HEADER");
    strcpy(hr->method,method); char *q=strchr(uri,'?');
    if (q) { *q++=0; strcpy(hr->query_str,q); }
    http_url_decode_internal(uri,hr->path,sizeof hr->path);
    char *p=e+2; int has_length=0, te=0, ce=0, ct=0, too_large=0;
    while (p<end) {
        e=memmem(p,(size_t)(end-p)+2,"\r\n",2);
        if (!e || *p==' ' || *p=='\t') return form_fail(400,"BAD_PART_HEADER");
        char *colon=memchr(p,':',(size_t)(e-p));
        if (!colon || colon==p || colon-p>511 || hr->nheaders==MAX_HEADERS) return form_fail(431,"HEADER_LIMIT");
        for (char *c=p;c<colon;c++) if (!token_char((unsigned char)*c)) return form_fail(400,"BAD_PART_HEADER");
        char *v=colon+1; while (v<e && (*v==' ' || *v=='\t')) v++;
        char *last=e; while (last>v && (last[-1]==' ' || last[-1]=='\t')) last--;
        if (last-v>511) return form_fail(431,"HEADER_LIMIT");
        for (char *c=v;c<last;c++) if (((unsigned char)*c<32 && *c!='\t') || *c==127) return form_fail(400,"BAD_PART_HEADER");
        char *key=hr->headers[hr->nheaders][0], *value=hr->headers[hr->nheaders++][1];
        memcpy(key,p,(size_t)(colon-p)); key[colon-p]=0;
        memcpy(value,v,(size_t)(last-v)); value[last-v]=0;
        if (!strcasecmp(key,"Content-Length")) {
            if (has_length++ || !*value) return form_fail(400,"BAD_CONTENT_LENGTH");
            uint64_t length=0;
            for (char *c=value;*c;c++) {
                if (*c<'0' || *c>'9' || length>(UINT64_MAX-(unsigned)(*c-'0'))/10) return form_fail(400,"BAD_CONTENT_LENGTH");
                length=length*10+(unsigned)(*c-'0');
            }
            if (length>MAX_BODY_SIZE) too_large=1;
            else hr->content_length=(int)length;
        } else if (!strcasecmp(key,"Content-Type")) {
            if (ct++ || strlen(value)>=sizeof hr->content_type) return form_fail(400,"BAD_PART_HEADER");
            strcpy(hr->content_type,value);
        } else if (!strcasecmp(key,"Transfer-Encoding")) te=1;
        else if (!strcasecmp(key,"Content-Encoding")) ce=1;
        p=e+2;
    }
    if (te) return form_fail(has_length?400:501,has_length?"BAD_CONTENT_LENGTH":"TRANSFER_ENCODING_UNSUPPORTED");
    if (too_large) return form_fail(413,"UPLOAD_LIMIT");
    if (form_type(hr->content_type) && !has_length) return form_fail(411,"LENGTH_REQUIRED");
    if (form_type(hr->content_type) && ce) return form_fail(415,"UNSUPPORTED_ENCODING");
    hr->body=raw+*offset;
    hr->body_len=(int)(len-*offset);
    return 0;
}

/* Read exactly one bounded request; never dispatch a partial declared body. */
static int form_receive(int fd, char **raw_out) {
    size_t cap=MAX_HEADER_TOTAL+1, got=0, offset=0, target=0;
    char *raw=malloc(cap); if (!raw) return form_fail(500,"UPLOAD_IO_FAILED");
    int flags=fcntl(fd,F_GETFL);
    if (flags<0 || fcntl(fd,F_SETFL,flags|O_NONBLOCK)) {
        free(raw); return form_fail(500,"UPLOAD_IO_FAILED");
    }
    double deadline=monotonic_seconds()+HEADER_RECV_TIMEOUT;
    int headers=0, result=0;
    short events=POLLIN;
    for (;;) {
        double remaining=deadline-monotonic_seconds();
        if (remaining<=0) { result=form_fail(408,"UPLOAD_TIMEOUT"); break; }
        if (!http_ssl || !SSL_pending(http_ssl)) {
            struct pollfd wait={.fd=fd,.events=events};
            int ready=poll(&wait,1,(int)(remaining*1000)+1);
            if (ready<0 && errno==EINTR) continue;
            if (!ready) { result=form_fail(408,"UPLOAD_TIMEOUT"); break; }
            if (ready<0) { result=form_fail(400,"INCOMPLETE_BODY"); break; }
        }
        size_t want=headers?target-got:cap-got-1;
        if (!want) { result=form_fail(431,"HEADER_LIMIT"); break; }
        if (http_ssl) ERR_clear_error();
        ssize_t n=transport_read(fd,raw+got,want);
        if (n<0 && http_ssl) {
            int ssl_error=SSL_get_error(http_ssl,(int)n);
            if (ssl_error==SSL_ERROR_WANT_READ || ssl_error==SSL_ERROR_WANT_WRITE) {
                events=ssl_error==SSL_ERROR_WANT_READ?POLLIN:POLLOUT; continue;
            }
        }
        if (n<0 && !http_ssl && (errno==EAGAIN || errno==EWOULDBLOCK)) continue;
        if (n<0 && errno==EINTR) continue;
        if (n<=0) {
            if (n<0 && (errno==EAGAIN || errno==EWOULDBLOCK)) result=form_fail(408,"UPLOAD_TIMEOUT");
            else result=got?form_fail(400,"INCOMPLETE_BODY"):0;
            break;
        }
        if (monotonic_seconds()>deadline) { result=form_fail(408,"UPLOAD_TIMEOUT"); break; }
        got+=(size_t)n; raw[got]=0;
        if (!headers) {
            char *end=memmem(raw,got,"\r\n\r\n",4);
            if (!end) continue;
            HttpRequest hr;
            result=form_headers(raw,got,&hr,&offset); if (result<0) break;
            target=offset+(size_t)hr.content_length;
            headers=1; deadline=monotonic_seconds()+60;
            if (got>=target) { got=target; result=(int)got; break; }
            char *next=realloc(raw,target+1);
            if (!next) { result=form_fail(500,"UPLOAD_IO_FAILED"); break; }
            raw=next;
        } else if (got==target) { result=(int)got; break; }
    }
    fcntl(fd,F_SETFL,flags);
    if (result>0) { raw[got]=0; *raw_out=raw; }
    else free(raw);
    return result;
}

typedef struct { char name[17], marker[23]; int fd; dev_t dev; ino_t ino; } FormFile;
typedef struct {
    int dir; char base[PATH_MAX]; FormFile files[4]; int count;
    FLValue fields, values; size_t field_bytes; int field_count;
} FormContext;

static int form_owned_dir(int parent, const char *name) {
    if (mkdirat(parent,name,0700) && errno!=EEXIST) return -1;
    int fd=openat(parent,name,O_RDONLY|O_DIRECTORY|O_NOFOLLOW|O_CLOEXEC);
    struct stat st;
    if (fd>=0 && (fstat(fd,&st) || st.st_uid!=geteuid() || (st.st_mode&077)!=0)) { close(fd); fd=-1; }
    return fd;
}
static int form_identity(int dir, const char *name, dev_t dev, ino_t ino) {
    struct stat st;
    if (fstatat(dir,name,&st,AT_SYMLINK_NOFOLLOW)) return errno==ENOENT?0:-1;
    return S_ISREG(st.st_mode) && st.st_dev==dev && st.st_ino==ino?1:-1;
}
static int form_remove(int dir, const char *name, dev_t dev, ino_t ino) {
    int same=form_identity(dir,name,dev,ino);
    if (!same) return 1;
    return same>0 && (!unlinkat(dir,name,0) || errno==ENOENT);
}
/* Marker is a hard link to the same inode; flock distinguishes live requests.
 * Creating the marker first makes every crash point discoverable without a
 * PID heuristic or deleting someone else's directory contents. */
static int form_recover(int dir) {
    DIR *d=fdopendir(dup(dir)); if (!d) return 0;
    int ok=1; struct dirent *e;
    while ((e=readdir(d))) {
        if (strlen(e->d_name)!=21 || strncmp(e->d_name,".l06-",5)) continue;
        int valid=1; for (int i=5;i<21;i++) if (!strchr("0123456789abcdef",e->d_name[i])) valid=0;
        if (!valid) continue;
        int fd=openat(dir,e->d_name,O_RDWR|O_NOFOLLOW|O_CLOEXEC|O_NONBLOCK);
        if (fd<0) { ok=0; continue; }
        if (flock(fd,LOCK_EX|LOCK_NB)) { close(fd); continue; }
        struct stat st;
        if (fstat(fd,&st) || !S_ISREG(st.st_mode) || st.st_uid!=geteuid() || (st.st_mode&077) || st.st_nlink>2) ok=0;
        else if (!form_remove(dir,e->d_name+5,st.st_dev,st.st_ino) ||
                 !form_remove(dir,e->d_name,st.st_dev,st.st_ino)) ok=0;
        close(fd);
    }
    closedir(d); return ok;
}
static int form_storage(FormContext *c) {
    if (c->dir>=0) return 0;
    const char *base=getenv("FL_FILE_BASE"), *tmp=getenv("TMPDIR"); char expected[PATH_MAX];
    if (!base || *base!='/' || !tmp || strlen(base)+32>=PATH_MAX || !realpath(base,c->base) || strcmp(base,c->base))
        return form_fail(503,"UPLOAD_STORAGE_UNAVAILABLE");
    snprintf(expected,sizeof expected,"%s/tmp",c->base);
    if (strcmp(tmp,expected)) return form_fail(503,"UPLOAD_STORAGE_UNAVAILABLE");
    int root=open(c->base,O_RDONLY|O_DIRECTORY|O_NOFOLLOW|O_CLOEXEC);
    if (root<0) return form_fail(503,"UPLOAD_STORAGE_UNAVAILABLE");
    int t=form_owned_dir(root,"tmp"); close(root);
    if (t<0) return form_fail(503,"UPLOAD_STORAGE_UNAVAILABLE");
    c->dir=form_owned_dir(t,"fl-uploads"); close(t);
    if (c->dir<0) return form_fail(503,"UPLOAD_STORAGE_UNAVAILABLE");
    if (!form_recover(c->dir)) return form_fail(500,"UPLOAD_CLEANUP_FAILED");
    return 0;
}
static void form_recover_startup(void) {
    if (!getenv("FL_FILE_BASE")) return;
    FormContext c = { .dir = -1 };
    int rc = form_storage(&c);
    if (rc) fprintf(stderr,"[http] upload recovery: %s\n",http_error);
    if (c.dir >= 0) close(c.dir);
}
static int form_cleanup(FormContext *c) {
    int ok=1;
    for (int i=0;i<c->count;i++) {
        FormFile *f=&c->files[i];
        if (!form_remove(c->dir,f->name,f->dev,f->ino)) ok=0;
        else if (!form_remove(c->dir,f->marker,f->dev,f->ino)) ok=0;
        close(f->fd);
    }
    c->count=0;
    if (c->dir>=0) close(c->dir);
    c->dir=-1; return ok;
}
static int form_save(FormContext *c, const char *p, size_t n, FLValue meta) {
    if (c->count==4 || n>FORM_FILE_LIMIT) return form_fail(413,"UPLOAD_LIMIT");
    int rc=form_storage(c); if (rc) return rc;
    FormFile *f=&c->files[c->count]; memset(f,0,sizeof *f); f->fd=-1;
    for (int attempt=0;attempt<16;attempt++) {
        unsigned char random[8]; size_t done=0;
        while (done<sizeof random) {
            ssize_t k=getrandom(random+done,sizeof random-done,0);
            if (k<0 && errno==EINTR) continue;
            if (k<=0) return form_fail(500,"UPLOAD_IO_FAILED");
            done+=(size_t)k;
        }
        for (int i=0;i<8;i++) snprintf(f->name+2*i,3,"%02x",random[i]);
        snprintf(f->marker,sizeof f->marker,".l06-%s",f->name);
        /* Lock before publishing the recovery link: other instances must not
         * collect an inode between its creation and acquisition of flock. */
        f->fd=openat(c->dir,".",O_RDWR|O_TMPFILE|O_CLOEXEC,0600);
        if (f->fd<0) return form_fail(503,"UPLOAD_STORAGE_UNAVAILABLE");
        struct stat st;
        if (flock(f->fd,LOCK_EX|LOCK_NB) || fchmod(f->fd,0600) || fstat(f->fd,&st)) {
            close(f->fd); return form_fail(500,"UPLOAD_IO_FAILED");
        }
        f->dev=st.st_dev; f->ino=st.st_ino;
        if (linkat(f->fd,"",c->dir,f->marker,AT_EMPTY_PATH)) {
            int saved=errno; close(f->fd); f->fd=-1;
            if (saved==EEXIST) continue;
            return form_fail(500,"UPLOAD_IO_FAILED");
        }
        if (linkat(c->dir,f->marker,c->dir,f->name,0)) {
            int saved=errno; form_remove(c->dir,f->marker,f->dev,f->ino); close(f->fd); f->fd=-1;
            if (saved==EEXIST) continue;
            return form_fail(500,"UPLOAD_IO_FAILED");
        }
        c->count++;
        FLValue data=form_bytes(p,n);
        if (data.tag!=FL_STRING) return form_fail(500,"UPLOAD_IO_FAILED");
        FLValue result=fl_file_write_reserved_result(f->fd,data);
        FLValue ok=fl_map_get(result,fl_str_val("ok")), bytes=fl_map_get(result,fl_str_val("bytes"));
        if (!fl_truthy(ok) || bytes.tag!=FL_INT || bytes.i!=(int64_t)n ||
            form_identity(c->dir,f->name,f->dev,f->ino)!=1)
            return form_fail(500,"UPLOAD_IO_FAILED");
        char path[PATH_MAX]; snprintf(path,sizeof path,"%s/tmp/fl-uploads/%s",c->base,f->name);
        meta=fl_map_set(meta,fl_str_val("filename"),fl_str_val(f->name));
        meta=fl_map_set(meta,fl_str_val("path"),fl_str_val(path));
        meta=fl_map_set(meta,fl_str_val("size"),fl_int((int64_t)n));
        c->values=fl_vec_push(c->values,meta); return 0;
    }
    return form_fail(500,"UPLOAD_IO_FAILED");
}
static int form_field(FormContext *c, FLValue key, FLValue value) {
    FLString *k=(FLString *)key.obj, *v=(FLString *)value.obj;
    if (!k->len || !form_utf8((unsigned char *)k->data,k->len) || !form_utf8((unsigned char *)v->data,v->len))
        return form_fail(400,"BAD_FIELD_ENCODING");
    if (k->len>128 || v->len>FORM_FIELD_LIMIT || c->field_bytes+v->len>FORM_FIELDS_LIMIT || c->field_count==32)
        return form_fail(413,"UPLOAD_LIMIT");
    for (uint32_t i=0;i<k->len;i++) if ((unsigned char)k->data[i]<32 || k->data[i]==127) return form_fail(400,"BAD_FIELD_ENCODING");
    if (fl_map_get(c->fields,key).tag!=FL_NIL) return form_fail(400,"DUPLICATE_FIELD");
    FLVector *files=(FLVector *)c->values.obj;
    for (uint32_t i=0;i<files->len;i++) if (fl_truthy(fl_eq(fl_map_get(files->data[i],fl_str_val("fieldname")),key)))
        return form_fail(400,"DUPLICATE_FIELD");
    c->fields=fl_map_set(c->fields,key,value); c->field_count++; c->field_bytes+=v->len; return 0;
}
static int hex_digit(unsigned char c) {
    if (c>='0' && c<='9') return c-'0';
    if (c>='a' && c<='f') return c-'a'+10;
    if (c>='A' && c<='F') return c-'A'+10;
    return -1;
}
static FLValue form_decode(const char *p, size_t n) {
    FLValue out=form_bytes(p,n);
    if (out.tag!=FL_STRING) { http_error="UPLOAD_IO_FAILED"; return out; }
    FLString *s=(FLString *)out.obj; size_t at=0;
    for (size_t i=0;i<n;i++) {
        if (p[i]=='%') {
            if (i+2>=n || hex_digit(p[i+1])<0 || hex_digit(p[i+2])<0) {
                http_error="BAD_FIELD_ENCODING"; return fl_nil();
            }
            s->data[at++]=(char)(hex_digit(p[i+1])*16+hex_digit(p[i+2])); i+=2;
        } else s->data[at++]=p[i]=='+'?' ':p[i];
    }
    s->len=(uint32_t)at; s->data[at]=0; return out;
}
static int form_urlencoded(FormContext *c, const char *p, size_t n) {
    size_t i=0;
    while (i<n) {
        const char *end=memchr(p+i,'&',n-i); if (!end) end=p+n;
        const char *eq=memchr(p+i,'=',(size_t)(end-p-i)); if (!eq) eq=end;
        FLValue key=form_decode(p+i,(size_t)(eq-p-i));
        FLValue value=form_decode(eq<end?eq+1:end,eq<end?(size_t)(end-eq-1):0);
        if (key.tag!=FL_STRING || value.tag!=FL_STRING) {
            if (http_error && !strcmp(http_error,"UPLOAD_IO_FAILED")) return form_fail(500,"UPLOAD_IO_FAILED");
            return form_fail(400,"BAD_FIELD_ENCODING");
        }
        int rc=form_field(c,key,value); if (rc) return rc;
        i=(size_t)(end-p)+1;
    }
    return 0;
}
static int form_boundary(const char *s) {
    size_t n=strlen(s); if (!n || n>70 || s[n-1]==' ') return 0;
    for (size_t i=0;i<n;i++) if (!isalnum((unsigned char)s[i]) && !strchr("'()+_,-./:=? ",s[i])) return 0;
    return 1;
}
static const char *form_delimiter(const char *p, const char *end, const char *b, size_t n) {
    while ((size_t)(end-p)>=n+2) {
        const char *q=memmem(p,(size_t)(end-p),b,n); if (!q) return NULL;
        const char *v=q+n;
        size_t left=(size_t)(end-v);
        if (left>=2 && !memcmp(v,"\r\n",2)) return q;
        if (left>=2 && !memcmp(v,"--",2) &&
            (left==2 || (left>=4 && !memcmp(v+2,"\r\n",2)))) return q;
        p=q+1;
    }
    return NULL;
}
static int form_multipart(FormContext *c, HttpRequest *hr, FLValue params) {
    FLValue bv=fl_map_get(params,fl_str_val("boundary"));
    if (bv.tag!=FL_STRING || !form_boundary(strval(bv))) return form_fail(400,"BAD_BOUNDARY");
    char first[73], delimiter[75]; snprintf(first,sizeof first,"--%s",strval(bv));
    snprintf(delimiter,sizeof delimiter,"\r\n%s",first);
    size_t fn=strlen(first), dn=strlen(delimiter);
    const char *end=hr->body+hr->body_len, *p=hr->body, *start=NULL;
    if (form_delimiter(p,end,first,fn)==p) start=p;
    else { const char *found=form_delimiter(p,end,delimiter,dn); if (found) start=found+2; }
    if (!start) return form_fail(400,"BAD_MULTIPART");
    p=start+fn; int parts=0;
    for (;;) {
        if (end-p<2) return form_fail(400,"BAD_MULTIPART");
        if (!memcmp(p,"--",2)) {
            p+=2;
            if (p!=end && (end-p<2 || memcmp(p,"\r\n",2))) return form_fail(400,"BAD_MULTIPART");
            return 0;
        }
        if (memcmp(p,"\r\n",2)) return form_fail(400,"BAD_MULTIPART");
        p+=2; if (++parts>36) return form_fail(413,"UPLOAD_LIMIT");
        const char *he=memmem(p,(size_t)(end-p),"\r\n\r\n",4);
        if (!he) return form_fail(400,"BAD_PART_HEADER");
        if (he+4-p>8192) return form_fail(413,"UPLOAD_LIMIT");
        if (memchr(p,0,(size_t)(he-p))) return form_fail(400,"BAD_PART_HEADER");
        FLValue disposition=fl_nil(); char mime[8193]="application/octet-stream";
        int headers=0, has_type=0;
        while (p<he) {
            const char *e=memmem(p,(size_t)(he-p)+2,"\r\n",2);
            if (!e) return form_fail(400,"BAD_PART_HEADER");
            if (++headers>16) return form_fail(413,"UPLOAD_LIMIT");
            const char *colon=memchr(p,':',(size_t)(e-p));
            if (!colon || colon==p) return form_fail(400,"BAD_PART_HEADER");
            char key[8193], value[8193]; size_t kn=(size_t)(colon-p);
            for (size_t i=0;i<kn;i++) if (!token_char((unsigned char)p[i])) return form_fail(400,"BAD_PART_HEADER");
            memcpy(key,p,kn); key[kn]=0; const char *v=colon+1;
            while (v<e && (*v==' ' || *v=='\t')) v++;
            const char *last=e; while (last>v && (last[-1]==' ' || last[-1]=='\t')) last--;
            for (const char *z=v;z<last;z++) if ((unsigned char)*z<32 || *z==127) return form_fail(400,"BAD_PART_HEADER");
            memcpy(value,v,(size_t)(last-v)); value[last-v]=0;
            if (!strcasecmp(key,"Content-Disposition")) {
                char type[128];
                if (disposition.tag!=FL_NIL || !form_parameters(value,type,sizeof type,&disposition) || strcmp(type,"form-data"))
                    return form_fail(400,"BAD_PART_HEADER");
            } else if (!strcasecmp(key,"Content-Type")) {
                FLValue mp; char type[8193];
                if (has_type++ || !form_parameters(value,type,sizeof type,&mp)) return form_fail(400,"BAD_PART_HEADER");
                FLValue charset=fl_map_get(mp,fl_str_val("charset"));
                if (!strncasecmp(type,"multipart/",10) || (charset.tag==FL_STRING && strcasecmp(strval(charset),"utf-8")))
                    return form_fail(415,"UNSUPPORTED_ENCODING");
                if (strlen(value)>=sizeof mime) return form_fail(413,"UPLOAD_LIMIT");
                strcpy(mime,value);
            } else if (!strcasecmp(key,"Content-Transfer-Encoding")) return form_fail(415,"UNSUPPORTED_ENCODING");
            p=e+2;
        }
        if (disposition.tag==FL_NIL) return form_fail(400,"BAD_PART_HEADER");
        FLValue name=fl_map_get(disposition,fl_str_val("name"));
        FLValue filename=fl_map_get(disposition,fl_str_val("filename"));
        if (name.tag!=FL_STRING || !*((FLString *)name.obj)->data ||
            !form_utf8((unsigned char *)strval(name),((FLString *)name.obj)->len)) return form_fail(400,"BAD_FIELD_ENCODING");
        if (((FLString *)name.obj)->len>128) return form_fail(413,"UPLOAD_LIMIT");
        for (const char *z=strval(name);*z;z++) if ((unsigned char)*z<32 || *z==127) return form_fail(400,"BAD_FIELD_ENCODING");
        if (fl_map_get(disposition,fl_str_val("filename*")).tag!=FL_NIL) return form_fail(400,"BAD_PART_HEADER");
        p=he+4; const char *next=form_delimiter(p,end,delimiter,dn);
        if (!next) return form_fail(400,"BAD_MULTIPART");
        size_t length=(size_t)(next-p); int rc;
        if (filename.tag==FL_STRING) {
            FLString *s=(FLString *)filename.obj;
            if (s->len>255) return form_fail(413,"UPLOAD_LIMIT");
            if (!form_utf8((unsigned char *)s->data,s->len) || !strcmp(s->data,".") || !strcmp(s->data,"..") || strpbrk(s->data,"/\\:"))
                return form_fail(400,"UNSAFE_FILENAME");
            for (uint32_t i=0;i<s->len;i++) if ((unsigned char)s->data[i]<32 || s->data[i]==127) return form_fail(400,"UNSAFE_FILENAME");
            if (!s->len) { if (length) return form_fail(400,"BAD_MULTIPART"); }
            else {
                if (fl_map_get(c->fields,name).tag!=FL_NIL) return form_fail(400,"DUPLICATE_FIELD");
                FLValue meta=fl_map_new();
                meta=fl_map_set(meta,fl_str_val("fieldname"),name);
                meta=fl_map_set(meta,fl_str_val("originalname"),filename);
                meta=fl_map_set(meta,fl_str_val("mimetype"),fl_str_val(mime));
                rc=form_save(c,p,length,meta); if (rc) return rc;
            }
        } else {
            if (length>FORM_FIELD_LIMIT) return form_fail(413,"UPLOAD_LIMIT");
            FLValue value=form_bytes(p,length); if (value.tag!=FL_STRING) return form_fail(500,"UPLOAD_IO_FAILED");
            rc=form_field(c,name,value); if (rc) return rc;
        }
        p=next+dn;
    }
}
static int http_form_parse(FormContext *c, HttpRequest *hr) {
    memset(c,0,sizeof *c); c->dir=-1; c->fields=fl_map_new(); c->values=fl_vec_new();
    int kind=form_type(hr->content_type); if (!kind) return 0;
    char type[128]; FLValue params;
    if (!form_parameters(hr->content_type,type,sizeof type,&params))
        return form_fail(400,kind==2?"BAD_BOUNDARY":"BAD_FIELD_ENCODING");
    FLValue charset=fl_map_get(params,fl_str_val("charset"));
    if (charset.tag==FL_STRING && strcasecmp(strval(charset),"utf-8")) return form_fail(415,"UNSUPPORTED_ENCODING");
    return kind==1?form_urlencoded(c,hr->body,(size_t)hr->body_len):form_multipart(c,hr,params);
}
