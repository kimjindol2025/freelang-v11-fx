#define _GNU_SOURCE
#include "runtime.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/resource.h>
#include <dirent.h>
#include <errno.h>
#include <time.h>
#include <png.h>
static int checks;
static void check(int ok,const char *label){printf("IMAGE_RESOURCE_%s=%s\n",label,ok?"PASS":"FAIL");fflush(stdout);if(!ok)exit(1);checks++;}
static const char *text(FLValue v){return v.tag==FL_STRING?((FLString*)v.obj)->data:"";}
static FLValue options(int pixels,int timeout){
    FLValue r=fl_map_new();const char *names[]={"max_pixels","max_bytes","memory_bytes","timeout_ms","jpeg_quality"};
    int values[]={pixels,5242880,536870912,timeout,85};
    for(int i=0;i<5;i++)r=fl_map_set(r,fl_str_val(names[i]),fl_int(values[i]));return r;
}
static FLValue outputs(const char *name){FLValue r=fl_map_new();r=fl_map_set(r,fl_str_val("path"),fl_str_val(name));r=fl_map_set(r,fl_str_val("long_edge"),fl_int(320));return fl_vec_from(&r,1);}
static int error(FLValue r,const char *code){return !fl_truthy(fl_map_get(r,fl_str_val("ok")))&&!strcmp(text(fl_map_get(r,fl_str_val("error"))),code);}
static int descriptors(void){DIR *d=opendir("/proc/self/fd");if(!d)return -1;int count=0;while(readdir(d))count++;closedir(d);return count;}
static int temporary_count(void){DIR *d=opendir(".");if(!d)return -1;int count=0;struct dirent *e;while((e=readdir(d)))if(!strncmp(e->d_name,".fl-image-",10))count++;closedir(d);return count;}
static long milliseconds(void){struct timespec t;clock_gettime(CLOCK_MONOTONIC,&t);return t.tv_sec*1000+t.tv_nsec/1000000;}
static unsigned char *png_pixels(const char *path,png_image *p){
    memset(p,0,sizeof *p);p->version=PNG_IMAGE_VERSION;if(!png_image_begin_read_from_file(p,path))return NULL;
    p->format=PNG_FORMAT_RGBA;unsigned char *data=malloc(PNG_IMAGE_SIZE(*p));if(!data)return NULL;
    if(!png_image_finish_read(p,NULL,data,0,NULL)){free(data);return NULL;}return data;
}
int main(int argc,char **argv){
    if(argc!=4)return 2;check(!chdir(argv[1]),"SETUP");setenv("FL_FILE_BASE",argv[1],1);setenv("FL_IMAGE_WORKER",argv[2],1);fl_arena_begin();
    check(error(fl_image_info_result(fl_str_val("../escape.png"),options(20000000,5000)),"PATH_DENIED"),"INPUT_TRAVERSAL");
    check(error(fl_image_process_result(fl_str_val("alpha.png"),outputs("../escape.png"),options(20000000,5000)),"PATH_DENIED"),"OUTPUT_TRAVERSAL");
    check(error(fl_image_process_result(fl_str_val("alpha.png"),outputs("link-output.png"),options(20000000,5000)),"PATH_DENIED"),"OUTPUT_SYMLINK");
    check(error(fl_image_process_result(fl_str_val("alpha.png"),outputs("protected.png"),options(20000000,5000)),"OUTPUT_EXISTS"),"NO_OVERWRITE");
    check(error(fl_image_info_result(fl_int(1),options(20000000,5000)),"INVALID_ARGUMENT"),"TYPE");
    FLValue low=options(20000000,5000);low=fl_map_set(low,fl_str_val("memory_bytes"),fl_int(1048576));
    FLValue low_result=fl_image_process_result(fl_str_val("alpha.png"),outputs("limited-check.png"),low);
    check(!fl_truthy(fl_map_get(low_result,fl_str_val("ok")))&&*text(fl_map_get(low_result,fl_str_val("error")))&&access("limited-check.png",F_OK),"MEMORY_CLEANUP");
    setenv("FL_IMAGE_WORKER",argv[3],1);long start=milliseconds();
    check(error(fl_image_process_result(fl_str_val("tiny.png"),outputs("timeout.png"),options(101,50)),"TIMEOUT"),"TIMEOUT");
    check(milliseconds()-start<1000&&access("timeout.png",F_OK)&&temporary_count()==0,"TIMEOUT_CLEANUP");
    check(error(fl_image_process_result(fl_str_val("tiny.png"),outputs("exited.png"),options(102,5000)),"WORKER_FAILED")&&access("exited.png",F_OK)&&temporary_count()==0,"EXIT_CLEANUP");
    check(error(fl_image_info_result(fl_str_val("tiny.png"),options(103,5000)),"WORKER_FAILED"),"BOUNDED_REPLY");
    check(error(fl_image_info_result(fl_str_val("tiny.png"),options(104,5000)),"WORKER_FAILED"),"INVALID_REPLY");
    int gate[2];check(!pipe(gate),"CONCURRENCY_SETUP");pid_t peers[4];
    for(int i=0;i<4;i++){
        peers[i]=fork();if(peers[i]==0){close(gate[1]);char c;if(read(gate[0],&c,1)!=1)_exit(2);
            FLValue r=fl_image_info_result(fl_str_val("tiny.png"),options(105,5000));_exit(fl_truthy(fl_map_get(r,fl_str_val("ok")))?0:1);}
        check(peers[i]>0,"CONCURRENCY_FORK");
    }
    close(gate[0]);check(write(gate[1],"xxxx",4)==4,"CONCURRENCY_RELEASE");close(gate[1]);
    for(int i=0;i<4;i++){int status;check(waitpid(peers[i],&status,0)==peers[i]&&WIFEXITED(status)&&!WEXITSTATUS(status),"SINGLE_WORKER");}
    errno=0;int status;check(waitpid(-1,&status,WNOHANG)==-1&&errno==ECHILD,"NO_CHILDREN");
    setenv("FL_IMAGE_WORKER",argv[2],1);
    FLValue alpha=fl_image_process_result(fl_str_val("alpha.png"),outputs("alpha-verify.png"),options(20000000,5000));
    check(fl_truthy(fl_map_get(alpha,fl_str_val("ok"))),"ALPHA_PROCESS");
    png_image original,resized;unsigned char *a=png_pixels("alpha.png",&original),*b=png_pixels("alpha-verify.png",&resized);int same=a&&b;
    if(same)for(unsigned y=0;y<resized.height;y++)for(unsigned x=0;x<resized.width;x++)if(memcmp(b+((size_t)y*resized.width+x)*4,a+((size_t)(y*original.height/resized.height)*original.width+x*original.width/resized.width)*4,4))same=0;
    check(same,"ALPHA_PIXELS");free(a);free(b);png_image_free(&original);png_image_free(&resized);
    FLValue first=fl_image_process_result(fl_str_val("animated.gif"),outputs("first-verify.png"),options(20000000,5000));
    check(fl_truthy(fl_map_get(first,fl_str_val("ok"))),"GIF_FIRST_PROCESS");
    png_image frame;unsigned char *f=png_pixels("first-verify.png",&frame);int correct=f&&frame.width==20&&frame.height==10&&f[0]==255&&f[1]==0&&f[3]==255&&f[7]==0;
    check(correct,"GIF_FIRST_PIXELS");free(f);png_image_free(&frame);
    int fd_before=descriptors();fl_arena_end();
    for(int i=0;i<200;i++){fl_arena_begin();FLValue r=fl_image_info_result(fl_str_val("tiny.png"),options(20000000,5000));int ok=fl_truthy(fl_map_get(r,fl_str_val("ok")));fl_arena_end();if(!ok)return 1;}
    check(descriptors()==fd_before&&temporary_count()==0,"REPEAT_FDS_AND_TEMPS");
    errno=0;check(waitpid(-1,&status,WNOHANG)==-1&&errno==ECHILD,"REPEAT_NO_CHILDREN");
    printf("IMAGE_RESOURCE_PASS=%d FAIL=0 REPEAT=200 CONCURRENT=4\n",checks);return 0;
}
