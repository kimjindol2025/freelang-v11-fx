/* Test-only publication of the port chosen atomically by bind(port=0). */
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
int __real_listen(int,int);
int __wrap_listen(int fd,int backlog) {
    int rc=__real_listen(fd,backlog);
    if(!rc){struct sockaddr_in a;socklen_t n=sizeof(a);const char *path=getenv("L08_PORT_FILE");
        if(path&&!getsockname(fd,(struct sockaddr*)&a,&n)){FILE*f=fopen(path,"w");if(!f)exit(1);fprintf(f,"%u\n",ntohs(a.sin_port));fclose(f);}}
    return rc;
}
