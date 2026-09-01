#ifndef __CSK_MINI_PLAYER_H__
#define __CSK_MINI_PLAYER_H__

#include <stdint.h>
#include <lisa_typedef.h>
#include "lisa_queue.h"
#include "lisa_semaphore.h"
#include "lisa_thread.h"

// 播放器消息线程名称
#define MINI_PLAYER_TD_NAME ("mini_td")
// 播放器消息线程栈大小
#define MINI_PLAYER_TD_STACK_SIZE (384)
// 播放器消息线程优先级
#define MINI_PLAYER_TD_PRIORITY (LISA_OS_PRIORITY_NORMAL)
// mini mp3播放器消息队列大小
#define MINI_PLAYER_MSG_QUE_COUNT (5)
// mini mp3播放器消息队列名称
#define MINI_PLAYER_MSG_QUE_NAME ("mini_msg")
// 提示音列表最大长度
#define MINI_PLAYER_PLAY_MAX_TONE (5)

typedef enum mini_player_state {
	MINI_PLAYER_ST_NONE,
	MINI_PLAYER_ST_PLAYING,
	MINI_PLAYER_ST_STOPED,
	MINI_PLAYER_ST_COMPLETE,
	MINI_PLAYER_ST_ERROR,
} mini_player_state;

typedef enum MINI_PLAYER_DECODE_CODE {
	MINI_PLAYER_DECODE_OK = 0,
	MINI_PLAYER_DECODE_EXIT,
	MINI_PLAYER_DECODE_ERR,
} MINI_PLAYER_DECODE_CODE;

typedef enum mini_player_msg_type {
	MINI_PLAYER_MSG_NONE,
	MINI_PLAYER_MSG_PLAY,
} mini_player_msg_type;

typedef struct mini_player_msg_s {
	mini_player_msg_type type;
} mini_player_msg_t;

typedef void (*player_status_cb)(mini_player_state state);

typedef struct audio_item {
    uint8_t tone_size;
    uint8_t tone_pos;
	uint8_t tone_is_pcm;
	uint8_t need_skip;
    uint8_t tondid[MINI_PLAYER_PLAY_MAX_TONE];
} audio_item_t;

typedef struct volume_ctx {
	int8_t amp_min;
	int8_t amp_max;
	int8_t amp_cur;
	float multiplier;
} volume_ctx_t;

typedef struct mini_player_s {
	mini_player_state m_state;
	uint8_t m_want_stop;
	uint8_t m_want_play;
	uint8_t m_running;
	player_status_cb cb;
	lisa_thread_t *m_td;
	lisa_queue_t *m_msg_que;
	lisa_semaphore_t *m_stop_sem;
	lisa_semaphore_t *m_play_sem;
	audio_item_t m_item;
} mini_player_t;

/**
 * @brief 	创建播放器
 */
void lisa_miniplayer_create(char *dec_share_mem);

/**
 * @brief 	播放toneid
 * @param 	tone_id 提示音id
*/
int lisa_miniplayer_play(uint8_t tone_id);

/**
 * @brief 	播放toneid，不需要跳过开头静音段
 * @param 	tone_id 提示音id
*/
int lisa_miniplayer_play_notskip(uint8_t tone_id);

/**
 * @brief 	播放toneid列表
 * @param 	audios 提示音id列表
 * @param 	len 提示音id个数
*/
int lisa_miniplayer_play_by_ids(const uint8_t *const audios, uint8_t len);

/**
 * @brief 	播放toneid列表
 * @param 	audios 提示音id列表
 * @param 	len 提示音id个数
*/
int lisa_miniplayer_play_by_ids_notskip(const uint8_t *const audios, uint8_t len);

/**
 * @brief 	播放pcm音频toneid
 * @param 	tone_id 提示音id
*/
int lisa_miniplayer_play_pcm(uint8_t tone_id);

/**
 * @brief 	播放pcm音频toneid列表
 * @param 	audios 提示音id列表
 * @param 	len 提示音id个数
*/
int lisa_miniplayer_play_pcm_by_ids(const uint8_t *const audios, uint8_t len);

/**
 * @brief 	设置播放状态回调
 * @param 	cb 回调
*/
void lisa_miniplayer_set_state_cb(player_status_cb cb);

/**
 * @brief 	停止播放
*/
int lisa_miniplayer_stop();

/**
 * @brief 	设置播放器音量
 * @param 	vol 音量值[0,100]
*/
int lisa_miniplayer_set_vol(uint8_t vol);

/**
 * @brief 获取播放状态
 * 
 * @return mini_player_state
 */
mini_player_state lisa_miniplayer_get_state();

/**
 * @brief 	销毁播放器
*/
// void lisa_miniplayer_destory();

#endif //__CSK_MINI_PLAYER_H__