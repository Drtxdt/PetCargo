/*
 * 配置解析管理模块：
 * 1. 按 ls_app_cfg_user.h 中生成的静态表解析关键词和串口协议；
 * 2. 维护唤醒态、多唤醒词切换、播报开关以及语音注册流程状态；
 * 3. 对外提供欢迎语、超时处理和各类运行期配置查询接口。
 */
#include "ls_app_cfg_mgr.h"
#include "ls_app_cfg_user.h"
#include "ls_app_config.h"
#include "ls_app_volume.h"
#include "mini_player.h"
#include "resmgr.h"
#include "task_algo.h"
#include <stdbool.h>
#include <string.h>
#include "ls_app_algo.h"

#define TAG "cfg_parser"

#define MAIN_KEYWORD_NUM_MAX    (3) //主唤醒词最大数量
// 指定切换模式下的等待标志。置位后，下一条命中的唤醒词不走普通唤醒流程，
// 而是作为“目标唤醒词”写回当前唤醒词索引。
static bool s_into_rollcall_sw_wakeword = false;    //指定切换标志
#define PROTOCOL_SWITCH_DEFAULT_WAKE_WORD     (0xFF)    //协议切换默认唤醒词

// 语音注册/删除流程内部状态机。
// *_ASK 表示已经收到一级控制词，正在等待用户二次确认；
// DELING / REGING 表示已进入具体注册或删除执行流程。
typedef enum {
	REGISTER_ACTION_INIT = 0,
	REGISTER_ACTION_REGING,
	REGISTER_ACTION_DEL_WAKE_ASK,
	REGISTER_ACTION_DEL_CMD_ASK,
	REGISTER_ACTION_DEL_ALL_ASK,
	REGISTER_ACTION_DELING,
	REGISTER_ACTION_INTR_REG_CMD_ASK, //指定注册命令词询问
	REGISTER_ACTION_INTR_DEL_CMD_ASK //指定删除命令词询问
} register_action_t;

// 运行期状态：
// s_reply_switch 读取通用配置中的回复开关；
// s_voice_reg_type 标记学习模式（连续学习 / 指定学习）；
// s_reg_action 跟踪当前注册状态机；
// g_replay_enable 为最终的播报使能开关，会被语音命令和恢复出厂流程改写。
static int s_reply_switch = 1;  //默认控制播报语为开状态
static int s_voice_reg_type = 0;  // 1 连续学习 2 指定学习
static register_action_t s_reg_action = REGISTER_ACTION_INIT;
uint8_t g_replay_enable = 1;     // 播报是否使能 默认开启播报 1开启 0关闭

// 主动播报提示音
// playtype: 0 主动播报 1 被动播报 2 强制播报（不受播报使能控制）
static int active_play_audios(int playtype, const char *audios, uint8_t size)
{
    if (playtype == 2) {
        lisa_miniplayer_play_by_ids((const uint8_t *const)audios, size);
        return 0;
    } else if ((playtype==0) && g_replay_enable) {
        lisa_miniplayer_play_by_ids((const uint8_t *const)audios, size);
        return 0;
    }

    return -1;
}

// 被动播报提示音
static int passive_play_audios(int playtype, const char *audios, uint8_t size)
{
    if (playtype == 2) {
        lisa_miniplayer_play_by_ids((const uint8_t *const)audios, size);
        return 0;
    } else if ((playtype==1) && g_replay_enable) {
        lisa_miniplayer_play_by_ids((const uint8_t *const)audios, size);
        return 0;
    }

    return -1;
}

// 发送串口控制协议
static int uart_send_data(const char *data, uint32_t len)
{
    if (len > 0) {
        ls_app_uart_send((const uint8_t *)data, len);
        return 0;
    }

    return -1;
}

// 长度受控的子串匹配函数，不依赖 '\0' 结尾。
// 当接收协议长度大于模板协议长度时，用它判断接收数据中是否包含目标协议片段。
static bool simple_strstr(const char* str, uint8_t str_len, const char* sub_str, uint8_t sub_str_len) {
    if (!str || !sub_str || (sub_str_len > str_len)) {
        return false;
    }

    uint8_t sub_str_index = 0;

    uint8_t cmp_max_cnt = str_len - sub_str_len;
    for (size_t str_index = 0; str_index <= cmp_max_cnt; str_index++) {
        sub_str_index = 0;
        if (str[str_index] == sub_str[sub_str_index]) {
            for (; sub_str_index < sub_str_len; sub_str_index++) {
                if (str[str_index + sub_str_index] != sub_str[sub_str_index]) {
                    break;
                }
            }

            if (sub_str_index == sub_str_len) return true;
        }
    }

    return false;
}

// 串口协议比较
static bool protocol_compare(const char *recv_msg, uint8_t recv_msg_len, const char *pro_msg, uint8_t pro_msg_len)
{
    if (recv_msg_len == 0 || pro_msg_len == 0) return false;

    if (recv_msg_len == pro_msg_len) {
        return (memcmp(recv_msg, pro_msg, recv_msg_len) == 0) ? true : false;
    } else {
        return simple_strstr(recv_msg, recv_msg_len, pro_msg, pro_msg_len);
    }
}

// keyword比较
static bool keyword_compare(const char *keyword1, uint8_t len1, const char *keyword2, uint8_t len2)
{
    if (len1 == len2 && strncmp(keyword1, keyword2, len1) == 0) {
        return true;
    }

    return false;
}

/**
 * @brief 解析唤醒词、唤醒词查询和恢复默认唤醒词命令。
 * @param keyword 当前识别到的关键词，不能为空。
 * @param current_wakeid 当前生效的唤醒词索引；命中恢复默认命令时会被更新。
 * @param is_waked 输入/输出唤醒状态；命中有效唤醒后会被置为 1。
 * @return true 表示该 keyword 已被本模块消费；false 表示不是唤醒相关命令。
 */
extern uint32_t lisa_os_get_time(void);
// 虚拟唤醒意图，用于语音注册场景下绕过常规唤醒词选择逻辑。
#define VIRTUAL_KEYWORD_INTENT_CHAR   "xu ni yu yin zhu ce huan xing yi tu"
bool parser_wakeup_item(const char *keyword, uint8_t *current_wakeid, int *is_waked)
{
    uint8_t enable = multi_wakeup.enable;
    uint8_t type = multi_wakeup.sw_type;

	for (size_t i = 0; i < sizeof(wakeup_items)/sizeof(wakeup_item_t); i++) {
        wakeup_item_t tmp_item = wakeup_items[i];
		if (keyword) {
            if (keyword_compare(keyword, strlen(keyword), (const char *)tmp_item.keywords, tmp_item.keyword_size)) {
                bool tmp_is_waked = false;
                if (strcmp(keyword, VIRTUAL_KEYWORD_INTENT_CHAR) == 0) {
                    tmp_is_waked = true;    // 0. 虚拟语音注册唤醒意图常驻唤醒
                } else if (!enable || i == (*current_wakeid)) {
                    tmp_is_waked = true;    // 1. 多唤醒词切换为开启，则可直接唤醒 2. 当前唤醒词ID匹配，则可直接唤醒
                } else if (enable) {
                    // 3. 当前是默认唤醒词，且默认唤醒词冻结，则唤醒
                    if (WAKEUP_SWITCH_TYPE_VOICE == type) {
                        for (size_t j = 0; j < multi_wakeup.switch_list_size; j++) {
                            wakeup_switch_item_t tmp_wakeword  = multi_wakeup.switch_list[j];
                            if (keyword_compare(keyword, strlen(keyword), (const char *)tmp_wakeword.keywords, tmp_wakeword.keyword_size)) {
                                if (tmp_wakeword.default_keyword == 1 && tmp_wakeword.frozen == 1) tmp_is_waked = true;
                                break;
                            }
                        }
                    } else if (WAKEUP_SWITCH_TYPE_PROTOCOL == type) {
                        // 协议切换未查询到具体唤醒词，所有唤醒词均可唤醒
                        if ((*current_wakeid) == PROTOCOL_SWITCH_DEFAULT_WAKE_WORD) tmp_is_waked = true;
                    }
                }

               if (tmp_is_waked) {
                    // 指定注册询问过程中唤醒，重置状态
                    if (s_reg_action == REGISTER_ACTION_INTR_REG_CMD_ASK || s_reg_action == REGISTER_ACTION_INTR_DEL_CMD_ASK) {
                        s_reg_action = REGISTER_ACTION_INIT;
                    }
                    *is_waked = 1;
                    // 唤醒音频随机播放
                    uint8_t audios_num = tmp_item.audio_size;
                    uint8_t play_idx = (audios_num>1) ? (lisa_os_get_time()%audios_num) : 0;
                    char tmp_audios[] = {tmp_item.play_audio[play_idx]};
                    active_play_audios(tmp_item.play_type, (const char *)tmp_audios, 1);

                    uart_send_data((const char *)tmp_item.send_pro_buf, tmp_item.send_pro_len);
                } else {
    		        LISA_LOGE(TAG, "cur wk id: %d != %d", *current_wakeid, i);
                }

				return true;
            }
		} else {
    		LISA_LOGE(TAG, "in params error!");
			break;
		}
	}

    if (enable) {
        if (keyword_compare(keyword, strlen(keyword), (const char *)multi_wakeup.query_info.keywords, multi_wakeup.query_info.keyword_size)) {
            LISA_LOGD(TAG, "in voice wakeup qry, keyword: %s", keyword);
            // 查询唤醒词
            if (*current_wakeid < multi_wakeup.switch_list_size) {
                wakeup_switch_item_t current_wakeword_info = multi_wakeup.switch_list[*current_wakeid];
                active_play_audios(2, (const char *)current_wakeword_info.play_audio, current_wakeword_info.audio_size);
            }
            return true;
        } else if (keyword_compare(keyword, strlen(keyword), (const char *)multi_wakeup.restore_info.keywords, multi_wakeup.restore_info.keyword_size)) {
            LISA_LOGD(TAG, "in voice wakeup reset, keyword: %s", keyword);
            if (!(*is_waked)) {
                LISA_LOGE(TAG, "not waked!");
                return true;
            }
            // 恢复默认唤醒词
            if (WAKEUP_SWITCH_TYPE_VOICE == type) {
                *current_wakeid = cfg_parser_get_default_keyword_id();
                ls_app_cfg_refresh_all(CFG_REFRESH_WAKEUP);
                if (*current_wakeid < multi_wakeup.switch_list_size) {
                    wakeup_switch_item_t current_wakeword_info = multi_wakeup.switch_list[*current_wakeid];
                    active_play_audios(2, (const char *)current_wakeword_info.play_audio, current_wakeword_info.audio_size);
                }
            } else {
                LISA_LOGD(TAG, "type is not voice switch, unsupport!");
            }
            return true;
        }
    }

	return false;
}

/**
 * @brief 解析多唤醒词切换命令。
 * @param keyword 语音切换场景下的识别结果；不解析语音时传 NULL。
 * @param recv_msg 协议切换场景下的串口消息；不解析协议时传 NULL。
 * @param cur_wakeid 输入/输出当前生效的唤醒词索引。
 * @return true 表示命中切换流程或切换目标词；false 表示不是切换相关命令。
 */
bool parser_wakeup_switch_item(const char *keyword, const ls_uart_msg_t *recv_msg, uint8_t *cur_wakeid)
{
    uint8_t enable = multi_wakeup.enable;
    uint8_t type = multi_wakeup.sw_type;
    uint8_t mode = multi_wakeup.sw_mode;

    if (!enable) return false;

    if (keyword) {
        if (keyword_compare(keyword, strlen(keyword), (const char *)multi_wakeup.switch_info.keywords, multi_wakeup.switch_info.keyword_size)) {
            if (WAKEUP_SWITCH_TYPE_VOICE == type) {                 // 1.  语音切换唤醒词
                LISA_LOGD(TAG, "in voice wakeup switch, keyword: %s cur_wakeid: %d", keyword, *cur_wakeid);
                if (WAKEUP_SWITCH_MODE_LOOP == mode) {              // 1.1 循环切换
                    int tmp_wakeid = *cur_wakeid;
                    int search_cnt = 0;
                    do {
                        tmp_wakeid++;
                        if (tmp_wakeid >= multi_wakeup.switch_list_size) tmp_wakeid = 0;
                        if (!(multi_wakeup.switch_list[tmp_wakeid].default_keyword == 1 && multi_wakeup.switch_list[tmp_wakeid].frozen == 1)) break;
                        if (++search_cnt > multi_wakeup.switch_list_size) break;
                    } while (1);
                    *cur_wakeid = tmp_wakeid;
                    LISA_LOGD(TAG, "loop change cur_wakeid: %d", *cur_wakeid);
                    ls_app_cfg_refresh_all(CFG_REFRESH_WAKEUP);

                    wakeup_switch_item_t current_wakeword_info = multi_wakeup.switch_list[*cur_wakeid];
                    active_play_audios(2, (const char *)current_wakeword_info.play_audio, current_wakeword_info.audio_size);
                    uart_send_data((const char *)current_wakeword_info.send_pro_buf, current_wakeword_info.send_pro_len);
                } else if (WAKEUP_SWITCH_MODE_ROLLCALL == mode) {   // 1.2 指定切换
                    s_into_rollcall_sw_wakeword = true;
                    active_play_audios(2, (const char *)multi_wakeup.switch_info.play_audio, multi_wakeup.switch_info.audio_size);
                }
            }
            return true;
        } else {
            for (size_t index = 0; index < multi_wakeup.switch_list_size; index++) {
                wakeup_switch_item_t tmp_item = multi_wakeup.switch_list[index];
                if (keyword_compare(keyword, strlen(keyword), (const char *)tmp_item.keywords, tmp_item.keyword_size)) {
                    if (s_into_rollcall_sw_wakeword) {
                        s_into_rollcall_sw_wakeword = false;
                        *cur_wakeid = index;
                        LISA_LOGD(TAG, "spec change cur_wakeid: %d", *cur_wakeid);
                        ls_app_cfg_refresh_all(CFG_REFRESH_WAKEUP);
                        active_play_audios(2, (const char *)tmp_item.play_audio, tmp_item.audio_size);
                    }

                    return true;
                }
            }
        }
    } else if (recv_msg) {
        // 2. 协议切换
        for (size_t index = 0; index < multi_wakeup.switch_list_size; index++) {
            wakeup_switch_item_t tmp_item = multi_wakeup.switch_list[index];
            if (protocol_compare((const char *)recv_msg->data, recv_msg->len, (const char *)tmp_item.recv_pro_buf, tmp_item.recv_pro_len)) {
                *cur_wakeid = index;
                active_play_audios(2, (const char *)tmp_item.play_audio, tmp_item.audio_size);
                return true;
            }
        }
    }
	
	return false;
}

/**
 * @brief 解析语音播报开关命令。
 * @param keyword 语音命令；不解析语音时传 NULL。
 * @param recv_msg 串口协议；不解析协议时传 NULL。
 * @param voice_sw 输出语音播报开关状态，1 为开，0 为关。
 * @return true 表示命中语音播报开关命令；false 表示不是该类命令。
 */
bool parser_voice_item(const char *keyword, const ls_uart_msg_t *recv_msg, uint8_t *voice_sw)
{
    for (size_t i = 0; i < sizeof(voice_items) / sizeof(voice_item_t); i++) {
        voice_item_t tmp_item = voice_items[i];
		if (keyword) {
            if (keyword_compare(keyword, strlen(keyword), (const char *)tmp_item.keywords, tmp_item.keyword_size)) {
                if (tmp_item.type == VOICE_FUNCTION_OPEN) *voice_sw = 1;  // VOICE_FUNCTION_OPEN -> 打开播报
                else if (tmp_item.type == VOICE_FUNCTION_CLOSE) *voice_sw = 0;  // VOICE_FUNCTION_CLOSE -> 关闭播报
                if (tmp_item.play_type == 0) active_play_audios(2, (const char *)tmp_item.play_audio, tmp_item.audio_size);
                // ls_app_cfg_refresh_all(CFG_REFRESH_VOICE);
                uart_send_data((const char *)tmp_item.send_pro_buf, tmp_item.send_pro_len);
				return true;
			}
		} else if (recv_msg) {
            if (protocol_compare((const char *)recv_msg->data, recv_msg->len, (const char *)tmp_item.recv_pro_buf, tmp_item.recv_pro_len)) {
                if (tmp_item.type == VOICE_FUNCTION_OPEN) *voice_sw = 1;  // VOICE_FUNCTION_OPEN -> 打开播报
                else if (tmp_item.type == VOICE_FUNCTION_CLOSE) *voice_sw = 0;  // VOICE_FUNCTION_CLOSE -> 关闭播报
                if (tmp_item.play_type == 1) passive_play_audios(2, (const char *)tmp_item.play_audio, tmp_item.audio_size);
                // ls_app_cfg_refresh_all(CFG_REFRESH_VOICE);
                return true;
            }
		} else {
    		LISA_LOGE(TAG, "in params error!");
			break;
		}
	}
	
	return false;
}

/**
 * @brief 解析恢复出厂设置命令。
 * @param keyword 语音命令；不解析语音时传 NULL。
 * @param recv_msg 串口协议；不解析协议时传 NULL。
 * @return true 表示命中恢复出厂设置命令；false 表示不是该类命令。
 */
bool parser_reset_item(const char *keyword, const ls_uart_msg_t *recv_msg)
{
	for (size_t i = 0; i < sizeof(reset_items) / sizeof(reset_item_t); i++) {
        reset_item_t tmp_item = reset_items[i];
		if (keyword) {
            if (keyword_compare(keyword, strlen(keyword), (const char *)tmp_item.keywords, tmp_item.keyword_size)) {
                active_play_audios(tmp_item.play_type, (const char *)tmp_item.play_audio, tmp_item.audio_size);
                g_replay_enable = 1;
                uart_send_data((const char *)tmp_item.send_pro_buf, tmp_item.send_pro_len);
                if (tmp_item.play_type == 0) ls_app_cfg_clear_all(); // 配置信息恢复为默认
				return true;
			}
		} else if (recv_msg) {
            if (protocol_compare((const char *)recv_msg->data, recv_msg->len, (const char *)tmp_item.recv_pro_buf, tmp_item.recv_pro_len)) {
                passive_play_audios(tmp_item.play_type, (const char *)tmp_item.play_audio, tmp_item.audio_size);
                g_replay_enable = 1;
                if (tmp_item.play_type == 1) ls_app_cfg_clear_all(); // 配置信息恢复为默认
                return true;
            }
		} else {
    		LISA_LOGE(TAG, "in params error!");
			break;
		}
	}
	
	return false;
}

/**
 * @brief 解析“退出识别模式”命令。
 * @param keyword 语音命令；不解析语音时传 NULL。
 * @param recv_msg 串口协议；不解析协议时传 NULL。
 * @return true 表示命中退出识别模式命令；false 表示不是该类命令。
 */
bool parser_quit_asr_item(const char *keyword, const ls_uart_msg_t *recv_msg)
{
	for (size_t i = 0; i < sizeof(quit_asr_items) / sizeof(quit_asr_item_t); i++) {
        quit_asr_item_t tmp_item = quit_asr_items[i];
		if (keyword) {
            if (keyword_compare(keyword, strlen(keyword), (const char *)tmp_item.keywords, tmp_item.keyword_size)) {
                active_play_audios(tmp_item.play_type, (const char *)tmp_item.play_audio, tmp_item.audio_size);
                uart_send_data((const char *)tmp_item.send_pro_buf, tmp_item.send_pro_len);
                if (tmp_item.play_type == 0) send_to_algo_task_msg(e_algo_msg_type_esr_wakeup_mode);
				return true;
			}
		} else if (recv_msg) {
            if (protocol_compare((const char *)recv_msg->data, recv_msg->len, (const char *)tmp_item.recv_pro_buf, tmp_item.recv_pro_len)) {
                passive_play_audios(tmp_item.play_type, (const char *)tmp_item.play_audio, tmp_item.audio_size);
                if (tmp_item.play_type == 1) send_to_algo_task_msg(e_algo_msg_type_esr_wakeup_mode);
                return true;
            }
		} else {
    		LISA_LOGE(TAG, "in params error!");
			break;
		}
	}
	
	return false;
}

/**
 * @brief 解析心跳包协议并按配置回复。
 * @param recv_msg 收到的串口消息，不能为空。
 * @return true 表示命中心跳协议并已发送回复；false 表示不是心跳包。
 */
bool parser_heartbeat_item(const ls_uart_msg_t *recv_msg)
{
	for (size_t i = 0; i < sizeof(heartbeat_items) / sizeof(heartbeat_item_t); i++) {
        heartbeat_item_t tmp_item = heartbeat_items[i];
        if (recv_msg) {
            if (protocol_compare((const char *)recv_msg->data, recv_msg->len, (const char *)tmp_item.recv_pro_buf, tmp_item.recv_pro_len)) {
                uart_send_data((const char *)tmp_item.send_pro_buf, tmp_item.send_pro_len);
				return true;
			}
		} else {
    		LISA_LOGE(TAG, "in params error!");
			break;
		}
	}

	return false;
}

/**
 * @brief 解析普通控制命令词或对应协议。
 * @param keyword 语音命令；不解析语音时传 NULL。
 * @param recv_msg 串口协议；不解析协议时传 NULL。
 * @return true 表示命中普通控制命令；false 表示不是该类命令。
 */
bool parser_command_item(const char *keyword, const ls_uart_msg_t *recv_msg)
{
	for (size_t i = 0; i < sizeof(command_items) / sizeof(command_item_t); i++) {
        command_item_t tmp_item = command_items[i];
		if (keyword) {
            if (keyword_compare(keyword, strlen(keyword), (const char *)tmp_item.keywords, tmp_item.keyword_size)) {
                if (s_reply_switch != 0) active_play_audios(tmp_item.play_type, (const char *)tmp_item.play_audio, tmp_item.audio_size);
                uart_send_data((const char *)tmp_item.send_pro_buf, tmp_item.send_pro_len);
				return true;
			}
		} else if (recv_msg) {
            if (protocol_compare((const char *)recv_msg->data, recv_msg->len, (const char *)tmp_item.recv_pro_buf, tmp_item.recv_pro_len)) {
                if (s_reply_switch != 0) passive_play_audios(tmp_item.play_type, (const char *)tmp_item.play_audio, tmp_item.audio_size);
                return true;
            }
		} else {
    		LISA_LOGE(TAG, "in params error!");
			break;
		}
	}
	
	return false;
}

// 按配置中的控制类型直接操作当前音量，不负责播报和串口协议发送。
static void control_volume(vol_ctrl_t type)
{
    if (type == CTRL_TURN_UP_VOLUME) ls_app_vol_adjust(1);
    else if (type == CTRL_TURN_DOWN_VOLUME) ls_app_vol_adjust(-1);
    else if (type == CTRL_MEDIMUM_VOLUME) ls_app_set_med_volume();
    else if (type == CTRL_MAXIMUM_VOLUME) ls_app_set_max_volume();
    else if (type == CTRL_MINIMUM_VOLUME) ls_app_set_min_volume();
    else LISA_LOGE(TAG, "vol type:%d not found!", type);
}

/**
 * @brief 解析音量控制命令词或对应协议。
 * @param keyword 语音命令；不解析语音时传 NULL。
 * @param recv_msg 串口协议；不解析协议时传 NULL。
 * @return true 表示命中音量控制命令；false 表示不是该类命令。
 */
bool parser_volume_item(const char *keyword, const ls_uart_msg_t *recv_msg)
{
	for (size_t i = 0; i < volume.vol_cmd_item_size; i++) {
        volume_cmd_item_t tmp_item = volume.vol_cmd_items[i];
		if (keyword) {
            if (keyword_compare(keyword, strlen(keyword), (const char *)tmp_item.keywords, tmp_item.keyword_size)) {
                if (tmp_item.play_type == 0) control_volume(tmp_item.ctrl_type);
                uart_send_data((const char *)tmp_item.send_pro_buf, tmp_item.send_pro_len);
				return true;
			}
		} else if (recv_msg) {
            if (protocol_compare((const char *)recv_msg->data, recv_msg->len, (const char *)tmp_item.recv_pro_buf, tmp_item.recv_pro_len)) {
                if (tmp_item.play_type == 1) control_volume(tmp_item.ctrl_type);
                return true;
            }
		} else {
    		LISA_LOGE(TAG, "in params error!");
			break;
		}
	}

	return false;
}

// 判断 keyword 是否命中“指定学习 / 指定删除”命令词模板。
// 命中后返回原始模板词、模板下标以及对应播报音信息。
bool is_voice_register_command(const char *keyword, char *out_reg_keyword, int *out_reg_idx, char **audios, uint8_t *size, bool is_reg)
{
    if (keyword) {
        for (size_t i = 0; i < study_config.reg_commands_item_size; i++) {
            reg_word_item_t *tmp_item = (reg_word_item_t *)(&study_config.reg_commands_items[i]);

            uint8_t tmp_src_keyword[64] = {0};
            uint8_t format_keyword[64] = {0};

            memcpy(tmp_src_keyword, (const char *)tmp_item->keywords, tmp_item->keyword_size);
            ls_app_algo_keyword_format(tmp_src_keyword, format_keyword, 64-1);
            
            if (strcmp(keyword, (const char *)format_keyword) == 0) {
                strcpy(out_reg_keyword, (const char *)tmp_src_keyword);
                *out_reg_idx = i;
                if (is_reg) {
                    *audios = (char *)(tmp_item->play_audio);
                    *size = tmp_item->audio_size;
                } else {
                    *audios = (char *)(tmp_item->del_play_audio);
                    *size = tmp_item->del_audio_size;
                }

                return true;
            }
        }
    }

    return false;
}

// 查询当前 keyword 是否为语音注册控制词，并取回对应的动作类型和提示音。
action_type_t parser_get_voice_reg_action_type(const char *keyword, char **audios, uint8_t *size)
{
    if (keyword) {
        for (size_t i = 0; i < study_config.control_cmd_item_size; i++) {
            control_cmd_item_t *tmp_item = (control_cmd_item_t *)(&study_config.control_cmd_items[i]);
            if (keyword_compare(keyword, strlen(keyword), (const char *)tmp_item->keywords, tmp_item->keyword_size)) {
                if (!study_config.enable) {
                    LISA_LOGW(TAG, "study func is closed.");
                    return ACTION_TYPE_UNKNOW;
                }

                *audios = (char *)tmp_item->play_audio;
                *size = tmp_item->audio_size;
                return tmp_item->action_type;
            }
        }
    }

    return ACTION_TYPE_UNKNOW;
}

/**
 * @brief 解析语音注册/删除相关控制词。
 * @param keyword 当前识别到的关键词。
 * @return true 表示命中了语音注册控制流程；false 表示不是注册相关命令。
 */
bool parser_study_item(const char *keyword)
{
    if (keyword) {
        for (size_t i = 0; i < study_config.control_cmd_item_size; i++) {
            control_cmd_item_t tmp_item = study_config.control_cmd_items[i];

            // char tmp_debug[128] = {0};
            // memcpy(tmp_debug, ADDR_2_CHAR((const char *)tmp_item.keywords_addr, ptr), tmp_item.keyword_size);
            // LISA_LOGD(TAG, "(1) in - keyword[%d]: %s", strlen(keyword), keyword);
            // LISA_LOGD(TAG, "(2) %02d - keyword[%d]: %s", i, tmp_item.keyword_size, tmp_debug);

            if (keyword_compare(keyword, strlen(keyword), (const char *)tmp_item.keywords, tmp_item.keyword_size)) {
                if (!study_config.enable) {
                    LISA_LOGW(TAG, "study func is closed.");
                    return true;
                }

                if (s_voice_reg_type != 1 && s_voice_reg_type != 2) {
                    LISA_LOGW(TAG, "reg type [%d] unknow (!=1/2)!", s_voice_reg_type);
                    return true;
                }

                const char *audios = (const char *)tmp_item.play_audio;
                uint8_t audio_size = tmp_item.audio_size;

                if (s_reg_action == REGISTER_ACTION_INIT) {
                    switch (tmp_item.action_type)
                    {
                    case ACTION_TYPE_STUDY_WAKE:
                        s_reg_action = REGISTER_ACTION_REGING;
                        ls_app_register_type_process(REGISTER_TYPE_WAKE_WORD, audios, audio_size);
                        break;
                    case ACTION_TYPE_STUDY_CMD:
                        if (s_voice_reg_type == 1) {
                            s_reg_action = REGISTER_ACTION_REGING;
                            ls_app_register_type_process(REGISTER_TYPE_CMD_LIST, audios, audio_size);
                        } else {
                            s_reg_action = REGISTER_ACTION_INTR_REG_CMD_ASK;
                            cfg_parser_study_reply_by_label(REPLY_LABEL_TYPE_TIP_ASR_START);
                        }
                        break;
                    case ACTION_TYPE_DEL_WAKE:
                        s_reg_action = REGISTER_ACTION_DEL_WAKE_ASK;
                        cfg_parser_study_reply_by_label(REPLY_LABEL_TYPE_TIP_DELETE_WAKE_START);
                        break;
                    case ACTION_TYPE_DEL_CMD:
                        if (s_voice_reg_type == 1) {
                            s_reg_action = REGISTER_ACTION_DEL_CMD_ASK;
                        } else {
                            s_reg_action = REGISTER_ACTION_INTR_DEL_CMD_ASK;
                        }
                        cfg_parser_study_reply_by_label(REPLY_LABEL_TYPE_TIP_DELETE_CMD_START);
                        break;
                        /*
                    case ACTION_TYPE_DEL_ALL_CMD:
                        s_reg_action = REGISTER_ACTION_DEL_CMD_ASK;
                        cfg_parser_study_reply_by_label(REPLY_LABEL_TYPE_TIP_DELETE_ALL_START);
                        break;
                        */
                    case ACTION_TYPE_DEL_ALL:
                        s_reg_action = REGISTER_ACTION_DEL_ALL_ASK;
                        cfg_parser_study_reply_by_label(REPLY_LABEL_TYPE_TIP_DELETE_ALL_START);
                        break;
                    default:
                        break;
                    }
                } else if (s_reg_action == REGISTER_ACTION_DEL_WAKE_ASK ||
                        s_reg_action == REGISTER_ACTION_DEL_CMD_ASK ||
                        s_reg_action == REGISTER_ACTION_DEL_ALL_ASK) {		//删除注册选择
                    switch (tmp_item.action_type)
                    {
                    case ACTION_TYPE_DEL_QUIT:
                        clear_voice_reg_flag(false);
                        active_play_audios(2, audios, audio_size);
                        break;
                    case ACTION_TYPE_DEL_WAKE:
                        if (s_reg_action == REGISTER_ACTION_DEL_WAKE_ASK) {
                            s_reg_action = REGISTER_ACTION_DELING;
                            ls_app_register_del_process(REGISTER_DELETE_WAKE, audios, audio_size);
                        } else {
                            s_reg_action = REGISTER_ACTION_DEL_WAKE_ASK;
                            cfg_parser_study_reply_by_label(REPLY_LABEL_TYPE_TIP_DELETE_WAKE_START);
                        }
                        break;
                    case ACTION_TYPE_DEL_CMD:
                        if (s_reg_action == REGISTER_ACTION_DEL_CMD_ASK) {
                            s_reg_action = REGISTER_ACTION_DELING;
                            ls_app_register_del_process(REGISTER_DELETE_CMD, audios, audio_size);
                        } else {
                            s_reg_action = REGISTER_ACTION_DEL_CMD_ASK;
                            cfg_parser_study_reply_by_label(REPLY_LABEL_TYPE_TIP_DELETE_CMD_START);
                        }
                        break;
                    case ACTION_TYPE_DEL_ALL:
                        if (s_reg_action == REGISTER_ACTION_DEL_ALL_ASK) {
                            s_reg_action = REGISTER_ACTION_DELING;
                            ls_app_register_del_process(REGISTER_DELETE_ALL, audios, audio_size);
                        } else {
                            s_reg_action = REGISTER_ACTION_DEL_ALL_ASK;
                            cfg_parser_study_reply_by_label(REPLY_LABEL_TYPE_TIP_DELETE_ALL_START);
                        }
                        break;
                    default:
                        break;
                    }
                }

                return true;
            }
        }
	}
	
	return false;
}

// ======================== 对外配置访问与事件处理接口 ========================
static int s_is_waked = 0;	// 是否已经唤醒

// 清理语音注册状态；当 reset_waked 为真时，同时退出已唤醒状态。
void clear_voice_reg_flag(int reset_waked)
{
	if (reset_waked) s_is_waked = false;
	s_reg_action = REGISTER_ACTION_INIT;
}

/**
 * @brief 初始化解析模块运行态。
 * @details 启动时会校正持久化配置中的唤醒词索引和音量索引，
 *          防止配置项数量变化后读到越界值。
 * @return 0 表示初始化完成。
 */
int cfg_parser_init()
{
    bool cfg_changed = false;
    // 判断保存的唤醒词下标是否大于唤醒词数量，若大于则恢复配置
    if (g_run_cfg.wakeup_words != PROTOCOL_SWITCH_DEFAULT_WAKE_WORD && \
            g_run_cfg.wakeup_words >= sizeof(wakeup_items) / sizeof(wakeup_items[0])) {
        g_run_cfg.wakeup_words = 0;
        cfg_changed = true;
    }

    // 判断保存的音量小标是否大于配置中音量等级数量，若大于则将保存的配置恢复默认
    uint8_t level_size = volume.level_size;
    if (g_run_cfg.volume_level >= level_size) {
        g_run_cfg.volume_level = volume.default_vol;
        cfg_changed = true;
    }

    if (cfg_changed) {
        LISA_LOGD(TAG, "int cfg changed");
        ls_app_cfg_refresh_all(CFG_REFRESH_ALL);
    }

    s_reply_switch = general_config.reply_switch;
    s_voice_reg_type = cfg_parser_get_voice_reg_type();

    return 0;
}

// 以下接口直接对外暴露解析后的静态配置内容，不改写运行期状态。
const char *cfg_parser_get_version()
{
    return (const char *)general_config.version;
}

adc_gain_t cfg_parser_get_adc_gain()
{
    adc_gain_t adc_gain = {0};
    adc_gain.adc_again = general_config.adc_again;
    adc_gain.adc_dgain = general_config.adc_dgain;

    return adc_gain;
}

int cfg_parser_get_loglev()
{
    return general_config.log_level;
}

pa_config_item_t cfg_parser_get_pa_config()
{
    return pa_config;
}

persisted_info_t cfg_parser_get_persisted_info()
{
    persisted_info_t pers_info = {0};
    pers_info.voice = general_config.persisted_voice;
    pers_info.volume = general_config.persisted_volume;
    pers_info.wakeup = general_config.persisted_wakeup;

    return pers_info;
}

int cfg_parser_get_vol_info(volume_info_t *vol_info)
{
    memcpy(vol_info->level, volume.level, volume.level_size);
    vol_info->level_size = volume.level_size;
    vol_info->default_vol = volume.default_vol;
    vol_info->adjmax_vol_tip.audios = (uint8_t *)volume.adjmax_play_audio;
    vol_info->adjmax_vol_tip.audio_size = volume.adjmax_play_audio_size;
    vol_info->adjmin_vol_tip.audios = (uint8_t *)volume.adjmin_play_audio;
    vol_info->adjmin_vol_tip.audio_size = volume.adjmin_play_audio_size;

	for (size_t i = 0; i < volume.vol_cmd_item_size; i++) {
        switch (volume.vol_cmd_items[i].ctrl_type)
        {
        case CTRL_TURN_UP_VOLUME:
            vol_info->up_vol_tip.audios = (uint8_t *)volume.vol_cmd_items[i].play_audio;
            vol_info->up_vol_tip.audio_size = volume.vol_cmd_items[i].audio_size;
            break;
        case CTRL_TURN_DOWN_VOLUME:
            vol_info->down_vol_tip.audios = (uint8_t *)volume.vol_cmd_items[i].play_audio;
            vol_info->down_vol_tip.audio_size = volume.vol_cmd_items[i].audio_size;
            break;
        case CTRL_MEDIMUM_VOLUME:
            vol_info->med_vol_tip.audios = (uint8_t *)volume.vol_cmd_items[i].play_audio;
            vol_info->med_vol_tip.audio_size = volume.vol_cmd_items[i].audio_size;
            break;
        case CTRL_MAXIMUM_VOLUME:
            vol_info->max_vol_tip.audios = (uint8_t *)volume.vol_cmd_items[i].play_audio;
            vol_info->max_vol_tip.audio_size = volume.vol_cmd_items[i].audio_size;
            break;
        case CTRL_MINIMUM_VOLUME:
            vol_info->min_vol_tip.audios = (uint8_t *)volume.vol_cmd_items[i].play_audio;
            vol_info->min_vol_tip.audio_size = volume.vol_cmd_items[i].audio_size;
            break;
        default:
            LISA_LOGE(TAG, "get: vol ctrl_type:%d not found!", volume.vol_cmd_items[i].ctrl_type);
            break;
        }
	}

    return 0;
}

int cfg_parser_get_timeout_time()
{   
    return timeout.time;
}

int cfg_parser_welcome_handler()
{
    //主动播报并发协议
    active_play_audios(welcome.play_type, (const char *)welcome.play_audio, welcome.audio_size);
    uart_send_data((const char *)welcome.send_pro_buf, welcome.send_pro_len);

    return 0;
}

/**
 * @brief 处理串口接收到的协议数据。
 * @details 按普通命令、音量、唤醒词切换、语音开关、恢复出厂、退出识别、
 *          欢迎语、心跳和超时播报的顺序依次匹配。
 * @param uart_msg 当前收到的串口消息。
 * @return 0 表示已命中并完成处理；-1 表示没有任何配置项匹配。
 */
int cfg_parser_recv_data_handler(const ls_uart_msg_t *uart_msg)
{
    bool ret;

    // 判断是否为普通控制命令词
    ret =  parser_command_item(
            NULL,
            uart_msg
            );
    if (ret) return 0;

    ret =  parser_volume_item(
            NULL,
            uart_msg
            );
    if (ret) return 0;

    // 判断是否为切换唤醒词命令词
    ret = parser_wakeup_switch_item(
            NULL,
            uart_msg,
            &g_run_cfg.wakeup_words
            );
    if (ret) return 0;

    // 判断是否为语音开关命令词
    ret = parser_voice_item(
            NULL,
            uart_msg,
            &g_replay_enable
            );
    if (ret) return 0;

    // 判断是否为恢复出厂设置命令词
    ret = parser_reset_item(
            NULL,
            uart_msg
            );
    if (ret) return 0;

    // 判断是否为退出识别模式设置命令词
    ret = parser_quit_asr_item(
            NULL,
            uart_msg
            );
    if (ret) return 0;

    // 开机提示语被动播报
    if (protocol_compare((const char *)uart_msg->data, uart_msg->len, (const char *)welcome.recv_pro_buf, welcome.recv_pro_len)) {
        passive_play_audios(welcome.play_type, (const char *)welcome.play_audio, welcome.audio_size);
        return 0;
    }

    // 判断是否为心跳包命令词
    ret = parser_heartbeat_item(
            uart_msg
            );
    if (ret) return 0;

    // 超时提示语被动播报
    if (protocol_compare((const char *)uart_msg->data, uart_msg->len, (const char *)timeout.recv_pro_buf, timeout.recv_pro_len)) {
        passive_play_audios(timeout.play_type, (const char *)timeout.play_audio, timeout.audio_size);
        return 0;
    }

    LISA_LOGW(TAG, "recv data miss!");
    return -1;
}

/**
 * @brief 处理算法侧识别到的关键词。
 * @details 先判断唤醒态和指定切换态，再按普通命令、音量、唤醒词切换、
 *          语音开关、恢复出厂、退出识别和语音注册流程顺序进行匹配。
 * @param keyword 当前识别出的关键词文本。
 * @return 0 表示已处理或被显式忽略；-1 表示输入非法或未匹配到任何配置项。
 */
int cfg_parser_keyword_handler(const char *keyword)
{
    if (!keyword || strlen(keyword) <= 0) {
        LISA_LOGE(TAG, "Input keyword error!");
        return -1;
    }

    if (g_run_cfg.voice_switch == 0) {
        LISA_LOGE(TAG, "mute!");
        return -1;
    }

    bool result = false;

    // 进入指定切换唤醒词，不处理唤醒逻辑
    if (!s_into_rollcall_sw_wakeword) {
        result = parser_wakeup_item(
                keyword,
                &g_run_cfg.wakeup_words,
                &s_is_waked
                );
        if (result) return 0;
    }

    // 未唤醒状态不响应命令词
    if (!s_is_waked) {
        LISA_LOGE(TAG, "not waked!");
        return 0;
    }

    if (s_reg_action == REGISTER_ACTION_INTR_REG_CMD_ASK) {
        char reg_cmd_words[64] = {0};
        int reg_cmd_idx = 0;
        char *audios_ptr = NULL;
        uint8_t audio_size = 0;
            
        action_type_t action_type = parser_get_voice_reg_action_type(
            keyword,
            &audios_ptr,
            &audio_size
            );
        
        LISA_LOGW(TAG, "study get action type: %d", action_type);
        if (action_type == ACTION_TYPE_STUDY_QUIT) {
            s_reg_action = REGISTER_ACTION_INIT;
            lisa_miniplayer_play_by_ids((const uint8_t *const)audios_ptr, audio_size);
            return 0;
        }

        result = is_voice_register_command(
                keyword,
                reg_cmd_words,
                &reg_cmd_idx,
                &audios_ptr,
                &audio_size,
                true
                );

        if (result) {
            s_reg_action = REGISTER_ACTION_REGING;
            ls_app_register_set_cmd_intr_words(reg_cmd_words, reg_cmd_idx);
            ls_app_register_type_process(REGISTER_TYPE_CMD_INTR, audios_ptr, audio_size);
        } else {
            s_reg_action = REGISTER_ACTION_INTR_REG_CMD_ASK;
            cfg_parser_study_reply_by_label(REPLY_LABEL_TYPE_TIP_ASR_OVV);
        }

        return 0;
    } else if (s_reg_action == REGISTER_ACTION_INTR_DEL_CMD_ASK) {
        char reg_cmd_words[64] = {0};
        int reg_cmd_idx = 0;
        char *audios_ptr = NULL;
        uint8_t audio_size = 0;

        action_type_t action_type = parser_get_voice_reg_action_type(
            keyword,
            &audios_ptr,
            &audio_size
            );
        
        LISA_LOGW(TAG, "del get action type: %d", action_type);
        if (action_type == ACTION_TYPE_DEL_QUIT) {
            s_reg_action = REGISTER_ACTION_INIT;
            lisa_miniplayer_play_by_ids((const uint8_t *const)audios_ptr, audio_size);
            return 0;
        } else if (action_type == ACTION_TYPE_DEL_ALL_CMD) {
            ls_app_register_del_process(REGISTER_DELETE_ALL_CMD, audios_ptr, audio_size);
            return 0;
        }

        result = is_voice_register_command(
                keyword,
                reg_cmd_words,
                &reg_cmd_idx,
                &audios_ptr,
                &audio_size,
                false
                );

        if (result) {
            s_reg_action = REGISTER_ACTION_REGING;
            ls_app_register_set_cmd_intr_words(reg_cmd_words, reg_cmd_idx);
            ls_app_register_del_process(REGISTER_DELETE_INTR_CMD, audios_ptr, audio_size);
        } else {
            s_reg_action = REGISTER_ACTION_INTR_DEL_CMD_ASK;
            cfg_parser_study_reply_by_label(REPLY_LABEL_TYPE_TIP_ASR_OVV);
        }

        return 0;
    }

    // 判断是否为普通控制命令词
    result =  parser_command_item(
            keyword,
            NULL
            );
    if (result) return 0;

    // 判断是否为音量控制命令词
    result =  parser_volume_item(
            keyword,
            NULL
            );
    if (result) return 0;

    // 判断是否为切换唤醒词命令词
    result = parser_wakeup_switch_item(
            keyword,
            NULL,
            &g_run_cfg.wakeup_words
            );
    if (result) return 0;

    // 判断是否为语音开关命令词
    result = parser_voice_item(
            keyword,
            NULL,
            &g_replay_enable
            );
    if (result) return 0;

    // 判断是否为恢复出厂设置命令词
    result = parser_reset_item(
            keyword,
            NULL
            );
    if (result) return 0;

    result = parser_quit_asr_item(
            keyword,
            NULL
            );
    if (result) return 0;

    result = parser_study_item(
            keyword
            );
    if (result) return 0;

    LISA_LOGE(TAG, "%s miss!", keyword);
    return -1;
}

/**
 * @brief 处理唤醒超时事件。
 * @details 超时后会清理唤醒态、注册状态和指定切换状态，
 *          然后按配置播放超时提示音并发送对应协议。
 * @return 0 表示处理完成。
 */
int cfg_parser_timeout_handler()
{
    LISA_LOGD(TAG, "Wk timeout");

    if (s_is_waked) {
        s_is_waked = false;
        s_reg_action = REGISTER_ACTION_INIT;
        s_into_rollcall_sw_wakeword = false;
        // 播放超时提示语
        active_play_audios(timeout.play_type, (const char *)timeout.play_audio, timeout.audio_size);
        uart_send_data((const char *)timeout.send_pro_buf, timeout.send_pro_len);
    }

    return 0;
}

// 以下接口用于读取超时提示和语音注册相关配置。
int cfg_parser_get_timeout_audio(audio_info_t *audio_info)
{
    audio_info->audios = (uint8_t *)timeout.play_audio;
    audio_info->audio_size = timeout.audio_size;
    return 0;
}

int cfg_parser_get_study_switch()
{
    return study_config.enable;
}

int cfg_parser_get_study_user_cfg(study_user_cfg_t *user_cfg)
{
    user_cfg->wake_sensitivity = study_config.user_cfg.wake_sensitivity;
    user_cfg->wake_min_words = study_config.user_cfg.wake_min_words;
    user_cfg->wake_max_words = study_config.user_cfg.wake_max_words;
    user_cfg->wake_reg_max = study_config.user_cfg.wake_reg_max;
    user_cfg->wake_repeat_count = study_config.user_cfg.wake_repeat_count;
    user_cfg->wake_retry_count = study_config.user_cfg.wake_retry_count;

    user_cfg->cmd_sensitivity = study_config.user_cfg.cmd_sensitivity;
    user_cfg->cmd_min_words = study_config.user_cfg.cmd_min_words;
    user_cfg->cmd_max_words = study_config.user_cfg.cmd_max_words;
    user_cfg->cmd_reg_max = study_config.user_cfg.cmd_reg_max;
    user_cfg->cmd_repeat_count = study_config.user_cfg.cmd_repeat_count;
    user_cfg->cmd_retry_count = study_config.user_cfg.cmd_retry_count;

    return 0;
}

int cfg_parser_study_reply_by_label(reply_label_t reply_label)
{
    for (size_t i = 0; i < study_config.other_replys_item_size; i++)
    {
        if (study_config.other_replys_items[i].label_type == reply_label) {
            active_play_audios(2, (const char *)study_config.other_replys_items[i].play_audio, study_config.other_replys_items[i].audio_size);
            return 0;
        }
    }

    return -1;
}

int cfg_parser_study_get_audio_by_label(reply_label_t reply_label, audio_info_t *audio_info)
{
    for (size_t i = 0; i < study_config.other_replys_item_size; i++)
    {
        if (study_config.other_replys_items[i].label_type == reply_label) {
            audio_info->audios = (uint8_t *)(study_config.other_replys_items[i].play_audio);
            audio_info->audio_size = study_config.other_replys_items[i].audio_size;
            return 0;
        }
    }

    return -1;
}

int cfg_parser_study_get_reg_wake_words(reg_word_info_t *reg_word_info)
{
    reg_word_info->keywords_ptr = (char *)(study_config.reg_wakewords_items[0].keywords);
    reg_word_info->keyword_size = study_config.reg_wakewords_items[0].keyword_size;
    reg_word_info->play_audio_ptr = (char *)(study_config.reg_wakewords_items[0].play_audio);
    reg_word_info->audio_size = study_config.reg_wakewords_items[0].audio_size;

    return 0;
}

int cfg_parser_study_get_reg_cmd_count()
{
    return study_config.reg_commands_item_size;
}

int cfg_parser_study_get_reg_cmd_words_by_index(int index, reg_word_info_t *reg_word_info)
{
    if (0 <= index && index < study_config.reg_commands_item_size) {
        reg_word_info->keywords_ptr = (char *)(study_config.reg_commands_items[index].keywords);
        reg_word_info->keyword_size = study_config.reg_commands_items[index].keyword_size;
        reg_word_info->play_audio_ptr = (char *)(study_config.reg_commands_items[index].play_audio);
        reg_word_info->audio_size = study_config.reg_commands_items[index].audio_size;
        return 0;
    }

    return -1;
}

/**
 * @brief 处理语音注册进行中的控制关键词。
 * @details 仅在算法已进入“等待下一步”状态时调用，用于响应“下一条/重学/退出”等控制词。
 * @param keyword 当前识别到的关键词。
 * @return 0 表示流程已处理完成或已给出冲突反馈。
 */
int cfg_parser_register_inprogress_keyword_handler(const char *keyword)
{
    if (keyword) {
        for (size_t i = 0; i < study_config.control_cmd_item_size; i++) {
            control_cmd_item_t tmp_item = study_config.control_cmd_items[i];

            if (keyword_compare(keyword, strlen(keyword), (const char *)tmp_item.keywords, tmp_item.keyword_size)) {
                LISA_LOGD(TAG, "action type: %d", tmp_item.action_type);
                if (!study_config.enable) {
                    LISA_LOGW(TAG, "study func is closed.");
                    return 0;
                }

                const char *audios = (const char *)tmp_item.play_audio;
                uint8_t audio_size = tmp_item.audio_size;

                switch (tmp_item.action_type)
                {
                case ACTION_TYPE_STUDY_NEXT:
                    voice_register_status_process(REGISTER_STATUS_USER_NEXT, audios, audio_size);
                    return 0;
                case ACTION_TYPE_STUDY_RELEARN:
                    voice_register_status_process(REGISTER_STATUS_USER_RETRY, audios, audio_size);
                    return 0;
                case ACTION_TYPE_STUDY_QUIT:
                    ls_app_register_type_process(REGISTER_TYPE_USER_QUIT, audios, audio_size);
                    return 0;
                default:
                    voice_register_status_process(REGISTER_STATUS_ERROR_CONFLICT, NULL, 0);
                    return 0;
                }
            }
        }

        voice_register_status_process(REGISTER_STATUS_ERROR_CONFLICT, NULL, 0);
    }

    return 0;
}

int cfg_parser_get_voice_reg_type()
{
    return study_config.type;
}

int cfg_parser_get_voice_reg_word_cnt()
{
    return study_config.reg_wakewords_item_size;
}

int cfg_parser_get_voice_reg_cmd_cnt()
{
    return study_config.reg_commands_item_size;
}

/**
 * @brief 协议切换型多唤醒词启动时，发送查询/探测协议。
 * @details 调用后会先把当前唤醒词标记为“协议默认值”，
 *          等待设备通过串口回包告知实际生效的唤醒词。
 * @return 0 表示已进入协议切换查询流程；-1 表示当前不是协议切换模式。
 */
int cfg_parser_switch_wakeword_info_to_send_protocol()
{
    if (multi_wakeup.enable && WAKEUP_SWITCH_TYPE_PROTOCOL == multi_wakeup.sw_type) {
        LISA_LOGD(TAG, "start send uart data qry dev type!");
        g_run_cfg.wakeup_words = PROTOCOL_SWITCH_DEFAULT_WAKE_WORD;
        ls_app_cfg_refresh_all(CFG_REFRESH_WAKEUP);
        if (multi_wakeup.switch_list_size > 0)
            uart_send_data((const char *)multi_wakeup.switch_list[0].send_pro_buf, multi_wakeup.switch_list[0].send_pro_len);
        
        return 0;
    }

    return -1;
}

// 获取默认唤醒词索引。语音切换模式下查找 frozen 且 default_keyword 的项，
// 协议切换模式下返回协议默认占位值。
int cfg_parser_get_default_keyword_id()
{
    int default_keyword_id = 0;

    if (multi_wakeup.enable) {
        if (WAKEUP_SWITCH_TYPE_VOICE == multi_wakeup.sw_type) {
            for (size_t i = 0; i < multi_wakeup.switch_list_size; i++) {
                wakeup_switch_item_t item = multi_wakeup.switch_list[i];
                if (item.default_keyword == 1 && item.frozen == 1) {
                    default_keyword_id = i;
                    break;
                }
            }
        } else if (WAKEUP_SWITCH_TYPE_PROTOCOL == multi_wakeup.sw_type) {
            default_keyword_id = PROTOCOL_SWITCH_DEFAULT_WAKE_WORD;
        }
    }

    return default_keyword_id;
}

// 获取串口配置，供初始化串口驱动时使用。
uart_config_t cfg_parser_get_uart_config()
{
    return uart_config;
}