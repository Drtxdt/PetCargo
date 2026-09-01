
#include "autoconf.h"

#if CONFIG_RESMGR_AUTO_INCLUDED
.globl __res_start
.globl __res_end
.section .rodata
.align 4
__res_start:
.incbin __RESFILE__
__res_end:
#endif
