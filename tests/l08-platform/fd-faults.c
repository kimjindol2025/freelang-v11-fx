#define _GNU_SOURCE
#include <dlfcn.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
static int write_count,read_count,sync_count,close_count,guard=-1;
static int target(int fd){char name[64],path[4096];snprintf(name,sizeof(name),"/proc/self/fd/%d",fd);ssize_t n=readlink(name,path,sizeof(path)-1);if(n<0)return 0;path[n]=0;const char*p=strrchr(path,'/');return p&&!strcmp(p,"/fault.wal");}
static int mode(const char*s){const char*p=getenv("L08_FD_FAULT");return p&&!strcmp(p,s);}
static void active(void){fprintf(stderr,"FAULT_MODE=%s\n",getenv("L08_FD_FAULT"));}
ssize_t write(int fd,const void*p,size_t n){
 ssize_t(*real)(int,const void*,size_t)=dlsym(RTLD_NEXT,"write");
 if(target(fd)){
  if(mode("write-zero")){if(!write_count++)active();return 0;}
  if(mode("write-partial")){if(!write_count++){active();errno=EINTR;return-1;}if(n>2)n=2;}
 }
 return real(fd,p,n);
}
ssize_t read(int fd,void*p,size_t n){
 ssize_t(*real)(int,void*,size_t)=dlsym(RTLD_NEXT,"read");
 if(target(fd)&&!read_count++){
  if(mode("read-eintr")){active();errno=EINTR;return-1;}
  if(mode("read-short")){active();if(n>1)n=1;}
 }
 return real(fd,p,n);
}
int fsync(int fd){
 int(*real)(int)=dlsym(RTLD_NEXT,"fsync");
 if(target(fd)&&!sync_count++){
  if(mode("fsync-eintr")){active();errno=EINTR;return-1;}
  if(mode("fsync-eio")){active();errno=EIO;return-1;}
 }
 return real(fd);
}
int close(int fd){
 int(*real)(int)=dlsym(RTLD_NEXT,"close");
 if(target(fd)&&!close_count++&&(mode("close-eintr")||mode("close-eio"))){
  active();if(real(fd))_exit(91);guard=open("/dev/null",O_RDONLY);if(guard!=fd)_exit(92);
  errno=mode("close-eintr")?EINTR:EIO;return-1;
 }
 return real(fd);
}
__attribute__((destructor)) static void verify_guard(void){if(guard>=0){if(fcntl(guard,F_GETFD)<0){fprintf(stderr,"FD_REUSE_GUARD=FAIL\n");_exit(93);}fprintf(stderr,"FD_REUSE_GUARD=PASS\n");}}
