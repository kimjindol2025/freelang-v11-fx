#include "extern-c.h"
FLValue native_ext_value(FLValue value){return fl_add(value,fl_int(100));}
FLValue native_ext_zero(void){return fl_int(17);}
