#ifndef __LISA_COMMON_TYPEDEF__
#define __LISA_COMMON_TYPEDEF__

#define LISA_OS_WAIT_FOREVER         (0xffffffffU) /* Wait forever timeout value */

typedef enum  {
    LISA_OS_PRIORITY_LOW             = 0,
    LISA_OS_PRIORITY_BELOW_NORMAL    = 1,
    LISA_OS_PRIORITY_NORMAL          = 2,
    LISA_OS_PRIORITY_ABOVE_NORMAL    = 3,
    LISA_OS_PRIORITY_HIGH            = 4,
} LISA_OS_Priority;

#endif  //
