#include <string.h>
#include <stdlib.h>
#include "appinc.h"
#include "ls_app_config.h"
#include "ls_app_volume.h"
#include "ls_app_version.h"
#include "lisa_timer.h"
#include "ls_app_cfg_mgr.h"
#include "ls_app_flash.h"

#define TAG "config"
#define APP_CONFIG_REFRESH_TIME (5000)

typedef struct config_item8
{
	uint8_t key;
	uint8_t vaule;
} config_item8_t;

typedef struct config_item16
{
	uint8_t key;
	uint16_t vaule;
} config_item16_t;

typedef struct config_item32
{
	uint8_t key;
	uint32_t vaule;
} config_item32_t;

typedef struct app_config
{
	config_item8_t voice;
	config_item8_t volume;
	config_item8_t wakeup_words;
	config_item8_t reg_save_flag;
	config_item16_t reg_save_size;
	config_item32_t reg_cmd_info;
} __attribute__((aligned(4))) app_config_t;

app_run_cfg_t g_run_cfg = {
	.voice_switch = LSUP_CFG_DEFAULT_VOICE_SWITCH,
	.volume_level = LSUP_CFG_DEFAULT_VOLUME_LEVEL,
	.wakeup_words = LSUP_CFG_DEFAULT_WAKEUP_WORD,
	.reg_save_flag = LSUP_CFG_DEFAULT_REG_SAVE_FLAG,
	.reg_save_size = LSUP_CFG_DEFAULT_REG_SAVE_SIZE,
	.reg_cmd_info = LSUP_CFG_DEFAULT_REG_CMD_INFO
};

static lisa_timer_t *s_config_timer = NULL;
static app_config_t s_app_config;
static mod_inst_t s_tnvs_mdrv;
static bool s_want_save = false;
static bool s_otp_locked = false;
static persisted_info_t s_pers_info;
static bool s_is_new_app_flash = false;
static char *s_voice_mem = NULL;

#define LS_CFG_PRINT_D(name, val) printk("%-16s:%12d\n", (name), (val));
#define LS_CFG_PRINT_F(name, val) printk("%-16s:%12f\n", (name), (val));
#define LS_CFG_PRINT_S(name, val) printk("%-16s:%12s\n", (name), (val));

void app_run_config_save()
{
	if (s_want_save && s_otp_locked) {
		s_want_save = false;
		// 判断每一项是否需要持久化，不需要则保存默认值
		uint8_t volume = s_pers_info.volume ? g_run_cfg.volume_level : LSUP_CFG_DEFAULT_VOLUME_LEVEL;
		uint8_t voice = s_pers_info.voice ? g_run_cfg.voice_switch : LSUP_CFG_DEFAULT_VOICE_SWITCH;
		uint8_t wkword = s_pers_info.wakeup ? g_run_cfg.wakeup_words : cfg_parser_get_default_keyword_id();
		uint8_t reg_save_flag = g_run_cfg.reg_save_flag;
		uint16_t reg_save_size = g_run_cfg.reg_save_size;
		uint32_t reg_cmd_info = g_run_cfg.reg_cmd_info;
		reg_cmd_info_t tmp_dbug;
		tmp_dbug.value = reg_cmd_info;
		LISA_LOGD(TAG, "refresh config volume=%d voice=%d, wkword=%d, regSaveFlag=%d, regSave=%d, regCmdInfo=cnt[%d]-st[%d]",
			volume, voice, wkword, reg_save_flag, reg_save_size, tmp_dbug.count, tmp_dbug.status);

		s_app_config.voice.key = PREF_KEY_VOICE_SWITCH;
		s_app_config.voice.vaule = voice;

		s_app_config.volume.key = PREF_KEY_VOLUME;
		s_app_config.volume.vaule = volume;

		s_app_config.wakeup_words.key = PREF_KEY_WAKEUP_WORD;
		s_app_config.wakeup_words.vaule = wkword;

		s_app_config.reg_save_flag.key = PREF_KEY_REG_SAVE_FLAG;
		s_app_config.reg_save_flag.vaule = reg_save_flag;

		s_app_config.reg_save_size.key = PREF_KEY_REG_SAVE_SIZE;
		s_app_config.reg_save_size.vaule = reg_save_size;

		s_app_config.reg_cmd_info.key = PREF_KEY_REG_CMD_INFO;
		s_app_config.reg_cmd_info.vaule = reg_cmd_info;

		int ret;
		if (!s_is_new_app_flash) {
			ret = __mapi_write(s_tnvs_mdrv, &s_app_config, sizeof(app_config_t), XINFINITE);
		} else {
			ret = listen_flash_config_write(&s_app_config, sizeof(app_config_t));
			if (s_voice_mem && reg_save_size > 0) listen_flash_voice_write(s_voice_mem, reg_save_size);
		}

		if (ret <= 0) {
			LISA_LOGD(TAG, "save config failed");
		} else {
			LISA_LOGD(TAG, "save config success");
		}
	}
}

static void __refresh_app_config(void *arg)
{
	s_want_save = true;
}

void ls_app_load_cfg(char *voice_mem)
{
	if (voice_mem) {
		s_voice_mem = voice_mem;
	}

	if (cfg_parser_get_study_switch() == 1) {
		LISA_LOGD(TAG, "Eanble new app flash.");
		s_is_new_app_flash = true;
	}

	s_pers_info = cfg_parser_get_persisted_info();
	if (s_pers_info.volume != 1) {
		volume_info_t volume_info;
		cfg_parser_get_vol_info(&volume_info);
		g_run_cfg.volume_level = volume_info.default_vol;
	}

	int ret = 0, lockmsk = 1;
	if (!s_is_new_app_flash) {
    	ret = __mapi_ctrl(__MAPI_FIND(CONFIG_TNVS_NAME), MAPI_TNVS_CTRL_OTP_LOCK_SET, &lockmsk);
	}

	if (ret != 0) {
		LISA_LOGE(TAG, "lock failed ret : %d\n", ret);
	} else {
		s_otp_locked = true;
	}

	if (s_otp_locked) {
		memset(&s_app_config, 0, sizeof(app_config_t));
		if (!s_is_new_app_flash) {
			s_tnvs_mdrv = __MAPI_FIND(CONFIG_TNVS_NAME);
			ret = __mapi_read(s_tnvs_mdrv, &s_app_config, sizeof(app_config_t), XINFINITE);
			s_app_config.reg_save_flag.vaule = 0;
			s_app_config.reg_save_size.vaule = 0;
			s_app_config.reg_cmd_info.vaule = 0;
		} else {
			ret = listen_flash_config_read(&s_app_config, sizeof(app_config_t));
		}

		if (ret > 0) {
			// 判断每一项是否需要持久化，若需要则读取flash中的值进行初始化
			if (s_pers_info.voice == 1 && s_app_config.voice.key == PREF_KEY_VOICE_SWITCH) {
				g_run_cfg.voice_switch = s_app_config.voice.vaule;
			}
			if (s_pers_info.volume == 1 && s_app_config.volume.key == PREF_KEY_VOLUME) {
				g_run_cfg.volume_level = s_app_config.volume.vaule;
			}
			if (s_pers_info.wakeup == 1 && s_app_config.wakeup_words.key == PREF_KEY_WAKEUP_WORD) {
				g_run_cfg.wakeup_words = s_app_config.wakeup_words.vaule;
			}
			if (s_app_config.reg_save_flag.key == PREF_KEY_REG_SAVE_FLAG) {
				g_run_cfg.reg_save_flag = s_app_config.reg_save_flag.vaule;
			}
			if (s_app_config.reg_save_size.key == PREF_KEY_REG_SAVE_SIZE) {
				g_run_cfg.reg_save_size = s_app_config.reg_save_size.vaule;
			}
			if (s_app_config.reg_cmd_info.key == PREF_KEY_REG_CMD_INFO) {
				g_run_cfg.reg_cmd_info = s_app_config.reg_cmd_info.vaule;
			}
		} else {
			LISA_LOGD(TAG, "no config!!!\n");
		}
	}

	printk("\n============= Running Config =============\n");
	LS_CFG_PRINT_D("volume", g_run_cfg.volume_level);
	LS_CFG_PRINT_D("voice", g_run_cfg.voice_switch);
	LS_CFG_PRINT_D("wkword", g_run_cfg.wakeup_words);
	LS_CFG_PRINT_D("regSaveFlag", g_run_cfg.reg_save_flag);
	LS_CFG_PRINT_D("regSaveSize", g_run_cfg.reg_save_size);
	reg_cmd_info_t tmp_dbug;
	tmp_dbug.value = g_run_cfg.reg_cmd_info;
	LS_CFG_PRINT_D("reg_cmd_count", tmp_dbug.count);
	LS_CFG_PRINT_D("reg_cmd_status", tmp_dbug.status);
	printk("==========================================\n");
}

void ls_app_cfg_refresh_all(cfg_refresh_source_t source)
{
	if (source == CFG_REFRESH_VOICE && s_pers_info.voice != 1) return;
	else if (source == CFG_REFRESH_VOLUME && s_pers_info.volume != 1) return;
	else if (source == CFG_REFRESH_WAKEUP && s_pers_info.wakeup != 1) return;

	LISA_LOGD(TAG, "refresh all in, s_otp_locked: %d", s_otp_locked);
	if (s_otp_locked) {
		if (s_config_timer == NULL) {
			s_config_timer = lisa_timer_create(APP_CONFIG_REFRESH_TIME, __refresh_app_config, NULL);
		}
		lisa_timer_stop(s_config_timer);
		lisa_timer_start(s_config_timer);
	}
}

void ls_app_cfg_clear_all()
{
	volume_info_t volume_info;
	cfg_parser_get_vol_info(&volume_info);
	g_run_cfg.volume_level = volume_info.default_vol;
	g_run_cfg.voice_switch = LSUP_CFG_DEFAULT_VOICE_SWITCH;
	g_run_cfg.wakeup_words = cfg_parser_get_default_keyword_id();
	g_run_cfg.reg_save_flag = LSUP_CFG_DEFAULT_REG_SAVE_FLAG;
	g_run_cfg.reg_save_size = LSUP_CFG_DEFAULT_REG_SAVE_SIZE;
	g_run_cfg.reg_cmd_info = LSUP_CFG_DEFAULT_REG_CMD_INFO;
	s_want_save = true;
}

void shell_clear_config_all()
{
	SHELL_ITEM_EXPORT("clear.configall", shell_clear_config_all, "clear config all");
	ls_app_cfg_clear_all();
	ls_app_cfg_refresh_all(CFG_REFRESH_ALL);
}