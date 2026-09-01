#include "ls_app_volume.h"
#include "ls_app_device.h"
#include "ls_app_config.h"
#include "mini_player.h"
#include "lisa_log.h"
#include "ls_app_cfg_mgr.h"

#define TAG "vol"

static int s_vol_index_max = 0;
static int s_vol_index_min = 0;
static int s_vol_index_med = 0;

static volume_info_t s_volume_info;
extern int g_replay_enable;

void ls_app_volume_init()
{
	cfg_parser_get_vol_info(&s_volume_info);

	LISA_LOGD(TAG, "vol lev size: %d", s_volume_info.level_size);
	LISA_LOGD(TAG, "df vol: %d", s_volume_info.default_vol);

	s_vol_index_max = s_volume_info.level_size - 1;
	int tmp_index = s_volume_info.level_size;
	s_vol_index_med = tmp_index/2 + tmp_index%2 - 1;
	s_vol_index_min = 0;

	// 设置初始化音量
	LISA_LOGD(TAG, "init vol lev: %d", g_run_cfg.volume_level);
	ls_app_set_volume(g_run_cfg.volume_level);
}

void ls_app_set_volume(int vol)
{
	vol = vol > s_vol_index_max ? s_vol_index_max : vol;
	vol = vol < s_vol_index_min ? s_vol_index_min : vol;

	lisa_miniplayer_set_vol(s_volume_info.level[vol]);

	LISA_LOGD(TAG, "set vol: %d -> %d", g_run_cfg.volume_level, vol);
	// refresh local config
	if (g_run_cfg.volume_level != vol) {
		g_run_cfg.volume_level = vol;
		ls_app_cfg_refresh_all(CFG_REFRESH_VOLUME);
	}
}

void ls_app_vol_adjust(int vol)
{
	int target_vol = -1;
	uint8_t *tondids = NULL;
	uint8_t tondid_len = 0;

	if (vol > 0 && g_run_cfg.volume_level >= s_vol_index_max) {
		if (s_volume_info.adjmax_vol_tip.audio_size > 0) {
			tondids = s_volume_info.adjmax_vol_tip.audios;		// 音量已最大
			tondid_len = s_volume_info.adjmax_vol_tip.audio_size;
		} else {
			tondids = s_volume_info.max_vol_tip.audios;		// 音量已最大
			tondid_len = s_volume_info.max_vol_tip.audio_size;
		}
	} else if (vol < 0 && g_run_cfg.volume_level <= s_vol_index_min) {
		if (s_volume_info.adjmin_vol_tip.audio_size > 0) {
			tondids = s_volume_info.adjmin_vol_tip.audios;		// 音量已最大
			tondid_len = s_volume_info.adjmin_vol_tip.audio_size;
		} else {
			tondids = s_volume_info.min_vol_tip.audios;		// 音量已最小
			tondid_len = s_volume_info.min_vol_tip.audio_size;
		}
	} else {
		tondids = (vol>0) ? s_volume_info.up_vol_tip.audios : s_volume_info.down_vol_tip.audios;
		tondid_len = (vol>0) ? s_volume_info.up_vol_tip.audio_size : s_volume_info.down_vol_tip.audio_size;
		target_vol = g_run_cfg.volume_level + vol;
	}

	if (target_vol != -1) ls_app_set_volume(target_vol);
	if (g_replay_enable) lisa_miniplayer_play_by_ids(tondids, tondid_len);
}

void ls_app_volume_mute()
{
	LISA_LOGD(TAG, "app set mute vol: %d", s_vol_index_min);
	ls_app_set_volume(s_vol_index_min);
}

void ls_app_set_max_volume()
{
	LISA_LOGD(TAG, "app set max vol: %d", s_vol_index_max);
	ls_app_set_volume(s_vol_index_max);
	uint8_t *tondids = s_volume_info.max_vol_tip.audios;
	uint8_t tondid_len = s_volume_info.max_vol_tip.audio_size;
	if (g_replay_enable) lisa_miniplayer_play_by_ids(tondids, tondid_len);
}

void ls_app_set_min_volume()
{
	LISA_LOGD(TAG, "app set min vol: %d", s_vol_index_min);
	ls_app_set_volume(s_vol_index_min);
	uint8_t *tondids = s_volume_info.min_vol_tip.audios;
	uint8_t tondid_len = s_volume_info.min_vol_tip.audio_size;
	if (g_replay_enable) lisa_miniplayer_play_by_ids(tondids, tondid_len);
}

void ls_app_set_med_volume()
{
	LISA_LOGD(TAG, "app set med vol: %d", s_vol_index_med);
	ls_app_set_volume(s_vol_index_med);
	uint8_t *tondids = s_volume_info.med_vol_tip.audios;
	uint8_t tondid_len = s_volume_info.med_vol_tip.audio_size;
	if (g_replay_enable) lisa_miniplayer_play_by_ids(tondids, tondid_len);
}