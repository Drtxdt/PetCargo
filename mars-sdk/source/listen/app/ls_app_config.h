#ifndef __LISTENAI_CONFIG_H__
#define __LISTENAI_CONFIG_H__

#include <stdint.h>
#include "lisa_log.h"

#define CFG_USER_DEFINE_BEGIN (1)
#define PREF_KEY_VOICE_SWITCH CFG_USER_DEFINE_BEGIN
#define PREF_KEY_VOLUME (CFG_USER_DEFINE_BEGIN + 1)
#define PREF_KEY_WAKEUP_WORD (CFG_USER_DEFINE_BEGIN + 2)
#define PREF_KEY_REG_SAVE_FLAG (CFG_USER_DEFINE_BEGIN + 3)
#define PREF_KEY_REG_SAVE_SIZE (CFG_USER_DEFINE_BEGIN + 4)
#define PREF_KEY_REG_CMD_INFO (CFG_USER_DEFINE_BEGIN + 5)

/** 默认语音开关 */
#define LSUP_CFG_DEFAULT_VOICE_SWITCH 	(1)
/** 默认音量 */
#define LSUP_CFG_DEFAULT_VOLUME_LEVEL	(1)
/** 默认唤醒词 */
#define LSUP_CFG_DEFAULT_WAKEUP_WORD	(0)
/** 默认语音注册保存标志 */
#define LSUP_CFG_DEFAULT_REG_SAVE_FLAG	(1)
#define LSUP_CFG_DEFAULT_REG_SAVE_SIZE	(0)
#define LSUP_CFG_DEFAULT_REG_CMD_INFO	(0)

#define SET_BIT_VAL(A, k)        (A |= (1 << k))
#define GET_BIT_VAL(A, k)        ((A & (1 << k)) >> k)
#define CLEAR_BIT_VAL(A, k)      (A &= ~(1 << k))

/**
 * @brief 程序运行当中的配置
 * 
 */
typedef struct {
	/** 语音开关 */
	uint8_t voice_switch;
	/** 音量 */
	uint8_t volume_level;
	/** 唤醒词 */
	uint8_t wakeup_words;
	/** 语音注册 */
	uint8_t reg_save_flag;	//是否保存
	uint16_t reg_save_size;	//保存大小
	uint32_t reg_cmd_info;	//保存注册命令信息(是否注册)
} app_run_cfg_t;

typedef enum {
	CFG_REFRESH_ALL,
	CFG_REFRESH_VOICE,
	CFG_REFRESH_VOLUME,
	CFG_REFRESH_WAKEUP
} cfg_refresh_source_t;

typedef union
{
	uint32_t value;
	struct {
		uint32_t count : 8;
		uint32_t status : 24;
	};
} reg_cmd_info_t;

extern app_run_cfg_t g_run_cfg;

/**
 * @brief 加载配置
 * 
 */
void ls_app_load_cfg(char *voice_mem);

/**
 * @brief 更新所有配置
 * 
 */
void ls_app_cfg_refresh_all(cfg_refresh_source_t source);

/**
 * @brief 清除所有配置
 * 
 */
void ls_app_cfg_clear_all();

#endif
