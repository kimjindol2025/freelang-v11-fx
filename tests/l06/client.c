#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <dirent.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <openssl/ssl.h>
#include <openssl/evp.h>
#include <time.h>
#include <pthread.h>
#include <poll.h>

typedef struct { char *p; size_t n, cap; } Buf;
static int port, use_tls, passes, failures;
static const char *root;
static SSL_CTX *ctx;
static void add(Buf *b,const void *s,size_t n) {
    if (b->n+n+1>b->cap) { b->cap=(b->n+n+1)*2; b->p=realloc(b->p,b->cap); if (!b->p) abort(); }
    memcpy(b->p+b->n,s,n); b->n+=n; b->p[b->n]=0;
}
static void text(Buf *b,const char *s) { add(b,s,strlen(s)); }
static int connect_server(void) {
    int fd=socket(AF_INET,SOCK_STREAM,0); struct sockaddr_in a={.sin_family=AF_INET,.sin_port=htons(port)};
    a.sin_addr.s_addr=htonl(INADDR_LOOPBACK);
    struct timeval tv={75,0}; setsockopt(fd,SOL_SOCKET,SO_RCVTIMEO,&tv,sizeof tv); setsockopt(fd,SOL_SOCKET,SO_SNDTIMEO,&tv,sizeof tv);
    if (connect(fd,(void*)&a,sizeof a)) { perror("connect"); exit(2); }
    return fd;
}
static void write_all(int fd,SSL *ssl,const char *p,size_t n,size_t split) {
    while (n) {
        size_t amount=split && split<n?split:n;
        int k=ssl?SSL_write(ssl,p,(int)amount):(int)send(fd,p,amount,MSG_NOSIGNAL);
        if (k<=0) break;
        n-=(size_t)k; p+=k;
    }
}
static Buf request(const char *headers,const void *body,size_t n,size_t split,int half) {
    int fd=connect_server(); SSL *ssl=NULL;
    if (use_tls) { ssl=SSL_new(ctx); SSL_set_fd(ssl,fd); if (SSL_connect(ssl)!=1) abort(); }
    write_all(fd,ssl,headers,strlen(headers),split);
    write_all(fd,ssl,body,n,split);
    if (half) shutdown(fd,SHUT_WR);
    Buf result={0}; char b[16384]; int k;
    while ((k=ssl?SSL_read(ssl,b,sizeof b):(int)recv(fd,b,sizeof b,0))>0) add(&result,b,(size_t)k);
    if (ssl) { SSL_free(ssl); } close(fd);
    if (!result.p) text(&result,"");
    return result;
}
static Buf post(const char *path,const char *ct,const Buf *body,const char *extra,size_t split) {
    char h[2048]; snprintf(h,sizeof h,"POST %s HTTP/1.1\r\nHost: localhost\r\nConnection: close\r\nContent-Type: %s\r\nContent-Length: %zu\r\n%s\r\n",path,ct,body->n,extra?extra:"");
    return request(h,body->p,body->n,split,0);
}
static int status(Buf *r) { int n=0; sscanf(r->p,"HTTP/1.1 %d",&n); return n; }
static char *body(Buf *r) { char *p=strstr(r->p,"\r\n\r\n"); return p?p+4:r->p; }
static int has(Buf *r,const char *s) { return strstr(body(r),s)!=NULL; }
static void check(int yes,const char *name) {
    if (yes) { printf("PASS %s\n",name); passes++; }
    else { printf("FAIL %s\n",name); failures++; }
    fflush(stdout);
}
static int count_handler(void) {
    Buf r=request("GET /count HTTP/1.1\r\nHost: localhost\r\nConnection: close\r\n\r\n","",0,0,0);
    int count=atoi(body(&r)); free(r.p); return count;
}
static int files_left(void) {
    char path[4096]; snprintf(path,sizeof path,"%s/tmp/fl-uploads",root);
    DIR *d=opendir(path); if (!d) return 0;
    struct dirent *e; int n=0;
    while ((e=readdir(d))) if (strcmp(e->d_name,".") && strcmp(e->d_name,"..") && strcmp(e->d_name,"0000000000000000")) n++;
    closedir(d); return n;
}
static void expect_raw(const char *name,const char *h,const char *data,size_t n,int wanted,const char *error,int half) {
    int before=count_handler();
    Buf r=request(h,data,n,0,half);
    int aborted_tls = use_tls && half && status(&r)==0;
    if (aborted_tls) printf("TLS_PEER_ABORT_NO_RESPONSE=1\n");
    int ok=(aborted_tls || (status(&r)==wanted && (!error || has(&r,error)))) && count_handler()==before && !files_left();
    if (!ok) printf("DETAIL status=%d expected=%d response=%.200s\n",status(&r),wanted,body(&r));
    check(ok,name); free(r.p);
}
static void part(Buf *b,const char *boundary,const char *name,const char *filename,const void *data,size_t n,const char *extra) {
    char h[1024]; snprintf(h,sizeof h,"--%s\r\nContent-Disposition: form-data; name=\"%s\"%s",boundary,name,filename?"; filename=\"":""); text(b,h);
    if (filename) { text(b,filename); text(b,"\""); }
    text(b,"\r\n"); if (extra) text(b,extra); text(b,"\r\n"); add(b,data,n); text(b,"\r\n");
}
static void finish(Buf *b,const char *boundary) { text(b,"--"); text(b,boundary); text(b,"--\r\n"); }
static void expected_post(const char *name,const char *ct,Buf *b,int wanted,const char *error) {
    int before=count_handler(); Buf r=post("/upload",ct,b,NULL,0);
    int ok=status(&r)==wanted && (!error || has(&r,error)) && count_handler()==before+(wanted==200) && !files_left();
    if (!ok) printf("DETAIL status=%d expected=%d response=%.200s\n",status(&r),wanted,body(&r));
    check(ok,name); free(r.p);
}
static void file_test(const char *name,size_t size,int expected,int split) {
    Buf b={0}; unsigned char *data=malloc(size?size:1);
    for (size_t i=0;i<size;i++) data[i]=(unsigned char)i;
    part(&b,"Boundary06","image","bytes.bin",data,size,NULL); finish(&b,"Boundary06");
    Buf r=post("/upload","multipart/form-data; boundary=Boundary06",&b,NULL,split);
    unsigned char hash[32]; unsigned hn; char hex[65], size_json[64];
    EVP_Digest(data,size,hash,&hn,EVP_sha256(),NULL);
    for (int i=0;i<32;i++) sprintf(hex+2*i,"%02x",hash[i]);
    snprintf(size_json,sizeof size_json,"\"bytes\":%zu",size);
    int ok=status(&r)==expected && !files_left();
    if (expected==200) ok=ok && has(&r,hex) && has(&r,size_json) && has(&r,"\"independent\":true") && has(&r,"\"safe\":true") && has(&r,"\"pattern\":true");
    if (!ok) printf("DETAIL %s status=%d response=%.300s\n",name,status(&r),body(&r));
    check(ok,name); free(r.p); free(b.p); free(data);
}
static void *concurrent(void *unused) {
    (void)unused; Buf b={0}; const char data[]={'A',0,'B'};
    part(&b,"Concurrent","image","a.bin",data,sizeof data,NULL); finish(&b,"Concurrent");
    Buf r=post("/upload","multipart/form-data; boundary=Concurrent",&b,NULL,0);
    int ok=status(&r)==200 && has(&r,"76fe3925c7167317f2df68454339f5ec3650e4062178b4f2be219b105a507907");
    free(r.p); free(b.p); return (void *)(intptr_t)ok;
}
static void timeout_case(int trickle) {
    int before=count_handler(), fd=connect_server();
    const char *h="POST /upload HTTP/1.1\r\nHost: localhost\r\nContent-Type: multipart/form-data; boundary=B\r\nContent-Length: 1000\r\n\r\n";
    write_all(fd,NULL,h,strlen(h),0);
    struct timespec start,end; clock_gettime(CLOCK_MONOTONIC,&start);
    Buf r={0};
    for (;;) {
        struct pollfd p={.fd=fd,.events=POLLIN};
        int rc=poll(&p,1,1000);
        if (rc>0) { char data[1024]; int n=(int)recv(fd,data,sizeof data,0); if (n<=0) break; add(&r,data,(size_t)n); }
        else if (rc==0 && trickle) write_all(fd,NULL,"x",1,0);
        clock_gettime(CLOCK_MONOTONIC,&end);
        if (end.tv_sec-start.tv_sec>70) break;
    }
    clock_gettime(CLOCK_MONOTONIC,&end); close(fd);
    double seconds=end.tv_sec-start.tv_sec+(end.tv_nsec-start.tv_nsec)/1e9;
    printf("TIMEOUT_%s_HTTP=%d SECONDS=%.3f\n",trickle?"TRICKLE":"IDLE",r.p?status(&r):0,seconds);
    check(r.p && status(&r)==408 && seconds>=59 && seconds<=65 && count_handler()==before && !files_left(),trickle?"absolute-deadline-trickle":"body-idle-timeout");
    free(r.p);
}

int main(int argc,char **argv) {
    if (argc<3) return 2;
    port=atoi(argv[1]); root=argv[2]; use_tls=argc>3 && !strcmp(argv[3],"tls");
    signal(SIGPIPE,SIG_IGN); ctx=SSL_CTX_new(TLS_client_method()); SSL_CTX_set_verify(ctx,SSL_VERIFY_NONE,NULL);
    if (argc>3 && !strcmp(argv[3],"timeouts")) {
        timeout_case(0); timeout_case(1);
        printf("L06_TIMEOUT_PASS=%d FAIL=%d\n",passes,failures);
        return failures?1:0;
    }
    Buf b={0}, r;
    const char *form="name=%ED%95%9C%EA%B8%80&space=a+b&plus=%2B&eq=%3D&empty=";
    text(&b,form); r=post("/upload","Application/X-Www-Form-Urlencoded; charset=utf-8",&b,NULL,1);
    check(status(&r)==200 && has(&r,"\"aliases\":true") && has(&r,"한글") && has(&r,"a b") && has(&r,"\"plus\":\"+\"") && has(&r,"\"empty\":\"\""),"urlencoded-map-unicode-plus-empty"); free(r.p); free(b.p); b=(Buf){0};
    expected_post("urlencoded-empty","application/x-www-form-urlencoded",&b,200,NULL);
    const char *bad[]={"a=%","a=%GG","a=%00","a=%FF","a=1&a=2","=x"};
    for (int i=0;i<6;i++) { text(&b,bad[i]); expected_post(bad[i],"application/x-www-form-urlencoded",&b,400,NULL); b.n=0; }
    free(b.p); b=(Buf){0};
    const char nul[]={'A',0,'B'};
    part(&b,"B","csrf",NULL,"L06_FAKE_SECRET",15,NULL);
    part(&b,"B","image","a.bin",nul,3,NULL); part(&b,"B","image","한글.bin","",0,NULL); finish(&b,"B");
    r=post("/upload","multipart/form-data; boundary=\"B\"",&b,NULL,1);
    check(status(&r)==200 && has(&r,"76fe3925c7167317f2df68454339f5ec3650e4062178b4f2be219b105a507907") && has(&r,"a.bin") && has(&r,"한글.bin") && !files_left(),"multipart-multiple-same-name-nul-quoted-boundary"); free(r.p); free(b.p); b=(Buf){0};
    size_t sizes[]={0,65535,65536,65537,15*1024*1024-1,15*1024*1024,15*1024*1024+1,8193};
    for (int i=0;i<8;i++) { char name[64]; snprintf(name,sizeof name,"binary-file-%zu",sizes[i]); file_test(name,sizes[i],i==6?413:200,i==3?7:0); }
    for (int count=4;count<=5;count++) {
        for (int i=0;i<count;i++) part(&b,"B","files","x.bin",nul,3,NULL);
        finish(&b,"B"); expected_post(count==4?"four-files":"five-files","multipart/form-data; boundary=B",&b,count==4?200:413,NULL); free(b.p); b=(Buf){0};
    }
    /* Four full-sized files remain below the overall 64 MiB cap. */
    char *large=calloc(1,15*1024*1024);
    for (int i=0;i<4;i++) part(&b,"B","files","full.bin",large,15*1024*1024,NULL);
    finish(&b,"B"); expected_post("four-15MiB-files","multipart/form-data; boundary=B",&b,200,NULL); free(large); free(b.p); b=(Buf){0};
    /* Preamble padding tests total body independently of each part limit. */
    for (int delta=-1;delta<=1;delta++) {
        size_t total=64*1024*1024+delta;
        b.p=malloc(total+1); b.cap=total+1; b.n=total-7; memset(b.p,'x',b.n); text(&b,"\r\n--B--");
        char name[64]; snprintf(name,sizeof name,"body-64MiB%+d",delta);
        expected_post(name,"multipart/form-data; boundary=B",&b,delta==1?413:200,NULL); free(b.p); b=(Buf){0};
    }
    const char *names[]={"../x","/x","C:\\\\x",".","..","a:b","%2e%2e%2f"};
    for (int i=0;i<7;i++) { part(&b,"B","f",names[i],"x",1,NULL); finish(&b,"B"); expected_post(names[i],"multipart/form-data; boundary=B",&b,i==6?200:400,NULL); free(b.p); b=(Buf){0}; }
    part(&b,"B","f","","",0,NULL); finish(&b,"B"); expected_post("unselected-file","multipart/form-data; boundary=B",&b,200,NULL); free(b.p); b=(Buf){0};
    part(&b,"B","f","","x",1,NULL); finish(&b,"B"); expected_post("empty-name-nonempty-file","multipart/form-data; boundary=B",&b,400,NULL); free(b.p); b=(Buf){0};
    const char *cts[]={"multipart/form-data","multipart/form-data; boundary=\"\"","multipart/form-data; boundary=B; boundary=C","multipart/form-data; boundary=\"B \""};
    text(&b,"--B--");
    for (int i=0;i<4;i++) expected_post(cts[i],cts[i],&b,400,"BAD_BOUNDARY");
    free(b.p); b=(Buf){0};
    char boundary[72]; memset(boundary,'b',70); boundary[70]=0;
    finish(&b,boundary); char ct[256]; snprintf(ct,sizeof ct,"multipart/form-data; boundary=%s",boundary); expected_post("boundary-70",ct,&b,200,NULL); free(b.p); b=(Buf){0};
    boundary[70]='b'; boundary[71]=0; finish(&b,boundary); snprintf(ct,sizeof ct,"multipart/form-data; boundary=%s",boundary); expected_post("boundary-71",ct,&b,400,NULL); free(b.p); b=(Buf){0};
    part(&b,"B","f","x","x",1,NULL); expected_post("missing-close-boundary","multipart/form-data; boundary=B",&b,400,NULL); free(b.p); b=(Buf){0};
    const char *enc[]={"Content-Transfer-Encoding: base64\r\n","Content-Type: multipart/mixed\r\n","Content-Type: text/plain; charset=iso-8859-1\r\n"};
    for (int i=0;i<3;i++) { part(&b,"B","f","x","x",1,enc[i]); finish(&b,"B"); expected_post(enc[i],"multipart/form-data; boundary=B",&b,415,NULL); free(b.p); b=(Buf){0}; }
    text(&b,"--B\r\nContent-Disposition: form-data; name=\"f\"; filename*=UTF-8''x\r\n\r\nx\r\n--B--\r\n"); expected_post("filename-star","multipart/form-data; boundary=B",&b,400,NULL); free(b.p); b=(Buf){0};
    part(&b,"B","f",NULL,"a",1,NULL); part(&b,"B","f","x","b",1,NULL); finish(&b,"B"); expected_post("field-file-collision","multipart/form-data; boundary=B",&b,400,NULL); free(b.p); b=(Buf){0};
    /* Text field and name byte limits, independently of the body limit. */
    for (int delta=-1;delta<=1;delta++) { size_t n=65536+delta; char *data=malloc(n); memset(data,'a',n); part(&b,"B","text",NULL,data,n,NULL); finish(&b,"B"); expected_post("field-byte-boundary","multipart/form-data; boundary=B",&b,delta==1?413:200,NULL); free(data); free(b.p); b=(Buf){0}; }
    for (int count=31;count<=33;count++) { for (int i=0;i<count;i++) { char name[16]; snprintf(name,sizeof name,"k%d",i); part(&b,"B",name,NULL,"x",1,NULL); } finish(&b,"B"); expected_post("field-count-boundary","multipart/form-data; boundary=B",&b,count<=32?200:413,NULL); free(b.p); b=(Buf){0}; }
    const char *headers[]={
      "Content-Length: -1\r\n", "Content-Length: nope\r\n", "Content-Length: 0\r\nContent-Length: 0\r\n",
      "Content-Length: 18446744073709551616\r\n", "Transfer-Encoding: chunked\r\nContent-Length: 0\r\n",
      "Transfer-Encoding: chunked\r\n", "", "Content-Length: 0\r\nContent-Encoding: gzip\r\n",
      "Content-Length: 67108865\r\nTransfer-Encoding: chunked\r\n",
      "Content-Length: 67108865\r\nContent-Length: 0\r\n"};
    int codes[]={400,400,400,400,400,501,411,415,400,400};
    for (int i=0;i<10;i++) { char h[1024]; snprintf(h,sizeof h,"POST /upload HTTP/1.1\r\nHost: localhost\r\nContent-Type: multipart/form-data; boundary=B\r\n%s\r\n",headers[i]); expect_raw("framing-error",h,"",0,codes[i],NULL,0); }
    expect_raw("incomplete-body-half-close","POST /upload HTTP/1.1\r\nHost: localhost\r\nContent-Type: multipart/form-data; boundary=B\r\nContent-Length: 100\r\n\r\n","--B\r\n",5,400,"INCOMPLETE_BODY",1);
    part(&b,"B","image","x",nul,3,NULL); finish(&b,"B");
    const char *paths[]={"/forbid","/throw","/copy","/delete"}; int path_codes[]={403,500,200,200};
    for (int i=0;i<4;i++) { r=post(paths[i],"multipart/form-data; boundary=B",&b,NULL,0); check(status(&r)==path_codes[i] && !files_left(),paths[i]); free(r.p); }
    free(b.p); b=(Buf){0};
    pthread_t threads[4]; for (int i=0;i<4;i++) pthread_create(&threads[i],NULL,concurrent,NULL);
    int all=1; for (int i=0;i<4;i++) { void *result; pthread_join(threads[i],&result); all &= (int)(intptr_t)result; }
    check(all && !files_left(),"concurrent-request-isolation");
    /* Each fault travels over real HTTP and through the public .fl handler. */
    const char *faults[]={"WRITE","SHORT","FLUSH","CLOSE","ALLOC","UNLINK","RANDOM","WRITE_SECOND"};
    for (int i=0;i<8;i++) {
        char data[8192]; memset(data,'q',sizeof data);
        part(&b,"B","f","fault.bin",data,sizeof data,NULL); finish(&b,"B");
        if (i==7) { b.n-=7; part(&b,"B","f","second.bin",data,sizeof data,NULL); finish(&b,"B"); }
        char extra[128]; snprintf(extra,sizeof extra,"X-L06-Fault: %s\r\n",faults[i]);
        char sentinel[4096]; snprintf(sentinel,sizeof sentinel,"%s/tmp/fl-uploads/0000000000000000",root);
        if (i==6) { FILE *f=fopen(sentinel,"w"); fputs("KEEP",f); fclose(f); }
        int before=count_handler(); r=post("/upload","multipart/form-data; boundary=B",&b,extra,0);
        int expected=(i==1 || i==6)?200:500;
        int ok=status(&r)==expected && count_handler()==before+((i==1 || i==5 || i==6)?1:0);
        if (i==5) ok=ok && files_left()==2;
        else ok=ok && files_left()==0;
        if (i==6) { char data[5]={0}; FILE *f=fopen(sentinel,"r"); if (f) { fread(data,1,4,f); fclose(f); } ok=ok && !strcmp(data,"KEEP"); unlink(sentinel); }
        check(ok,faults[i]); free(r.p); free(b.p); b=(Buf){0};
        if (i==5) { file_test("recover-after-unlink-failure",3,200,0); }
    }
    text(&b,"long="); for (int i=0;i<7000;i++) text(&b,"a"); expected_post("urlencoded-over-1024","application/x-www-form-urlencoded",&b,200,NULL); free(b.p); b=(Buf){0};
    for (int delta=-1;delta<=1;delta++) {
        char *data=malloc(65536); memset(data,'z',65536);
        for (int i=0;i<4;i++) { char name[16]; snprintf(name,sizeof name,"k%d",i); part(&b,"B",name,NULL,data,65536-(i==3 && delta==-1),NULL); }
        if (delta==1) part(&b,"B","overflow",NULL,"x",1,NULL);
        finish(&b,"B"); expected_post("total-field-byte-boundary","multipart/form-data; boundary=B",&b,delta==1?413:200,NULL); free(data); free(b.p); b=(Buf){0};
    }
    for (int count=35;count<=37;count++) {
        for (int i=0;i<count;i++) part(&b,"B","skipped","","",0,NULL);
        finish(&b,"B"); expected_post("total-part-count-boundary","multipart/form-data; boundary=B",&b,count<=36?200:413,NULL); free(b.p); b=(Buf){0};
    }
    for (int n=127;n<=129;n++) { char name[130]; memset(name,'n',n); name[n]=0; part(&b,"B",name,NULL,"x",1,NULL); finish(&b,"B"); expected_post("field-name-boundary","multipart/form-data; boundary=B",&b,n<=128?200:413,NULL); free(b.p); b=(Buf){0}; }
    for (int n=254;n<=256;n++) { char name[257]; memset(name,'n',n); name[n]=0; part(&b,"B","f",name,"x",1,NULL); finish(&b,"B"); expected_post("filename-boundary","multipart/form-data; boundary=B",&b,n<=255?200:413,NULL); free(b.p); b=(Buf){0}; }
    for (int n=15;n<=17;n++) {
        text(&b,"--B\r\nContent-Disposition: form-data; name=\"f\"\r\n");
        for (int i=1;i<n;i++) text(&b,"X-Ignored: x\r\n");
        text(&b,"\r\nx\r\n--B--\r\n"); expected_post("part-header-count","multipart/form-data; boundary=B",&b,n<=16?200:413,NULL); free(b.p); b=(Buf){0};
    }
    for (int delta=-1;delta<=1;delta++) {
        text(&b,"--B\r\n"); size_t start=b.n;
        text(&b,"Content-Disposition: form-data; name=\"f\"\r\nX-Padding: ");
        size_t padding=8192+delta-(b.n-start)-4;
        for (size_t i=0;i<padding;i++) text(&b,"x");
        text(&b,"\r\n\r\nx\r\n--B--\r\n"); expected_post("part-header-byte-boundary","multipart/form-data; boundary=B",&b,delta==1?413:200,NULL); free(b.p); b=(Buf){0};
    }
    const char similar[]="X\r\n--BXX\000Y\r\n--B--X\r\n";
    part(&b,"B","f","similar.bin",similar,sizeof similar-1,NULL); finish(&b,"B");
    r=post("/upload","multipart/form-data; boundary=B",&b,NULL,1);
    unsigned char dh[32]; unsigned dhn; char dhex[65]; EVP_Digest(similar,sizeof similar-1,dh,&dhn,EVP_sha256(),NULL);
    for (int i=0;i<32;i++) sprintf(dhex+2*i,"%02x",dh[i]);
    check(status(&r)==200 && has(&r,dhex),"false-boundary-prefix-preserved"); free(r.p); free(b.p); b=(Buf){0};
    /* A complete second request after a form must never reach its handler. */
    int before_pipeline=count_handler();
    const char *pipeline="POST /upload HTTP/1.1\r\nHost: localhost\r\nContent-Type: application/x-www-form-urlencoded\r\nContent-Length: 3\r\n\r\na=bPOST /upload HTTP/1.1\r\nHost: localhost\r\nContent-Length: 0\r\n\r\n";
    r=request(pipeline,"",0,0,0); check(status(&r)==200 && count_handler()==before_pipeline+1 && strstr(r.p,"Connection: close"),"form-pipeline-closes-after-first"); free(r.p);

    unsigned char image_samples[][24]={
        {0x89,'P','N','G',13,10,26,10,0,0,0,13,'I','H','D','R',0,0,0,1,0,0,0,1},
        {'G','I','F','8','9','a',1,0,1,0,0x80,0,0,0,0,0,255,255,255,0x3b},
        {0xff,0xd8,0xff,0xe0,0,16,'J','F','I','F',0,1,1,0,0,1,0,1,0,0,0xff,0xd9}};
    const char *image_names[]={"sample.png","sample.gif","sample.jpg"};
    for (int i=0;i<3;i++) {
        part(&b,"B","f",image_names[i],image_samples[i],sizeof image_samples[i],NULL); finish(&b,"B");
        r=post("/upload","multipart/form-data; boundary=B",&b,NULL,0);
        unsigned char hash[32]; unsigned n; char hex[65]; EVP_Digest(image_samples[i],sizeof image_samples[i],hash,&n,EVP_sha256(),NULL);
        for (int j=0;j<32;j++) sprintf(hex+2*j,"%02x",hash[j]);
        check(status(&r)==200 && has(&r,hex) && has(&r,"\"bytes\":24"),image_names[i]); free(r.p); free(b.p); b=(Buf){0};
    }
    /* More than 255 bytes must still produce the documented size error. */
    char longname[601]; memset(longname,'n',600); longname[600]=0;
    part(&b,"B","f",longname,"x",1,NULL); finish(&b,"B"); expected_post("filename-600-size-error","multipart/form-data; boundary=B",&b,413,NULL); free(b.p); b=(Buf){0};
    for (int delta=-1;delta<=1;delta++) {
        Buf h={0}; text(&h,"POST /upload HTTP/1.1\r\nHost: localhost\r\nContent-Type: multipart/form-data; boundary=B\r\nContent-Length: 5\r\n");
        for (int i=0;i<31;i++) { for (int j=0;j<500;j++) text(&h,"k"); text(&h,": "); for (int j=0;j<500;j++) text(&h,"v"); text(&h,"\r\n"); }
        size_t remain=32768+delta-h.n-2;
        while (remain) {
            size_t line=remain>1026?1026:remain;
            if (remain>line && remain-line<5) line-=5;
            size_t content=line-4, key=content>511?511:content;
            for (size_t j=0;j<key;j++) text(&h,"r");
            text(&h,": ");
            for (size_t j=key;j<content;j++) text(&h,"s");
            text(&h,"\r\n"); remain-=line;
        }
        text(&h,"\r\n");
        int before=count_handler(); r=request(h.p,"--B--",5,0,0);
        check(status(&r)==(delta==1?431:200) && count_handler()==before+(delta<=0),"http-header-total-boundary");
        free(r.p); free(h.p);
    }

    const char *protocol_data[]={"Upgrade: websocket\r\n","text/event-stream\r\n"};
    for (int i=0;i<2;i++) {
        size_t len=strlen(protocol_data[i]);
        part(&b,"B","f","protocol.bin",protocol_data[i],len,NULL); finish(&b,"B");
        r=post("/upload","multipart/form-data; boundary=B",&b,NULL,0);
        unsigned char hash[32]; unsigned n; char hex[65]; EVP_Digest(protocol_data[i],len,hash,&n,EVP_sha256(),NULL);
        for (int j=0;j<32;j++) sprintf(hex+2*j,"%02x",hash[j]);
        printf("PROTOCOL_BODY_%s_HTTP=%d\n",i?"SSE":"WS",status(&r));
        check(status(&r)==200 && has(&r,hex) && !files_left(),i?"sse-text-in-upload-is-data":"websocket-text-in-upload-is-data");
        free(r.p); free(b.p); b=(Buf){0};
    }
    r=request("GET /upload HTTP/1.1\r\nHost: localhost\r\nAccept: text/event-stream\r\nConnection: close\r\n\r\n","",0,0,0);
    check(use_tls?status(&r)==404:(status(&r)==200 && has(&r,"SSE_PROBE")),"existing-sse-route-regression");
    free(r.p);

    text(&b,"{\"ok\":true}"); r=post("/upload","application/json",&b,NULL,0); check(status(&r)==200 && has(&r,"\"ok\":true"),"json-regression"); free(r.p); free(b.p); b=(Buf){0};
    text(&b,"ordinary"); r=post("/upload","text/plain",&b,NULL,0); check(status(&r)==200 && has(&r,"ordinary"),"raw-body-regression"); free(r.p); free(b.p);
    printf("L06_%s_PASS=%d FAIL=%d\n",use_tls?"TLS":"HTTP",passes,failures);
    SSL_CTX_free(ctx); return failures?1:0;
}
