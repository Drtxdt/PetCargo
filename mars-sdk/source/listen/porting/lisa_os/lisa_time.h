#ifndef __LISA_OS_TIME__
#define __LISA_OS_TIME__

#include <stddef.h>
#include <stdint.h>

/**
 * @brief   获取系统启动毫秒数
 * @return  毫秒
 */
uint64_t lisa_os_get_tick_ms(void);

/**
 * @brief   获取系统启动秒数
 * @return  秒
 */
uint32_t lisa_os_get_time(void);

/**
 * @brief   随机数
 */
uint32_t lisa_rand32(void);

uint64_t lisa_os_get_time_ssl(void *arg);

#endif  //__LISA_OS_TIME__