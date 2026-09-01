#include "appinc.h"
#include "ls_app_factory.h"
#include "amplifier.h"
#include "lisa_log.h"
#include "lisa_thread.h"
#include "lisa_typedef.h"
#include "task_algo.h"
#include "mini_player.h"

#define TAG "factory"

#define LS_FACTORY_EMC_TEST     (0)
#define LS_FACTORY_RECORD_PLAY  (1)

extern int send_audio_to_dac(const void *const data, uint32_t data_size);

#if LS_FACTORY_EMC_TEST
static int s_ft_volume;
static bool s_play;


static volume_ctx_t s_ft_volume_ctx = {
	.amp_min = -100,
	.amp_max = 0,
	.amp_cur = 0,
	.multiplier = 1, /* according to amp_cur */
};

static void ft_process_16bits(int16_t *const pcm_in, uint16_t sample_cnt)
{
	if (s_ft_volume_ctx.amp_cur > -100) {
		amplifier_16bits(pcm_in, sample_cnt, pcm_in, s_ft_volume_ctx.multiplier);
	} else {
		memset(pcm_in, 0, sample_cnt << 1);
	}
}

static int8_t ft_normalize_amp(int8_t amp)
{
	if (amp >= s_ft_volume_ctx.amp_min && amp <= s_ft_volume_ctx.amp_max) {
		return amp;
	} else if (amp < s_ft_volume_ctx.amp_min) {
		return s_ft_volume_ctx.amp_min;
	} else /* if (amp > ctx->amp_max) */ {
		return s_ft_volume_ctx.amp_max;
	}
}

static void play_test_pcm()
{
    s_play = true;
    s_ft_volume = 100;
    uint32_t size = 0;
    mod_inst_t const ft_clsd = __MAPI_FIND(CONFIG_CLSD_NAME);
    aud_step_t *const addr = res_item_by_id(CONFIG_RESMGR_BASE_SOUNDRES + 0, &size);
    const int blks = size / sizeof(aud_step_t);
    int16_t play_buff[160];
    for (int i = 0; i < blks; i++) {
        if (s_ft_volume == 0) {
            // 关闭classd
            __mapi_ctrl(ft_clsd, MAPI_CLSD_CTRL_PLAY_STOP, NULL);
            x_task_sleep(10);
        } else {
            if (s_play) {
                __mapi_ctrl(ft_clsd, MAPI_CLSD_CTRL_PLAY_START, NULL);
                memcpy(play_buff, addr+i, 320);
                ft_process_16bits((int16_t *)play_buff, 160);
                send_audio_to_dac(play_buff, 320);
            } else {
                x_task_sleep(10);
            }
        }
        if (i >= (blks-1)) i = 0;
    }
}

void shell_ft_set_volume(int vol)
{
    SHELL_ITEM_EXPORT("factory.setvol", shell_ft_set_volume, "factory set volume");
    LISA_LOGD(TAG, "factory set vol : %d", vol);
    s_ft_volume = vol;
	int8_t amp = vol - 100;
	amp = ft_normalize_amp(amp);
	s_ft_volume_ctx.amp_cur = amp;
	s_ft_volume_ctx.multiplier = calc_multiplier(amp);
}

void shell_ft_play_start()
{
    SHELL_ITEM_EXPORT("factory.play.start", shell_ft_play_start, "factory play start");
    LISA_LOGD(TAG, "factory play start");
    s_play = true;
}

void shell_ft_play_stop()
{
    SHELL_ITEM_EXPORT("factory.play.stop", shell_ft_play_stop, "factory play stop");
    LISA_LOGD(TAG, "factory play stop");
    s_play = false;
}
#endif

void ls_app_factory_init()
{
#if LS_FACTORY_EMC_TEST
    play_test_pcm();
#endif
}

#if LS_FACTORY_RECORD_PLAY
static bool s_record_play_run = false;
static int s_record_play_time = 3;

static void record_play_proc(void *arg)
{
    send_to_algo_task_msg(e_algo_msg_type_factory_test);
    // 停止播音
    lisa_miniplayer_stop();
    LISA_LOGD(TAG, "algo stoped");
    // 开启录音
    mod_inst_t const ft_aadc = __MAPI_FIND(CONFIG_AADC_NAME);
    ASSERT(ft_aadc, CONFIG_AADC_NAME);
    __mapi_ctrl(ft_aadc, MAPI_AADC_CTRL_REC_START, NULL);

#if CONFIG_I2SO_ENABLE
    mod_inst_t const ft_i2so = __MAPI_FIND(CONFIG_I2SO_NAME);
    ASSERT(ft_i2so, CONFIG_CLSD_NAME);
#endif

    int16_t *record_data = NULL;
    int record_tick = s_record_play_time * 100;
    while (record_tick > 0)
    {
        record_tick--;
        __mapi_read(ft_aadc, &record_data, 0, XINFINITE);
        send_audio_to_dac(record_data, 320);
    #if CONFIG_I2SO_ENABLE
        __mapi_write(ft_i2so, record_data, 160, 0);
    #endif
    }
    LISA_LOGD(TAG, "record play complete");
    __mapi_ctrl(ft_aadc, MAPI_AADC_CTRL_REC_STOP, NULL);
    s_record_play_run = false;
    lisa_thread_delete(NULL);
}

void shell_ft_record_play(int time, int again)
{
    if (s_record_play_run) {
        LISA_LOGD(TAG, "factory record play running");
        return;
    }
    mod_inst_t const ft_aadc = __MAPI_FIND(CONFIG_AADC_NAME);
    ASSERT(ft_aadc, "aadc");
    int gain[2] = {again, 0};
    __mapi_ctrl(ft_aadc, MAPI_AADC_CTRL_SET_GAIN, &gain[0]);      // set gain
    s_record_play_run = true;
    s_record_play_time = time;
    SHELL_ITEM_EXPORT("factory.record.play", shell_ft_record_play, "factory.record.play time gain");
    LISA_LOGD(TAG, "factory record play");
    lisa_thread_attr_t thread_attr;
	thread_attr.name = (uint8_t *)"record_play";
	thread_attr.stack_size = OS_STACK_DEF * 2;
	thread_attr.priority = LISA_OS_PRIORITY_NORMAL;
	lisa_thread_create(&thread_attr, record_play_proc, NULL);
}
#endif