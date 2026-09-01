#include <string.h>
#include <stdio.h>
#include "mini_player.h"
#include "lisa_log.h"
#include "lisa_mem.h"
#include "appinc.h"
#include "lisa_time.h"
#include "amplifier.h"
#include "mp3dec.h"
#include "mp3common.h"
#include "amplifier.h"
#include "task_algo.h"
#include "ls_app_device.h"
#include "../tone/tone_buf.h"
#if (EXTER_PA_ENABLE == 1)
#include "pa_manager.h"
#endif

#define TAG "mini-player"

#define LS_PA_ON_MAX_TIME           (12000) // 12秒
#define MINI_PLAYER_ONE_PLAY_SIZE 	(320)
#define PCM_MIN_FRAME_SIZE 			(MINI_PLAYER_ONE_PLAY_SIZE)
#define MP3_DECODE_FRAME_SIZE		(576)
#define HEAD_SKIP_TIME 				(0)  // 配置跳过头 50ms 的音频
#define MAX_SEND_BUFFER_COUNT		(5)
#define CLASSD_WRITE_TIMEOUT_MS     (200)
#define MINI_PLAYER_STOP_WAIT_MS    (CLASSD_WRITE_TIMEOUT_MS + 300)
#define MINI_PLAYER_ALGO_WAIT_MS    (500)

// 播放音频缓存队列
typedef struct {
    uint8_t buffer[MAX_SEND_BUFFER_COUNT][MINI_PLAYER_ONE_PLAY_SIZE];
    uint8_t curr_pos;
} play_buffer_data_t;

static mini_player_t *s_handle = NULL;
static mod_inst_t s_play_mdrv;
static unsigned char s_play_audio[MINI_PLAYER_ONE_PLAY_SIZE];
static int s_play_curr_pos = 0;
static play_buffer_data_t s_play_buffer;
static short s_pcm_data[MP3_DECODE_FRAME_SIZE];
static volume_ctx_t s_volume_ctx = {
	.amp_min = -100,
	.amp_max = 0,
	.amp_cur = 0,
	.multiplier = 1, /* according to amp_cur */
};

static void drain_semaphore(lisa_semaphore_t *sem)
{
	while (lisa_semaphore_take(sem, 0) == LISA_OK) {
	}
}

#define MP3DEC_PER_TIME_COST (0)
#define MP3DEC_TIME_COST (0)
#define LOAD_MP3DEC (0)
#include <sys/time.h>
extern uint64_t dmacp_gettime(struct timeval* tp);
extern void dmacp_getspan(struct timeval* tpd, struct timeval* tp2, struct timeval* tp1);
extern void app_run_config_save();

#if MP3DEC_PER_TIME_COST
struct timeval tp3, tp4;
#endif

#if MP3DEC_TIME_COST
struct timeval tp0, tp1, tp2;
static uint32_t g_pcm_total_size = 0;
#endif

static void load_mp3dec_section(void)
{
#if LOAD_MP3DEC
	extern uint32_t __lmaof_mp3dec, __vmaof_mp3dec, __lenof_mp3dec;
	// dmacopy(&__vmaof_mp3dec, &__lmaof_mp3dec, (int)&__lenof_mp3dec, 1000);
	memcpy(&__vmaof_mp3dec, &__lmaof_mp3dec, (int)&__lenof_mp3dec);
	__FENCE_I();
#endif
}

static void process_16bits(int16_t *const pcm_in, uint16_t sample_cnt)
{
	if (s_volume_ctx.amp_cur > -100) {
		amplifier_16bits(pcm_in, sample_cnt, pcm_in, s_volume_ctx.multiplier);
	} else {
		memset(pcm_in, 0, sample_cnt << 1);
	}
}

int send_audio_to_dac(const void *const data, uint32_t data_size)
{
#if MP3DEC_TIME_COST
	g_pcm_total_size += data_size;
	return 0;
#endif
	int ret = 0;
	memcpy(s_play_buffer.buffer[s_play_buffer.curr_pos], data, data_size);
	ret = __mapi_write(s_play_mdrv, s_play_buffer.buffer[s_play_buffer.curr_pos], data_size >> 1, CLASSD_WRITE_TIMEOUT_MS);
	if (ret != pdTRUE) {
		LISA_LOGE(TAG, "send pcm error %d !!!", ret);
	}
	s_play_buffer.curr_pos++;
	if (s_play_buffer.curr_pos == MAX_SEND_BUFFER_COUNT) {
		s_play_buffer.curr_pos = 0;
	}
	return ret;
}

void algo_process_complete()
{
	if (s_handle->m_state != MINI_PLAYER_ST_PLAYING) app_run_config_save();

	if (s_handle->m_want_play) {
		lisa_semaphore_give(s_handle->m_play_sem);
		s_handle->m_want_play = false;
		// set playing state
		s_handle->m_state = MINI_PLAYER_ST_PLAYING;
		if (s_handle->cb) {
			s_handle->cb(MINI_PLAYER_ST_PLAYING);
		}
	}
}

static int mp3_dec(const uint8_t *const in_buff, int in_size, uint8_t is_first, uint8_t need_skip)
{
	HMP3Decoder hMP3Decoder;  // mp3解码器指针
	MP3FrameInfo mp3FrameInfo;  // mp3帧信息
	int bytesLeft = in_size;  // 从mp3文件读入缓冲区中的剩余字节数
	int offset = 0;  // 读偏移指针
	int frames = 0;  // 记录数据帧个数
	int err = 0, outputSamps = 0, decode_size = 0;
	uint8_t *readPtr = (uint8_t *)in_buff;

	/* 初始化MP3解码器 */
	hMP3Decoder = MP3InitDecoder();
	if (hMP3Decoder == 0) {
		LISA_LOGE(TAG, "decoder init failed\n");
		return MINI_PLAYER_DECODE_ERR;
	}
	// LISA_LOGD(TAG, "decoder init success\n");

	uint32_t droped_size = 0;   //已被抛弃的数据长度
	uint32_t head_skip_size = need_skip ? (HEAD_SKIP_TIME << 5) : 0;
	while (bytesLeft > 0) {
		if (s_handle->m_want_stop) {
			// LISA_LOGD(TAG, "m_want_stop");
			MP3FreeDecoder(hMP3Decoder);
			return MINI_PLAYER_DECODE_EXIT;
		}

		#if MP3DEC_PER_TIME_COST
		dmacp_gettime(&tp3);
		#endif

		offset = MP3FindSyncWord(readPtr, bytesLeft);
		if (offset < 0) {  // 没找到数据帧（一般都能找到，毕竟ID3V2数据不多）
			LISA_LOGE(TAG, "not find frame readPtr = 0x%p, bytesLeft = %d\n", readPtr, bytesLeft);
			MP3FreeDecoder(hMP3Decoder);
			return MINI_PLAYER_DECODE_ERR;
		} else {  // 找到数据帧
			// LISA_LOGV(TAG, "find frame readPtr = 0x%p, bytesLeft = %d, offset = %d\n", readPtr, bytesLeft, offset);
			readPtr += offset;  // 偏移至同步字的位置
			bytesLeft -= offset;  // 同步字之后的数据大小（可能含多个数据帧或不足一个数据帧）

			err = MP3Decode(hMP3Decoder, &readPtr, &bytesLeft, s_pcm_data, 0);  // 调用一次解码一帧数据帧
			frames++;

			if (err != ERR_MP3_NONE) {  // 解码错误
				switch (err) {
					case ERR_MP3_INDATA_UNDERFLOW:
						LISA_LOGW(TAG, "ERR_MP3_INDATA_UNDERFLOW\r\n");
					case ERR_MP3_MAINDATA_UNDERFLOW:
						LISA_LOGW(TAG, "ERR_MP3_MAINDATA_UNDERFLOW\r\n");
						break;
					case ERR_MP3_OUT_OF_MEMORY:
						LISA_LOGE(TAG, "ERR_MP3_OUT_OF_MEMORY\r\n");
						MP3FreeDecoder(hMP3Decoder);
						return MINI_PLAYER_DECODE_ERR;
					default:
						LISA_LOGW(TAG, "UNKNOWN ERROR:%d\r\n", err);
						// 跳过此帧
						if (bytesLeft > 0) {
							bytesLeft--;
							readPtr++;
						}
						break;
				}
			} else {  // 解码正常
				MP3GetLastFrameInfo(hMP3Decoder, &mp3FrameInfo);  // 获取解码信息
				outputSamps = mp3FrameInfo.outputSamps;  // PCM数据个数

				#if MP3DEC_PER_TIME_COST
				dmacp_gettime(&tp4);
				uint32_t cost_us = (tp4.tv_sec*1000000 + tp4.tv_usec - (tp3.tv_sec*1000000 + tp3.tv_usec));
				LISA_LOGD(TAG, "smpls:%d, %ld us, ", outputSamps, cost_us);
				#endif

				uint8_t *out = (uint8_t *)(s_pcm_data);
				decode_size =  outputSamps << 1;
				int buf_size = decode_size;
				// LISA_LOGV(TAG, "decode frames=%d, bytesLeft=%d, buf_size=%d\n", frames, bytesLeft, buf_size);

				//判断是否满足前跳长度
				if (droped_size < head_skip_size) {
					if(buf_size + droped_size <= head_skip_size) {
						droped_size += buf_size;
						// LISA_LOGV(TAG, "droped left %lu Byte data", droped_size);
						continue;
					} else {
						int last_drop_size = head_skip_size - droped_size;
						buf_size -= last_drop_size;
						out += last_drop_size;
						// LISA_LOGV(TAG, "droped last %d Byte data", last_drop_size);
						droped_size = head_skip_size;
					}
				}

				process_16bits((int16_t *)out, buf_size >> 1);

				if (s_play_curr_pos > 0 && s_play_curr_pos < MINI_PLAYER_ONE_PLAY_SIZE) {
					int need_size = MINI_PLAYER_ONE_PLAY_SIZE - s_play_curr_pos;
					memcpy(s_play_audio + s_play_curr_pos, out, need_size);
					buf_size -= need_size;
					s_play_curr_pos = 0;
					send_audio_to_dac(s_play_audio, MINI_PLAYER_ONE_PLAY_SIZE);
				}

				while (buf_size >= MINI_PLAYER_ONE_PLAY_SIZE) {
					send_audio_to_dac(out + decode_size - buf_size, MINI_PLAYER_ONE_PLAY_SIZE);
					buf_size -= MINI_PLAYER_ONE_PLAY_SIZE;
					if (s_handle->m_want_stop) {
						buf_size = 0;
						s_play_curr_pos = 0;
						break;
					}
				}
				if (buf_size > 0) {
					s_play_curr_pos = buf_size;
					memcpy(s_play_audio, out + decode_size - buf_size, buf_size);
				}
			}
		}
	}

	// 最后未播完的补全播放
	if (s_play_curr_pos > 0) {
		memset(s_play_audio + s_play_curr_pos, 0, MINI_PLAYER_ONE_PLAY_SIZE - s_play_curr_pos);
		send_audio_to_dac(s_play_audio, MINI_PLAYER_ONE_PLAY_SIZE);
		s_play_curr_pos = 0;
	}

	MP3FreeDecoder(hMP3Decoder);
	// LISA_LOGD(TAG, "mp3 decode complete");
	return MINI_PLAYER_DECODE_OK;
}

static int pcm_dec(const uint8_t *const in_buff, int in_size, uint8_t is_first)
{
	long use_size = 0;
	int pcm_once_size = 0;
	while (use_size < in_size) {
		if (s_handle->m_want_stop) {
			// LISA_LOGD(TAG, "m_want_stop");
			return MINI_PLAYER_DECODE_EXIT;
		}

        pcm_once_size = (in_size - use_size >= PCM_MIN_FRAME_SIZE) ? PCM_MIN_FRAME_SIZE : (in_size - use_size);
		if (pcm_once_size < PCM_MIN_FRAME_SIZE) {
			memset(s_play_audio, 0, PCM_MIN_FRAME_SIZE);
		}
        memcpy(s_play_audio, in_buff + use_size, pcm_once_size);
		use_size += pcm_once_size;
		send_audio_to_dac(s_play_audio, PCM_MIN_FRAME_SIZE);
	}
	// LISA_LOGD(TAG, "pcm decode complete");
	return MINI_PLAYER_DECODE_OK;
}

static int audio_dec_list(void *const data)
{
	struct audio_item *const item = data;
	int ret;
	while (1) {
		if (item->tone_pos >= item->tone_size) {
			return 0;
		}
		if (s_handle->m_want_stop) {
			// LISA_LOGD(TAG, "audio_dec_list m_want_stop");
			return MINI_PLAYER_DECODE_EXIT;
		}
		uint8_t is_first = (item->tone_pos == 0) ? 1 : 0;
		uint8_t tone_id = item->tondid[item->tone_pos];

		const uint8_t *addr = tone_id_list[tone_id].data;
		const uint32_t size = tone_id_list[tone_id].len;
		LISA_LOGD(TAG, "play id : %d start:%p length:%lu ", tone_id, addr, size);

		if (item->tone_is_pcm) {
			ret = pcm_dec(addr, size, is_first);
		} else {
			#if MP3DEC_TIME_COST
			g_pcm_total_size = 0;
			dmacp_gettime(&tp0);
			#endif

			load_mp3dec_section();

			#if MP3DEC_TIME_COST
			dmacp_gettime(&tp1);
			uint32_t cost_us = (tp1.tv_sec*1000000 + tp1.tv_usec - (tp0.tv_sec*1000000 + tp0.tv_usec));
			LISA_LOGD(TAG, "mp3 load cost_us:%ld", cost_us);
			#endif

			ret = mp3_dec(addr, size, is_first, item->need_skip);

			#if MP3DEC_TIME_COST
			dmacp_gettime(&tp2);
			uint32_t cost_ms = (tp2.tv_sec*1000000 + tp2.tv_usec - (tp1.tv_sec*1000000 + tp1.tv_usec))/1000;
			uint32_t pcm_speed = g_pcm_total_size*1000/1024/cost_ms;
			uint32_t mp3_speed = size*1000/1024/cost_ms;
			LISA_LOGD(TAG, "=====play id : %d, pcmsize:%ld, mp3size:%ld, t1:%lld.%06ld, t2:%lld.%06ld, cost:%ld ms, speed pcm:%ld KB/s, mp3:%ld KB/s",
				tone_id, g_pcm_total_size, size, tp1.tv_sec, tp1.tv_usec, tp2.tv_sec, tp2.tv_usec, cost_ms, pcm_speed, mp3_speed);
			#endif

		}
		if (ret == MINI_PLAYER_DECODE_EXIT) {
			// LISA_LOGD(TAG, "audio dec m_want_stop");
			return MINI_PLAYER_DECODE_EXIT;
		}
		item->tone_pos++;
	}

	return 0;
}

static void _mini_player_td_func(void *arg)
{
	mini_player_msg_t msg;
	while (s_handle->m_running) {
		lisa_queue_pop(s_handle->m_msg_que, &msg, sizeof(mini_player_msg_t), LISA_OS_WAIT_FOREVER);
		if (msg.type == MINI_PLAYER_MSG_NONE) {
			continue;
		}
		if (msg.type == MINI_PLAYER_MSG_PLAY) {
			// set playing state
			if (s_handle->m_state != MINI_PLAYER_ST_PLAYING) {
				s_handle->m_state = MINI_PLAYER_ST_PLAYING;
				if (s_handle->cb) {
					s_handle->cb(MINI_PLAYER_ST_PLAYING);
				}
			}

			audio_dec_list(&(s_handle->m_item));

			s_handle->m_item.tone_size = 0;
			s_handle->m_item.tone_pos = 0;
			if (s_handle->m_want_stop) {
				s_handle->m_want_stop = false;
				s_handle->m_state = MINI_PLAYER_ST_STOPED;
			} else {
				// reset state
				s_handle->m_state = MINI_PLAYER_ST_COMPLETE;
			}
			if (s_handle->cb) {
				s_handle->cb(s_handle->m_state);
			}
			lisa_semaphore_give(s_handle->m_stop_sem);
		}
	}
	lisa_thread_delete(NULL);
}

static int __fast__ mini_play_event(uint32_t uarg, void *parg, void *user)
{
	LISA_LOGD(TAG, "mini player done");
	return 0;
}

void lisa_miniplayer_create(char *dec_share_mem)
{
	if (s_handle != NULL) {
		return;
	}
	LISA_LOGD(TAG, "mini player create [in]");
	s_handle = (mini_player_t *)lisa_mem_calloc(1, sizeof(mini_player_t));
	if (!s_handle) {
		LISA_LOGD(TAG, "mini player create [err]");
		return;
	}

	SetMP3DecShareAddr(dec_share_mem);

	s_play_mdrv = __MAPI_FIND(CONFIG_CLSD_NAME);
	__mapi_hook(s_play_mdrv, mini_play_event, NULL);

	s_handle->m_state = MINI_PLAYER_ST_NONE;
	s_handle->m_want_stop = 0;
	s_handle->m_want_play = 0;

	s_handle->m_stop_sem = lisa_semaphore_create(1);
	if (!s_handle->m_stop_sem) {
		LISA_LOGE(TAG, "create micor player stop sem fail");
		goto EXIT_MINI_STOP_SEM_FAIL;
	}

	s_handle->m_play_sem = lisa_semaphore_create(1);
	if (!s_handle->m_play_sem) {
		LISA_LOGE(TAG, "create micor player play sem fail");
		goto EXIT_MINI_PLAY_SEM_FAIL;
	}

	s_handle->m_msg_que = lisa_queue_create(MINI_PLAYER_MSG_QUE_COUNT,
			(uint8_t *)(MINI_PLAYER_MSG_QUE_NAME), sizeof(mini_player_msg_t));
	if (!s_handle->m_msg_que) {
		LISA_LOGE(TAG, "create micor player msg queue fail");
		goto EXIT_MINI_MSG_QUEUE_FAIL;
	}

	s_handle->m_running = 1;

	lisa_thread_attr_t attr;
	attr.name = (uint8_t *)MINI_PLAYER_TD_NAME;
	attr.priority = MINI_PLAYER_TD_PRIORITY;
	attr.stack_size = MINI_PLAYER_TD_STACK_SIZE;
	s_handle->m_td = lisa_thread_create(&attr, _mini_player_td_func, NULL);
	if (!s_handle->m_td) {
		LISA_LOGE(TAG, "create mini player thread fail");
		goto EXIT_MINI_THREAD_FAIL;
	}

	LISA_LOGD(TAG, "mini player create [out]");
	return;
EXIT_MINI_THREAD_FAIL:
	lisa_queue_delete(s_handle->m_msg_que);
EXIT_MINI_MSG_QUEUE_FAIL:
	lisa_semaphore_delete(s_handle->m_play_sem);
EXIT_MINI_PLAY_SEM_FAIL:
	lisa_semaphore_delete(s_handle->m_stop_sem);
EXIT_MINI_STOP_SEM_FAIL:
	lisa_mem_free(s_handle);
	LISA_LOGE(TAG, "mini player create [err]");
	return;
}

int lisa_miniplayer_play_by_ids_by_skip(const uint8_t *const audios, uint8_t len, uint8_t need_skip)
{
	if (len > MINI_PLAYER_PLAY_MAX_TONE) {
		LISA_LOGE(TAG, "audio len is large : %d", len);
		return -1;
	}
	if (lisa_miniplayer_stop() != 0) {
		return -1;
	}

#if (EXTER_PA_ENABLE == 1)
	pa_manager_refresh_on_sync(LS_PA_ON_MAX_TIME, "miniplayer_play");
#endif

	if (s_handle) {
		// 算法运行时播音等待算法一大帧处理完成
		if (get_algo_state() == e_state_running) {
			drain_semaphore(s_handle->m_play_sem);
			s_handle->m_want_play = 1;
			if (lisa_semaphore_take(s_handle->m_play_sem, MINI_PLAYER_ALGO_WAIT_MS) != LISA_OK) {
				LISA_LOGW(TAG, "wait algo process timeout");
				s_handle->m_want_play = 0;
			}
		}
		for (int i = 0; i < len; i++) {
			s_handle->m_item.tondid[i] = audios[i];
		}
		s_handle->m_item.tone_size = len;
		s_handle->m_item.tone_pos = 0;
		s_handle->m_item.tone_is_pcm = 0;
		s_handle->m_item.need_skip = need_skip;
		// 发送msg
		mini_player_msg_t msg = {
				.type = MINI_PLAYER_MSG_PLAY,
		};
		if (lisa_queue_push(s_handle->m_msg_que, &msg, sizeof(mini_player_msg_t), 0) != LISA_OK) {
			LISA_LOGE(TAG, "push play event to que fail");
			return -1;
		}
		return 0;
	}
	return -1;
}

int lisa_miniplayer_play(uint8_t tone_id)
{
	return lisa_miniplayer_play_by_ids(&tone_id, 1);
}

int lisa_miniplayer_play_notskip(uint8_t tone_id)
{
	return lisa_miniplayer_play_by_ids_by_skip(&tone_id, 1, false);
}

int lisa_miniplayer_play_by_ids(const uint8_t *const audios, uint8_t len)
{
	return lisa_miniplayer_play_by_ids_by_skip(audios, len, true);
}

int lisa_miniplayer_play_by_ids_notskip(const uint8_t *const audios, uint8_t len)
{
	return lisa_miniplayer_play_by_ids_by_skip(audios, len, false);
}

int lisa_miniplayer_play_pcm(uint8_t tone_id)
{
	return lisa_miniplayer_play_pcm_by_ids(&tone_id, 1);
}

int lisa_miniplayer_play_pcm_by_ids(const uint8_t *const audios, uint8_t len)
{
	if (len > MINI_PLAYER_PLAY_MAX_TONE) {
		LISA_LOGE(TAG, "pcm audio len is large : %d", len);
		return -1;
	}
	if (lisa_miniplayer_stop() != 0) {
		return -1;
	}
	if (s_handle) {
		for (int i = 0; i < len; i++) {
			s_handle->m_item.tondid[i] = audios[i];
		}
		s_handle->m_item.tone_size = len;
		s_handle->m_item.tone_pos = 0;
		s_handle->m_item.tone_is_pcm = 1;
		// 发送msg
		mini_player_msg_t msg = {
				.type = MINI_PLAYER_MSG_PLAY,
		};
		if (lisa_queue_push(s_handle->m_msg_que, &msg, sizeof(mini_player_msg_t), 0) != LISA_OK) {
			LISA_LOGE(TAG, "push play event to que fail");
			return -1;
		}
		return 0;
	}
	return -1;
}

void lisa_miniplayer_set_state_cb(player_status_cb cb)
{
	if (s_handle) {
		s_handle->cb = cb;
	}
}

static int8_t normalize_amp(int8_t amp)
{
	if (amp >= s_volume_ctx.amp_min && amp <= s_volume_ctx.amp_max) {
		return amp;
	} else if (amp < s_volume_ctx.amp_min) {
		return s_volume_ctx.amp_min;
	} else /* if (amp > ctx->amp_max) */ {
		return s_volume_ctx.amp_max;
	}
}

int lisa_miniplayer_set_vol(uint8_t vol)
{
	LISA_LOGD(TAG, "mini player set vol : %d", vol);
	int8_t amp = vol - 100;
	amp = normalize_amp(amp);
	s_volume_ctx.amp_cur = amp;
	s_volume_ctx.multiplier = calc_multiplier(amp);
	return amp;
}


int lisa_miniplayer_stop()
{
	if (s_handle) {
		if (s_handle->m_state == MINI_PLAYER_ST_PLAYING) {
			LISA_LOGD(TAG, "want stop mini player");
			drain_semaphore(s_handle->m_stop_sem);
			s_handle->m_want_stop = 1;
			if (lisa_semaphore_take(s_handle->m_stop_sem, MINI_PLAYER_STOP_WAIT_MS) != LISA_OK) {
				if (s_handle->m_state == MINI_PLAYER_ST_PLAYING) {
					LISA_LOGE(TAG, "stop mini player timeout");
					return -1;
				}
				s_handle->m_want_stop = 0;
			}
		} else {
			// LISA_LOGD(TAG, "mini player not playing, dont need stop");
		}
	}
	return 0;
}

mini_player_state lisa_miniplayer_get_state()
{
	if (s_handle) {
		return s_handle->m_state;
	}
	return MINI_PLAYER_ST_NONE;
}

// void lisa_miniplayer_destory()
// {
// 	LISA_LOGD(TAG, "mini player destory [in]");
// 	if (s_handle) {
// 		// Exit Thread
// 		s_handle->m_running = 0;
// 		mini_player_msg_t msg = {
// 				.type = MINI_PLAYER_MSG_NONE,
// 		};
// 		if (lisa_queue_push(s_handle->m_msg_que, &msg, sizeof(mini_player_msg_t), 0) != LISA_OK) {
// 			LISA_LOGE(TAG, "push exit event to que fail");
// 		}
// 		os_time_dly(50);

// 		// Delete Queue
// 		lisa_queue_delete(s_handle->m_msg_que);
// 		lisa_mem_free(s_handle);
// 		s_handle = NULL;
// 	}
// 	LISA_LOGD(TAG, "mini player destory [out]");
// }
