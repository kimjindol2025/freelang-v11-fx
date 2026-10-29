/* C11: synthetic handles, shared by threads; C10 validates every path component. */
#include <pthread.h>
#include <math.h>
typedef struct VFd {int64_t id;int fd;char mode;struct VFd*next;} VFd;
static VFd*vfd_head;
static int64_t vfd_next=1000;
static pthread_mutex_t vfd_lock=PTHREAD_MUTEX_INITIALIZER;
static FLValue vfd_error(const char*op,int code){
    char name[80],message[256];size_t i=0;
    for(;op[i]&&i<sizeof(name)-1;i++)name[i]=op[i]=='-'?'_':op[i];
    name[i]=0;
    snprintf(message,sizeof(message),"%s failed: %s",name,strerror(code));
    fl_throw(fl_str_val(message));return fl_nil();
}
static int vfd_integer(FLValue value,int64_t*result){
    if(value.tag==FL_INT){*result=value.i;return 1;}
    if(value.tag!=FL_FLOAT||!isfinite(value.f)||trunc(value.f)!=value.f||value.f<0||value.f>=0x1p63)return 0;
    *result=(int64_t)value.f;return 1;
}
static int vfd_sync(int fd){int rc;do{rc=fsync(fd);}while(rc<0&&errno==EINTR);return rc;}
FLValue fl_v11_fd(FLValue name,FLValue argv){
    const char*op=_strval(name);FLVector*v=(FLVector*)argv.obj;
    FLValue arg=v->len?v->data[0]:fl_nil(),value=v->len>1?v->data[1]:fl_nil();
    if(!strcmp(op,"fd-open")){
        if(value.tag!=FL_STRING)return vfd_error(op,EINVAL);
        FLString*mode=(FLString*)value.obj;
        if(mode->len!=1||!mode->data[0]||!strchr("rwa",mode->data[0]))return vfd_error(op,EINVAL);
        char leaf[NAME_MAX+1];int parent=vf_parent(arg,0,leaf);
        if(parent<0)return vfd_error(op,errno);
        int flags=mode->data[0]=='r'?O_RDONLY:O_WRONLY|O_CREAT;
        if(mode->data[0]=='a')flags|=O_APPEND;
        int fd=openat(parent,leaf,flags|O_NOFOLLOW|O_CLOEXEC|O_NONBLOCK,0666),err=errno;
        close(parent);
        if(fd<0)return vfd_error(op,err);
        struct stat st;
        if(fstat(fd,&st)){err=errno;close(fd);return vfd_error(op,err);}
        if(!S_ISREG(st.st_mode)){close(fd);return vfd_error(op,EINVAL);}
        if(st.st_size<0||(uint64_t)st.st_size>FL_FILE_IO_MAX_BYTES){close(fd);return vfd_error(op,EFBIG);}
        if(mode->data[0]=='w'&&ftruncate(fd,0)){err=errno;close(fd);return vfd_error(op,err);}
        VFd*entry=malloc(sizeof(*entry));
        if(!entry){close(fd);return vfd_error(op,ENOMEM);}
        pthread_mutex_lock(&vfd_lock);
        if(vfd_next==INT64_MAX){pthread_mutex_unlock(&vfd_lock);free(entry);close(fd);return vfd_error(op,EMFILE);}
        *entry=(VFd){vfd_next++,fd,mode->data[0],vfd_head};vfd_head=entry;
        int64_t id=entry->id;
        pthread_mutex_unlock(&vfd_lock);
        return fl_int(id);
    }
    int64_t id=0,amount=0;
    int flushing=!strcmp(op,"fd-flush");
    if(!flushing&&!vfd_integer(arg,&id))return vfd_error(op,EBADF);
    if(!strcmp(op,"fd-read")&&(!vfd_integer(value,&amount)||amount<0||(uint64_t)amount>FL_FILE_IO_MAX_BYTES))return vfd_error(op,EINVAL);
    if(!strcmp(op,"fd-write")&&value.tag!=FL_STRING)return vfd_error(op,EINVAL);
    pthread_mutex_lock(&vfd_lock);
    int err=0;
    if(flushing){
        for(VFd*p=vfd_head;p;p=p->next)if(vfd_sync(p->fd)){err=errno;break;}
        pthread_mutex_unlock(&vfd_lock);
        return err?vfd_error(op,err):fl_bool(true);
    }
    VFd**slot=&vfd_head;
    while(*slot&&(*slot)->id!=id)slot=&(*slot)->next;
    if(!*slot){pthread_mutex_unlock(&vfd_lock);return vfd_error(op,EBADF);}
    VFd*entry=*slot;int fd=entry->fd;
    if(!strcmp(op,"fd-close")){
        *slot=entry->next;free(entry);
        /* Linux closes the descriptor even on EINTR; never retry a reused fd. */
        if(close(fd))err=errno;
        pthread_mutex_unlock(&vfd_lock);
        return err?vfd_error(op,err):fl_bool(true);
    }
    if(!strcmp(op,"fd-fsync")){
        if(vfd_sync(fd))err=errno;
        pthread_mutex_unlock(&vfd_lock);
        return err?vfd_error(op,err):fl_bool(true);
    }
    if(!strcmp(op,"fd-seek")){
        struct stat st;if(fstat(fd,&st))err=errno;
        pthread_mutex_unlock(&vfd_lock);
        return err?vfd_error(op,err):fl_int(st.st_size);
    }
    if(!strcmp(op,"fd-write")){
        FLString*s=(FLString*)value.obj;struct stat st;off_t position=0;
        if(entry->mode=='r')err=EBADF;
        else if(fstat(fd,&st))err=errno;
        else if(entry->mode=='a')position=st.st_size;
        else if((position=lseek(fd,0,SEEK_CUR))<0)err=errno;
        if(!err&&((uint64_t)position>FL_FILE_IO_MAX_BYTES||s->len>FL_FILE_IO_MAX_BYTES-(uint64_t)position))err=EFBIG;
        size_t done=0;
        while(!err&&done<s->len){
            ssize_t wrote=write(fd,s->data+done,s->len-done);
            if(wrote<0&&errno==EINTR)continue;
            if(wrote<=0){err=wrote<0?errno:EIO;break;}
            done+=(size_t)wrote;
        }
        pthread_mutex_unlock(&vfd_lock);
        return err?vfd_error(op,err):fl_bool(true);
    }
    if(!strcmp(op,"fd-read")){
        char*buffer=malloc((size_t)amount+1);
        if(!buffer){pthread_mutex_unlock(&vfd_lock);return vfd_error(op,ENOMEM);}
        ssize_t got;do{got=read(fd,buffer,(size_t)amount);}while(got<0&&errno==EINTR);
        if(got<0)err=errno;
        pthread_mutex_unlock(&vfd_lock);
        if(err){free(buffer);return vfd_error(op,err);}
        FLValue result=vf_bytes(buffer,(size_t)got);free(buffer);return result;
    }
    pthread_mutex_unlock(&vfd_lock);
    return vfd_error(op,EINVAL);
}
