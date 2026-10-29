#include "store-fsync.h"
#include <errno.h>
#include <stdatomic.h>
static atomic_int armed;
int __real_fsync(int);
int __wrap_fsync(int fd){
 if(atomic_exchange(&armed,0)){errno=EIO;return -1;}
 return __real_fsync(fd);
}
FLValue store_fsync_fail_next(void){atomic_store(&armed,1);return fl_nil();}
