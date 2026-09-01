#ifndef __LISA_RTT_LOG__
#define __LISA_RTT_LOG__

#include <stdint.h>
#include <stdio.h>
#include <FreeRTOS.h>
#include <task.h>

#define LISA_LOG_LEVEL_NONE (0)
#define LISA_LOG_LEVEL_ERROR (1)
#define LISA_LOG_LEVEL_WARN (2)
#define LISA_LOG_LEVEL_INFO (3)
#define LISA_LOG_LEVEL_DEBUG (4)
#define LISA_LOG_LEVEL_VERBOSE (5)

extern int printk(const char *fmt, ...);

/**
 * @brief 设置LOG等级
 * @param level log等级 (LISA_LOG_LEVEL_NONE ~ LISA_LOG_LEVEL_VERBOSE)
 * @return 0 设置成功
 * @return -1 设置失败
 */
int lisa_log_set_level(uint8_t level);

/**
 * @brief 获取LOG等级
 * @return log level
 */
uint8_t lisa_log_get_level();

#define LISA_PRINT(tag, format, ...)						\
	do {													\
		listenai_print(tag, "", format, ##__VA_ARGS__);		\
	} while (0)

#define LISA_LOGV(tag, format, ...)                           \
	do {                                                      \
		if (lisa_log_get_level() >= LISA_LOG_LEVEL_VERBOSE) { \
			listenai_print(tag, "V", format, ##__VA_ARGS__);  \
		}                                                     \
	} while (0)

#define LISA_LOGD(tag, format, ...)                          \
	do {                                                     \
		if (lisa_log_get_level() >= LISA_LOG_LEVEL_DEBUG) {  \
			listenai_print(tag, "D", format, ##__VA_ARGS__); \
		}                                                    \
	} while (0)

#define LISA_LOGI(tag, format, ...)                                               \
	do {                                                                          \
		if (lisa_log_get_level() >= LISA_LOG_LEVEL_INFO) {                        \
			listenai_print(tag, "I", format, ##__VA_ARGS__); \
		}                                                                         \
	} while (0)

#define LISA_LOGW(tag, format, ...)                                                \
	do {                                                                           \
		if (lisa_log_get_level() >= LISA_LOG_LEVEL_WARN) {                         \
			listenai_print(tag, "W", format, ##__VA_ARGS__); \
		}                                                                          \
	} while (0)

#define LISA_LOGE(tag, format, ...)                                            \
	do {                                                                       \
		if (lisa_log_get_level() >= LISA_LOG_LEVEL_ERROR) {                    \
			listenai_print(tag, "E", format, ##__VA_ARGS__); \
		}                                                                      \
	} while (0)

#define LISA_LOG(tag, format, ...)                                                \
	do {                                                                          \
		if (lisa_log_get_level() >= LISA_LOG_LEVEL_INFO) {                        \
			listenai_print(tag, "O", format, ##__VA_ARGS__); \
		}                                                                         \
	} while (0)

#define LISA_LOGF(tag, format, ...)                                                \
	do {                                                                          \
		if (lisa_log_get_level() >= LISA_LOG_LEVEL_ERROR) {                        \
			listenai_print(tag, "F", format, ##__VA_ARGS__); \
		}                                                                         \
	} while (0)

#define LISA_LOGB(tag, buf, len, format, ...)                   					\
	do {                                                        					\
		if (lisa_log_get_level() >= LISA_LOG_LEVEL_DEBUG) {     					\
			printk("[B][" tag "] " format ": ", ##__VA_ARGS__); 					\
			for (int i = 0; i < len; i++) {                     					\
				printk("%02X ", buf[i]);                         					\
			}                                                   					\
			printk("\n");                                       					\
		}                                                      						\
	} while (0)

#define listenai_print(tag, level_info, format, ...)                                \
	do {                                                                            \
		TaskHandle_t task = xTaskGetCurrentTaskHandle();                            \
		char *name = pcTaskGetName(task);   									\
		printk("[%s][%s # " tag "] " format "\n", level_info, name, ##__VA_ARGS__); \
	} while (0)

#endif  // __LISA_RTT_LOG__
