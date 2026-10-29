#include <sys/socket.h>
#include <openssl/ssl.h>
#include <errno.h>
#include <stdio.h>
#include <stdatomic.h>
static atomic_ulong raw_eagain,tls_want;
int __real_accept(int,struct sockaddr*,socklen_t*);
ssize_t __real_send(int,const void*,size_t,int);
int __real_SSL_write(SSL*,const void*,int);
int __wrap_accept(int fd,struct sockaddr*address,socklen_t*length){int client=__real_accept(fd,address,length);if(client>=0){int size=4096;setsockopt(client,SOL_SOCKET,SO_SNDBUF,&size,sizeof size);}return client;}
ssize_t __wrap_send(int fd,const void*data,size_t n,int flags){ssize_t result=__real_send(fd,data,n,flags);if(result<0&&(errno==EAGAIN||errno==EWOULDBLOCK))atomic_fetch_add(&raw_eagain,1);return result;}
int __wrap_SSL_write(SSL*ssl,const void*data,int n){int result=__real_SSL_write(ssl,data,n),saved=errno;if(result<=0){int error=SSL_get_error(ssl,result);if(error==SSL_ERROR_WANT_READ||error==SSL_ERROR_WANT_WRITE)atomic_fetch_add(&tls_want,1);}errno=saved;return result;}
__attribute__((destructor))static void counters(void){fprintf(stderr,"RAW_SEND_EAGAIN_COUNT=%lu\nSSL_WRITE_WANT_COUNT=%lu\n",atomic_load(&raw_eagain),atomic_load(&tls_want));}
