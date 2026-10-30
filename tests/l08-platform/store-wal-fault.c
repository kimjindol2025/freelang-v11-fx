#include "store-wal-fault.h"
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <stdatomic.h>
static _Atomic int mode,closes,faults;
static void faulted(void){
 faults++;
 const char *path=getenv("L08_WAL_FAULT_MARKER");
 if(path){FILE *out=fopen(path,"a");if(!out)abort();if(fputs("FAULT=1\n",out)<0){fclose(out);abort();}if(fclose(out))abort();}
}
static int target(int fd){
 const char *path=getenv("AFJ_DB_WAL");struct stat a,b;
 return path&&!fstat(fd,&a)&&!stat(path,&b)&&a.st_dev==b.st_dev&&a.st_ino==b.st_ino;
}
FLValue store_fault_arm(FLValue name){
 if(name.tag!=FL_STRING)fl_throw(fl_str_val("fault mode must be string"));
 const char *s=((FLString*)name.obj)->data;
 mode=!strcmp(s,"prepare-write")?1:!strcmp(s,"commit-sync")?2:!strcmp(s,"prepare-close")?3:!strcmp(s,"commit-close")?4:!strcmp(s,"prepare-partial")?5:!strcmp(s,"commit-partial")?7:0;
 if(!mode)fl_throw(fl_str_val("unknown WAL fault mode"));closes=faults=0;return fl_nil();
}
ssize_t __real_write(int,const void*,size_t);
ssize_t __wrap_write(int fd,const void *data,size_t n){
 if(target(fd)){
  if((mode==5||(mode==7&&closes==1))&&n>1){ssize_t sent=__real_write(fd,data,n/2);if(sent>0)mode=6;return sent;}
  if(mode==1||mode==6){mode=0;faulted();errno=EIO;return -1;}
 }
 return __real_write(fd,data,n);
}
int __real_fsync(int);
int __wrap_fsync(int fd){if(mode==2&&target(fd)){mode=0;faulted();errno=EIO;return -1;}return __real_fsync(fd);}
int __real_close(int);
int __wrap_close(int fd){
 if((mode==3||mode==4||mode==7)&&target(fd)){
  closes++;
  if(mode==3||(mode==4&&closes==2)){mode=0;faulted();int rc=__real_close(fd);if(rc)return rc;errno=EIO;return -1;}
 }
 return __real_close(fd);
}
FLValue store_fault_count(void){return fl_int(faults);}
FLValue store_fault_open_count(void){
 DIR *d=opendir("/proc/self/fd");if(!d)fl_throw(fl_str_val("cannot inspect own descriptors"));
 int count=0;struct dirent *e;
 while((e=readdir(d))){char *end;long fd=strtol(e->d_name,&end,10);if(*e->d_name&&!*end&&fd>=0&&fd<=INT32_MAX&&target((int)fd))count++;}
 closedir(d);return fl_int(count);
}
