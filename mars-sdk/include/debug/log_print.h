/*
 * log_print.h
 *
 *  Created on: 2022
 *      Author: USER
 */

#ifndef INCLUDE_DEBUG_LOG_PRINT_H_
#define INCLUDE_DEBUG_LOG_PRINT_H_

#include <stdio.h>
#include <stdarg.h>

#define CLOG(fmt, ...)   printf(fmt"\n", ##__VA_ARGS__)

#define CLOG_LEVEL_NONE     0
#define CLOG_LEVEL_ERROR    1
#define CLOG_LEVEL_WARN     2
#define CLOG_LEVEL_INFO     3
#define CLOG_LEVEL_DEBUG    4
#define CLOG_LEVEL_VERBOSE  5

#define LOGLEVEL    (CLOG_LEVEL_DEBUG)

#define CLOGE(fmt, ...)     do { if (LOGLEVEL >= CLOG_LEVEL_ERROR)  { CLOG("[E] " fmt, ##__VA_ARGS__); } } while(0)
#define CLOGW(fmt, ...)     do { if (LOGLEVEL >= CLOG_LEVEL_WARN)   { CLOG("[W] " fmt, ##__VA_ARGS__); } } while(0)
#define CLOGI(fmt, ...)     do { if (LOGLEVEL >= CLOG_LEVEL_INFO)   { CLOG("[I] " fmt, ##__VA_ARGS__); } } while(0)
#define CLOGD(fmt, ...)     do { if (LOGLEVEL >= CLOG_LEVEL_DEBUG)  { CLOG("[D] " fmt, ##__VA_ARGS__); } } while(0)

#endif /* INCLUDE_DEBUG_LOG_PRINT_H_ */
