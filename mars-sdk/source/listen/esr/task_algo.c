#if 1
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include "appinc.h"
#include "task_algo.h"
#include "lisa_log.h"
#include "lisa_mem.h"
#include "lisa_thread.h"
#include "lisa_typedef.h"
#include "ls_app_algo.h"
#include "ls_app_msg.h"
#include "lisa_queue.h"
#include "resmgr.h"
#include "ivw_cfg.h"
#include "mini_player.h"
#include "ls_app_device.h"
#include "ivw_errcode.h"
#include "evs_utils.h"
#include "ls_app_config.h"
#include "ls_app_flash.h"
#include "ls_app_cfg_mgr.h"

#define TAG "algo-task"

////////////////////////////////////////////////////////////////////////////////////////////////////
int cloglvl = 0;
int logDbg(const char *fmt, ...)
{
    return 0;
}
int portIvwTrace(int level, const char *fmt, ...)
{
    // switch (level) {
    // default:
    // case LOG_LEVEL_N: return 0;
    // case LOG_LEVEL_I: level = LL_INFO; break;
    // case LOG_LEVEL_V: level = LL_VERB; break;
    // case LOG_LEVEL_D: level = LL_DBG ; break;
    // case LOG_LEVEL_W: level = LL_WARN; break;
    // case LOG_LEVEL_E: level = LL_ERR ; break;
    // }
    // char *const buf = x_malloc(CONFIG_SHELL_LINE_LIMIT + 1, 1);
    // va_list ap;
    // va_start(ap, fmt);
    // buf[vsprintf(buf, fmt, ap)] = 0;
    // va_end(ap);
    // _LOG_(level, "[ALGO: %s]", buf);
    // x_free(buf);

    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);

    // printk(fmt, args);
    return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

typedef enum { e_stage_main, e_stage_esr } wake_stage_e;
typedef enum { e_thre_grade_disable, e_high_thre_grade, e_low_thre_grade } algo_thre_grade_e;
typedef enum { e_algo_start = 0, e_algo_stop } algo_cmd_e;

typedef struct {
	int16_t mic1;
} esr_mic_ref_t;

typedef struct esr_algo_s {
	WIVW_INST           hIvwInst;
	uint8_t 			*IvwInstAddr;       //提供给算法的实例内存
    uint8_t 			*IvwShareAddr;      //提供给算法的共享内存
    uint8_t 			*VoiceBinAddr;      //提供给voice.bin的共享内存
    uint32_t            IvwRamSize;   //内存池总大小，单位字节
    uint32_t            VoiceSize;   //voice.bin最大大小，单位字节
	TIvwRes 			res_mgr;
} esr_algo_t;

typedef struct task_algo_s {
    uint8_t             m_running;
	esr_algo_state_t    m_cur_state;
	esr_algo_state_t    m_prev_state;
    bool				m_is_playing;		  //当前是否正在播放提示音
    bool                m_is_ft_test;         //当前是否是产测模式
    uint32_t		    m_esr_hold_frms;  	  //超时时间
    uint32_t		    m_thre_grade_hold_frms; //算法高门限超时时间
    algo_thre_grade_e   m_thre_grade_state;     //当前算法门限状态
	wake_stage_e	 	m_algo_mode;	      //算法模式
	int	 				m_wkchn;			
	esr_algo_t			*m_esr_handle;
	// uint32_t			res_addr;			 //算法模型资源地址
	char 				*m_wakeup_word;		 //当前唤醒词
    lisa_queue_t        *m_msg_queue;
    tIvwRegistInfo      m_regInfo;
} task_algo_t;

typedef enum {
    VOICE_REG_STATUS_INIT = 0,
    VOICE_REG_STATUS_READY,
    VOICE_REG_STATUS_START,
    VOICE_REG_STATUS_SUCCESS,
    VOICE_REG_STATUS_FAILED,
    VOICE_REG_STATUS_TIMEOUT_QUIT,
    VOICE_REG_STATUS_USER_QUIT,
    VOICE_REG_STATUS_DELETE
} reg_status_t;

#define ALGO_RES_BASE       (CONFIG_RESMGR_BASE_ALGORES)
#define ALGO_RES_MODEL      (ALGO_RES_BASE + 0)
#define ALGO_RES_MLPUNET    (ALGO_RES_BASE + 1)
#define ALGO_RES_MAINKEY    (ALGO_RES_BASE + 2)
#define ALGO_RES_CMDKEY     (ALGO_RES_BASE + 3)
#define ALGO_RES_VOICE      (ALGO_RES_BASE + 4)

#define	ALIGN_SIZE(size, align)		((size + (align - 1)) & (~(align - 1)))
#define	ALIGN_PTR(ptr, align) 		(((size_t)ptr + (align - 1)) & (~(align - 1)))
// task name
#define ALGO_TASK_NAME              ("algo_task")
#define ALGO_TASK_TD_STACK_SIZE     (OS_STACK_DEF * 10)
#define ALGO_TASK_QUEUE_COUNT		(16)

#define ALGO_ESR_BLK_SAMPS 			(160)
#define ALGO_ESR_BLK_CNT   			(8)
#define MIC_SAMPLE_RATE   			(16000)
#define ALGO_MSG_QUEUE_NAME         ("algo_msg")
#define ALGO_MSG_QUEUE_COUNT_MAX    (3)

#define ALGO_TIMEOUT_FRMS	            (188)//默认15秒, x * 40ms
#define ALGO_THRE_GRADE_TIMEOUT_FRMS	(11250)  //默认15 * 60秒, x * 40ms
#define SECS2FRMS(SECS)                 ((SECS) / 0.08f) // 10ms per frame

#define TAG_BASE(x) 			        (x * 1000)
#define CONSISTENCY_TAG_VALUE(blk, tag) ((blk < 1) ? tag : 20000)

#define     ABSORB_WORD_KEY_START           ("\"bAbsorb\":")
#define     ABSORB_WORD_KEY_START_LEN       (10)
#define     ABSORB_WORD_KEY_END             ("}")
#define     MAIN_WORD_KEY_START             ("\"bMain\":")
#define     MAIN_WORD_KEY_START_LEN         (8)
#define     MAIN_WORD_KEY_END               (":")
#define     WAKPUP_WORD_KEY_START           ("\"keyword\":\"")
#define     WAKPUP_WORD_KEY_START_LEN       (11)            // "keyword":"
#define     WAKPUP_WORD_KEY_END             ("\"")
#define     LS_ALGO_KEYWORD_MAX_LEN         (64)

static task_algo_t  *s_handle = NULL;
static mod_inst_t   s_aadc_mdrv;
static uint32_t     s_algo_tick;

#if ENABLE_CONSISTENCY
#if CONFIG_I2SO_ENABLE
static mod_inst_t   s_i2s_mdrv;
#endif
static uint16_t     esr_in_data[ALGO_ESR_BLK_SAMPS];
#endif
static uint32_t		esr_hold_frms = ALGO_TIMEOUT_FRMS;
static uint32_t		thre_grade_hold_frms = ALGO_THRE_GRADE_TIMEOUT_FRMS;
static bool         s_first_frame = false;
static reg_status_t s_reg_status = VOICE_REG_STATUS_INIT;
static uint8_t error_retry_cnt = 0;
static register_type_t s_reg_type = REGISTER_TYPE_UNKONW;

typedef struct reg_cmd
{
    char *intent_str;
    uint8_t tone_id;
} reg_cmd_t;

typedef struct
{
    char play_audio[8];
    uint8_t audio_size;
} play_audio_t;

#define REGISTER_MAX_WORD_NUM   (20)    //注册最大词数量 3个唤醒词 + 17个命令词

static tIvwRegistInfo s_tmp_regInfo = {0};
static int s_reg_cmd_list_index = 0;
static reg_cmd_info_t s_reg_cmd_info;
static char s_intr_cmd_keyword[64] = {0};
static char s_intr_cmd_index = 0;

// typedef short algo_samp_t, algo_block_t[ALGO_ESR_BLK_SAMPS];
// algo_block_t *xrec = NULL;

extern void algo_process_complete();
extern void clear_voice_reg_flag(int reset_waked);

#if ENABLE_CRC_CHECK
static uint16_t     mic_crc_data[ALGO_ESR_BLK_CNT][ALGO_ESR_BLK_SAMPS];
static bool         s_crc_start = false;
#endif

#if ENABLE_CONSISTENCY
uint32_t send_adc_data_to_i2s(uint16_t *data, int len)
{
    #if CONFIG_I2SO_ENABLE
        __mapi_write(s_i2s_mdrv, data, len, 0);
    #endif
    return 0;
}

static void consistency_tag_send(void)
{
    for (int samp = 0; samp < ALGO_ESR_BLK_SAMPS; samp ++) {
        esr_in_data[samp] = 20000;
    }
    for (int blk = 0; blk < 8; blk ++) {
        send_adc_data_to_i2s(esr_in_data, ALGO_ESR_BLK_SAMPS);
    }
    s_first_frame = false;
}
#endif//ENABLE_CONSISTENCY

static int get_json_keyword_int_value(const char *json, char *key, int key_len, char *end)
{
    char *start_pos = strstr(json, key);
    if(start_pos == NULL)  {
        LISA_LOGE(TAG, "no %s key", key);
        return -1;
    }
    char *end_pos = strstr(start_pos + key_len, end);
    if(end_pos == NULL) {
        LISA_LOGE(TAG, "no %s end", end);
        return -1;
    }
    uint16_t real_value_len = end_pos - start_pos - key_len;
    // LISA_LOGD(TAG, "real_value_len = %d", real_value_len);
    char *value_str = (char *)lisa_mem_calloc(1, real_value_len + 1);
    value_str[real_value_len] = '\0';
    memcpy(value_str, start_pos + key_len, real_value_len);
    // LISA_LOGD(TAG, "value_str = %s", value_str);
    int value = atoi(value_str);
    lisa_mem_free(value_str);

    return value;
}

static bool s_reging_special_cmd_process = false;  //注册过程中特殊命令处理
static bool s_is_user_retry = false;
int __handle_voice_reg_absorb_conflict(void *usr_data)
{
    LISA_LOGD(TAG, "absorb word conflict");
    voice_register_status_process(REGISTER_STATUS_ERROR_CONFLICT, NULL, 0);

    return 0;
}

static int process_wakeup_absorb_wrod(const char *p_result, uint8_t *keyword, int max_len)
{
    int absorb_value = get_json_keyword_int_value(p_result, ABSORB_WORD_KEY_START, ABSORB_WORD_KEY_START_LEN, ABSORB_WORD_KEY_END);
    ///吸收词，不做任何处理
    if(absorb_value) {
        // LISA_LOGD(TAG, "absorb word = %s", p_result);
        if (s_reg_status != VOICE_REG_STATUS_INIT) {
            s_reging_special_cmd_process = true;
            evs_handler_post_runnable(__handle_voice_reg_absorb_conflict, NULL);
        }
        return -1;
    }
#if 0
    int main_value = get_json_keyword_int_value(p_result, MAIN_WORD_KEY_START, MAIN_WORD_KEY_START_LEN, MAIN_WORD_KEY_END);
    ///与当前唤醒词匹配
    if(main_value && s_handle->m_wakeup_word != NULL) {
        int ret = ls_app_algo_keyword_extract((const uint8_t *)p_result, keyword, max_len, NULL, 0);
        if (ret != 0) {
            LISA_LOGE(TAG, "extra keyword fail");
            return -1;
        }
        if(strncmp(s_handle->m_wakeup_word, (const char *)keyword, strlen(s_handle->m_wakeup_word))) {
            // LISA_LOGI(TAG, "current wakeup word = %s, but keyword = %s", s_handle->m_wakeup_word, keyword);
            return -1;
        }
    } else if (!main_value) {
        // 命令词
        int ret = ls_app_algo_keyword_extract((const uint8_t *)p_result, keyword, max_len, NULL, 0);
        if (ret != 0) {
            LISA_LOGE(TAG, "extra keyword fail");
            return -1;
        }
    }
#endif

    return ls_app_algo_keyword_extract((const uint8_t *)p_result, keyword, max_len);
}

esr_algo_state_t get_algo_state()
{
    if (s_handle) {
        return s_handle->m_cur_state;
    }
    return e_state_idle;
}

static void algo_status(esr_algo_state_t new)
{
    s_handle->m_prev_state = s_handle->m_cur_state;
    s_handle->m_cur_state = new;
}

static int32_t init_res_mgr(PIvwRes p_res_mgr, int32_t res_count, char * fsmn_res, char * mlp_cnn_res,
	char * wfst_res, char * main_keyword_res, char * asr_keyword_res, char * mlp_unet_res, char * voice_res)
{
	int32_t ret = 0;

	p_res_mgr->res_buf_arr_[0] = fsmn_res;
	p_res_mgr->res_buf_arr_[1] = wfst_res;
	p_res_mgr->res_buf_arr_[2] = main_keyword_res;
	p_res_mgr->res_buf_arr_[3] = asr_keyword_res;
	p_res_mgr->res_buf_arr_[4] = mlp_cnn_res;
    p_res_mgr->res_buf_arr_[5] = mlp_unet_res;
    p_res_mgr->res_buf_arr_[7] = voice_res;

	return ret;
}

static void print_esr_info(PIvwInfo ivwInfo)
{
    LISA_LOGD(TAG, "Engine info: =======================================");
	LISA_LOGD(TAG, "Version           = %s",ivwInfo->version);
	LISA_LOGD(TAG, "MLP TYPE, Channel = %ld, %ld",ivwInfo->mlp_res_type,ivwInfo->input_pcm_channel);
	LISA_LOGD(TAG, "Win Frame(In,Out) = %ld, %ld",ivwInfo->input_pcm_frame_count,ivwInfo->input_pcm_frame_count * 10 / ivwInfo->output_frame_ms);
	LISA_LOGD(TAG, "Keyword(Main,Asr) = %ld, %ld",ivwInfo->main_word_count,ivwInfo->asr_word_count);
	LISA_LOGD(TAG, "Size(Inst,share)  = %ld, %ld",ivwInfo->inst_mem_size,ivwInfo->share_mem_size);
	LISA_LOGD(TAG, "====================================================");
}

static uint8_t s_tmp_keyword[LS_ALGO_KEYWORD_MAX_LEN] = {0};
static bool s_want_save_cfg = false;

static void save_app_config()
{
    if (s_want_save_cfg) {
        s_want_save_cfg = false;
        ls_app_cfg_refresh_all(CFG_REFRESH_ALL);
    }
}

int __handle_voice_reg_timeout_quit(void *usr_data)
{
    LISA_LOGD(TAG, "handle voice reg timeout quit!!!");

    s_reg_status = VOICE_REG_STATUS_TIMEOUT_QUIT;

    send_to_algo_task_msg(e_algo_msg_type_esr_restart);
    send_to_algo_task_msg(e_algo_msg_type_esr_wakeup_mode);

    clear_voice_reg_flag(true);

    audio_info_t audio_info = {0};
    cfg_parser_get_timeout_audio(&audio_info);
    lisa_miniplayer_play_by_ids(audio_info.audios, audio_info.audio_size);

    save_app_config();

    return 0;
}

//注册过程中命令处理
int __handle_reging_cmd_process(void *usr_data)
{
    LISA_LOGD(TAG, "reging process keyword: %s", s_tmp_keyword);
    cfg_parser_register_inprogress_keyword_handler((const char *)s_tmp_keyword);

    return 0;
}

static int32_t IvwCallBackWakeupChannel0(const char* pIvwParam, void *pUserParam)
{
	// LISA_LOGD(TAG, "Channel0 wakeup: %s", pIvwParam);
    memset(s_tmp_keyword, 0, sizeof(s_tmp_keyword)/sizeof(uint8_t));
    if(0 == process_wakeup_absorb_wrod(pIvwParam, s_tmp_keyword, sizeof(s_tmp_keyword) - 1)) {
        s_handle->m_wkchn = 0;

        //在注册过程中唤醒处理
        if (s_reg_status != VOICE_REG_STATUS_INIT) {
	        LISA_LOGD(TAG, "reging wakeup: %s", pIvwParam);
            s_reging_special_cmd_process = true;
            evs_handler_post_runnable(__handle_reging_cmd_process, NULL);
        } else {
            ls_app_msg_send(MSG_TYPE_WAKEUP, (const uint8_t *)pIvwParam, strlen(pIvwParam));
        }
    }

    return 0;
}

static int32_t IvwCallBackPreWakeupChannel0(const char* pIvwParam, void *pUserParam)
{
	// LISA_LOGD(TAG, "Channel0 prewakeup: %s", pIvwParam);
    // if(0 == process_wakeup_absorb_wrod(pIvwParam)) {
    //     ls_app_msg_send(MSG_TYPE_PRE_WAKEUP, (const uint8_t *)pIvwParam, strlen(pIvwParam));
    // }

	return 0;
}

void set_algo_hold_frms(uint32_t frms)
{
    if(NULL == s_handle || 0 == frms) {
        LISA_LOGE(TAG, "set_algo_hold_frms failed");
        return;
    }
    esr_hold_frms = SECS2FRMS(frms);
    s_handle->m_esr_hold_frms = esr_hold_frms;
	LISA_LOGD(TAG, "algo hold secs = %ld hold_frms = %ld", frms, s_handle->m_esr_hold_frms);
}

void set_algo_thre_grade_hold_frms(uint32_t frms)
{
    if(NULL == s_handle || 0 == frms || SECS2FRMS(frms) < esr_hold_frms) {
        LISA_LOGE(TAG, "set_algo_thre_grade_hold_frms failed");
        return;
    }
    thre_grade_hold_frms = SECS2FRMS(frms);
    s_handle->m_thre_grade_hold_frms = thre_grade_hold_frms;
	LISA_LOGD(TAG, "algo thre grade secs = %ld hold_frms = %ld", frms, s_handle->m_thre_grade_hold_frms);    
}

int __handle_voice_reg_res_error(void *usr_data)
{
    LISA_LOGD(TAG, "handle voice reg res error!");
    g_run_cfg.reg_save_size = 0;
    s_reg_cmd_info.status = 0;
    g_run_cfg.reg_cmd_info = s_reg_cmd_info.value;
    s_want_save_cfg = true;
    save_app_config();
    send_to_algo_task_msg(e_algo_msg_type_esr_restart);

    return 0;
}

/**
 * @brief           启动与停止算法
 * @param cmd       e_algo_start:算法启动，e_algo_stop:算法停止
 * @param type      指定启动时的启动类型
 * @return 
 */
static int32_t ai_set_running_state(algo_cmd_e cmd, EResetType type)
{
    int ret = 0; 

    if(NULL == s_handle->m_esr_handle->hIvwInst) {
        LISA_LOGD(TAG, "ai_set_running_state esr handle not exist ...");
        return -1;
    }
    if(e_algo_start == cmd) {
        ret = wIvwStart(s_handle->m_esr_handle->hIvwInst, type);
		if (0 != ret) {
			LISA_LOGD(TAG, "wIvwStart fail | ret:%d", ret);
            if (VOICE_REGIST_ERROR_RES == ret)
                evs_handler_post_runnable(__handle_voice_reg_res_error, NULL);
			return -1;
		}
    }
    else {
        ret = wIvwStop(s_handle->m_esr_handle->hIvwInst);
		if (0 != ret) {
			LISA_LOGD(TAG, "wIvwStop fail | ret:%d", ret);
			return -1;
		}
    }

    return 0;
}

/**
 * @brief           设置算法模式
 * @param wakeup    0:wakeup mode   1:esr mode
 */
static void algo_set_mode(wake_stage_e stage)
{
    int ret = 0;
    int wakeup_type = MAIN;
    int wakeup_mode = WAKEUP_MODE;
    int wdec_delay_max = 0;
    int wdec_delay_min = 0;
    int state_threshold = 500;
    int noise_coefficient = 90;

	if (s_handle->m_algo_mode == stage)
		return;
    ai_set_running_state(e_algo_stop, ALL_RESET);
    if(stage == e_stage_esr) {
        wakeup_type = ASR;
        wakeup_mode = ASR_MODE;
        wdec_delay_max = 20;
        wdec_delay_min = 5;
        state_threshold = 0;
        noise_coefficient = 100;
        if(s_handle->m_thre_grade_state != e_thre_grade_disable)
        {
            ret |= wIvwSetParameter(s_handle->m_esr_handle->hIvwInst, PARAM_W_DEC_THRESHOLD_GRADE, 0);  //高门限关闭
            s_handle->m_thre_grade_state = e_low_thre_grade;
        }
    }
	ret |= wIvwSetParameter(s_handle->m_esr_handle->hIvwInst, PARAM_W_DEC_WAKEUP_MODE, wakeup_mode);
	ret |= wIvwSetParameter(s_handle->m_esr_handle->hIvwInst, PARAM_W_DEC_KEYWORD_TYPE, wakeup_type);
	ret |= wIvwSetParameter(s_handle->m_esr_handle->hIvwInst, PARAM_W_DEC_WAKEUP_DELAY_FRAME_MAX, wdec_delay_max);
	ret |= wIvwSetParameter(s_handle->m_esr_handle->hIvwInst, PARAM_W_DEC_WAKEUP_DELAY_FRAME_MIN, wdec_delay_min);
    ret |= wIvwSetParameter(s_handle->m_esr_handle->hIvwInst, PARAM_W_DEC_WAKEUP_START_STATE_THRESHOLD, state_threshold);
    ret |= wIvwSetParameter(s_handle->m_esr_handle->hIvwInst, PARAM_W_MLP_E2E_MAIN_NOISE_COEFFICIENT, noise_coefficient);
    if (0 != ret) {
		LISA_LOGE(TAG, "algo_set_mode wIvwSetParameter fail | ret:%d", ret);
        return;
	}
    if(ai_set_running_state(e_algo_start, DEFAULT_RESET) != 0) {
        LISA_LOGE(TAG, "algo_set_mode failed");
        return;
    }
    s_handle->m_algo_mode = stage;
	LISA_LOGD(TAG, "MODE=%d", stage);
}

void algo_set_threshold_grade(bool enable)
{
    int ret = 0;

    if(s_handle->m_thre_grade_state == e_thre_grade_disable) return;

    ai_set_running_state(e_algo_stop, ALL_RESET);
    ret |= wIvwSetParameter(s_handle->m_esr_handle->hIvwInst, PARAM_W_DEC_THRESHOLD_GRADE, enable);  //高门限设置，1：开启，0：关闭
	if (0 != ret) {
		LISA_LOGE(TAG, "algo_set_threshold_grade wIvwSetParameter fail | ret:%d", ret);
        return;
	}
    if(ai_set_running_state(e_algo_start, DEFAULT_RESET) != 0) {
        LISA_LOGE(TAG, "algo_set_mode failed");
        return;
    }
    s_handle->m_thre_grade_state = enable ? e_high_thre_grade : e_low_thre_grade;
    LISA_LOGI(TAG, "set algo threshold grade %d ...", enable);  

}

static int32_t ai_create(void)
{
    int ret = 0;
	TIvwInfo ivwInfo = {0};
	TIvwInitParam ivwInitParam;
    uint32_t esr_mlp_size   = 0;
    uint32_t mlp_unet_size  = 0;
	uint32_t esr_main_size  = 0;
	uint32_t esr_cmds_size  = 0;
    uint32_t esr_voice_size  = 0;
    int wakeup_type = MAIN;
    int wakeup_mode = WAKEUP_MODE;
    int wdec_delay_max = 0;
    int wdec_delay_min = 0;
    int state_threshold = 500;
    int noise_coefficient = 90;

    char *buf_cnn_mlp_res = NULL;
    char *buf_mlp_unet_res = NULL;
	char *buf_asr_keywords = NULL;
	char *buf_main_keywords = NULL;
    char *buf_voice_res = NULL;
    
    if(s_handle->m_esr_handle->hIvwInst) {
        LISA_LOGD(TAG, "esr handle exist ...");
        return -1;
    }

    buf_cnn_mlp_res = res_item_by_id(ALGO_RES_MODEL, &esr_mlp_size);
    buf_main_keywords = res_item_by_id(ALGO_RES_MAINKEY, &esr_main_size);
    buf_asr_keywords = res_item_by_id(ALGO_RES_CMDKEY, &esr_cmds_size);
    buf_mlp_unet_res = res_item_by_id(ALGO_RES_MLPUNET, &mlp_unet_size);

    if (g_run_cfg.reg_save_flag == 1) {
        // new flash未保存时，先从old flash中读取默认voice.bin，并拷贝到psram中，并保存到new flash中
        int reg_type = cfg_parser_get_voice_reg_type();
        LISA_LOGD(TAG, "voice reg type: %d", reg_type);

        bool need_overload = false;
        int cmd_res_cnt = -1;
        if (reg_type == 2) {
            cmd_res_cnt = cfg_parser_get_voice_reg_cmd_cnt();
            LISA_LOGD(TAG, "cmd res cnt: %d", cmd_res_cnt);
            if (cmd_res_cnt <= 0) {
                need_overload = false;
            } else if (g_run_cfg.reg_save_size <= 128) {
                g_run_cfg.reg_save_size = 0;
                need_overload = true;
            } else {
                int ret = intr_cmd_voice_bin_check_count(s_handle->m_esr_handle->VoiceBinAddr, cmd_res_cnt);
                if (ret == 0) need_overload = false;
                else need_overload = true;
            }
        }

        if (need_overload) {
            buf_voice_res = res_item_by_id(ALGO_RES_VOICE, &esr_voice_size);
            memcpy(s_handle->m_esr_handle->VoiceBinAddr, buf_voice_res, esr_voice_size);

            int new_voice_size = 0;
            intr_cmd_voice_bin_init(s_handle->m_esr_handle->VoiceBinAddr, cmd_res_cnt, &new_voice_size);
            if (new_voice_size > 0) {
                g_run_cfg.reg_save_size = new_voice_size;
                s_reg_cmd_info.status = 0;
                g_run_cfg.reg_cmd_info = s_reg_cmd_info.value;
                LISA_LOGD(TAG, "copy voice.bin [%d] to psram.", g_run_cfg.reg_save_size);
                s_want_save_cfg = true;
                save_app_config();
            }
        } else {
            if (g_run_cfg.reg_save_size == 0) {
                buf_voice_res = res_item_by_id(ALGO_RES_VOICE, &esr_voice_size);
                memcpy(s_handle->m_esr_handle->VoiceBinAddr, buf_voice_res, esr_voice_size);
                g_run_cfg.reg_save_size = esr_voice_size;
                s_reg_cmd_info.status = 0;
                g_run_cfg.reg_cmd_info = s_reg_cmd_info.value;
                LISA_LOGD(TAG, "copy voice.bin [%d] to psram.", g_run_cfg.reg_save_size);
                s_want_save_cfg = true;
                save_app_config();
            }
        }

        esr_voice_size = g_run_cfg.reg_save_size;
        buf_voice_res = (char *)s_handle->m_esr_handle->VoiceBinAddr;
    } else {
        buf_voice_res = res_item_by_id(ALGO_RES_VOICE, &esr_voice_size);
    }

    LISA_LOGD(TAG, "ESR info:");
    LISA_LOGD(TAG, "- size: mlp=%ld, unet=%ld, main=%ld, cmds=%ld, voice=%ld",  esr_mlp_size, mlp_unet_size, esr_main_size, esr_cmds_size, esr_voice_size);
    LISA_LOGD(TAG, "- addr: mlp=%p, unet=%p, main=%p, cmds=%p, voice=%p",  buf_cnn_mlp_res, buf_mlp_unet_res, buf_main_keywords, buf_asr_keywords, buf_voice_res);

    //init res
    init_res_mgr(&s_handle->m_esr_handle->res_mgr, 5, NULL, buf_cnn_mlp_res, NULL, buf_main_keywords, buf_asr_keywords, buf_mlp_unet_res, buf_voice_res);
    //get info
    ivwInfo.max_regist_num = REGISTER_MAX_WORD_NUM;
	wIvwGetInfo(&ivwInfo, &s_handle->m_esr_handle->res_mgr);
    // LISA_LOGD(TAG, "wIvwGetInfo: inst=%ld share=%ld", ivwInfo.inst_mem_size, ivwInfo.share_mem_size);
	//init miniesr engine inst, create miniesr inst in psram or share memory

	ivwInitParam.inst_mem_addr = (char *)(ALIGN_PTR(s_handle->m_esr_handle->IvwInstAddr, 4));
	ivwInitParam.inst_mem_size = ivwInfo.inst_mem_size;
    ASSERT((uint32_t)ivwInitParam.inst_mem_addr + ivwInitParam.inst_mem_size < AP_RAM_G1,
        "Invalid inst address:[%p, %p]", ivwInitParam.inst_mem_addr, ivwInitParam.inst_mem_addr+ivwInitParam.inst_mem_size);
	ivwInitParam.ivw_res = s_handle->m_esr_handle->res_mgr;
    ivwInitParam.info = &ivwInfo;
    // set miniesr share memory address and size, can not overlap with inst memory
	ivwInitParam.share_mem_addr = (char *)(ALIGN_PTR((s_handle->m_esr_handle->IvwShareAddr), 32));
	ivwInitParam.share_mem_size = ivwInfo.share_mem_size;
    ASSERT((uint32_t)ivwInitParam.share_mem_addr + ivwInitParam.share_mem_size < AP_RAM_G1,
        "Invalid share address:[%p, %p]", ivwInitParam.share_mem_addr, ivwInitParam.share_mem_addr + ivwInitParam.share_mem_size);

    print_esr_info(&ivwInfo);
    if((ivwInfo.inst_mem_size + ivwInfo.share_mem_size) > s_handle->m_esr_handle->IvwRamSize) {
        LISA_LOGE(TAG, "failed: algo need %ld + %ld byte memory but provide %ld Byte", ivwInfo.inst_mem_size, ivwInfo.share_mem_size, s_handle->m_esr_handle->IvwRamSize);
        return -1;
    }
	ret = wIvwCreate(&s_handle->m_esr_handle->hIvwInst, &ivwInitParam);
	if (0 != ret) {
		LISA_LOGE(TAG, "wIvwCreate fail | ret:%d", ret);
		return -1;
	}
    //Register callback
	ret = wIvwRegisterCallBacks(s_handle->m_esr_handle->hIvwInst, 
                        CallBackFuncNameWakeUpChannel0, IvwCallBackWakeupChannel0, NULL);
    ret |= wIvwRegisterCallBacks(s_handle->m_esr_handle->hIvwInst, 
                        CallBackFuncNamePreWakeUpChannel0, IvwCallBackPreWakeupChannel0, NULL);
#if ENABLE_CONSISTENCY
    ///固定为识别模式
    wakeup_type = ASR;
    wakeup_mode = ASR_MODE;
    wdec_delay_max = 20;
    wdec_delay_min = 5;
    state_threshold = 0;
    noise_coefficient = 100;
    LISA_LOGD(TAG, "ENABLE ALGO CONSISTENCY");
#endif
    //SetParameter
    ret |= wIvwSetParameter(s_handle->m_esr_handle->hIvwInst, PARAM_W_DEC_WAKEUP_MODE, wakeup_mode);
    ret |= wIvwSetParameter(s_handle->m_esr_handle->hIvwInst, PARAM_W_DEC_KEYWORD_TYPE, wakeup_type);
    ret |= wIvwSetParameter(s_handle->m_esr_handle->hIvwInst, PARAM_W_FEA_USE_MVA, 1);
    ret |= wIvwSetParameter(s_handle->m_esr_handle->hIvwInst, PARAM_W_DEC_WAKEUP_DELAY_FRAME_MAX, wdec_delay_max);
    ret |= wIvwSetParameter(s_handle->m_esr_handle->hIvwInst, PARAM_W_DEC_WAKEUP_DELAY_FRAME_MIN, wdec_delay_min);
    ret |= wIvwSetParameter(s_handle->m_esr_handle->hIvwInst, PARAM_W_IVW_HISTORY_RESET_FLAG, 0);
    ret |= wIvwSetParameter(s_handle->m_esr_handle->hIvwInst, PARAM_W_DEC_KEYWORD_PREWAKEUP_PRESTATE, 5);
    ret |= wIvwSetParameter(s_handle->m_esr_handle->hIvwInst, PARAM_W_DEC_NCMTHRESHOLD_PREWAKE_KEYWORD1_ARC1, 600);

    ret |= wIvwSetParameter(s_handle->m_esr_handle->hIvwInst, PARAM_W_IVW_E2E_ENABLE, 2); //modify
    ret |= wIvwSetParameter(s_handle->m_esr_handle->hIvwInst, PARAM_W_IVW_MAIN_WAKE_E2E_LOW_THRESHOLD, 1);
    ret |= wIvwSetParameter(s_handle->m_esr_handle->hIvwInst, PARAM_W_IVW_MAIN_WAKE_E2E_NEED_NODE_NUMS, 2);
    ret |= wIvwSetParameter(s_handle->m_esr_handle->hIvwInst, PARAM_W_IVW_MAIN_WAKE_E2E_MAX_LOW_WAKE_FRAME, 63);
    ret |= wIvwSetParameter(s_handle->m_esr_handle->hIvwInst, PARAM_W_IVW_MAIN_WAKE_E2E_MIN_LOW_WAKE_FRAME, 10);

    ret |= wIvwSetParameter(s_handle->m_esr_handle->hIvwInst, PARAM_W_IVW_FSP_ENABLE, 2); //modify
    ret |= wIvwSetParameter(s_handle->m_esr_handle->hIvwInst, PARAM_W_IVW_MAIN_WAKE_ONLY_E2E_INTERVAL_NUMS, 8);
    ret |= wIvwSetParameter(s_handle->m_esr_handle->hIvwInst, PARAM_W_IVW_MAIN_WAKE_ONLY_E2E_DURATION_NUMS, 40);
    ret |= wIvwSetParameter(s_handle->m_esr_handle->hIvwInst, PARAM_W_IVW_FSP_PK_WIN_FRMAES, 5);
    ret |= wIvwSetParameter(s_handle->m_esr_handle->hIvwInst, PARAM_W_IVW_MLC_WAKE_ONLY_E2E_INTERVAL_NUMS, 8);
    ret |= wIvwSetParameter(s_handle->m_esr_handle->hIvwInst, PARAM_W_DEC_WAKEUP_START_STATE_THRESHOLD, state_threshold);
    ret |= wIvwSetParameter(s_handle->m_esr_handle->hIvwInst, PARAM_W_MLP_VAD_START_CONTINUE_NUMS, 2);
    ret |= wIvwSetParameter(s_handle->m_esr_handle->hIvwInst, PARAM_W_MLP_VAD_END_CONTINUE_NUMS, 2);
    ret |= wIvwSetParameter(s_handle->m_esr_handle->hIvwInst, PARAM_W_IVW_FSP_VAD_INTERVAL, 4);

    // ret |= wIvwSetParameter(s_handle->m_esr_handle->hIvwInst, PARAM_W_IVW_FSP_ENDPOINT_INTERVAL_NUMS, 12);
    ret |= wIvwSetParameter(s_handle->m_esr_handle->hIvwInst, PARAM_W_IVW_PK_WITHOUT_VAD, 1);
    ret |= wIvwSetParameter(s_handle->m_esr_handle->hIvwInst, PARAM_W_MLP_E2E_MLC_NOISE_COEFFICIENT, 100);
    ret |= wIvwSetParameter(s_handle->m_esr_handle->hIvwInst, PARAM_W_MLP_FREE_NOISE_COEFFICIENT, 100);
    ret |= wIvwSetParameter(s_handle->m_esr_handle->hIvwInst, PARAM_W_FEA_DENOISE_MASK_FLOOR, 100);

    ret |= wIvwSetParameter(s_handle->m_esr_handle->hIvwInst, PARAM_W_MLP_VAD_LINK_COUNT, 3);
    ret |= wIvwSetParameter(s_handle->m_esr_handle->hIvwInst, PARAM_W_MLP_VAD_LINK_FRAME_NUMS, 4);

    // ret |= wIvwSetParameter(s_handle->m_esr_handle->hIvwInst, PARAM_W_IVW_MAIN_WAKE_E2E_NEED_NODE_WIN, 2);
    ret |= wIvwSetParameter(s_handle->m_esr_handle->hIvwInst, PARAM_W_IVW_MLC_WAKE_E2E_NEED_NODE_NUMS, 2);
    ret |= wIvwSetParameter(s_handle->m_esr_handle->hIvwInst, PARAM_W_IVW_MLC_WAKE_E2E_NEED_NODE_WIN, 4);

    // ret |= wIvwSetParameter(s_handle->m_esr_handle->hIvwInst, PARAM_W_IVW_FSP_WAKE_E2E_NEED_NODE_NUMS, 2);
    // ret |= wIvwSetParameter(s_handle->m_esr_handle->hIvwInst, PARAM_W_IVW_FSP_WAKE_E2E_NEED_NODE_WIN, 2);
    ret |= wIvwSetParameter(s_handle->m_esr_handle->hIvwInst, PARAM_W_MLP_E2E_MAIN_NOISE_COEFFICIENT, noise_coefficient);
    ret |= wIvwSetParameter(s_handle->m_esr_handle->hIvwInst, PARAM_W_IVW_VOICE_REGIST_MODE, 0);

    ret |= wIvwSetParameter(s_handle->m_esr_handle->hIvwInst, PARAM_W_IVW_MLC_DUL_INTENT_ON, 0);
    ret |= wIvwSetParameter(s_handle->m_esr_handle->hIvwInst, PARAM_W_IVW_DUL_INTENT_RANGE, 6);
    ret |= wIvwSetParameter(s_handle->m_esr_handle->hIvwInst, PARAM_W_IVW_DUL_INTENT_DISTANCE, 3);

    ret |= wIvwSetParameter(s_handle->m_esr_handle->hIvwInst, PARAM_W_IVW_VOICE_REGIST_HIGH_SENSITIVITY_MAIN_THRESHOLD, 102);
    ret |= wIvwSetParameter(s_handle->m_esr_handle->hIvwInst, PARAM_W_IVW_VOICE_REGIST_HIGH_SENSITIVITY_ASR_THRESHOLD, 102);
    ret |= wIvwSetParameter(s_handle->m_esr_handle->hIvwInst, PARAM_W_IVW_VOICE_REGIST_MID_SENSITIVITY_MAIN_THRESHOLD, 128);
    ret |= wIvwSetParameter(s_handle->m_esr_handle->hIvwInst, PARAM_W_IVW_VOICE_REGIST_MID_SENSITIVITY_ASR_THRESHOLD, 128);
    ret |= wIvwSetParameter(s_handle->m_esr_handle->hIvwInst, PARAM_W_IVW_VOICE_REGIST_LOW_SENSITIVITY_MAIN_THRESHOLD, 153);
    ret |= wIvwSetParameter(s_handle->m_esr_handle->hIvwInst, PARAM_W_IVW_VOICE_REGIST_LOW_SENSITIVITY_ASR_THRESHOLD, 153);

    // int32_t value;
    // wIvwGetParameter(s_handle->m_esr_handle->hIvwInst, PARAM_W_MLP_E2E_MLC_NOISE_COEFFICIENT, &value);
    // LISA_LOGD(TAG, "PARAM_W_MLP_E2E_MLC_NOISE_COEFFICIENT : %d", value);
    // wIvwGetParameter(s_handle->m_esr_handle->hIvwInst, PARAM_W_FEA_DENOISE_MASK_FLOOR, &value);
    // LISA_LOGD(TAG, "PARAM_W_FEA_DENOISE_MASK_FLOOR : %d", value);

    // 设置pcen参数
    // ret |= wIvwSetParameter(s_handle->m_esr_handle->hIvwInst, PARAM_W_FEA_TYPE, 1);

    if(s_handle->m_thre_grade_state != e_thre_grade_disable) {
        ret |= wIvwSetParameter(s_handle->m_esr_handle->hIvwInst, PARAM_W_DEC_THRESHOLD_GRADE, 1);  //设置高门限开启
        LISA_LOGD(TAG, "ENABLE ALGO THRESHOLD GRADE");
    } else {
        ret |= wIvwSetParameter(s_handle->m_esr_handle->hIvwInst, PARAM_W_DEC_THRESHOLD_GRADE, 0);
    }

    if (s_reg_status == VOICE_REG_STATUS_READY) {
        ret |= wIvwSetParameter(s_handle->m_esr_handle->hIvwInst, PARAM_W_IVW_VOICE_REGIST_MODE, 1);
        ret |= wIvwSetParameter(s_handle->m_esr_handle->hIvwInst, PARAM_W_DEC_KEYWORD_TYPE, ASR);
        ret |= wIvwSetParameter(s_handle->m_esr_handle->hIvwInst, PARAM_W_DEC_WAKEUP_MODE, ASR_MODE);
        ret |= wIvwSetParameter(s_handle->m_esr_handle->hIvwInst, PARAM_W_IVW_PK_WITHOUT_VAD, 0);
        memcpy(&s_handle->m_regInfo, &s_tmp_regInfo, sizeof(tIvwRegistInfo));
    } else if (s_reg_status == VOICE_REG_STATUS_USER_QUIT ||
                s_reg_status == VOICE_REG_STATUS_SUCCESS ||
                s_reg_status == VOICE_REG_STATUS_FAILED ||
                s_reg_status == VOICE_REG_STATUS_DELETE) {
        //用户主动退出时，学习成功/失败后保持识别模式
        ret |= wIvwSetParameter(s_handle->m_esr_handle->hIvwInst, PARAM_W_DEC_KEYWORD_TYPE, ASR);
        ret |= wIvwSetParameter(s_handle->m_esr_handle->hIvwInst, PARAM_W_DEC_WAKEUP_MODE, ASR_MODE);
    }

	if (0 != ret) {
		LISA_LOGE(TAG, "wIvwSetParameter fail | ret:%d", ret);
        s_handle->m_esr_handle->hIvwInst = NULL;
        return -1;
	}

    LISA_LOGD(TAG, "ai create success!!!");

    return 0;
}

static int32_t ai_destroy(void)
{
    if(NULL == s_handle->m_esr_handle->hIvwInst) {
        LISA_LOGD(TAG, "ai_destroy esr handle not exist ...");
        return -1;
    }
    ai_set_running_state(e_algo_stop, ALL_RESET);
    wIvwDestroy(s_handle->m_esr_handle->hIvwInst);
    s_handle->m_esr_handle->hIvwInst = NULL;

    return 0;
}

static void algo_result_proc(void)
{
#if !ENABLE_CONSISTENCY
	int chn = s_handle->m_wkchn;
	if (s_handle->m_wkchn < 0) {
        ///已唤醒，记录超时
		if (s_handle->m_esr_hold_frms > 0) {
			s_handle->m_esr_hold_frms--;
		} else {
            if (s_handle->m_algo_mode == e_stage_esr)
            {
                ///超时，进入识别模式
                LISA_LOGI(TAG, "TIME_OUT wkchn = %d %d",s_handle->m_wkchn, chn);
                algo_set_mode(e_stage_main);
                if(s_handle->m_wkchn == chn)
                {
                    if (s_reg_status == VOICE_REG_STATUS_INIT) ls_app_msg_send(MSG_TYPE_WAKEUP_TIMEOUT, NULL, 0);
                    else evs_handler_post_runnable(__handle_voice_reg_timeout_quit, NULL);

                    s_handle->m_esr_hold_frms = 0;
                }
                else ///防止超时时抛出识别结果
                {
                    s_handle->m_esr_hold_frms = esr_hold_frms;
                    algo_set_mode(e_stage_esr);
                }
            }
		}
        if(s_handle->m_thre_grade_state == e_low_thre_grade)
            s_handle->m_thre_grade_hold_frms--;
        
        if(s_handle->m_thre_grade_hold_frms <= 0 && s_handle->m_thre_grade_state ==  e_low_thre_grade &&
            s_handle->m_algo_mode == e_stage_main) {
            algo_set_threshold_grade(true);
        }
	} else {                                                                                 
        ///被唤醒，进入识别模式
        LISA_LOGD(TAG, "wake up ready to asr mode");
		s_handle->m_esr_hold_frms = esr_hold_frms;
        s_handle->m_thre_grade_hold_frms = thre_grade_hold_frms;
		algo_set_mode(e_stage_esr);
	}
#endif
}

#define VOICE_REG_SUCCESS_MAX_COUNT (3) //最大次数为3
TRegistTmp s_ivw_regist_res[VOICE_REG_SUCCESS_MAX_COUNT] = {0};
TRegistTmp s_cmpTmpRes[VOICE_REG_SUCCESS_MAX_COUNT] = {0};
static int s_reg_success_max_count = 2; //默认学习次数为2 支持1/2/3
static int s_reg_success_count = 0;
static int s_reg_res_save_cnt = 0;
static bool s_regw_failed_processing = false;   //处理reg write失败标记

static uint16_t crc16_MAXIM(uint8_t *addr, int num)
{
	uint16_t crc = 0x0000;//初始值
	int i;
	for (; num > 0; num--)
	{
		//printf("data =%x\n", *addr);
		crc = crc ^ (*addr++);     //低8位异或
		for (i = 0; i < 8; i++)
		{
			if (crc & 0x0001)       //由于前面和后面省去了反转，所以这里是右移，且异或的值为多项式的反转值
				crc = (crc >> 1) ^ 0xA001;//右移后与多项式反转后异或
			else                   //否则直接右移
				crc >>= 1;
		}
	}
	return(crc ^ 0xffff);            //返回校验值 
}

static int32_t save_new_voice_bin(uint8_t *old_voice_bin, TRegistRes *p_out_res)
{
    if (!old_voice_bin) return -1;

    int header_size = sizeof(tIvwRegistResHeader);
    int res_size = sizeof(TRegistRes);
	int new_voice_bin[4096 / 4];
	memset(new_voice_bin, 0, sizeof(new_voice_bin));

    // 获取旧资源数量
	short old_res_count = *(short *)((char *)old_voice_bin + header_size);
	if (old_res_count >= REGISTER_MAX_WORD_NUM)
		return VOICE_REGIST_ERROR_MAX_LIMIT;
    
    // 获取旧资源大小
    int old_voice_bin_size = header_size + 32 + old_res_count * res_size;

    // 将旧资源拷贝给新资源，并将资源数量加1
	memcpy(new_voice_bin, old_voice_bin, old_voice_bin_size);
	short *new_count_ptr = (short *)((char *)new_voice_bin + header_size);
	*new_count_ptr = s_is_user_retry ? old_res_count : (old_res_count + 1);

    // 将新获取的res拷贝给新资源
    int res_cp_offset = s_is_user_retry ? (old_voice_bin_size - res_size) : old_voice_bin_size;
	memcpy((char *)new_voice_bin + res_cp_offset, p_out_res, res_size);

    // 赋值crc
	uint16_t crc16 = crc16_MAXIM((uint8_t *)new_voice_bin + header_size, 32 + (*new_count_ptr) * res_size);
	int *res_len = (int *)((char *)new_voice_bin + 16 * 3);
	*res_len = 32 + (*new_count_ptr) * res_size;
	uint16_t *crc = (uint16_t *)((char *)new_voice_bin + 16 * 3 + 10);
	*crc = crc16;

    // 获取新资源大小
    int new_voice_bin_size = header_size + 32 + (*new_count_ptr) * res_size;
    if (new_voice_bin_size > s_handle->m_esr_handle->VoiceSize) {
        LISA_LOGE(TAG, "new voice bin size (%d) > max value (%d)!!!", new_voice_bin_size, s_handle->m_esr_handle->VoiceSize);
        return -1;
    }

    // 将新资源拷贝到内存
    memcpy(s_handle->m_esr_handle->VoiceBinAddr, new_voice_bin, new_voice_bin_size);
    g_run_cfg.reg_save_size = new_voice_bin_size;
    s_want_save_cfg = true;
    LISA_LOGD(TAG, "save new voice.bin: %d->%d", old_voice_bin_size, new_voice_bin_size);

	return 0;
}

#define VOICE_RES_HEADER_SIZE       (sizeof(tIvwRegistResHeader))   // 96 byte
#define VOICE_OTHER_HEADER_SIZE     (32)
#define VOICE_HEADER_ALL_SIZE       (VOICE_RES_HEADER_SIZE + VOICE_OTHER_HEADER_SIZE)   // 128 byte
#define VOICE_RES_LEN_OFFSET        (48)    // 16 * 3
#define VOICE_CRC_VAL_OFFSET        (58)    // 16 * 3 + 10

extern unsigned int cal_crc32_with_salt(unsigned char *buf, unsigned int len, int salt);
int intr_cmd_voice_bin_init(uint8_t *voice_bin_addr, int cmd_res_cnt, int *out_voice_size)
{
    if (!voice_bin_addr) return -1;

    int res_size = sizeof(TRegistRes);

	int new_voice_bin[4096 / 4] = {0};
	memcpy(new_voice_bin, voice_bin_addr, VOICE_HEADER_ALL_SIZE);  // 将旧资源头所有数据拷贝给新资源

    // 赋值 res
    int res_cp_offset = VOICE_HEADER_ALL_SIZE;
    TRegistRes tmp_res = {0};
    tmp_res.weight_q = 7;
    tmp_res.threshold_main = 500;
    tmp_res.threshold_asr = 500;
    tmp_res.intent_crc32 = cal_crc32_with_salt((unsigned char *)"N", 1, tmp_res.salt);

    for (size_t i = 0; i < cmd_res_cnt; i++)
	    memcpy((char *)new_voice_bin + res_cp_offset + (i * res_size), &tmp_res, res_size);

    // 赋值res cnt
    short *res_count = (short *)((char *)new_voice_bin + VOICE_RES_HEADER_SIZE);
    *res_count = cmd_res_cnt;

    // 赋值res len
	int *res_len = (int *)((char *)new_voice_bin + VOICE_RES_LEN_OFFSET);
    *res_len = 32 + cmd_res_cnt * res_size;

    // 赋值crc
	uint16_t crc16 = crc16_MAXIM((uint8_t *)new_voice_bin + VOICE_RES_HEADER_SIZE, *res_len);
	uint16_t *crc = (uint16_t *)((char *)new_voice_bin + VOICE_CRC_VAL_OFFSET);
	*crc = crc16;

    // 将新资源拷贝到内存
    *out_voice_size = VOICE_HEADER_ALL_SIZE + cmd_res_cnt * res_size;
    memcpy(s_handle->m_esr_handle->VoiceBinAddr, new_voice_bin, *out_voice_size);
    s_want_save_cfg = true;
    LISA_LOGD(TAG, "new voice.bin init [%d]", *out_voice_size);

	return 0;
}

// ret 0 检查正确无异常  非0 检查异常
int intr_cmd_voice_bin_check_count(uint8_t *voice_bin_addr, int cmd_res_cnt)
{
    if (!voice_bin_addr) return -1;

    short cmd_res_all_count = 0;
	short res_all_count = *(short *)((char *)voice_bin_addr + VOICE_RES_HEADER_SIZE);
    TRegistRes *res_list_ptr = (TRegistRes *)(voice_bin_addr + VOICE_HEADER_ALL_SIZE);
    for (size_t i = 0; i < res_all_count; i++) {
        if (res_list_ptr[i].type == 0) cmd_res_all_count++;
        else break;
    }

    LISA_LOGD(TAG, "cmd cnt: save[%d] cfg[%d]", cmd_res_all_count, cmd_res_cnt);
    if (cmd_res_all_count != cmd_res_cnt) return -1;

    return 0;
}

static int32_t intr_cmd_voice_bin_change(uint8_t *voice_bin_addr, TRegistRes *p_out_res, int res_idx)
{
    if (!voice_bin_addr) return -1;

    int res_size = sizeof(TRegistRes);

    // 获取旧资源数量
    short cmd_res_all_count = 0;
	short res_all_count = *(short *)((char *)voice_bin_addr + VOICE_RES_HEADER_SIZE);
    TRegistRes *res_list_ptr = (TRegistRes *)(voice_bin_addr + VOICE_HEADER_ALL_SIZE);
    for (size_t i = 0; i < res_all_count; i++) {
        if (res_list_ptr[i].type == 0) cmd_res_all_count++;
    }

	if (cmd_res_all_count >= REGISTER_MAX_WORD_NUM)
		return VOICE_REGIST_ERROR_MAX_LIMIT;
    
    LISA_LOGD(TAG, "change -> all cnt: %d, cmd cnt: %d idx: %d", res_all_count, cmd_res_all_count, res_idx);
    if (cmd_res_all_count <= res_idx || res_idx < 0) {
        LISA_LOGE(TAG, "res idx out of range");
        return VOICE_REGIST_ERROR_MAX_LIMIT;
    }
    
	int new_voice_bin[4096 / 4] = {0};
    int voice_bin_size = VOICE_HEADER_ALL_SIZE + res_all_count * res_size;   // 获取资源大小
	memcpy(new_voice_bin, voice_bin_addr, voice_bin_size);  // 将旧资源拷贝给新资源

    // 修改特定资源数据
    int res_cp_offset = VOICE_HEADER_ALL_SIZE + res_size * res_idx;
	memcpy((char *)new_voice_bin + res_cp_offset, p_out_res, res_size);

    // 赋值crc
	int *res_len = (int *)((char *)new_voice_bin + VOICE_RES_LEN_OFFSET);
	uint16_t crc16 = crc16_MAXIM((uint8_t *)new_voice_bin + VOICE_RES_HEADER_SIZE, *res_len);
	uint16_t *crc = (uint16_t *)((char *)new_voice_bin + VOICE_CRC_VAL_OFFSET);
	*crc = crc16;

    // 将新资源拷贝到内存
    memcpy(s_handle->m_esr_handle->VoiceBinAddr, new_voice_bin, voice_bin_size);
    s_want_save_cfg = true;
    LISA_LOGD(TAG, "%d new voice.bin change", voice_bin_size);

	return 0;
}

// res idx = -1 ,删除全部命令词
static int32_t intr_cmd_voice_bin_clear(uint8_t *voice_bin_addr, TRegistRes *p_out_res, int res_idx)
{
    if (!voice_bin_addr) return -1;

    int res_size = sizeof(TRegistRes);

    // 获取旧资源数量
    short cmd_res_all_count = 0;
	short res_all_count = *(short *)((char *)voice_bin_addr + VOICE_RES_HEADER_SIZE);
    TRegistRes *res_list_ptr = (TRegistRes *)(voice_bin_addr + VOICE_HEADER_ALL_SIZE);
    for (size_t i = 0; i < res_all_count; i++) {
        if (res_list_ptr[i].type == 0) cmd_res_all_count++;
    }

	if (cmd_res_all_count >= REGISTER_MAX_WORD_NUM)
		return VOICE_REGIST_ERROR_MAX_LIMIT;
    
    if (cmd_res_all_count <= res_idx) {
        LISA_LOGE(TAG, "res_idx[%d] out of range [0, %d]", res_idx, cmd_res_all_count);
        return VOICE_REGIST_ERROR_MAX_LIMIT;
    }

    // 修改特定资源数据
    if (res_idx < 0) {
        for (size_t i = 0; i < res_all_count; i++) {
            if (res_list_ptr[i].type == 0) {
                int res_cp_offset = VOICE_HEADER_ALL_SIZE + res_size * i;
                memcpy((char *)voice_bin_addr + res_cp_offset, p_out_res, res_size);
            }
        }
    } else {
        int res_cp_offset = VOICE_HEADER_ALL_SIZE + res_size * res_idx;
        memcpy((char *)voice_bin_addr + res_cp_offset, p_out_res, res_size);
    }

    // 赋值crc
	int *res_len = (int *)((char *)voice_bin_addr + VOICE_RES_LEN_OFFSET);
	uint16_t crc16 = crc16_MAXIM((uint8_t *)voice_bin_addr + VOICE_RES_HEADER_SIZE, *res_len);
	uint16_t *crc = (uint16_t *)((char *)voice_bin_addr + VOICE_CRC_VAL_OFFSET);
	*crc = crc16;

    // 将新资源拷贝到内存
    s_want_save_cfg = true;
    LISA_LOGD(TAG, "new voice.bin clear");

	return 0;
}

static uint8_t s_reg_wake_num_max = 2;
static uint8_t s_reg_cmd_num_max = 18;
//check_type 0:cmd 1:wake
//0:success !0:failed
static int32_t voice_bin_check_num(const uint8_t *voice_bin, int check_type)
{
    int header_size = sizeof(tIvwRegistResHeader);

    // 获取资源数量
	short res_count = *(short *)((char *)voice_bin + header_size);
    TRegistRes *res_list_ptr = (TRegistRes *)(voice_bin + header_size + 32);

    int wake_res_num = 0;
    int cmd_res_num = 0;
    for (size_t res_idx = 0; res_idx < res_count; res_idx++) {
        //type: 0 命令词 1唤醒词 2吸收词
        if (check_type == 0 && res_list_ptr[res_idx].type == 0) {
            if (++cmd_res_num >= s_reg_cmd_num_max) return -1;
        } else if (check_type == 1 && res_list_ptr[res_idx].type == 1) {
            if (++wake_res_num >= s_reg_wake_num_max) return -1;
        }
    }

    return 0;
}

static int32_t delete_voice_reg_res(uint8_t *voice_bin, register_delete_t delete_type)
{
    LISA_LOGD(TAG, "del voice reg type: %d", delete_type);

    if (delete_type == REGISTER_DELETE_ALL) {
        g_run_cfg.reg_save_size = 0;
        s_reg_cmd_info.status = 0;
        g_run_cfg.reg_cmd_info = s_reg_cmd_info.value;
        s_want_save_cfg = true;
        return 0;
    }

    if (delete_type != REGISTER_DELETE_CMD && delete_type != REGISTER_DELETE_WAKE) {
        LISA_LOGE(TAG, "unsupport del voice reg type!");
        return -1;  
    }

    int header_size = sizeof(tIvwRegistResHeader);
    int one_res_size = sizeof(TRegistRes);

    // 获取资源数量
	short *res_cnt_ptr = (short *)((char *)voice_bin + header_size);
	if (*res_cnt_ptr >= REGISTER_MAX_WORD_NUM)
		return VOICE_REGIST_ERROR_MAX_LIMIT;

    TRegistRes *res_list_ptr = (TRegistRes *)(voice_bin + header_size + 32);

    size_t new_res_idx = 0;
    for (size_t res_idx = 0; res_idx < *res_cnt_ptr; res_idx++) {
        //type: 0 命令词 1唤醒词 2吸收词
        if ((delete_type == REGISTER_DELETE_CMD) && (res_list_ptr[res_idx].type == 0))
            continue;
        
        if ((delete_type == REGISTER_DELETE_WAKE) && (res_list_ptr[res_idx].type == 1))
            continue;
        
        if (res_idx != new_res_idx)
            memcpy(&res_list_ptr[new_res_idx], &res_list_ptr[res_idx], one_res_size);

        new_res_idx++;
    }
    *res_cnt_ptr = new_res_idx;

    // 赋值crc
	uint16_t crc16 = crc16_MAXIM((uint8_t *)voice_bin + header_size, 32 + (*res_cnt_ptr) * one_res_size);
	int *res_len = (int *)((char *)voice_bin + 16 * 3);
	*res_len = 32 + (*res_cnt_ptr) * one_res_size;
	uint16_t *crc = (uint16_t *)((char *)voice_bin + 16 * 3 + 10);
	*crc = crc16;

    g_run_cfg.reg_save_size = header_size + 32 + (*res_cnt_ptr) * one_res_size;
    s_want_save_cfg = true;

    LISA_LOGD(TAG, "save new voice.bin [%d] to psram.", g_run_cfg.reg_save_size);
    
    return 0;
}

void voice_reg_end(bool isSuccess)
{
    s_reg_status = isSuccess ? VOICE_REG_STATUS_SUCCESS : VOICE_REG_STATUS_FAILED;
    
    //42 学习模板已达上限 28 学习成功 30 学习失败 
    reply_label_t reply_label = REPLY_LABEL_TYPE_TIP_UNKNOW;
    if (isSuccess) {
        if (s_handle->m_regInfo.type == REGISTER_INNER_TYPE_CMD &&
            s_reg_type == REGISTER_TYPE_CMD_LIST) {
                reply_label = REPLY_LABEL_TYPE_TIP_TEMPLATE_FULL;
        } else {
            reply_label = REPLY_LABEL_TYPE_TIP_STUDY_SUCCESS;
        }
    } else {
        reply_label = REPLY_LABEL_TYPE_TIP_STUDY_FAILED;
    }

    audio_info_t audio_info = {0};
    cfg_parser_study_get_audio_by_label(reply_label, &audio_info);

    send_to_algo_task_msg(e_algo_msg_type_esr_restart);
    clear_voice_reg_flag(false);
    lisa_miniplayer_play_by_ids(audio_info.audios, audio_info.audio_size);
    save_app_config();

}

int __handle_voice_reg_again(void *usr_data)
{
    LISA_LOGD(TAG, "reg again!");
    s_reg_status = VOICE_REG_STATUS_READY;
    error_retry_cnt = 0;

    send_to_algo_task_msg(e_algo_msg_type_esr_restart);
    audio_info_t audio_info = {0};  //录入成功, 请再说一次
    cfg_parser_study_get_audio_by_label(REPLY_LABEL_TYPE_TIP_RECORD_SUCCESS, &audio_info);
    lisa_miniplayer_play_by_ids(audio_info.audios, audio_info.audio_size);

    return 0;
}

int __handle_voice_reg_cmd_over_success(void *usr_data)
{
    LISA_LOGD(TAG, "reg cmd over success!");
    s_reg_status = VOICE_REG_STATUS_SUCCESS;
    uint8_t tondids[10] = {0};
    uint8_t tondids_cnt = 0;

    audio_info_t audio_info1 = {0};
    audio_info_t audio_info2 = {0};

    cfg_parser_study_get_audio_by_label(REPLY_LABEL_TYPE_TIP_STUDY_SUCCESS, &audio_info1);
    cfg_parser_study_get_audio_by_label(REPLY_LABEL_TYPE_TIP_TEMPLATE_FULL, &audio_info2);

    for (size_t i = 0; i < audio_info1.audio_size; i++)
        tondids[tondids_cnt++] = audio_info1.audios[i];

    for (size_t i = 0; i < audio_info2.audio_size; i++)
        tondids[tondids_cnt++] = audio_info2.audios[i];

    send_to_algo_task_msg(e_algo_msg_type_esr_restart);
    clear_voice_reg_flag(false);
    lisa_miniplayer_play_by_ids(tondids, tondids_cnt);
    save_app_config();

    return 0;
}

int __handle_voice_reg_success(void *usr_data)
{
    LISA_LOGD(TAG, "reg success!");
    voice_reg_end(true);

    return 0;
}

int __handle_voice_regging_over(void *usr_data)
{
    LISA_LOGD(TAG, "voice regging over!");
    s_reg_status = VOICE_REG_STATUS_SUCCESS;

    send_to_algo_task_msg(e_algo_msg_type_esr_restart);
    clear_voice_reg_flag(false);

    audio_info_t audio_info = {0};  //学习模板已达上限
    cfg_parser_study_get_audio_by_label(REPLY_LABEL_TYPE_TIP_TEMPLATE_FULL, &audio_info);
    lisa_miniplayer_play_by_ids(audio_info.audios, audio_info.audio_size);
    save_app_config();

    return 0;
}

int __handle_voice_reg_over(void *usr_data)
{
    LISA_LOGD(TAG, "reg over!");
    // s_reg_status = VOICE_REG_STATUS_SUCCESS;

    // send_to_algo_task_msg(e_algo_msg_type_esr_restart);
    // send_to_algo_task_msg(e_algo_msg_type_esr_wakeup_mode);

    clear_voice_reg_flag(false);

    audio_info_t audio_info = {0};  //学习模板已达上限
    cfg_parser_study_get_audio_by_label(REPLY_LABEL_TYPE_TIP_TEMPLATE_FULL, &audio_info);
    lisa_miniplayer_play_by_ids(audio_info.audios, audio_info.audio_size);

    return 0;
}

int __handle_voice_reg_auto_next(void *usr_data)
{
    LISA_LOGD(TAG, "reg auto next!");
    voice_register_status_process(REGISTER_STATUS_AUTO_NEXT, NULL, 0);

    return 0;
}

int __handle_voice_reg_length_error(void *usr_data)
{
    LISA_LOGD(TAG, "reg length error!");
    voice_register_status_process(REGISTER_STATUS_ERROR_LENGTH, NULL, 0);

    return 0;
}

int __handle_voice_reg_speed_error(void *usr_data)
{
    LISA_LOGD(TAG, "reg speed error!");
    voice_register_status_process(REGISTER_STATUS_ERROR_SPEED, NULL, 0);

    return 0;
}

int __handle_voice_reg_simila_error(void *usr_data)
{
    LISA_LOGD(TAG, "reg simila error!");
    voice_register_status_process(REGISTER_STATUS_ERROR_SIMILA, NULL, 0);

    return 0;
}

int __handle_voice_reg_failed(void *usr_data)
{
    LISA_LOGD(TAG, "reg failed!");
    voice_reg_end(false);

    return 0;
}

int register_write_failed_process(int ret_code)
{
    LISA_LOGD(TAG, "wreg write failed: %d!", ret_code);
    if (s_regw_failed_processing) {
        LISA_LOGD(TAG, "wreg write failed running, return!");
        return 0;
    }

    s_regw_failed_processing = true;
    if (ret_code == VOICE_REGIST_ERROR_WORD_TOO_SHORT || ret_code == VOICE_REGIST_ERROR_WORD_TOO_LONG) {
        evs_handler_post_runnable(__handle_voice_reg_length_error, NULL);
    } else if (ret_code == VOICE_REGIST_ERROR_TOO_FAST || ret_code == VOICE_REGIST_ERROR_TOO_SLOW) {
        evs_handler_post_runnable(__handle_voice_reg_speed_error, NULL);
    } else if (ret_code == VOICE_REGIST_ERROR_SIMILA) {
        evs_handler_post_runnable(__handle_voice_reg_simila_error, NULL);
    } else {
        //TODO
        LISA_LOGE(TAG, "wreg failed unknow reson, dirc failed!");
        evs_handler_post_runnable(__handle_voice_reg_failed, NULL);
    }

    return 0;
}

static void algo_run_proc(void)
{
    int ret = 0;
    // void *esr_out = NULL;
    int16_t *p_data = NULL;
#if ENABLE_CRC_CHECK
    uint32_t mic_crc = 0;
#endif

    s_handle->m_wkchn = -1;

    bool is_mute = s_handle->m_is_playing;
    for(int blk = 0; blk < ALGO_ESR_BLK_CNT; blk++) {
        // p_data = ls_get_mic_data();
        // __mapi_ctrl(s_aadc_mdrv, 1, NULL);
        __mapi_read(s_aadc_mdrv, (uint32_t *)&p_data, ALGO_ESR_BLK_SAMPS, XINFINITE);
        // printk("p_data:%p\n", p_data);
        // printk("xrec1:%p\n", xrec);
        // __mapi_read(s_aadc_mdrv, &xrec, 0, XINFINITE);
        // printk("xrec2:%p\n", xrec);
        ASSERT(p_data != NULL, "ls_get_mic_data is NULL");
        extern void ls_app_uart_record_send_frame(const uint8_t *data, uint16_t len);
        ls_app_uart_record_send_frame((const uint8_t *)p_data, ALGO_ESR_BLK_SAMPS * sizeof(int16_t));
#if ENABLE_CONSISTENCY
        if(s_first_frame) continue;
#endif
        if(!is_mute) {
            // for (int samp = 0; samp < ALGO_ESR_BLK_SAMPS; samp++) {
            //     esr_in_data[samp] = p_data[samp];
            // }
        }
        else {
            // __mapi_ctrl(s_aadc_mdrv, 0, NULL);
            // lisa_thread_mdelay(10);
#if ENABLE_CONSISTENCY
            for (int samp = 0; samp < ALGO_ESR_BLK_SAMPS; samp ++) {
                esr_in_data[samp] = 30000;
            } 
            send_adc_data_to_i2s(esr_in_data, ALGO_ESR_BLK_SAMPS);
#endif
            continue; //不送音频数据到算法中
        }
#if ENABLE_CONSISTENCY
        //mic to uac
        send_adc_data_to_i2s((uint16_t *)p_data, ALGO_ESR_BLK_SAMPS);
#endif
        //mic out to algo
        if (s_reg_status == VOICE_REG_STATUS_DELETE) continue;

        if (s_reg_status != VOICE_REG_STATUS_START) {
            ret = wIvwWrite(s_handle->m_esr_handle->hIvwInst, (const short *)p_data, ALGO_ESR_BLK_SAMPS * 2);
            // // ret = wIvwWrite(s_handle->m_esr_handle->hIvwInst, *xrec, sizeof(algo_block_t));
            if (0 != ret) {
                LISA_LOGI(TAG, "wIvwWrite | ret:%d", ret);
            }
        } else {
            int8_t regist_flag;
            ret = wIvwRegistWrite(
                    s_handle->m_esr_handle->hIvwInst,
                    (const short *)p_data,
                    ALGO_ESR_BLK_SAMPS * 2,
                    &s_handle->m_regInfo,
                    &regist_flag,
                    &s_cmpTmpRes[0]);
            if (s_reging_special_cmd_process) continue;
            if (0 != ret) {
                LISA_LOGW(TAG, "wIvwRegistWrite fail | ret:%d", ret);
                register_write_failed_process(ret);
            } else {
                if (regist_flag != 0) {
                    LISA_LOGD(TAG, "regist_flag: %d, reg success count: %d, save cnt: %d", regist_flag, s_reg_success_count, s_reg_res_save_cnt);
                    TRegistRes out_res = {0};

                    if (s_reg_success_count == 1) {
                        for (size_t i = 0; i < s_reg_res_save_cnt; i++) {
                            memcpy(&(s_cmpTmpRes[1]), &(s_ivw_regist_res[i]), sizeof(TRegistTmp));
                            LISA_LOGD(TAG, "cmp save res idx: %d", i);
                            ret = wIvwRegistArbitrate(s_handle->m_esr_handle->hIvwInst, s_cmpTmpRes, s_reg_success_count + 1, &out_res);
                            if (ret != VOICE_REGIST_ERROR_SIMILA) break;
                        }

                        if (ret == VOICE_REGIST_ERROR_SIMILA && s_reg_res_save_cnt < VOICE_REG_SUCCESS_MAX_COUNT) // 相似错误时，缓存此次的资源，用于下次比较
                            memcpy(&(s_ivw_regist_res[s_reg_res_save_cnt++]), &(s_cmpTmpRes[0]), sizeof(TRegistTmp));
                    } else {
                        for (size_t i = 0; i < s_reg_res_save_cnt; i++)
                            memcpy(&(s_cmpTmpRes[i+1]), &(s_ivw_regist_res[i]), sizeof(TRegistTmp));

                        ret = wIvwRegistArbitrate(s_handle->m_esr_handle->hIvwInst, s_cmpTmpRes, s_reg_success_count + 1, &out_res);
                    }

                    if (0 != ret) {
                        LISA_LOGW(TAG, "wIvwRegistArbitrate fail | ret: %d", ret);
                        register_write_failed_process(ret);
                    } else {
                        LISA_LOGD(TAG, "wIvwRegistArbitrate success");
                        if (regist_flag == 1) {
                            if (++s_reg_success_count < s_reg_success_max_count) {
                                s_reg_res_save_cnt = s_reg_success_count;
                                for (size_t i = 0; i < s_reg_res_save_cnt; i++)
                                    memcpy(&(s_ivw_regist_res[i]), &(s_cmpTmpRes[i]), sizeof(TRegistTmp));
                                evs_handler_post_runnable(__handle_voice_reg_again, NULL);
                            } else {
                                int reg_type = cfg_parser_get_voice_reg_type();
                                int ret = -1;

                                if (reg_type == 2 && s_handle->m_regInfo.type == REGISTER_INNER_TYPE_CMD) {
                                    ret = intr_cmd_voice_bin_change(s_handle->m_esr_handle->VoiceBinAddr, &out_res, s_intr_cmd_index);
                                } else {
                                    ret = save_new_voice_bin(s_handle->m_esr_handle->VoiceBinAddr, &out_res);
                                }
 
                                if (ret != 0) {
                                    evs_handler_post_runnable(__handle_voice_reg_failed, NULL);
                                } else {
                                    if (s_handle->m_regInfo.type == REGISTER_INNER_TYPE_CMD && s_reg_type == REGISTER_TYPE_CMD_LIST) {
                                        evs_handler_post_runnable(__handle_voice_reg_auto_next, NULL);
                                    } else {
                                        evs_handler_post_runnable(__handle_voice_reg_success, NULL);
                                    }
                                }
                            }
                        }
                    }
                }
            }

        }
#if ENABLE_CRC_CHECK
        if (s_crc_start) {
            memcpy(mic_crc_data[blk], p_data, ALGO_ESR_BLK_SAMPS * 2);
        }
#endif
    }
    if(!is_mute && !s_first_frame) {
#if ENABLE_CRC_CHECK
        if (s_crc_start) {
            mic_crc = crc32_calc(mic_crc_data, ALGO_ESR_BLK_SAMPS * 2 * ALGO_ESR_BLK_CNT, 0);
            TRACE("%u", mic_crc);
        }
#endif
        algo_result_proc();
    }
    // algo_result_proc();
    // int32_t shared_release = 0;
    // wIvwGetParameter(s_handle->m_esr_handle->hIvwInst, PARAM_W_IVW_SHARED_MEM_RELEASE, &shared_release);
    // printk("shared_release:%d\n", shared_release);
    algo_process_complete();
#if ENABLE_CONSISTENCY    
    if(s_first_frame) {
        LISA_LOGI(TAG, "send tag head...");
        consistency_tag_send();
    }    
#endif
}

static void _handle_algo_task_msg_user(ls_algo_msg_type_t type)
{
    switch (type)
    {
    case e_algo_msg_type_esr_reinit:
        LISA_LOGI(TAG, "algo reinit");
        algo_status(e_state_reinit);
        break;
    case e_algo_msg_type_esr_start:
        if(s_handle->m_cur_state == e_state_idle && s_handle->m_prev_state == e_state_running) {
            LISA_LOGI(TAG, "algo state switch running");
            algo_status(e_state_running);
        }
        break;
    case e_algo_msg_type_esr_stop:
        if(s_handle->m_cur_state == e_state_running) {
            LISA_LOGI(TAG, "algo state switch idle");
            algo_set_mode(e_stage_main);
            algo_status(e_state_idle);
        }
        break;
    case e_algo_msg_type_esr_asr_mode:
        if (s_handle->m_cur_state == e_state_running) {
            LISA_LOGI(TAG, "swicth algo asr mode");
            s_handle->m_esr_hold_frms = esr_hold_frms;
            s_handle->m_thre_grade_hold_frms = thre_grade_hold_frms;
		    algo_set_mode(e_stage_esr);
        }
        break;
    case e_algo_msg_type_esr_wakeup_mode:
        if (s_handle->m_cur_state == e_state_running) {
            LISA_LOGI(TAG, "swicth algo wakeup mode");
		    algo_set_mode(e_stage_main);
        }
        break;
    case e_algo_msg_type_play_start:
        LISA_LOGI(TAG, "play start");
        s_handle->m_is_playing = true;
        break;
    case e_algo_msg_type_play_stop:
        LISA_LOGI(TAG, "play stop");
        s_handle->m_is_playing = false;
        // 提示音播完重置时间
        s_handle->m_esr_hold_frms = esr_hold_frms;
        break;
    case e_algo_msg_type_esr_init:
    case e_algo_msg_type_esr_restart:
        LISA_LOGI(TAG, "algo restart");
        #if ENABLE_CONSISTENCY
        algo_status(e_state_reinit);
        #else
        algo_status(e_state_restart);
        #endif
        break;
    case e_algo_msg_type_esr_deinit:
        algo_status(e_state_deinit);
        break;
    case e_algo_msg_type_factory_test:
        s_handle->m_is_ft_test = true;
        break;
    default:
        // LISA_LOG(TAG, "unknow algo msg type = %d", type);
        break;
    }
}

static void _algo_task_proc_func(void *arg)
{
    uint8_t msg;
    if(ai_create()) {
        LISA_LOGE(TAG, "ai_create failed!!!");
        goto EXIT;
    }
    algo_status(e_state_init);
    while(s_handle->m_running) {
        s_algo_tick++;
        msg = e_algo_msg_type_count;
        lisa_queue_pop(s_handle->m_msg_queue, &msg, sizeof(uint8_t), 0);
        _handle_algo_task_msg_user(msg);
        switch(s_handle->m_cur_state) {
            case e_state_init:
                // ls_mic_start_stream(MIC_SAMPLE_RATE);
                __mapi_ctrl(s_aadc_mdrv, 1, NULL);
                ai_set_running_state(e_algo_start, ALL_RESET);
                algo_status(e_state_running);
                LISA_LOGD(TAG, "algo state init, reg status: %d", s_reg_status);
                if (s_reg_status == VOICE_REG_STATUS_READY) {
                    s_reg_status = VOICE_REG_STATUS_START;
                } else if (s_reg_status == VOICE_REG_STATUS_SUCCESS ||
                                s_reg_status == VOICE_REG_STATUS_FAILED ||
                                s_reg_status == VOICE_REG_STATUS_TIMEOUT_QUIT ||
                                s_reg_status == VOICE_REG_STATUS_USER_QUIT ||
                                s_reg_status == VOICE_REG_STATUS_DELETE) {
                    s_reg_status = VOICE_REG_STATUS_INIT;
                }
                break;
            case e_state_reinit:
                //stop
                // ls_mic_stop_stream();
                __mapi_ctrl(s_aadc_mdrv, 0, NULL);
                ai_destroy();
                ///restart
                ai_create();
                algo_status(e_state_init);
                s_first_frame = true;
                break;
            case e_state_restart:
                //stop
                // ls_mic_stop_stream();
                __mapi_ctrl(s_aadc_mdrv, 0, NULL);
                ai_destroy();
                ///restart
                ai_create();
                algo_status(e_state_init);
                if (s_reg_status == VOICE_REG_STATUS_INIT)
                    s_handle->m_algo_mode = e_stage_main;
                break;
            case e_state_running:
                if (!s_handle->m_is_ft_test) {
                    algo_run_proc();
                }
                break;
            case e_state_deinit:
                //stop
                // ls_mic_stop_stream();
                __mapi_ctrl(s_aadc_mdrv, 0, NULL);
                ai_destroy();
                s_handle->m_algo_mode = e_stage_main;
                algo_status(e_state_idle);
                break;
            case e_state_idle:
                // LISA_LOGD(TAG, "algo task idle ...");
                lisa_thread_mdelay(50);
                // vTaskDelay(pdMS_TO_TICKS(50));
                break;
        default:
            LISA_LOGD(TAG, "algo task status = %d", s_handle->m_cur_state);
            // vTaskDelay(pdMS_TO_TICKS(1000));
            lisa_thread_mdelay(1000);
            break;
        }
    }

EXIT:
    LISA_LOGD(TAG, "algo task exit ...");
    lisa_mem_free(s_handle->m_esr_handle);
    lisa_queue_delete(s_handle->m_msg_queue);
    lisa_mem_free(s_handle);
    lisa_thread_delete(NULL);
    s_handle = NULL;
}

uint32_t get_task_algo_tick()
{
    return s_algo_tick;
}

void send_to_algo_task_msg(ls_algo_msg_type_t msg_type)
{
    if(NULL != s_handle) {
        if (msg_type == e_algo_msg_type_play_start) {
            LISA_LOGD(TAG, "play start");
            s_handle->m_is_playing = true;
        } else if (msg_type == e_algo_msg_type_play_stop) {
            LISA_LOGD(TAG, "play stop");
            s_handle->m_is_playing = false;
            // 提示音播完重置时间
            s_handle->m_esr_hold_frms = esr_hold_frms;
        } else {
            uint8_t msg = (uint8_t)msg_type;
            lisa_queue_push(s_handle->m_msg_queue, &msg, sizeof(uint8_t), 0);
        }
        // uint8_t msg = (uint8_t)msg_type;
        // lisa_queue_push(s_handle->m_msg_queue, &msg, sizeof(uint8_t), 0);
    }
}

void lisa_algo_task_create(uint8_t *algo_inst_mem, uint8_t *algo_share_mem, uint32_t pool_size,  uint8_t *algo_voice_mem, uint32_t voice_size, uint32_t high_grade_timeout)
{
    if(NULL != s_handle) {
        LISA_LOGD(TAG, "lisa algo task exist!!!");
        return;
    }
    LISA_LOGD(TAG, "lisa algo task create [in]");

    s_aadc_mdrv = __MAPI_FIND(CONFIG_AADC_NAME);
    int step = 0;
    __mapi_ctrl(s_aadc_mdrv, MAPI_AADC_CTRL_GET_SAMPS, &step);

#if ENABLE_CONSISTENCY && CONFIG_I2SO_ENABLE
    s_i2s_mdrv = __MAPI_FIND(CONFIG_I2SO_NAME);
#endif

    s_algo_tick = 0;

    s_handle = (task_algo_t *)lisa_mem_calloc(1, sizeof(task_algo_t));
	if (!s_handle) {
		LISA_LOGD(TAG, "lisa algo task create [err]");
		return;
	}
    // dma_copy_init();
    // s_handle->res_addr = res_init();

    s_handle->m_esr_handle = (esr_algo_t *)lisa_mem_calloc(1, sizeof(esr_algo_t));
    if(NULL == s_handle->m_esr_handle) {
		LISA_LOGD(TAG, "lisa algo esr_handle alloc [err]");
		goto ALGO_ESR_HANDLE_ERROR;
    }

    // uint32_t size = 0;
    // test_addr = res_item_by_id(CONFIG_RESMGR_BASE_SOUNDRES + 0, &size);
    // test_blks = size / sizeof(aud_step_t);
    // // for (int i = 0; i < blks; i++) __mapi_write(play_mdrv, addr + i, 160, XINFINITE);

    // printk("test_addr:%p, test_blks:%d size:%d\n", test_addr, test_blks, size);

    s_handle->m_msg_queue = lisa_queue_create(ALGO_MSG_QUEUE_COUNT_MAX, (uint8_t *)ALGO_MSG_QUEUE_NAME, sizeof(uint8_t));
    if(NULL == s_handle->m_msg_queue) {
        LISA_LOGD(TAG, "lisa algo msg queue [err]");
		goto ALGO_MSG_QUEUE_ERROR;
    }

    if(high_grade_timeout == 0) {
        s_handle->m_thre_grade_state = e_thre_grade_disable;
    }
    else {
        s_handle->m_thre_grade_state = e_high_thre_grade;
        thre_grade_hold_frms         = SECS2FRMS(high_grade_timeout) > ALGO_TIMEOUT_FRMS ? SECS2FRMS(high_grade_timeout) : thre_grade_hold_frms;
        s_handle->m_thre_grade_hold_frms = thre_grade_hold_frms;
    }
    s_handle->m_esr_handle->IvwInstAddr     = algo_inst_mem;
    s_handle->m_esr_handle->IvwShareAddr    = algo_share_mem; 
    s_handle->m_esr_handle->IvwRamSize      = pool_size;
    s_handle->m_esr_handle->VoiceBinAddr    = algo_voice_mem;
    s_handle->m_esr_handle->VoiceSize       = voice_size;
    s_handle->m_cur_state                   = e_state_fetchinit;
    s_handle->m_prev_state                  = e_state_fetchinit;
    s_handle->m_esr_hold_frms               = esr_hold_frms;
    s_handle->m_running                     = 1;
    s_handle->m_esr_handle->hIvwInst        = NULL;
    s_handle->m_wakeup_word                 = NULL;
    s_handle->m_is_playing                  = false;
    s_handle->m_is_ft_test                  = false;

    if (lisa_miniplayer_get_state() == MINI_PLAYER_ST_PLAYING) {
        send_to_algo_task_msg(e_algo_msg_type_play_start);
    }

    if ((g_run_cfg.reg_cmd_info == 0 && (cfg_parser_study_get_reg_cmd_count() != 0)) ||
            ((reg_cmd_info_t)g_run_cfg.reg_cmd_info).count != cfg_parser_study_get_reg_cmd_count()) {
        LISA_LOGD(TAG, "reset g_run_cfg reg info");
        //flash中保存的注册命令词数量和配置中不一致时
        s_reg_cmd_info.count = cfg_parser_study_get_reg_cmd_count();
        s_reg_cmd_info.status = 0;

        g_run_cfg.reg_cmd_info = s_reg_cmd_info.value;
        g_run_cfg.reg_save_size = 0;

        s_want_save_cfg = true;
        save_app_config();
    } else {
        s_reg_cmd_info.value = g_run_cfg.reg_cmd_info;
    }

    // new flash中已保存时，直接读取voice.bin到psram中
    if (g_run_cfg.reg_save_flag && g_run_cfg.reg_save_size > 0) {
        int ret = listen_flash_voice_read(s_handle->m_esr_handle->VoiceBinAddr, g_run_cfg.reg_save_size);
        LISA_LOGD(TAG, "listen flash voice read ret: %d", ret);
    }

    lisa_thread_attr_t attr;
	attr.name = (uint8_t *)ALGO_TASK_NAME;
	attr.priority = LISA_OS_PRIORITY_NORMAL;
	attr.stack_size = ALGO_TASK_TD_STACK_SIZE;
	lisa_thread_t *algo_td = lisa_thread_create(&attr, _algo_task_proc_func, NULL);

    if(!algo_td) {
        LISA_LOGD(TAG, "lisa algo create task failed");
        goto ALGO_TASK_CREATE_ERROR;
    }
    LISA_LOGD(TAG, "lisa algo task create [out]");
    return;

ALGO_TASK_CREATE_ERROR:
    lisa_queue_delete(s_handle->m_msg_queue);
ALGO_MSG_QUEUE_ERROR:
    lisa_mem_free(s_handle->m_esr_handle);
ALGO_ESR_HANDLE_ERROR:
    lisa_mem_free(s_handle);
}

#if ENABLE_CONSISTENCY
void shell_algo_restart(int flag)
{
    SHELL_ITEM_EXPORT("algo_restart", shell_algo_restart, "algo restart");
    if (flag == 1) {
        #if ENABLE_CRC_CHECK
        s_crc_start = true;
        #endif
        LISA_LOGD(TAG, "----------------restart_algo----------------");
        send_to_algo_task_msg(e_algo_msg_type_esr_reinit);
    }
}
#endif

typedef enum {
    GET_INDEX_BEFORE = -1,
    GET_INDEX_CUR = 0,
    GET_INDEX_NEXT = 1
} get_index_type_t;

// 学习下一个: 是学习下一个未学习的，已学习的跳过
// 重新学习: 是学习上一个已经学习成功的，未学习的跳过
int register_cmd_get_index(uint8_t start_index, get_index_type_t get_type)
{
    //若当前已经是学习过的词，重新学习不再寻找之前一个
    if (get_type == GET_INDEX_BEFORE && GET_BIT_VAL(s_reg_cmd_info.status, start_index)) {
        return start_index;
    }

    int index;
    for (index = start_index + get_type;
        (get_type == GET_INDEX_BEFORE) ? index >= 0 : index < s_reg_cmd_info.count;
        (get_type == GET_INDEX_BEFORE) ? index-- : index++) {
            // if (~((get_type == GET_INDEX_BEFORE) ^ GET_BIT_VAL(s_reg_cmd_info.status, index))) break;
            if (get_type == GET_INDEX_BEFORE && GET_BIT_VAL(s_reg_cmd_info.status, index)) break;
            else if (get_type != GET_INDEX_BEFORE && !GET_BIT_VAL(s_reg_cmd_info.status, index)) break;
        }

    LISA_LOGD(TAG, "cmdlist get[%d]: %d->%d", get_type, start_index, index);

    return index;
}

int ls_app_register_del_process(register_delete_t del_type, const char *audios, uint8_t size)
{
    LISA_LOGD(TAG, "reg del");
    if (del_type == REGISTER_DELETE_ALL || del_type == REGISTER_DELETE_CMD) {
        s_reg_cmd_list_index = 0;
        s_reg_cmd_info.status = 0;
        g_run_cfg.reg_cmd_info = s_reg_cmd_info.value;
    }

    s_reg_status = VOICE_REG_STATUS_DELETE;
    if (del_type == REGISTER_DELETE_INTR_CMD) {
        TRegistRes tmp_res = {0};
        tmp_res.weight_q = 7;
        tmp_res.threshold_main = 500;
        tmp_res.threshold_asr = 500;
        tmp_res.intent_crc32 = cal_crc32_with_salt((unsigned char *)"N", 1, tmp_res.salt);
        intr_cmd_voice_bin_clear(s_handle->m_esr_handle->VoiceBinAddr, &tmp_res, s_intr_cmd_index);
    } else if (del_type == REGISTER_DELETE_ALL_CMD) {
        TRegistRes tmp_res = {0};
        tmp_res.weight_q = 7;
        tmp_res.threshold_main = 500;
        tmp_res.threshold_asr = 500;
        tmp_res.intent_crc32 = cal_crc32_with_salt((unsigned char *)"N", 1, tmp_res.salt);
        intr_cmd_voice_bin_clear(s_handle->m_esr_handle->VoiceBinAddr, &tmp_res, -1);
    } else {
        delete_voice_reg_res(s_handle->m_esr_handle->VoiceBinAddr, del_type);
    }

    send_to_algo_task_msg(e_algo_msg_type_esr_restart);
    lisa_miniplayer_play_by_ids((const uint8_t *const)audios, size);

    clear_voice_reg_flag(false);
    s_want_save_cfg = true;
    save_app_config();

    return 0;
}

int ls_app_register_type_process(register_type_t reg_type, const char *audios, uint8_t size)
{
    uint8_t tondids[10] = {0};
    uint8_t tondids_cnt = 0;

    error_retry_cnt = 0;
    s_reg_success_count = 0;                                    //遍数清空
    s_reg_res_save_cnt = 0;
    memset(s_ivw_regist_res, 0, sizeof(s_ivw_regist_res));      //资源清除

    study_user_cfg_t study_user_cfg = {0};
    cfg_parser_get_study_user_cfg(&study_user_cfg);
    s_reg_cmd_num_max = study_user_cfg.cmd_reg_max;
    s_reg_wake_num_max = study_user_cfg.wake_reg_max;

    // 判断模板是否达上限
    if (reg_type == REGISTER_TYPE_WAKE_WORD || reg_type == REGISTER_TYPE_CMD_LIST) {
        int check_type = reg_type == REGISTER_TYPE_WAKE_WORD ? 1 : 0;
        int ret = voice_bin_check_num(s_handle->m_esr_handle->VoiceBinAddr, check_type);
        if (ret != 0) {
            evs_handler_post_runnable(__handle_voice_reg_over, NULL);
            return 0;
        }
    }

    // 赋值播放的音频
    for (size_t i = 0; i < size; i++)
        tondids[tondids_cnt++] = audios[i];

    switch (reg_type)
    {
    case REGISTER_TYPE_WAKE_WORD:
    {
        memset(&s_tmp_regInfo, 0, sizeof(tIvwRegistInfo));
        s_tmp_regInfo.maxWords = study_user_cfg.wake_max_words;
        s_tmp_regInfo.minWords = study_user_cfg.wake_min_words;
        s_tmp_regInfo.sensitivity = study_user_cfg.wake_sensitivity;

        reg_word_info_t reg_word_info = {0};
        cfg_parser_study_get_reg_wake_words(&reg_word_info);
        s_tmp_regInfo.intentStr = reg_word_info.keywords_ptr;
        LISA_LOGD(TAG, "reg wake word: %s", reg_word_info.keywords_ptr);
        s_tmp_regInfo.intentStrLen = reg_word_info.keyword_size;
        s_tmp_regInfo.type = REGISTER_INNER_TYPE_WAKE;  // 0 命令词   1 唤醒词   2 吸收词

        s_reg_success_max_count = study_user_cfg.wake_repeat_count;
        s_reg_status = VOICE_REG_STATUS_READY;
    } break;
    case REGISTER_TYPE_CMD_LIST:
    {
        memset(&s_tmp_regInfo, 0, sizeof(tIvwRegistInfo));

        s_reg_cmd_list_index = register_cmd_get_index(0, GET_INDEX_CUR);
        if (s_reg_cmd_list_index >= cfg_parser_study_get_reg_cmd_count()) {
            evs_handler_post_runnable(__handle_voice_reg_over, NULL);
            return 0;
        } else {
            reg_word_info_t reg_word_info = {0};
            cfg_parser_study_get_reg_cmd_words_by_index(s_reg_cmd_list_index, &reg_word_info);

            s_tmp_regInfo.maxWords = study_user_cfg.cmd_max_words;
            s_tmp_regInfo.minWords = study_user_cfg.cmd_min_words;
            s_tmp_regInfo.sensitivity = study_user_cfg.cmd_sensitivity;

            s_tmp_regInfo.type = REGISTER_INNER_TYPE_CMD;
            s_tmp_regInfo.intentStr = reg_word_info.keywords_ptr;
            s_tmp_regInfo.intentStrLen = reg_word_info.keyword_size;

            for (size_t i = 0; i < reg_word_info.audio_size; i++)
                tondids[tondids_cnt++] = reg_word_info.play_audio_ptr[i];

            s_reg_success_max_count = study_user_cfg.cmd_repeat_count;
            s_reg_status = VOICE_REG_STATUS_READY;
        }

    } break;
    case REGISTER_TYPE_CMD_INTR:
    {
        memset(&s_tmp_regInfo, 0, sizeof(tIvwRegistInfo));
        if (strlen(s_intr_cmd_keyword) > 0) {
            s_tmp_regInfo.maxWords = study_user_cfg.cmd_max_words;
            s_tmp_regInfo.minWords = study_user_cfg.cmd_min_words;
            s_tmp_regInfo.sensitivity = study_user_cfg.cmd_sensitivity;

            s_tmp_regInfo.type = REGISTER_INNER_TYPE_CMD;
            s_tmp_regInfo.intentStr = s_intr_cmd_keyword;
            s_tmp_regInfo.intentStrLen = strlen(s_intr_cmd_keyword);

            s_reg_success_max_count = study_user_cfg.cmd_repeat_count;
            s_reg_status = VOICE_REG_STATUS_READY;
        } else {
            return 0;
        }
    } break;
    case REGISTER_TYPE_USER_QUIT:
    {
        clear_voice_reg_flag(false);
        save_app_config();

        s_reg_status = VOICE_REG_STATUS_USER_QUIT;
    } break;
    default:
        break;
    }

    s_reg_type = reg_type;

    char tmp_intentstr[64] = {0};
    memcpy(tmp_intentstr, s_tmp_regInfo.intentStr, s_tmp_regInfo.intentStrLen);
    LISA_LOGD(TAG, "Reg info: keyword[%d]=%s, sensitivity=%d, type=%d, range=[%d,%d]", s_tmp_regInfo.intentStrLen, tmp_intentstr,
            s_tmp_regInfo.sensitivity, s_tmp_regInfo.type, s_tmp_regInfo.minWords, s_tmp_regInfo.maxWords);

    s_regw_failed_processing = false;
    s_is_user_retry = false;

    // 3. 重启算法
    send_to_algo_task_msg(e_algo_msg_type_esr_restart);
    s_reging_special_cmd_process = false;

    // 4. play
    lisa_miniplayer_play_by_ids(tondids, tondids_cnt);

    return 0;
}

int voice_register_status_process(register_status_t reg_st, const char *audios, uint8_t size)
{
    uint8_t tondids[10] = {0};
    uint8_t tondids_cnt = 0;
    reply_label_t reply_label = REPLY_LABEL_TYPE_TIP_UNKNOW;
    study_user_cfg_t study_user_cfg = {0};
    cfg_parser_get_study_user_cfg(&study_user_cfg);

    switch (reg_st)
    {
    case REGISTER_STATUS_ERROR_CONFLICT:
        reply_label = REPLY_LABEL_TYPE_TIP_COMMAND_CONFLICT;
        error_retry_cnt++;
        break;
    case REGISTER_STATUS_ERROR_LENGTH:
        reply_label = REPLY_LABEL_TYPE_TIP_LENGTH_ERROR;
        error_retry_cnt++;
        break;
    case REGISTER_STATUS_ERROR_SPEED:
        reply_label = REPLY_LABEL_TYPE_TIP_SPEED_ERROR;
        error_retry_cnt++;
        break;
    case REGISTER_STATUS_ERROR_SIMILA:
        reply_label = REPLY_LABEL_TYPE_TIP_SIMILA_ERROR;
        error_retry_cnt++;
        break;
    case REGISTER_STATUS_AUTO_NEXT:
        reply_label = REPLY_LABEL_TYPE_TIP_STUDY_SUCCESS;
        SET_BIT_VAL(s_reg_cmd_info.status, s_reg_cmd_list_index);
        g_run_cfg.reg_cmd_info = s_reg_cmd_info.value;
        s_want_save_cfg = true;
        s_reg_cmd_list_index = register_cmd_get_index(s_reg_cmd_list_index, GET_INDEX_NEXT);
        break;
    case REGISTER_STATUS_USER_NEXT:
        for (size_t i = 0; i < size; i++)
            tondids[tondids_cnt++] = audios[i];
        s_reg_cmd_list_index = register_cmd_get_index(s_reg_cmd_list_index, GET_INDEX_NEXT);
        break;
    case REGISTER_STATUS_USER_RETRY:
        for (size_t i = 0; i < size; i++)
            tondids[tondids_cnt++] = audios[i];
        if (s_reg_cmd_list_index > 0) {
            s_reg_cmd_list_index = register_cmd_get_index(s_reg_cmd_list_index, GET_INDEX_BEFORE);
        }
        break;
    default:
        break;
    }

    if (reply_label != REPLY_LABEL_TYPE_TIP_UNKNOW) {
        audio_info_t audio_info = {0};
        cfg_parser_study_get_audio_by_label(reply_label, &audio_info);

        for (size_t i = 0; i < audio_info.audio_size; i++)
            tondids[tondids_cnt++] = audio_info.audios[i];
    }

    reg_word_info_t reg_word_info = {0};
    if (s_tmp_regInfo.type == REGISTER_INNER_TYPE_CMD) {
        if (s_reg_cmd_list_index >= cfg_parser_study_get_reg_cmd_count()) {
            if (reg_st == REGISTER_STATUS_AUTO_NEXT) {
                evs_handler_post_runnable(__handle_voice_reg_cmd_over_success, NULL);
            }else {
                evs_handler_post_runnable(__handle_voice_reg_success, NULL);
            }
            return 0;
        } else if (s_reg_cmd_list_index < 0) {
            s_reg_cmd_list_index = 0; //重新学习时未找到上一个已经学习过的，则从第0个开始学习
        }

        cfg_parser_study_get_reg_cmd_words_by_index(s_reg_cmd_list_index, &reg_word_info);

        s_tmp_regInfo.intentStr = reg_word_info.keywords_ptr;
        s_tmp_regInfo.intentStrLen = reg_word_info.keyword_size;
    }

    //校验模板数量
    int check_type = s_tmp_regInfo.type == REGISTER_INNER_TYPE_WAKE ? 1 : 0;
    int ret = voice_bin_check_num(s_handle->m_esr_handle->VoiceBinAddr, check_type);
    if (ret != 0) {
        if (reg_st == REGISTER_STATUS_AUTO_NEXT) {
            evs_handler_post_runnable(__handle_voice_reg_cmd_over_success, NULL);
        } else {
            evs_handler_post_runnable(__handle_voice_regging_over, NULL);
        }
        return 0;
    }

    if (reg_st == REGISTER_STATUS_AUTO_NEXT ||
        reg_st == REGISTER_STATUS_USER_NEXT ||
        reg_st == REGISTER_STATUS_USER_RETRY) {
        error_retry_cnt = 0;
        s_reg_success_count = 0;                                    //重试时，遍数不清空
        s_reg_res_save_cnt = 0;
        memset(s_ivw_regist_res, 0, sizeof(s_ivw_regist_res));      //重试时，资源不清除

        if (s_tmp_regInfo.type == REGISTER_INNER_TYPE_CMD) {
            for (size_t i = 0; i < reg_word_info.audio_size; i++)
                tondids[tondids_cnt++] = reg_word_info.play_audio_ptr[i];   //请说出xx的注册指令
        }
    }

    int max_retry_cnt = s_tmp_regInfo.type == REGISTER_INNER_TYPE_CMD ? study_user_cfg.cmd_retry_count : study_user_cfg.wake_retry_count;
    if (error_retry_cnt >= max_retry_cnt) {
        LISA_LOGD(TAG, "error cnt > %d", max_retry_cnt);
        evs_handler_post_runnable(__handle_voice_reg_failed, NULL);
        return 0;
    }

    // 2. 设置启用wIvwRegistWrite接口
    s_reg_status = VOICE_REG_STATUS_READY;

    char tmp_intentstr[64] = {0};
    memcpy(tmp_intentstr, s_tmp_regInfo.intentStr, s_tmp_regInfo.intentStrLen);
    LISA_LOGD(TAG, "Reg info: key[%d]=%s,sensitivity=%d,type=%d,range=[%d,%d]", s_tmp_regInfo.intentStrLen, tmp_intentstr,
            s_tmp_regInfo.sensitivity, s_tmp_regInfo.type, s_tmp_regInfo.minWords, s_tmp_regInfo.maxWords);

    s_regw_failed_processing = false;

    if (reg_st == REGISTER_STATUS_USER_RETRY) s_is_user_retry =  true;
    else s_is_user_retry = false;

    // 3. 重启算法
    send_to_algo_task_msg(e_algo_msg_type_esr_restart);
    s_reging_special_cmd_process = false;

    // 4. play 
    lisa_miniplayer_play_by_ids(tondids, tondids_cnt);

    return 0;
}

int ls_app_register_set_cmd_intr_words(const char *reg_cmd_words, int reg_cmd_index)
{
    memset(s_intr_cmd_keyword, 0, sizeof(s_intr_cmd_keyword));
    strcpy(s_intr_cmd_keyword, reg_cmd_words);
    s_intr_cmd_index = reg_cmd_index;

    return 0;
}

#endif