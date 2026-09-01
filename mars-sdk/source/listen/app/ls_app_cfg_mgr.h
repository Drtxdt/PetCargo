#ifndef __LISTENAI_CONFIG_MGR_H__
#define __LISTENAI_CONFIG_MGR_H__

/*
 * 配置解析管理对外接口：
 * 1. 负责读取由 ls_app_cfg_user.h 生成的静态配置；
 * 2. 对外提供欢迎语、超时、关键词/串口消息处理入口；
 * 3. 提供音量、持久化、自学习等运行期配置查询接口。
 */
#include "ls_app_uart.h"
#include "ls_app_cfg_def.h"

// 配置资源在打包区中的偏移地址。
#define APP_CFG_RES_ADDR            (8)

// ADC 增益配置，分别对应模拟增益和数字增益。
typedef struct
{
	int16_t adc_again;  // 模拟增益
    int16_t adc_dgain;  // 数字增益
} adc_gain_t;

// 一组提示音信息，包含音频 ID 列表及其数量。
typedef struct
{
    uint8_t *audios;        // 音频
	uint8_t audio_size;     // 需要播放的音频列表的大小
} audio_info_t;

// 音量相关配置集合，包含音量等级、默认值以及各场景提示音。
typedef struct
{
    uint8_t level[10];      // 音量等级，0-100
	uint8_t level_size;     // 音量等级数量
    uint8_t default_vol;    // 上电默认音量
    audio_info_t max_vol_tip;      // 最大音量提示音
    audio_info_t min_vol_tip;      // 最小音量提示音
    audio_info_t med_vol_tip;      // 中音量提示音
    audio_info_t up_vol_tip;       // 调大音量提示音
    audio_info_t down_vol_tip;     // 调小音量提示音
    audio_info_t adjmax_vol_tip;   // 已到最大音量提示音
    audio_info_t adjmin_vol_tip;   // 已到最小音量提示音
} volume_info_t;

// 音频资源地址信息。
typedef struct
{
	uint8_t *addr;
	uint32_t size;
} tone_info_t;

// 持久化信息
typedef struct
{
    uint8_t volume;                   // 音量是否需要持久化
    uint8_t voice;                    // 语音开关是否需要持久化
    uint8_t wakeup;                   // 唤醒词是否需要持久化
} persisted_info_t;

// 语音注册（自学习）用户参数。
// 灵敏度字段通常约定为：0 低，1 中，2 高。
typedef struct
{
    uint8_t wake_sensitivity;       // 唤醒词灵敏度：0 低，1 中，2 高
    uint8_t wake_min_words;         // 唤醒词最少字数
    uint8_t wake_max_words;         // 唤醒词最多字数
    uint8_t wake_reg_max;           // 最大可注册唤醒词数量
    uint8_t wake_repeat_count;      // 唤醒词重复次数
    uint8_t wake_retry_count;       // 唤醒词重试次数
    uint8_t cmd_sensitivity;        // 命令词灵敏度：0 低，1 中，2 高
    uint8_t cmd_min_words;          // 命令词最少字数
    uint8_t cmd_max_words;          // 命令词最多字数
    uint8_t cmd_reg_max;            // 最大可注册命令词数量
    uint8_t cmd_repeat_count;       // 命令词重复次数
    uint8_t cmd_retry_count;        // 命令词重试次数
} study_user_cfg_t;

/**
 * @brief 初始化配置解析模块。
 * @return 0 表示初始化完成。
 */
int cfg_parser_init();

/**
 * @brief 获取配置版本字符串。
 * @return 指向配置版本号字符串的只读指针。
 */
const char *cfg_parser_get_version();

/**
 * @brief 获取 ADC 增益配置。
 * @return 当前配置中的模拟增益和数字增益。
 */
adc_gain_t cfg_parser_get_adc_gain();

/**
 * @brief 获取日志等级。
 * @return 当前配置中的日志级别。
 */
int cfg_parser_get_loglev();

/**
 * @brief 获取音量相关配置。
 * @param vol_info 输出音量信息结构体。
 * @return 0 表示获取成功。
 */
int cfg_parser_get_vol_info(volume_info_t *vol_info);

/**
 * @brief 获取唤醒超时时间。
 * @return 超时时长，单位为秒。
 */
int cfg_parser_get_timeout_time();

/**
 * @brief 获取持久化开关信息。
 * @return 音量、语音开关和唤醒词的持久化配置。
 */
persisted_info_t cfg_parser_get_persisted_info();

/**
 * @brief 处理开机欢迎语。
 * @details 按配置播放欢迎提示音，并发送对应的开机协议。
 * @return 0 表示处理完成。
 */
int cfg_parser_welcome_handler();

/**
 * @brief 处理唤醒超时。
 * @return 0 表示处理完成。
 */
int cfg_parser_timeout_handler();

/**
 * @brief 处理串口接收消息。
 * @param uart_msg 当前收到的串口消息。
 * @return 0 表示已命中并完成处理；-1 表示未命中任何配置项。
 */
int cfg_parser_recv_data_handler(const ls_uart_msg_t *uart_msg);

/**
 * @brief 处理关键词识别结果。
 * @param keyword 当前识别出的关键词文本。
 * @return 0 表示已处理；-1 表示输入非法或未命中任何配置项。
 */
int cfg_parser_keyword_handler(const char *keyword);

/**
 * @brief 处理语音注册进行中的控制关键词。
 * @param keyword 当前识别出的关键词文本。
 * @return 0 表示处理完成。
 */
int cfg_parser_register_inprogress_keyword_handler(const char *keyword);

/**
 * @brief 获取超时提示音信息。
 * @param audio_info 输出超时提示音信息。
 * @return 0 表示获取成功。
 */
int cfg_parser_get_timeout_audio(audio_info_t *audio_info);

/**
 * @brief 获取自学习功能开关。
 * @return 1 表示开启，0 表示关闭。
 */
int cfg_parser_get_study_switch();

/**
 * @brief 获取自学习用户参数。
 * @param user_cfg 输出自学习参数。
 * @return 0 表示获取成功。
 */
int cfg_parser_get_study_user_cfg(study_user_cfg_t *user_cfg);

/**
 * @brief 按提示标签主动播报对应提示音。
 * @param reply_label 提示标签枚举值。
 * @return 0 表示找到并完成播报；-1 表示未找到对应配置。
 */
int cfg_parser_study_reply_by_label(reply_label_t reply_label);

/**
 * @brief 按提示标签获取提示音信息。
 * @param reply_label 提示标签枚举值。
 * @param audio_info 输出提示音信息。
 * @return 0 表示获取成功；-1 表示未找到对应配置。
 */
int cfg_parser_study_get_audio_by_label(reply_label_t reply_label, audio_info_t *audio_info);

// 语音注册词模板信息。
typedef struct reg_word_info
{
    uint8_t keyword_size;   // 关键词拼音长度
	uint8_t audio_size;     // 需要播放的音频列表的大小
    char *keywords_ptr;     // 关键词拼音地址
    char *play_audio_ptr;   // 需要播放的音频列表地址
} reg_word_info_t;

/**
 * @brief 获取默认注册唤醒词模板。
 * @param reg_word_info 输出注册唤醒词模板信息。
 * @return 0 表示获取成功。
 */
int cfg_parser_study_get_reg_wake_words(reg_word_info_t *reg_word_info);

/**
 * @brief 获取可注册命令词模板数量。
 * @return 命令词模板数量。
 */
int cfg_parser_study_get_reg_cmd_count();

/**
 * @brief 按下标获取可注册命令词模板。
 * @param index 命令词模板下标。
 * @param reg_word_info 输出命令词模板信息。
 * @return 0 表示获取成功；-1 表示下标越界。
 */
int cfg_parser_study_get_reg_cmd_words_by_index(int index, reg_word_info_t *reg_word_info);

/**
 * @brief 清理语音注册状态。
 * @param reset_waked 非 0 时同时清除已唤醒状态。
 */
void clear_voice_reg_flag(int reset_waked);

/**
 * @brief 获取语音注册模式类型。
 * @return 语音注册模式，通常 1 表示连续学习，2 表示指定学习。
 */
int cfg_parser_get_voice_reg_type();

/**
 * @brief 获取可注册唤醒词模板数量。
 * @return 唤醒词模板数量。
 */
int cfg_parser_get_voice_reg_word_cnt();

/**
 * @brief 获取可注册命令词模板数量。
 * @return 命令词模板数量。
 */
int cfg_parser_get_voice_reg_cmd_cnt();

/**
 * @brief 在协议切换型多唤醒词模式下发送查询/探测协议。
 * @return 0 表示已发送；-1 表示当前不是协议切换模式。
 */
int cfg_parser_switch_wakeword_info_to_send_protocol();

/**
 * @brief 获取默认唤醒词索引。
 * @return 默认唤醒词下标；协议切换模式下可能返回协议默认占位值。
 */
int cfg_parser_get_default_keyword_id();

/**
 * @brief 获取 PA 配置。
 * @return PA 配置项。
 */
pa_config_item_t cfg_parser_get_pa_config();

/**
 * @brief 获取串口配置。
 * @return 串口配置项。
 */
uart_config_t cfg_parser_get_uart_config();

#endif