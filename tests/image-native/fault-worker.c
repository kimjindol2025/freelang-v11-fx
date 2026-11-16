/* Deliberately failing child for parent deadline/cleanup tests, never deployed. */
#include <unistd.h>
#include <sys/file.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
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
    return 2;
}
