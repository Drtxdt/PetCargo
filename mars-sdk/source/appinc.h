#ifndef __APP_COMMON_HEADER__
#define __APP_COMMON_HEADER__

#include "sdkinc.h"
#include "xutils.h"
#include "resmgr.h"
#include "mapi.h"

#define VER_MAJOR           0
#define VER_MINOR           9
#define VER_BUILD           1000

#define OS_STACK_DEF        (configMINIMAL_STACK_SIZE)
#define OS_PRIO_HIG         (configMAX_PRIORITIES - 1)
#define OS_PRIO_LOW         (tskIDLE_PRIORITY + 1)
#define OS_PRIO_DEF         ((OS_PRIO_HIG + OS_PRIO_LOW) >> 1)

#endif//__APP_COMMON_HEADER__
