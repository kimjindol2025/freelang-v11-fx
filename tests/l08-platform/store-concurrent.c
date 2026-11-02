#include "store-concurrent.h"
#include <time.h>
#include <errno.h>
static FLObject *catalog;
static __thread int worker;
FLValue __real_fl_atom_deref(FLValue atom);
FLValue store_test_catalog(FLValue atom){catalog=atom.obj;return fl_nil();}
FLValue store_test_worker(void){worker=1;return atom_test_start();}
FLValue __wrap_fl_atom_deref(FLValue atom){
 FLValue value=__real_fl_atom_deref(atom);
 if(worker&&atom.obj==catalog){struct timespec delay={0,500000};while(nanosleep(&delay,&delay)&&errno==EINTR){}}
 return value;
}
