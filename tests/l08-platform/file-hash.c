#include "file-hash.h"
#include <sys/stat.h>
#include <sys/resource.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
static char mode[32];
static int calls;
static int target(int fd){
 const char *path=getenv("L08_HASH_TARGET");struct stat a,b;
 return path&&!fstat(fd,&a)&&!stat(path,&b)&&a.st_dev==b.st_dev&&a.st_ino==b.st_ino;
}
FLValue hash_test_arm(FLValue value){
 if(value.tag!=FL_STRING||!value.obj)abort();
 const FLString *s=(const FLString*)value.obj;if(s->len>=sizeof mode)abort();
 memcpy(mode,s->data,s->len);mode[s->len]=0;calls=0;return fl_nil();
}
FLValue hash_test_fds(void){
 DIR *dir=opendir("/proc/self/fd");if(!dir)abort();int count=0;struct dirent *entry;
 while((entry=readdir(dir)))if(entry->d_name[0]!='.')count++;
 if(closedir(dir))abort();return fl_int(count);
}
FLValue hash_test_rss(void){struct rusage r;if(getrusage(RUSAGE_SELF,&r))abort();return fl_int(r.ru_maxrss);}
ssize_t __real_read(int,void*,size_t);
ssize_t __wrap_read(int fd,void *buf,size_t size){
 if(!*mode||!target(fd))return __real_read(fd,buf,size);
 calls++;
 if(!strcmp(mode,"eintr")&&calls==1){errno=EINTR;return -1;}
 if(!strcmp(mode,"read")&&calls>1){errno=EIO;return -1;}
 if(!strcmp(mode,"short")||!strcmp(mode,"read")||!strcmp(mode,"truncate"))if(size>7)size=7;
 ssize_t n=__real_read(fd,buf,size);
 if(calls==1&&n>0&&(!strcmp(mode,"grow")||!strcmp(mode,"rewrite")||!strcmp(mode,"truncate"))){
  int out=open(getenv("L08_HASH_TARGET"),O_WRONLY|O_CLOEXEC);if(out<0)abort();
  if(!strcmp(mode,"truncate")){if(ftruncate(out,0))abort();}
  else {if(!strcmp(mode,"grow")&&lseek(out,0,SEEK_END)<0)abort();if(write(out,"!",1)!=1)abort();}
  if(close(out))abort();
 }
 return n;
}
int __real_close(int);
int __wrap_close(int fd){
 if(!strcmp(mode,"close")&&target(fd)){mode[0]=0;int r=__real_close(fd);if(r)return r;errno=EIO;return -1;}
 return __real_close(fd);
}
