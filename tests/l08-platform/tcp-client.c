#define _GNU_SOURCE
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
typedef struct {int fd;SSL*ssl;} Connection;
static int port,tls,wrong_host;static SSL_CTX*ctx;
static void fail(const char*why){fprintf(stderr,"TCP_CLIENT_FAIL %s\n",why);exit(1);}
static int dial(void){int fd=socket(AF_INET,SOCK_STREAM,0);if(fd<0)fail("socket");struct timeval timeout={10,0};setsockopt(fd,SOL_SOCKET,SO_RCVTIMEO,&timeout,sizeof timeout);setsockopt(fd,SOL_SOCKET,SO_SNDTIMEO,&timeout,sizeof timeout);int one=1;setsockopt(fd,IPPROTO_TCP,TCP_NODELAY,&one,sizeof one);int small=4096;setsockopt(fd,SOL_SOCKET,SO_RCVBUF,&small,sizeof small);struct sockaddr_in address={.sin_family=AF_INET,.sin_port=htons(port),.sin_addr.s_addr=htonl(INADDR_LOOPBACK)};if(connect(fd,(void*)&address,sizeof address))fail("connect");return fd;}
static int open_connection(Connection*c){c->fd=dial();c->ssl=NULL;if(!tls)return 1;c->ssl=SSL_new(ctx);if(!c->ssl)return 0;SSL_set_fd(c->ssl,c->fd);SSL_set_tlsext_host_name(c->ssl,"localhost");SSL_set1_host(c->ssl,wrong_host?"wrong.invalid":"localhost");return SSL_connect(c->ssl)==1;}
static void close_connection(Connection*c){if(c->ssl){SSL_shutdown(c->ssl);SSL_free(c->ssl);}close(c->fd);}
static int send_all(Connection*c,const unsigned char*data,size_t length,size_t chunk){while(length){size_t amount=length<chunk?length:chunk;int n=c->ssl?SSL_write(c->ssl,data,(int)amount):(int)send(c->fd,data,amount,MSG_NOSIGNAL);if(n<0&&errno==EINTR)continue;if(n<=0)return 0;data+=n;length-=(size_t)n;}return 1;}
static int receive(Connection*c,unsigned char*data,size_t length){return c->ssl?SSL_read(c->ssl,data,(int)length):(int)recv(c->fd,data,length,0);}
static int echo(const unsigned char*data,size_t length,int slow){
 Connection c;if(!open_connection(&c)){close_connection(&c);return 0;}unsigned char command[2]={'E','\n'};int ok=send_all(&c,command,2,1)&&send_all(&c,data,length,4093);if(!c.ssl)shutdown(c.fd,SHUT_WR);
 size_t done=0;unsigned char buffer[1024];while(ok&&done<length){size_t amount=length-done<sizeof buffer?length-done:sizeof buffer;int n=receive(&c,buffer,amount);if(n<=0||memcmp(buffer,data+done,(size_t)n)){ok=0;break;}done+=(size_t)n;if(slow){struct timespec pause={0,200000};nanosleep(&pause,NULL);}}
 if(ok&&!c.ssl){int n=receive(&c,buffer,1);ok=n==0;}close_connection(&c);return ok&&done==length;
}
static void test(int ok,const char*name){printf("%s=%s\n",name,ok?"PASS":"FAIL");if(!ok)fail(name);}
static unsigned char*file(const char*root,const char*name,size_t*length){char path[4096];snprintf(path,sizeof path,"%s/%s",root,name);FILE*f=fopen(path,"rb");if(!f)fail("fixture open");if(fseek(f,0,SEEK_END))fail("fixture seek");long size=ftell(f);if(size<0||fseek(f,0,SEEK_SET))fail("fixture size");unsigned char*data=malloc((size_t)size+1);if(!data)fail("fixture allocation");*length=fread(data,1,(size_t)size,f);if(*length!=(size_t)size)fail("fixture read");fclose(f);return data;}
static void command(char command){Connection c;if(!open_connection(&c))fail("command handshake");unsigned char bytes[2]={(unsigned char)command,'\n'};if(!send_all(&c,bytes,2,1))fail("command send");if(command=='D'||command=='S'){unsigned char byte;int n=receive(&c,&byte,1);if(n>0)fail("command unexpected response");}close_connection(&c);}
static void*concurrent(void*unused){(void)unused;unsigned char data[65536];for(size_t i=0;i<sizeof data;i++)data[i]=(unsigned char)i;return(void*)(long)!echo(data,sizeof data,0);}
int main(int argc,char**argv){
 if(argc!=7)return 2;signal(SIGPIPE,SIG_IGN);const char*mode=argv[1];port=atoi(argv[2]);tls=strncmp(mode,"tls",3)==0;wrong_host=!strcmp(mode,"tls-hostname");
 if(!strcmp(mode,"abort")){int fd=dial();unsigned char bytes[2]={0x16,0x03};send(fd,bytes,2,MSG_NOSIGNAL);close(fd);puts("TLS_HANDSHAKE_ABORT=PASS");return 0;}
 if(tls){ctx=SSL_CTX_new(TLS_client_method());if(!ctx||SSL_CTX_load_verify_locations(ctx,argv[4],NULL)!=1)fail("client CA setup");SSL_CTX_set_verify(ctx,SSL_VERIFY_PEER,NULL);if(*argv[5]&&(SSL_CTX_use_certificate_file(ctx,argv[5],SSL_FILETYPE_PEM)!=1||SSL_CTX_use_PrivateKey_file(ctx,argv[6],SSL_FILETYPE_PEM)!=1))fail("client identity setup");}
 unsigned char sample[5]={'A',0,'B',0xff,'C'};
 if(!strcmp(mode,"tls-reject")||wrong_host){test(!echo(sample,sizeof sample,0),"TLS_REJECTED");SSL_CTX_free(ctx);return 0;}
 if(!strcmp(mode,"ping")||!strcmp(mode,"tls-ping")){test(echo(sample,sizeof sample,0),"SERVICE_STILL_ALIVE");if(ctx)SSL_CTX_free(ctx);return 0;}
 test(echo(sample,sizeof sample,0),"NUL_BINARY");test(echo(sample,0,0),"EMPTY_BINARY");unsigned char all[256];for(int i=0;i<256;i++)all[i]=(unsigned char)i;test(echo(all,sizeof all,0),"ALL_256_BYTES");
 size_t length=2u*1024*1024;unsigned char*large=malloc(length);if(!large)fail("payload allocation");for(size_t i=0;i<length;i++)large[i]=(unsigned char)i;test(echo(large,length,1),"LARGE_SLOW_READER");free(large);
 const char*names[]={"image.png","image.jpg","image.gif"};for(int i=0;i<3;i++){size_t size;unsigned char*data=file(argv[3],names[i],&size);test(echo(data,size,0),names[i]);free(data);}
 pthread_t workers[4];for(int i=0;i<4;i++)if(pthread_create(&workers[i],NULL,concurrent,NULL))fail("client thread");int ok=1;for(int i=0;i<4;i++){void*result;pthread_join(workers[i],&result);if(result)ok=0;}test(ok,"CONCURRENT_ECHO");
 command('D');puts("DROP_CONNECTION=PASS");command('X');struct timespec pause={0,50000000};nanosleep(&pause,NULL);test(echo(sample,sizeof sample,0),"HANDLER_ERROR_RECOVERY");command('S');puts("STOP_CONNECTION=PASS");if(ctx)SSL_CTX_free(ctx);return 0;
}
