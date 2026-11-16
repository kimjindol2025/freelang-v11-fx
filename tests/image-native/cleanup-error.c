/* Fault injection for the existing cleanup contract; no codec/parser substitute. */
#define main image_resources_unused_main
#include "resources.c"
#undef main
static int injected;
extern int __real_unlinkat(int,const char*,int);
int __wrap_unlinkat(int dir,const char *name,int flags){
    if(!injected&&!strncmp(name,".fl-image-",10)){injected=1;errno=EIO;return -1;}
    return __real_unlinkat(dir,name,flags);
}
int main(int argc,char **argv){
    if(argc!=3||chdir(argv[1]))return 2;
    setenv("FL_FILE_BASE",argv[1],1);setenv("FL_IMAGE_WORKER",argv[2],1);
    fl_arena_begin();
    FLValue r=fl_image_process_result(fl_str_val("tiny.png"),outputs("cleanup.png"),options(20000000,5000));
    int visible=error(r,"CLEANUP_FAILED"),clean=temporary_count()==0&&access("cleanup.png",F_OK);
    printf("IMAGE_CLEANUP_ERROR_VISIBLE=%s\n",visible?"PASS":"FAIL");
    printf("IMAGE_CLEANUP_TRANSIENT_RECOVERY=%s\n",clean?"PASS":"FAIL");
    /* Preserve the failing verdict, then remove only this test's own remnants. */
    DIR *d=opendir(".");struct dirent *e;
    if(d){while((e=readdir(d)))if(!strncmp(e->d_name,".fl-image-",10))unlink(e->d_name);closedir(d);}
    unlink("cleanup.png");fl_arena_end();return visible&&clean?0:1;
}
