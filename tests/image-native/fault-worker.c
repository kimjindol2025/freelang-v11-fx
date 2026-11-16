/* Deliberately failing child for parent deadline/cleanup tests, never deployed. */
#include <unistd.h>
#include <sys/file.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/resource.h>
int main(int argc,char **argv){
    if(argc!=7)return 2;
    unsigned pixels=strtoul(argv[1],NULL,10);
    if(pixels==101){write(4,"PARTIAL",7);sleep(2);return 0;}
    if(pixels==102){write(4,"PARTIAL",7);return 2;}
    if(pixels==103){for(int i=0;i<5000;i++)putchar('x');return fflush(stdout)?2:0;}
    if(pixels==104){puts("{}");return 0;}
    if(pixels==105){
        int sentinel=open("sentinel",O_RDWR|O_CREAT,0600);
        if(sentinel<0||flock(sentinel,LOCK_EX|LOCK_NB))return 2;
        usleep(30000);puts("{\"ok\":true,\"error\":\"\",\"data\":{}}");return fflush(stdout)?2:0;
    }
    if(pixels==106){
        if(write(4,"FIRST",5)!=5||write(5,"SECOND",6)!=6)return 2;
        int collision=open("collision.png",O_WRONLY|O_CREAT|O_EXCL,0600);
        if(collision<0||write(collision,"OTHER",5)!=5||close(collision))return 2;
        puts("{\"ok\":true,\"error\":\"\",\"data\":{}}");return fflush(stdout)?2:0;
    }
    if(pixels==107){
        struct rlimit memory,core;
        if(getrlimit(RLIMIT_AS,&memory)||getrlimit(RLIMIT_CORE,&core))return 2;
        printf("{\"ok\":true,\"error\":\"\",\"data\":{\"memory\":%llu,\"core\":%llu}}\n",(unsigned long long)memory.rlim_cur,(unsigned long long)core.rlim_cur);
        return fflush(stdout)?2:0;
    }
    return 2;
}
