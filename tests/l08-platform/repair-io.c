#include "repair-io.h"
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
static void put(const char *path,const void *data,size_t n){FILE *f=fopen(path,"wb");int ok=f&&fwrite(data,1,n,f)==n;if(f&&fclose(f))ok=0;if(!ok)fl_throw(fl_str_val("repair fixture write failed"));}
FLValue repair_test_setup(void){
 unsigned char bytes[256];for(int i=0;i<256;i++)bytes[i]=i;put("all-bytes",bytes,sizeof bytes);put("existing","keep",4);
 const char *outside=getenv("L08_REPAIR_OUTSIDE");if(!outside)fl_throw(fl_str_val("outside fixture path required"));
 char path[4096];snprintf(path,sizeof path,"%s/sentinel",outside);put(path,"outside",7);
 if(symlink(path,"link")||symlink(outside,"link-dir"))fl_throw(fl_str_val("repair fixture link failed"));
 return fl_nil();
}
FLValue repair_test_byte(FLValue data){if(data.tag!=FL_STRING||((FLString*)data.obj)->len!=1)fl_throw(fl_str_val("single raw byte required"));return fl_int((unsigned char)((FLString*)data.obj)->data[0]);}
FLValue repair_test_fds(void){DIR *d=opendir("/proc/self/fd");if(!d)fl_throw(fl_str_val("own descriptor inspection failed"));int n=0;struct dirent *e;while((e=readdir(d))){char *end;long fd=strtol(e->d_name,&end,10);if(*e->d_name&&!*end&&fd>=0&&fd!=dirfd(d))n++;}closedir(d);return fl_int(n);}
FLValue repair_test_mode(FLValue path){struct stat s;if(path.tag!=FL_STRING||stat(((FLString*)path.obj)->data,&s))fl_throw(fl_str_val("fixture stat failed"));return fl_int(s.st_mode&0777);}
FLValue repair_test_umask(FLValue mask){if(mask.tag!=FL_INT)fl_throw(fl_str_val("integer mask required"));return fl_int(umask((mode_t)mask.i));}
