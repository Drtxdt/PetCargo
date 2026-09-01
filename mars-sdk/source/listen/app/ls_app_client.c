#include <stdint.h>
#include "appinc.h"
#include "ls_app_client.h"
#include "lisa_mem.h"
#include "ls_app_algo.h"
#include "ls_app_uart.h"
#include "ls_app_volume.h"
#include "mini_player.h"
#include "lisa_log.h"
#include "evs_utils.h"
#include "ls_app_version.h"
#include "ls_app_device.h"
#include "ls_app_config.h"
// #include "ls_app_msg.h"
#include "task_algo.h"
#include "ls_app_cfg_mgr.h"
#include "ls_app_flash.h"
#if (EXTER_PA_ENABLE == 1)
#include "pa_manager.h"
#endif

#define TAG "app_client"

// Keyword最大长度
#define LS_ALGO_KEYWORD_MAX_LEN (64)

#define UART_MSG_MAX_COUNT (5)

#if (UART_RECEIVE_SUPPORT_CACHE == 1)
static ls_uart_msg_t s_uart_msg[UART_MSG_MAX_COUNT];
static uint8_t uart_msg_index = 0;
static uint8_t s_app_init = 0;
#else
static ls_uart_msg_t s_uart_msg;
#endif

// 是否上电启动
static uint8_t device_is_power_on()
{
	extern pmu_rstsrc_t power_reset_src;
	if (power_reset_src == PMU_RST_POR) {
		return 1;
	}
    return 0;
}

//播放器状态回调
static void miniplayer_status_cb(mini_player_state state)
{
	switch (state)
	{
	// case MINI_PLAYER_ST_STOPED:
	case MINI_PLAYER_ST_ERROR:
	case MINI_PLAYER_ST_COMPLETE:
	#if (EXTER_PA_ENABLE == 1)
		pa_manager_refresh(PA_MGR_OFF, cfg_parser_get_pa_config().hold_time, "miniplayer_over");
	#endif
		send_to_algo_task_msg(e_algo_msg_type_play_stop);
		break;
	case MINI_PLAYER_ST_PLAYING:
		send_to_algo_task_msg(e_algo_msg_type_play_start);
		break;
	default:
		break;
	}
}

// 处理MCU响应的消息
static int __handle_mcu_response(void *arg)
{
	ls_uart_msg_t *uart_msg = (ls_uart_msg_t *)arg;
	LISA_LOGB(TAG, uart_msg->data, uart_msg->len, "[RX] [%d]: <- ", uart_msg->len);

	if (uart_msg->len <= 0) {
		return -1;
	}

	cfg_parser_recv_data_handler(uart_msg);

	return 0;
}

// 串口消息回调
void uart_receive_msg_cb(const uint8_t *data, uint32_t len)
{
	// 在这里处理收到的串口消息，注意不能阻塞
	if (len > LS_UART_MAG_MAX_LEN) {
		LISA_LOGE(TAG, "uart msg len is large!!!");
		return;
	}

#if (UART_RECEIVE_SUPPORT_CACHE == 1)
	s_uart_msg[uart_msg_index].len = len;
	memcpy(s_uart_msg[uart_msg_index].data, data, len);
	evs_handler_post_runnable(__handle_mcu_response, &(s_uart_msg[uart_msg_index]));
	uart_msg_index++;
	if (uart_msg_index >= UART_MSG_MAX_COUNT) {
		uart_msg_index = 0;
	}
#else
	s_uart_msg.len = len;
	memcpy(s_uart_msg.data, data, len);
	evs_handler_post_runnable(__handle_mcu_response, &s_uart_msg);
#endif
}

#if CONFIG_AADC_ENABLE
void set_adc_config(int again, int dgain)
{
    mod_inst_t const aadc = __MAPI_FIND(CONFIG_AADC_NAME);
    ASSERT(aadc, "aadc");
    int gain[2] = {again, dgain};
    __mapi_ctrl(aadc, MAPI_AADC_CTRL_SET_GAIN, &gain[0]);      // set gain
}
#endif

void listen_client_create(char *share_mem, char *voice_mem)
{
	if (s_app_init) return;
	s_app_init = 1;

	listen_flash_init();

	// 资源初始化
	res_init();
	// 配置解析初始化
	cfg_parser_init();
	// 设置日志等级
	lisa_log_set_level(cfg_parser_get_loglev());

#if (EXTER_PA_ENABLE == 1)
	if (cfg_parser_get_pa_config().enable == 1) {
		pa_manager_pre_init();
		pa_manager_init(PA_MGR_OFF);
	}
#endif

	LISA_PRINT(TAG, "APP version: %s, config version: %s.", listen_get_fw_version(), cfg_parser_get_version());
	LISA_LOGD(TAG, "APP client create new addr 0x4D000 [S]");

	// 初始化配置
    ls_app_load_cfg(voice_mem);
	// extern int ls_app_register_init(char *voice_mem);
	// ls_app_register_init(voice_mem);

	// 工具类初始化
	evs_utils_init();

	// 串口模块初始化
	ls_app_uart_init();
	ls_app_uart_receive_cb(uart_receive_msg_cb);

    // 播放器初始化
    lisa_miniplayer_create(share_mem);
	lisa_miniplayer_set_state_cb(miniplayer_status_cb);
	// 音量模块初始化
	ls_app_volume_init();

	if (device_is_power_on()) {
		// 播放开机提示语
		int ret = cfg_parser_switch_wakeword_info_to_send_protocol();
		if (ret != 0) cfg_parser_welcome_handler();
	}

	// 设置增益
#if CONFIG_AADC_ENABLE
	adc_gain_t adc_gain = cfg_parser_get_adc_gain();
	set_adc_config(adc_gain.adc_again, adc_gain.adc_dgain);
#endif

	LISA_LOGD(TAG, "APP client create [E]");
	return;
}

void listen_client_pre_wakeup(const uint8_t *pre_wakeup_info, uint32_t len)
{
	// LISA_LOGD(TAG, "Pre Wakeup: %s", pre_wakeup_info);
}

void listen_client_wakeup(const uint8_t *data, uint32_t len)
{
    LISA_LOGI(TAG, "Wakeup: %s", data);

	uint8_t keyword[LS_ALGO_KEYWORD_MAX_LEN] = {0};
	int ret = ls_app_algo_keyword_extract(data, keyword, sizeof(keyword) - 1);
	if (ret != 0) {
		LISA_LOGE(TAG, "extra keyword fail");
		return;
	}

	cfg_parser_keyword_handler((const char *)keyword);
}

void listen_client_wakeup_timeout()
{
	cfg_parser_timeout_handler();
}