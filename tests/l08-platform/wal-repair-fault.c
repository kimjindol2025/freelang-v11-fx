#include "wal-repair-fault.h"
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
static int mode,faults;
static int same_file(int fd,const char *variable){
 const char *path=getenv(variable);struct stat opened,named;
 return path&&!fstat(fd,&opened)&&!stat(path,&named)&&opened.st_dev==named.st_dev&&opened.st_ino==named.st_ino;
}
FLValue repair_fault_arm(FLValue value){
 if(value.tag!=FL_STRING)fl_throw(fl_str_val("repair fault mode must be string"));
 const char *name=((FLString*)value.obj)->data;
 mode=!strcmp(name,"backup-write")?1:!strcmp(name,"backup-fsync")?2:!strcmp(name,"parent-fsync")?3:
      !strcmp(name,"backup-close")?4:!strcmp(name,"truncate")?5:!strcmp(name,"wal-fsync")?6:!strcmp(name,"wal-close")?7:0;
 if(!mode)fl_throw(fl_str_val("unknown repair fault mode"));faults=0;return fl_nil();
}
ssize_t __real_write(int,const void*,size_t);
ssize_t __wrap_write(int fd,const void *data,size_t length){
 if(same_file(fd,"L08_REPAIR_BACKUP")){
  if(mode==1&&length>1){ssize_t sent=__real_write(fd,data,length/2);if(sent>0)mode=8;return sent;}
  if(mode==8){mode=0;faults++;errno=EIO;return -1;}
 }
 return __real_write(fd,data,length);
}
int __real_fsync(int);
int __wrap_fsync(int fd){
 if((mode==2&&same_file(fd,"L08_REPAIR_BACKUP"))||(mode==3&&same_file(fd,"FL_FILE_BASE"))||(mode==6&&same_file(fd,"AFJ_DB_WAL"))){mode=0;faults++;errno=EIO;return -1;}
 return __real_fsync(fd);
}
int __real_ftruncate(int,off_t);
int __wrap_ftruncate(int fd,off_t length){
 if(mode==5&&same_file(fd,"AFJ_DB_WAL")){mode=0;faults++;errno=EIO;return -1;}
 return __real_ftruncate(fd,length);
}
int __real_close(int);
int __wrap_close(int fd){
 if((mode==4&&same_file(fd,"L08_REPAIR_BACKUP"))||(mode==7&&same_file(fd,"AFJ_DB_WAL"))){mode=0;faults++;int rc=__real_close(fd);if(rc)return rc;errno=EIO;return -1;}
 return __real_close(fd);
}
FLValue repair_fault_count(void){return fl_int(faults);}
