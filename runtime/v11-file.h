/* C10 underscore text APIs. Existing L07 byte APIs above stay unchanged. */
static int vf_path(FLValue path,char out[PATH_MAX]){
    if(path.tag!=FL_STRING)return EINVAL;
    FLString*s=(FLString*)path.obj;
    if(!s->len||s->len>=PATH_MAX||memchr(s->data,0,s->len))return EINVAL;
    if(s->data[0]=='/')memcpy(out,s->data,s->len+1);
    else{
        char cwd[PATH_MAX];
        if(!getcwd(cwd,sizeof(cwd)))return errno;
        if(snprintf(out,PATH_MAX,"%s/%s",cwd,s->data)>=PATH_MAX)return ENAMETOOLONG;
    }
    char copy[PATH_MAX];
    strcpy(copy,out);
    char*save=NULL;
    for(char*p=strtok_r(copy,"/",&save);p;p=strtok_r(NULL,"/",&save))if(!strcmp(p,".."))return EACCES;
    const char*base=getenv("FL_FILE_BASE");
    if(base&&*base){
        char root[PATH_MAX];
        if(!realpath(base,root))return errno;
        size_t n=strlen(root);
        if(strcmp(root,"/")&&strncmp(out,root,n))return EACCES;
        if(strcmp(root,"/")&&out[n]&&out[n]!='/')return EACCES;
    }
    return 0;
}

static int vf_walk(const char*path,int create,char leaf[NAME_MAX+1]){
    char copy[PATH_MAX];
    strcpy(copy,path);
    char*save=NULL,*part=strtok_r(copy,"/",&save);
    int fd=open("/",O_RDONLY|O_DIRECTORY|O_CLOEXEC);
    if(fd<0)return-1;
    if(!part){
        strcpy(leaf,".");
        return fd;
    }
    for(;;){
        char*next=strtok_r(NULL,"/",&save);
        if(strlen(part)>NAME_MAX){
            close(fd);
            errno=ENAMETOOLONG;
            return-1;
        }
        if(!next){
            strcpy(leaf,part);
            return fd;
        }
        int child=openat(fd,part,O_RDONLY|O_DIRECTORY|O_NOFOLLOW|O_CLOEXEC);
        if(child<0&&errno==ENOENT&&create){
            if(mkdirat(fd,part,0755)&&errno!=EEXIST){
                int e=errno;
                close(fd);
                errno=e;
                return-1;
            }
            child=openat(fd,part,O_RDONLY|O_DIRECTORY|O_NOFOLLOW|O_CLOEXEC);
        }
        int e=errno;
        close(fd);
        if(child<0){
            errno=e;
            return-1;
        }
        fd=child;
        part=next;
    }
}

static int vf_parent(FLValue path,int create,char leaf[NAME_MAX+1]){
    char absolute[PATH_MAX];
    int e=vf_path(path,absolute);
    if(e){
        errno=e;
        return-1;
    }
    /* Validate the configured base itself component-by-component, too. */
    const char*base=getenv("FL_FILE_BASE");
    if(base&&*base){
        char full[PATH_MAX];
        if(base[0]=='/')snprintf(full,sizeof(full),"%s",base);
        else{
            char cwd[PATH_MAX];
            if(!getcwd(cwd,sizeof(cwd)))return-1;
            if(snprintf(full,sizeof(full),"%s/%s",cwd,base)>=PATH_MAX){
                errno=ENAMETOOLONG;
                return-1;
            }
        }
        char name[NAME_MAX+1];
        int parent=vf_walk(full,0,name);
        if(parent<0)return-1;
        int root=openat(parent,name,O_RDONLY|O_DIRECTORY|O_NOFOLLOW|O_CLOEXEC);
        int err=errno;
        close(parent);
        if(root<0){
            errno=err;
            return-1;
        }
        close(root);
    }
    return vf_walk(absolute,create,leaf);
}

static FLValue vf_error(const char*op,int number){
    char msg[192];
    snprintf(msg,sizeof(msg),"%s failed: %s",op,number==EACCES||number==ELOOP||number==ENOTDIR?"PATH_OR_ACCESS_DENIED":number==ENOENT?"NOT_FOUND":number==EFBIG?"SIZE_LIMIT":number==EINVAL?"INVALID_ARGUMENT":"IO_ERROR");
    fl_throw(fl_str_val(msg));
    return fl_nil();
}

static FLValue vf_bytes(const char*p,size_t n){
    FLString*s=fl_arena_alloc(sizeof(*s)+n+1);
    if(!s)return vf_error("file_read",ENOMEM);
    s->base.type=FL_STRING;
    s->base.rc=1;
    s->len=n;
    memcpy(s->data,p,n);
    s->data[n]=0;
    return(FLValue){
        .tag=FL_STRING,.obj=(FLObject*)s
    };
}

FLValue fl_v11_file(FLValue name,FLValue argv){
    const char*op=_strval(name);
    FLVector*v=(FLVector*)argv.obj;
    FLValue path=v->len?v->data[0]:fl_nil(),value=v->len>1?v->data[1]:fl_nil();
    int query=!strcmp(op,"file_exists")||!strcmp(op,"file_is_file");
    int writing=!strcmp(op,"file_write")||!strcmp(op,"file_append");
    if(writing&&value.tag!=FL_STRING)return vf_error(op,EINVAL);
    char leaf[NAME_MAX+1];
    int parent=vf_parent(path,writing||!strcmp(op,"dir_create"),leaf);
    if(parent<0)return query?fl_bool(false):vf_error(op,errno);
    struct stat st;
    int exists=fstatat(parent,leaf,&st,AT_SYMLINK_NOFOLLOW)==0;
    int staterr=errno;
    if(exists&&S_ISLNK(st.st_mode)){
        close(parent);
        return query?fl_bool(false):vf_error(op,ELOOP);
    }
    if(query){
        close(parent);
        return fl_bool(exists&&(!strcmp(op,"file_exists")||S_ISREG(st.st_mode)));
    }
    if(!strcmp(op,"file_delete")){
        if(!exists&&staterr==ENOENT){
            close(parent);
            return fl_bool(false);
        }
        int rc=unlinkat(parent,leaf,0),e=errno;
        close(parent);
        return rc?vf_error(op,e):fl_bool(true);
    }
    if(!strcmp(op,"dir_create")){
        int rc=mkdirat(parent,leaf,0755),e=errno;
        if(rc&&e==EEXIST&&exists&&S_ISDIR(st.st_mode))rc=0;
        close(parent);
        return rc?vf_error(op,e):fl_bool(true);
    }
    int flags=writing?O_WRONLY|O_CREAT|(!strcmp(op,"file_append")?O_APPEND:0):O_RDONLY;
    int fd=openat(parent,leaf,flags|O_NOFOLLOW|O_CLOEXEC|O_NONBLOCK,0666);
    int e=errno;
    close(parent);
    if(fd<0)return vf_error(op,e);
    if(fstat(fd,&st)){
        e=errno;
        close(fd);
        return vf_error(op,e);
    }
    if(!strcmp(op,"dir_list")){
        if(!S_ISDIR(st.st_mode)){
            close(fd);
            return vf_error(op,ENOTDIR);
        }
        DIR*d=fdopendir(fd);
        if(!d){
            e=errno;
            close(fd);
            return vf_error(op,e);
        }
        FLValue result=fl_vec_new();
        errno=0;
        struct dirent*item;
        while((item=readdir(d)))if(strcmp(item->d_name,".")&&strcmp(item->d_name,".."))result=fl_vec_push(result,fl_str_val(item->d_name));
        e=errno;
        closedir(d);
        return e?vf_error(op,e):result;
    }
    if(!S_ISREG(st.st_mode)){
        close(fd);
        return vf_error(op,EINVAL);
    }
    if(!strcmp(op,"file_size")){
        close(fd);
        return fl_int(st.st_size);
    }
    if(st.st_size<0||(uint64_t)st.st_size>FL_FILE_IO_MAX_BYTES){
        close(fd);
        return vf_error(op,EFBIG);
    }
    if(writing){
        if(value.tag!=FL_STRING){
            close(fd);
            return vf_error(op,EINVAL);
        }
        FLString*s=(FLString*)value.obj;
        if(s->len>FL_FILE_IO_MAX_BYTES||(!strcmp(op,"file_append")&&(uint64_t)st.st_size+s->len>FL_FILE_IO_MAX_BYTES)){
            close(fd);
            return vf_error(op,EFBIG);
        }
        if(!strcmp(op,"file_write")&&ftruncate(fd,0)){
            e=errno;
            close(fd);
            return vf_error(op,e);
        }
        size_t done=0;
        while(done<s->len){
            ssize_t n=write(fd,s->data+done,s->len-done);
            if(n<0&&errno==EINTR)continue;
            if(n<=0){
                e=errno;
                close(fd);
                return vf_error(op,e);
            }
            done+=n;
        }
        if(close(fd))return vf_error(op,errno);
        return fl_bool(true);
    }
    if(!strcmp(op,"file_copy")){
        char target[NAME_MAX+1];
        int destparent=vf_parent(value,1,target);
        if(destparent<0){
            e=errno;
            close(fd);
            return vf_error(op,e);
        }
        int to=openat(destparent,target,O_WRONLY|O_CREAT|O_NOFOLLOW|O_CLOEXEC|O_NONBLOCK,0666);
        e=errno;
        close(destparent);
        if(to<0){
            close(fd);
            return vf_error(op,e);
        }
        struct stat dst;
        if(fstat(to,&dst)||!S_ISREG(dst.st_mode)||(dst.st_dev==st.st_dev&&dst.st_ino==st.st_ino)){
            close(fd);
            close(to);
            return vf_error(op,EINVAL);
        }
        if(ftruncate(to,0)){
            e=errno;
            close(fd);
            close(to);
            return vf_error(op,e);
        }
        char buffer[8192];
        size_t total=0;
        int error=0;
        for(;;){
            ssize_t n=read(fd,buffer,sizeof(buffer));
            if(n<0&&errno==EINTR)continue;
            if(n<0){
                error=errno;
                break;
            }
            if(!n)break;
            total+=n;
            if(total>FL_FILE_IO_MAX_BYTES){
                error=EFBIG;
                break;
            }
            size_t done=0;
            while(done<(size_t)n){
                ssize_t put=write(to,buffer+done,n-done);
                if(put<0&&errno==EINTR)continue;
                if(put<=0){
                    error=errno;
                    break;
                }
                done+=put;
            }
            if(error)break;
        }
        if(close(fd)&&!error)error=errno;
        if(close(to)&&!error)error=errno;
        return error?vf_error(op,error):fl_bool(true);
    }
    size_t length=st.st_size,done=0;
    char*data=malloc(length+1);
    if(!data){
        close(fd);
        return vf_error(op,ENOMEM);
    }
    while(done<length){
        ssize_t n=read(fd,data+done,length-done);
        if(n<0&&errno==EINTR)continue;
        if(n<=0){
            e=n<0?errno:EIO;
            free(data);
            close(fd);
            return vf_error(op,e);
        }
        done+=n;
    }
    char extra;
    ssize_t tail=read(fd,&extra,1);
    e=errno;
    close(fd);
    if(tail){
        free(data);
        return vf_error(op,tail>0?EAGAIN:e);
    }
    FLValue result=vf_bytes(data,length);
    free(data);
    return result;
}
