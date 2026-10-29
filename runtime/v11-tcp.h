/* Native raw TCP/TLS. Pollers share the timer callback dispatcher. */
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <openssl/ssl.h>
#include <openssl/pem.h>
#include <openssl/err.h>
#define VT_SERVERS 16
#define VT_CLIENTS 512
#define VT_CLOSES 1024
#define VT_ERRORS 128
#define VT_OUTPUT (64u*1024*1024)
typedef struct {int used,fd,port;uint64_t generation;FLValue handler;SSL_CTX*ctx;} VTServer;
typedef struct {int used,fd,server,ready,announced,eof,read_want,write_want,buffered;uint64_t generation,id;int64_t deadline;SSL*ssl;FLValue handler;char peer[INET6_ADDRSTRLEN];unsigned char*out;size_t length,offset,retry;} VTClient;
typedef struct {FLValue handler;uint64_t id;char peer[INET6_ADDRSTRLEN];} VTClose;
typedef struct {char source[32],message[1024];} VTError;
static VTServer vt_servers[VT_SERVERS];static VTClient vt_clients[VT_CLIENTS];
static VTClose vt_closes[VT_CLOSES];static unsigned vt_close_head,vt_close_count,vt_live;
static VTError vt_errors[VT_ERRORS];static unsigned vt_error_count;
static uint64_t vt_generation,vt_id;static size_t vt_output;
static pthread_mutex_t vt_mutex=PTHREAD_MUTEX_INITIALIZER;
static pthread_once_t vt_once=PTHREAD_ONCE_INIT;
static int64_t vt_now(void){struct timespec value;if(clock_gettime(CLOCK_MONOTONIC,&value))return -1;return(int64_t)value.tv_sec*1000+value.tv_nsec/1000000;}
static void vt_init(void){signal(SIGPIPE,SIG_IGN);}
static void vt_signal(void){(void)fl_schedule_wake_fd();if(timer_pipe[1]>=0){char byte=1;(void)write(timer_pipe[1],&byte,1);}}
static void vt_error_locked(const char*source,const char*message){
 unsigned index=vt_error_count<VT_ERRORS?vt_error_count++:VT_ERRORS-1;
 if(index==VT_ERRORS-1&&vt_error_count==VT_ERRORS){source="queue-overflow";message="native I/O error queue limit reached";}
 snprintf(vt_errors[index].source,sizeof vt_errors[index].source,"%s",source);snprintf(vt_errors[index].message,sizeof vt_errors[index].message,"%s",message);
}
static void vt_error(const char*source,const char*message){pthread_mutex_lock(&vt_mutex);vt_error_locked(source,message);pthread_mutex_unlock(&vt_mutex);}
static void vt_system_error_locked(const char*action,int number){char message[192];snprintf(message,sizeof message,"%s: %s",action,strerror(number));vt_error_locked("listen",message);}
static FLValue vt_string(FLValue value,const char*fallback){return fl_v11_utf8_string(value.tag==FL_NIL?fl_str_val(fallback):value);}
static const char*vt_bytes(FLValue value){return value.tag==FL_STRING?((FLString*)value.obj)->data:"";}
static int vt_port(FLValue value){
 FLValue number=fl_v11_to_number(value.tag==FL_NIL?fl_int(30390):value);double n=number.f;
 if(!isfinite(n)||floor(n)!=n||n<0||n>65535){fl_throw(fl_make_error("RangeError","TCP port must be an integer from 0 to 65535"));return -1;}return(int)n;
}
static void vt_close_locked(int slot,int notify){
 VTClient*c=&vt_clients[slot];if(!c->used)return;
 if(notify&&c->announced){VTClose*event=&vt_closes[(vt_close_head+vt_close_count)%VT_CLOSES];event->handler=c->handler;event->id=c->id;memcpy(event->peer,c->peer,sizeof event->peer);vt_close_count++;}
 else fl_heap_release(c->handler);
 vt_output-=c->length-c->offset;
 if(c->ssl){if(c->ready&&c->offset==c->length&&!c->retry)SSL_shutdown(c->ssl);SSL_free(c->ssl);ERR_clear_error();}free(c->out);
 close(c->fd);memset(c,0,sizeof(*c));vt_live--;
}
static void vt_stop_locked(int slot,int notify){
 VTServer*s=&vt_servers[slot];for(int i=0;i<VT_CLIENTS;i++)if(vt_clients[i].used&&vt_clients[i].server==slot)vt_close_locked(i,notify);
 close(s->fd);fl_heap_release(s->handler);if(s->ctx)SSL_CTX_free(s->ctx);memset(s,0,sizeof(*s));
}
void fl_io_shutdown(void){
 pthread_mutex_lock(&vt_mutex);for(int i=0;i<VT_SERVERS;i++)if(vt_servers[i].used)vt_stop_locked(i,0);
 while(vt_close_count){VTClose*event=&vt_closes[vt_close_head];fl_heap_release(event->handler);memset(event,0,sizeof(*event));vt_close_head=(vt_close_head+1)%VT_CLOSES;vt_close_count--;}
 pthread_mutex_unlock(&vt_mutex);
}
int fl_io_active(void){pthread_mutex_lock(&vt_mutex);int active=vt_close_count||vt_live;for(int i=0;i<VT_SERVERS&&!active;i++)active=vt_servers[i].used;pthread_mutex_unlock(&vt_mutex);return active;}
static int vt_no_password(char*buffer,int size,int writing,void*userdata){(void)buffer;(void)size;(void)writing;(void)userdata;return 0;}
static SSL_CTX*vt_tls(FLValue cert_path,FLValue key_path,FLValue ca_path,int require_client){
 FLValue cert=fl_nil(),key=fl_nil(),ca=fl_nil();if(fl_try_top>=FL_TRY_MAX)return NULL;
 FLTryFrame*frame=&fl_try_stack[fl_try_top++];
 if(setjmp(frame->buf)==0){
  FLValue a[1]={cert_path};cert=fl_v11_file(fl_str_val("file_read"),fl_vec_from(a,1));
  if(ca_path.tag==FL_STRING&&((FLString*)ca_path.obj)->len){a[0]=ca_path;ca=fl_v11_file(fl_str_val("file_read"),fl_vec_from(a,1));}
  a[0]=key_path;key=fl_v11_file(fl_str_val("file_read"),fl_vec_from(a,1));
  fl_try_top--;
 }else{fl_try_top--;return NULL;}
 FLString*c=(FLString*)cert.obj,*k=(FLString*)key.obj;
 SSL_CTX*ctx=SSL_CTX_new(TLS_server_method());if(!ctx){OPENSSL_cleanse(k->data,k->len);return NULL;}SSL_CTX_set_min_proto_version(ctx,TLS1_2_VERSION);
 SSL_CTX_set_mode(ctx,SSL_MODE_ENABLE_PARTIAL_WRITE|SSL_MODE_ACCEPT_MOVING_WRITE_BUFFER);
 BIO*cb=BIO_new_mem_buf(c->data,(int)c->len),*kb=BIO_new_mem_buf(k->data,(int)k->len);X509*leaf=cb?PEM_read_bio_X509(cb,NULL,NULL,NULL):NULL;EVP_PKEY*private_key=kb?PEM_read_bio_PrivateKey(kb,NULL,vt_no_password,NULL):NULL;
 int ok=leaf&&private_key&&SSL_CTX_use_certificate(ctx,leaf)==1&&SSL_CTX_use_PrivateKey(ctx,private_key)==1&&SSL_CTX_check_private_key(ctx)==1;
 if(leaf)X509_free(leaf);if(private_key)EVP_PKEY_free(private_key);
 if(ok){X509*extra;while((extra=PEM_read_bio_X509(cb,NULL,NULL,NULL))!=NULL){if(SSL_CTX_add_extra_chain_cert(ctx,extra)!=1){X509_free(extra);ok=0;break;}}}
 BIO_free(cb);BIO_free(kb);OPENSSL_cleanse(k->data,k->len);ERR_clear_error();
 if(ca.tag==FL_STRING){FLString*text=(FLString*)ca.obj;BIO*bio=BIO_new_mem_buf(text->data,(int)text->len);X509*root;int count=0;while(bio&&(root=PEM_read_bio_X509(bio,NULL,NULL,NULL))!=NULL){if(X509_STORE_add_cert(SSL_CTX_get_cert_store(ctx),root)==1)count++;X509_free(root);}BIO_free(bio);ERR_clear_error();if(!count)ok=0;}
 else if(require_client)ok=0;
 SSL_CTX_set_verify(ctx,require_client?SSL_VERIFY_PEER|SSL_VERIFY_FAIL_IF_NO_PEER_CERT:SSL_VERIFY_NONE,NULL);
 if(!ok){SSL_CTX_free(ctx);return NULL;}return ctx;
}
static int vt_start(int port,FLValue handler,FLValue host,SSL_CTX*ctx){
 char service[16];snprintf(service,sizeof service,"%d",port);struct addrinfo hints={.ai_family=AF_UNSPEC,.ai_socktype=SOCK_STREAM,.ai_flags=AI_PASSIVE},*addresses=NULL;
 if(getaddrinfo(vt_bytes(host),service,&hints,&addresses)){vt_error("listen","host resolution failed");if(ctx)SSL_CTX_free(ctx);return 0;}
 pthread_once(&vt_once,vt_init);(void)fl_schedule_wake_fd();
 pthread_mutex_lock(&vt_mutex);int slot=-1;
 for(int i=0;i<VT_SERVERS;i++){if(vt_servers[i].used&&vt_servers[i].port==port){pthread_mutex_unlock(&vt_mutex);freeaddrinfo(addresses);if(ctx)SSL_CTX_free(ctx);return 2;}if(!vt_servers[i].used&&slot<0)slot=i;}
 if(slot<0){vt_error_locked("listen","server limit reached");pthread_mutex_unlock(&vt_mutex);freeaddrinfo(addresses);if(ctx)SSL_CTX_free(ctx);return 0;}
 int fd=-1,saved=EADDRNOTAVAIL;
 for(struct addrinfo*a=addresses;a;a=a->ai_next){fd=socket(a->ai_family,a->ai_socktype,a->ai_protocol);if(fd<0){saved=errno;continue;}fcntl(fd,F_SETFL,O_NONBLOCK);fcntl(fd,F_SETFD,FD_CLOEXEC);int one=1;setsockopt(fd,SOL_SOCKET,SO_REUSEADDR,&one,sizeof one);if(bind(fd,a->ai_addr,a->ai_addrlen)==0&&listen(fd,128)==0)break;saved=errno;close(fd);fd=-1;}
 freeaddrinfo(addresses);
 if(fd<0){vt_system_error_locked("bind/listen",saved);pthread_mutex_unlock(&vt_mutex);if(ctx)SSL_CTX_free(ctx);return 0;}
 FLValue owned=fl_heap_copy(handler);if(owned.tag!=FL_STRING){close(fd);pthread_mutex_unlock(&vt_mutex);if(ctx)SSL_CTX_free(ctx);return 0;}
 vt_servers[slot]=(VTServer){.used=1,.fd=fd,.port=port,.generation=++vt_generation,.handler=owned,.ctx=ctx};pthread_mutex_unlock(&vt_mutex);vt_signal();return 1;
}
static void vt_flush_locked(int slot){
 VTClient*c=&vt_clients[slot];size_t budget=65536;
 while(c->used&&c->offset<c->length&&budget){
  size_t amount=c->length-c->offset;if(amount>budget)amount=budget;if(c->ssl&&c->retry)amount=c->retry;
  int n;
  if(c->ssl){ERR_clear_error();n=SSL_write(c->ssl,c->out+c->offset,(int)amount);if(n<=0){int error=SSL_get_error(c->ssl,n);if(error==SSL_ERROR_WANT_READ||error==SSL_ERROR_WANT_WRITE){c->retry=amount;c->write_want=error==SSL_ERROR_WANT_READ?POLLIN:POLLOUT;return;}vt_error_locked("write","TLS write failed");vt_close_locked(slot,1);return;}c->retry=0;c->write_want=POLLOUT;}
  else{n=(int)send(c->fd,c->out+c->offset,amount,MSG_NOSIGNAL);if(n<0&&errno==EINTR)continue;if(n<0&&(errno==EAGAIN||errno==EWOULDBLOCK))return;if(n<=0){vt_error_locked("write","TCP write failed");vt_close_locked(slot,1);return;}}
  c->offset+=(size_t)n;vt_output-=(size_t)n;budget=(size_t)n>=budget?0:budget-(size_t)n;
 }
 if(c->used&&c->offset==c->length){free(c->out);c->out=NULL;c->offset=c->length=c->retry=0;if(c->eof)vt_close_locked(slot,1);}
}
static int vt_emit(FLValue handler,const char*event,uint64_t id,const char*peer,const char*data,size_t length){
 void*volatile previous=NULL;volatile int entered=0;int success=0;
 if(fl_try_top>=FL_TRY_MAX){vt_error("handler","exception frame limit exceeded");fl_heap_release(handler);return 0;}
 FLTryFrame*frame=&fl_try_stack[fl_try_top++];
 if(setjmp(frame->buf)==0){
  previous=fl_arena_scope_enter();entered=1;char name[64];snprintf(name,sizeof name,"conn_%llu",(unsigned long long)id);
  FLValue fn=fl_named_fn(handler);FLValue args[4]={fl_str_val(event),fl_str_val(name),fl_v11_latin1_decode(data,length),fl_str_val(peer)};
  fl_fn_call(fn,4,args);fl_try_top--;success=1;
 }else{
  fl_try_top--;FLValue err=frame->err;FLValue text=fl_v11_builtin(fl_str_val("error-message"),fl_vec_from(&err,1));vt_error("handler",vt_bytes(text));
 }
 if(entered)fl_arena_scope_leave(previous);fl_heap_release(handler);return success;
}
static int vt_drain_closes(void){int count=0;for(int limit=0;limit<VT_CLOSES;limit++){
 pthread_mutex_lock(&vt_mutex);if(!vt_close_count){pthread_mutex_unlock(&vt_mutex);break;}VTClose event=vt_closes[vt_close_head];memset(&vt_closes[vt_close_head],0,sizeof event);vt_close_head=(vt_close_head+1)%VT_CLOSES;vt_close_count--;pthread_mutex_unlock(&vt_mutex);
 count+=vt_emit(event.handler,"close",event.id,event.peer,"",0);
 }return count;}
typedef struct {struct pollfd fd;int kind,index;uint64_t generation;} VTPoll;
static int vt_poll(int wait_ms,int external,int*external_events){
 pthread_once(&timer_once,timer_init);VTPoll snapshot[VT_SERVERS+VT_CLIENTS+2];struct pollfd descriptors[VT_SERVERS+VT_CLIENTS+2];int n=0,timeout=wait_ms<0?0:wait_ms;int64_t now=vt_now();if(now<0){vt_error("clock","monotonic clock failed");return 0;}
 pthread_mutex_lock(&vt_mutex);if(vt_close_count)timeout=0;
 for(int i=0;i<VT_SERVERS;i++)if(vt_servers[i].used){snapshot[n]=(VTPoll){{vt_servers[i].fd,POLLIN,0},0,i,vt_servers[i].generation};n++;}
 for(int i=0;i<VT_CLIENTS;i++)if(vt_clients[i].used){VTClient*c=&vt_clients[i];short events;
  if(!c->ready){events=(short)c->read_want;int64_t remain=c->deadline-now;if(remain<timeout)timeout=remain<0?0:(int)remain;}
  else{events=c->eof?0:(short)c->read_want;if(c->length>c->offset)events|=(short)c->write_want;if(c->ssl&&(SSL_pending(c->ssl)>0||c->buffered))timeout=0;}
  snapshot[n]=(VTPoll){{c->fd,events,0},1,i,c->generation};n++;
 }
 pthread_mutex_unlock(&vt_mutex);
 snapshot[n++]=(VTPoll){{timer_pipe[0],POLLIN,0},2,0,0};if(external>=0)snapshot[n++]=(VTPoll){{external,POLLIN,0},3,0,0};
 for(int i=0;i<n;i++)descriptors[i]=snapshot[i].fd;
 int result=poll(descriptors,(nfds_t)n,timeout);if(result<0)return 0;
 pthread_mutex_lock(&timer_dispatch);int count=vt_drain_closes();
 for(int p=0;p<n;p++){
  VTPoll item=snapshot[p];short ready=descriptors[p].revents;
  if(item.kind==3){if(external_events)*external_events=ready;continue;}if(item.kind==2)continue;
  if(item.kind==0){if(!(ready&POLLIN))continue;
   for(int accepted=0;accepted<64;accepted++){
    int64_t accepted_at=vt_now();if(accepted_at<0){vt_error("clock","monotonic clock failed");break;}
    pthread_mutex_lock(&vt_mutex);VTServer*s=&vt_servers[item.index];if(!s->used||s->generation!=item.generation){pthread_mutex_unlock(&vt_mutex);break;}
    struct sockaddr_storage address;socklen_t size=sizeof address;int fd=accept(s->fd,(void*)&address,&size);if(fd<0){pthread_mutex_unlock(&vt_mutex);if(errno==EINTR)continue;break;}
    int slot=-1;for(int i=0;i<VT_CLIENTS;i++)if(!vt_clients[i].used){slot=i;break;}
    if(slot<0||vt_live+vt_close_count>=VT_CLOSES){close(fd);vt_error_locked("accept","connection limit reached");pthread_mutex_unlock(&vt_mutex);continue;}
    fcntl(fd,F_SETFL,O_NONBLOCK);fcntl(fd,F_SETFD,FD_CLOEXEC);int one=1;setsockopt(fd,IPPROTO_TCP,TCP_NODELAY,&one,sizeof one);
    VTClient*c=&vt_clients[slot];memset(c,0,sizeof(*c));c->fd=fd;c->used=1;c->server=item.index;c->generation=++vt_generation;c->id=++vt_id;c->read_want=POLLIN;c->write_want=POLLOUT;c->handler=fl_heap_copy(s->handler);vt_live++;
    if(getnameinfo((void*)&address,size,c->peer,sizeof c->peer,NULL,0,NI_NUMERICHOST))strcpy(c->peer,"unknown");
    c->ready=s->ctx==NULL;c->announced=c->ready;c->deadline=accepted_at+10000;
    if(s->ctx){c->ssl=SSL_new(s->ctx);if(!c->ssl){vt_close_locked(slot,0);pthread_mutex_unlock(&vt_mutex);continue;}SSL_set_fd(c->ssl,fd);SSL_set_accept_state(c->ssl);}
    FLValue handler=c->ready?fl_heap_copy(c->handler):fl_nil();uint64_t id=c->id;char peer[INET6_ADDRSTRLEN];memcpy(peer,c->peer,sizeof peer);pthread_mutex_unlock(&vt_mutex);
    if(handler.tag==FL_STRING)count+=vt_emit(handler,"connect",id,peer,"",0);
   }continue;
  }
  int64_t checked_at=vt_now();if(checked_at<0){vt_error("clock","monotonic clock failed");break;}
  pthread_mutex_lock(&vt_mutex);VTClient*c=&vt_clients[item.index];if(!c->used||c->generation!=item.generation){pthread_mutex_unlock(&vt_mutex);continue;}
  if(ready&POLLNVAL){vt_error_locked("read","invalid socket");vt_close_locked(item.index,1);pthread_mutex_unlock(&vt_mutex);continue;}
  if(!c->ready){
   if(checked_at>=c->deadline){vt_error_locked("tls","TLS handshake timeout");vt_close_locked(item.index,0);pthread_mutex_unlock(&vt_mutex);continue;}
   if(!ready){pthread_mutex_unlock(&vt_mutex);continue;}
   ERR_clear_error();int accepted=SSL_accept(c->ssl);
   if(accepted!=1){int error=SSL_get_error(c->ssl,accepted);if(error==SSL_ERROR_WANT_READ||error==SSL_ERROR_WANT_WRITE){c->read_want=error==SSL_ERROR_WANT_READ?POLLIN:POLLOUT;pthread_mutex_unlock(&vt_mutex);continue;}vt_error_locked("tls","TLS handshake failed");vt_close_locked(item.index,0);pthread_mutex_unlock(&vt_mutex);continue;}
   c->ready=c->announced=1;c->read_want=POLLIN;FLValue handler=fl_heap_copy(c->handler);uint64_t id=c->id;char peer[INET6_ADDRSTRLEN];memcpy(peer,c->peer,sizeof peer);pthread_mutex_unlock(&vt_mutex);count+=vt_emit(handler,"connect",id,peer,"",0);continue;
  }
  if(c->ssl&&(SSL_pending(c->ssl)>0||c->buffered))ready|=POLLIN;
  if((ready&c->write_want)&&c->length>c->offset)vt_flush_locked(item.index);
  if(!c->used||c->generation!=item.generation){pthread_mutex_unlock(&vt_mutex);continue;}
  if(!c->eof&&(ready&(c->read_want|POLLHUP|POLLERR))&&!(c->retry&&c->write_want==POLLIN)){
   char data[65536];int received;
   if(c->ssl){ERR_clear_error();received=SSL_read(c->ssl,data,sizeof data);if(received<=0){int error=SSL_get_error(c->ssl,received);c->buffered=0;if(error==SSL_ERROR_WANT_READ||error==SSL_ERROR_WANT_WRITE){c->read_want=error==SSL_ERROR_WANT_READ?POLLIN:POLLOUT;pthread_mutex_unlock(&vt_mutex);continue;}if(error!=SSL_ERROR_ZERO_RETURN)vt_error_locked("tls","TLS read closed");received=0;}else{c->read_want=POLLIN;c->buffered=SSL_has_pending(c->ssl);}}
   else{received=(int)recv(c->fd,data,sizeof data,0);if(received<0&&(errno==EINTR||errno==EAGAIN||errno==EWOULDBLOCK)){pthread_mutex_unlock(&vt_mutex);continue;}}
   if(received>0){FLValue handler=fl_heap_copy(c->handler);uint64_t id=c->id;char peer[INET6_ADDRSTRLEN];memcpy(peer,c->peer,sizeof peer);pthread_mutex_unlock(&vt_mutex);count+=vt_emit(handler,"data",id,peer,data,(size_t)received);continue;}
   c->eof=1;if(c->offset==c->length)vt_close_locked(item.index,1);
  }
  pthread_mutex_unlock(&vt_mutex);
 }
 count+=vt_drain_closes();pthread_mutex_unlock(&timer_dispatch);return count;
}
int fl_io_poll(int wait_ms){return vt_poll(wait_ms,-1,NULL);}
int fl_io_wait(int fd,int wait_ms){int events=0;(void)vt_poll(wait_ms,fd,&events);return events;}
FLValue fl_v11_io(FLValue name,FLValue argv){
 const char*op=vt_bytes(name);FLVector*args=(FLVector*)argv.obj;size_t n=args->len;FLValue a=n?args->data[0]:fl_nil(),b=n>1?args->data[1]:fl_nil();
 if(!strcmp(op,"fl-error-queue-size")){pthread_mutex_lock(&vt_mutex);unsigned count=vt_error_count;pthread_mutex_unlock(&vt_mutex);return fl_int(count);}
 if(!strcmp(op,"fl-error-drain")){
  VTError errors[VT_ERRORS];pthread_mutex_lock(&vt_mutex);unsigned count=vt_error_count;memcpy(errors,vt_errors,count*sizeof(*errors));vt_error_count=0;pthread_mutex_unlock(&vt_mutex);
  FLValue values[VT_ERRORS];for(unsigned i=0;i<count;i++){FLValue fields[4]={fl_str_val("io-err"),fl_str_val("recoverable"),fl_str_val(errors[i].source),fl_str_val(errors[i].message)};values[i]=fl_vec_from(fields,4);}return fl_vec_from(values,count);
 }
 if(!strcmp(op,"tcp-write")||!strcmp(op,"tcp-drop")){
  FLValue text=vt_string(a,"");const char*id=vt_bytes(text);char*end=NULL;uint64_t number=strncmp(id,"conn_",5)?0:strtoull(id+5,&end,10);
  if(!number||!end||*end||((FLString*)text.obj)->len!=strlen(id))return fl_str_val("not-found");
  FLValue bytes=!strcmp(op,"tcp-write")?fl_v11_latin1_encode(b.tag==FL_NIL?fl_str_val(""):b):fl_nil();
  pthread_mutex_lock(&vt_mutex);int slot=-1;for(int i=0;i<VT_CLIENTS;i++)if(vt_clients[i].used&&vt_clients[i].id==number){slot=i;break;}
  if(slot<0){pthread_mutex_unlock(&vt_mutex);return fl_str_val("not-found");}
  if(!strcmp(op,"tcp-drop")){vt_close_locked(slot,1);pthread_mutex_unlock(&vt_mutex);vt_signal();return fl_str_val("ok");}
  VTClient*c=&vt_clients[slot];FLString*data=(FLString*)bytes.obj;size_t remaining=c->length-c->offset;
  if(!c->ready||data->len>VT_OUTPUT-vt_output){vt_error_locked("write","write buffer unavailable or limit reached");pthread_mutex_unlock(&vt_mutex);return fl_str_val("error");}
  if(c->offset){memmove(c->out,c->out+c->offset,remaining);c->length=remaining;c->offset=0;}
  if(data->len){unsigned char*buffer=realloc(c->out,remaining+data->len);if(!buffer){vt_error_locked("write","write allocation failed");pthread_mutex_unlock(&vt_mutex);return fl_str_val("error");}c->out=buffer;memcpy(buffer+remaining,data->data,data->len);c->length=remaining+data->len;vt_output+=data->len;}
  vt_flush_locked(slot);int ok=c->used;pthread_mutex_unlock(&vt_mutex);vt_signal();return fl_str_val(ok?"ok":"error");
 }
 if(strcmp(op,"tcp-server-raw")&&strcmp(op,"tcp-server-tls")&&strcmp(op,"tcp-server-stop")&&strcmp(op,"tcp-server-running?")){fl_throw(fl_make_error("Error","unknown native I/O operation"));return fl_nil();}
 int port=vt_port(a);
 if(!strcmp(op,"tcp-server-stop")||!strcmp(op,"tcp-server-running?")){
  pthread_mutex_lock(&vt_mutex);int slot=-1;for(int i=0;i<VT_SERVERS;i++)if(vt_servers[i].used&&vt_servers[i].port==port){slot=i;break;}
  if(!strcmp(op,"tcp-server-running?")){pthread_mutex_unlock(&vt_mutex);return fl_bool(slot>=0);}
  if(slot>=0)vt_stop_locked(slot,1);pthread_mutex_unlock(&vt_mutex);vt_signal();return fl_str_val(slot>=0?"ok":"not-running");
 }
 FLValue handler=vt_string(b,""),host=vt_string(n>(!strcmp(op,"tcp-server-tls")?4:2)?args->data[!strcmp(op,"tcp-server-tls")?4:2]:fl_nil(),"0.0.0.0");
 if(((FLString*)handler.obj)->len!=strlen(vt_bytes(handler))||((FLString*)host.obj)->len!=strlen(vt_bytes(host))){vt_error("listen","invalid host or handler");return fl_str_val("error");}
 pthread_mutex_lock(&vt_mutex);int existing=0;for(int i=0;i<VT_SERVERS;i++)if(vt_servers[i].used&&vt_servers[i].port==port)existing=1;pthread_mutex_unlock(&vt_mutex);if(existing)return fl_str_val("already-running");
 SSL_CTX*ctx=NULL;if(!strcmp(op,"tcp-server-tls")){ctx=vt_tls(n>2?args->data[2]:fl_nil(),n>3?args->data[3]:fl_nil(),n>5?args->data[5]:fl_nil(),n>6&&fl_v11_truthy(args->data[6]));if(!ctx){vt_error("tls","TLS configuration failed");return fl_str_val("error");}}
 int started=vt_start(port,handler,host,ctx);return fl_str_val(started==1?"ok":started==2?"already-running":"error");
}
