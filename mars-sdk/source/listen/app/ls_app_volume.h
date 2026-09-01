#ifndef __LS_APP_VOLUME_H__
#define __LS_APP_VOLUME_H__

/**
 * @brief 音量设置初始化
 */
void ls_app_volume_init();

/**
 * @brief  设置音量
 * 
 * @param  vol [APP_MIN_VOLUME_LEV,APP_MAX_VOLUME_LEV]
 */
void ls_app_set_volume(int vol);

/**
 * @brief       音量调整, 正负值
 * 
 * @param  vol	音量调整值
 */
void ls_app_vol_adjust(int vol);

/**
 * @brief 关闭音量
 * 
 */
void ls_app_volume_mute();

/**
 * @brief 设置最大音量
 * 
 */
void ls_app_set_max_volume();

/**
 * @brief 设置最小音量
 * 
 */
void ls_app_set_min_volume();

/**
 * @brief 设置中等音量
 * 
 */
void ls_app_set_med_volume();

#endif  // __LS_APP_VOLUME_H__
