#include <lisa_log.h>
#include "appinc.h"
#include "ls_app_device.h"

// 默认输出日志等级
#if (ENABLE_CONSISTENCY == 1)
static uint8_t ls_log_level = (LISA_LOG_LEVEL_DEBUG);
#else
static uint8_t ls_log_level = (LISA_LOG_LEVEL_ERROR);
#endif

int lisa_log_set_level(uint8_t level)
{
	if (level > LISA_LOG_LEVEL_VERBOSE) {
		ls_log_level = LISA_LOG_LEVEL_VERBOSE;
	} else if (level < LISA_LOG_LEVEL_NONE) {
		ls_log_level = LISA_LOG_LEVEL_NONE;
	} else {
		ls_log_level = level;
	}
	return 0;
}

inline uint8_t lisa_log_get_level()
{
	return ls_log_level;
}


void shell_set_loglevel(int level)
{
    SHELL_ITEM_EXPORT("loglevel", shell_set_loglevel, "app loglevel [0~5]");
	lisa_log_set_level(level);
}