/* Keep native application tests on loopback, with the OS choosing port 0. */
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
int __real_bind(int,const struct sockaddr*,socklen_t);
int __wrap_bind(int fd,const struct sockaddr*address,socklen_t length){
    if(address&&address->sa_family==AF_INET&&length>=sizeof(struct sockaddr_in)){
        struct sockaddr_in local;memcpy(&local,address,sizeof local);
        if(local.sin_addr.s_addr==htonl(INADDR_ANY))local.sin_addr.s_addr=htonl(INADDR_LOOPBACK);
        return __real_bind(fd,(const struct sockaddr*)&local,sizeof local);
    }
    return __real_bind(fd,address,length);
}
