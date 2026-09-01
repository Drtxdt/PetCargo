#ifndef __LISTENAI_CONFIG_DEF_H__
#define __LISTENAI_CONFIG_DEF_H__
#include <stdint.h>

// 通用上限定义
#define VOLUME_MAX_LEVEL           	(10)    // 音量等级表最大容量
#define VOLUME_MIN_LEVEL           	(0)     // 音量等级下限值
#define VOLUME_DEFAULT_LEVEL        (4)     // 默认音量等级
#define ALGO_KEYWORD_MAX_LEN        (64)    // 关键词拼音最大长度
#define PROTOCOL_DATA_LEN_MAX       (32)    // 单条协议数据最大字节数
#define PLAY_AUDIO_COUNT_MAX        (10)    // 单项配置可挂载的最大音频数量
#define VERSION_STRING_LEN_MAX      (128)   // 配置版本字符串最大长度
#define VOLUME_CTRL_CMD_CNT_MAX     (10)    // 音量控制命令最大数量
#define WAKEWORD_SWITCH_LIST_MAX    (20)    // 多唤醒词切换列表最大数量
#define VOICEREG_CONTROL_CMD_COUNT_MAX  (20)    // 语音注册控制命令最大数量
#define VOICEREG_OTHER_REPLY_COUNT_MAX  (30)    // 语音注册提示音标签最大数量
#define VOICEREG_REGWORD_COUNT_MAX      (30)    // 语音注册词模板最大数量

// PA 功放控制相关定义
#define CONTROL_IO_PAD_MAX_PIN (12)     // 单个 PAD 支持的最大 IO 编号
typedef enum {
    CONTROL_IO_PAD_A = 0,   // PAD A
    CONTROL_IO_PAD_B        // PAD B
} ctl_io_pad_t;

// PA 功放使能控制配置
typedef struct pa_config_item
{
    uint8_t enable;         // 是否使能 PA 功能
    ctl_io_pad_t ctl_io_pad;// 控制 IO 所在 PAD
    uint8_t ctl_io_num;     // 控制 IO 编号
    uint8_t enable_level;   // PA 使能有效电平
    uint32_t hold_time;     // 使能保持时间，单位 ms
} pa_config_item_t;

// 串口配置项
typedef struct
{
	uint8_t trace_uart;     // 日志串口号：0 表示 uart0，1 表示 uart1
	uint8_t uport_uart;     // 通信串口号：0 表示 uart0，1 表示 uart1
	uint32_t trace_baud;    // 日志串口波特率
	uint32_t uport_baud;    // 通信串口波特率
} uart_config_t;

// 通用配置
typedef enum
{
    REPLY_SWITCH_OFF = 0,   // 关闭播报
    REPLY_SWITCH_ON = 1     // 开启播报
} reply_switch_t;

typedef struct general_item
{
    uint8_t version[VERSION_STRING_LEN_MAX];    // 配置版本字符串
    uint8_t log_level;                          // 日志等级，通常为 0~4
    uint8_t persisted_volume;                   // 音量是否需要持久化：1 需要，0 不需要
    uint8_t persisted_voice;                    // 语音开关是否需要持久化
    uint8_t persisted_wakeup;                   // 唤醒词是否需要持久化
    reply_switch_t reply_switch;                // 播报开关
	int16_t adc_again;							// ADC 模拟增益
    int16_t adc_dgain;							// ADC 数字增益
} general_item_t;

// 欢迎语配置
typedef struct welcome_item
{
    uint8_t send_pro_len;       // 发送协议长度
    uint8_t recv_pro_len;       // 接收协议长度
	uint8_t audio_size;         // 播放音频列表长度
    uint8_t play_type;          // 播报类型：0 主动，1 被动，2 强制
	int8_t send_pro_buf[PROTOCOL_DATA_LEN_MAX];    // 发送协议缓冲区
	int8_t recv_pro_buf[PROTOCOL_DATA_LEN_MAX];    // 接收协议缓冲区
    uint8_t play_audio[PLAY_AUDIO_COUNT_MAX];      // 欢迎语提示音列表
} welcome_item_t;

// 超时配置
typedef struct timeout_item
{
    uint8_t send_pro_len;   // 发送协议长度
    uint8_t recv_pro_len;   // 接收协议长度
	uint8_t audio_size;     // 播放音频列表长度
    uint8_t play_type;      // 播报类型：0 主动，1 被动，2 强制
    uint32_t time;          // 唤醒超时时间，单位秒
    uint8_t play_audio[PLAY_AUDIO_COUNT_MAX];      // 超时提示音列表
	int8_t send_pro_buf[PROTOCOL_DATA_LEN_MAX];    // 发送协议缓冲区
	int8_t recv_pro_buf[PROTOCOL_DATA_LEN_MAX];    // 接收协议缓冲区
} timeout_item_t;

// 唤醒词配置
typedef struct wakeup_item
{
    uint8_t keyword_size;   // 唤醒词拼音长度
    uint8_t send_pro_len;   // 发送协议长度
	uint8_t audio_size;     // 随机播放音频候选数量
    uint8_t play_type;      // 播报类型：0 主动，1 被动，2 强制
	int8_t keywords[ALGO_KEYWORD_MAX_LEN];         // 唤醒词拼音内容
    int8_t send_pro_buf[PROTOCOL_DATA_LEN_MAX];    // 发送协议缓冲区
    uint8_t play_audio[PLAY_AUDIO_COUNT_MAX];      // 唤醒提示音候选列表
} wakeup_item_t;

// 普通命令词配置
typedef struct command_item
{
    uint8_t keyword_size;   // 命令词拼音长度
    uint8_t send_pro_len;   // 发送协议长度
    uint8_t recv_pro_len;   // 接收协议长度
	uint8_t audio_size;     // 播放音频列表长度
    uint8_t play_type;      // 播报类型：0 主动，1 被动，2 强制
    int8_t keywords[ALGO_KEYWORD_MAX_LEN];         // 命令词拼音内容
    int8_t send_pro_buf[PROTOCOL_DATA_LEN_MAX];    // 发送协议缓冲区
    int8_t recv_pro_buf[PROTOCOL_DATA_LEN_MAX];    // 接收协议缓冲区
    uint8_t play_audio[PLAY_AUDIO_COUNT_MAX];      // 命令反馈提示音列表
} command_item_t;

// 音量配置
typedef enum {
    CTRL_MAXIMUM_VOLUME,   // 设置为最大音量
    CTRL_MINIMUM_VOLUME,   // 设置为最小音量
    CTRL_MEDIMUM_VOLUME,   // 设置为中等音量
    CTRL_TURN_UP_VOLUME,   // 增加一级音量
    CTRL_TURN_DOWN_VOLUME, // 减少一级音量
} vol_ctrl_t;

typedef struct volume_cmd_item
{
    uint8_t keyword_size;   // 音量命令词拼音长度
    uint8_t send_pro_len;   // 发送协议长度
    uint8_t recv_pro_len;   // 接收协议长度
	uint8_t audio_size;     // 播放音频列表长度
	vol_ctrl_t ctrl_type;   // 音量操作类型
    uint8_t play_type;      // 播报类型：0 主动，1 被动，2 强制
	int8_t keywords[ALGO_KEYWORD_MAX_LEN];         // 音量命令词拼音内容
    int8_t send_pro_buf[PROTOCOL_DATA_LEN_MAX];    // 发送协议缓冲区
    int8_t recv_pro_buf[PROTOCOL_DATA_LEN_MAX];    // 接收协议缓冲区
    uint8_t play_audio[PLAY_AUDIO_COUNT_MAX];      // 音量操作提示音列表
} volume_cmd_item_t;

typedef struct volume_item
{
    uint8_t level[VOLUME_MAX_LEVEL];            // 音量等级表，通常映射为 0~100
	uint8_t level_size;							// 实际音量等级数量，范围 1~10
    uint8_t default_vol;						// 上电默认音量等级
    uint8_t adjmax_play_audio[PLAY_AUDIO_COUNT_MAX];// 到达最大音量时的提示音列表
	uint8_t adjmax_play_audio_size;                 // 到达最大音量时的提示音数量
    uint8_t adjmin_play_audio[PLAY_AUDIO_COUNT_MAX];// 到达最小音量时的提示音列表
	uint8_t adjmin_play_audio_size;                 // 到达最小音量时的提示音数量
	volume_cmd_item_t vol_cmd_items[VOLUME_CTRL_CMD_CNT_MAX];// 音量控制命令列表
	uint8_t vol_cmd_item_size;						// 音量控制命令数量
} volume_item_t;

// 退出识别模式配置项
typedef struct quit_asr_item
{
    uint8_t keyword_size;   // 退出识别模式命令词拼音长度
	uint8_t audio_size;     // 播放音频列表长度
    uint8_t send_pro_len;   // 发送协议长度
    uint8_t recv_pro_len;   // 接收协议长度
    uint8_t play_type;      // 播报类型：0 主动，1 被动，2 强制
	int8_t keywords[ALGO_KEYWORD_MAX_LEN];     // 退出识别模式命令词拼音
    uint8_t play_audio[PLAY_AUDIO_COUNT_MAX];  // 退出识别模式提示音列表
    int8_t send_pro_buf[PROTOCOL_DATA_LEN_MAX];// 发送协议缓冲区
    int8_t recv_pro_buf[PROTOCOL_DATA_LEN_MAX];// 接收协议缓冲区
} quit_asr_item_t;

// 恢复出厂设置配置项
typedef struct reset_item
{
    uint8_t keyword_size;   // 恢复出厂命令词拼音长度
	uint8_t audio_size;     // 播放音频列表长度
    uint8_t send_pro_len;   // 发送协议长度
    uint8_t recv_pro_len;   // 接收协议长度
    uint8_t play_type;      // 播报类型：0 主动，1 被动，2 强制
	int8_t keywords[ALGO_KEYWORD_MAX_LEN];     // 恢复出厂命令词拼音
    uint8_t play_audio[PLAY_AUDIO_COUNT_MAX];  // 恢复出厂提示音列表
    int8_t send_pro_buf[PROTOCOL_DATA_LEN_MAX];// 发送协议缓冲区
    int8_t recv_pro_buf[PROTOCOL_DATA_LEN_MAX];// 接收协议缓冲区
} reset_item_t;

// 语音功能配置项
typedef enum
{
    VOICE_FUNCTION_OPEN,    // 打开语音功能
    VOICE_FUNCTION_CLOSE,   // 关闭语音功能
} voice_type_t;

typedef struct voice_item
{
    uint8_t keyword_size;   // 语音开关命令词拼音长度
	uint8_t audio_size;     // 播放音频列表长度
	voice_type_t type;      // 语音操作类型
    uint8_t send_pro_len;   // 发送协议长度
    uint8_t recv_pro_len;   // 接收协议长度
    uint8_t play_type;      // 播报类型：0 主动，1 被动，2 强制
	int8_t keywords[ALGO_KEYWORD_MAX_LEN];     // 语音开关命令词拼音
    uint8_t play_audio[PLAY_AUDIO_COUNT_MAX];  // 语音开关提示音列表
    int8_t send_pro_buf[PROTOCOL_DATA_LEN_MAX];// 发送协议缓冲区
    int8_t recv_pro_buf[PROTOCOL_DATA_LEN_MAX];// 接收协议缓冲区
} voice_item_t;

// 心跳配置项
typedef struct heartbeat_item
{
    uint8_t send_pro_len;   // 心跳发送协议长度
    uint8_t recv_pro_len;   // 心跳接收协议长度
    int8_t send_pro_buf[PROTOCOL_DATA_LEN_MAX];    // 心跳发送协议缓冲区
    int8_t recv_pro_buf[PROTOCOL_DATA_LEN_MAX];    // 心跳接收协议缓冲区
} heartbeat_item_t;

/**********************************************************/
/********************  唤醒词切换配置  *********************/
/**********************************************************/

// 唤醒词切换类型
typedef enum
{
    WAKEUP_SWITCH_TYPE_UNKNOW = 0,  // 未知类型
    WAKEUP_SWITCH_TYPE_VOICE,       // 语音切换
    WAKEUP_SWITCH_TYPE_PROTOCOL     // 协议切换
} wakeup_switch_type_t;

// 唤醒词切换模式
typedef enum
{
    WAKEUP_SWITCH_MODE_UNKNOW = 0,  // 未知模式
    WAKEUP_SWITCH_MODE_LOOP,    // 循环切换
    WAKEUP_SWITCH_MODE_ROLLCALL // 指定切换
} wakeup_switch_mode_t;

// 多唤醒词切换命令配置
typedef struct
{
    uint8_t keyword_size;   // 命令词拼音长度
    uint8_t send_pro_len;   // 发送协议长度
    uint8_t recv_pro_len;   // 接收协议长度
	uint8_t audio_size;     // 播放音频列表长度
    uint8_t play_type;      // 播报类型：0 主动，1 被动，2 强制
    int8_t keywords[ALGO_KEYWORD_MAX_LEN];         // 切换命令词拼音
    int8_t send_pro_buf[PROTOCOL_DATA_LEN_MAX];    // 发送协议缓冲区
    int8_t recv_pro_buf[PROTOCOL_DATA_LEN_MAX];    // 接收协议缓冲区
    uint8_t play_audio[PLAY_AUDIO_COUNT_MAX];      // 切换提示音列表
} wakeup_switch_info_t;

// 多唤醒词查询命令配置
typedef struct
{
    uint8_t keyword_size;   // 命令词拼音长度
    uint8_t send_pro_len;   // 发送协议长度
    uint8_t recv_pro_len;   // 接收协议长度
	uint8_t audio_size;     // 播放音频列表长度
    uint8_t play_type;      // 播报类型：0 主动，1 被动，2 强制
    int8_t keywords[ALGO_KEYWORD_MAX_LEN];         // 查询命令词拼音
    int8_t send_pro_buf[PROTOCOL_DATA_LEN_MAX];    // 发送协议缓冲区
    int8_t recv_pro_buf[PROTOCOL_DATA_LEN_MAX];    // 接收协议缓冲区
    uint8_t play_audio[PLAY_AUDIO_COUNT_MAX];      // 查询提示音列表
} wakeup_query_info_t;

// 多唤醒词恢复默认命令配置
typedef struct
{
    uint8_t keyword_size;   // 命令词拼音长度
    uint8_t send_pro_len;   // 发送协议长度
    uint8_t recv_pro_len;   // 接收协议长度
	uint8_t audio_size;     // 播放音频列表长度
    uint8_t play_type;      // 播报类型：0 主动，1 被动，2 强制
    int8_t keywords[ALGO_KEYWORD_MAX_LEN];     // 恢复命令词拼音
    int8_t send_pro_buf[PROTOCOL_DATA_LEN_MAX];// 发送协议缓冲区
    int8_t recv_pro_buf[PROTOCOL_DATA_LEN_MAX];// 接收协议缓冲区
    uint8_t play_audio[PLAY_AUDIO_COUNT_MAX];  // 恢复提示音列表
} wakeup_restore_info_t;

// 单个唤醒词切换项（包含默认词、常驻等属性）
typedef struct
{
    uint8_t default_keyword;    // 是否为默认唤醒词
    uint8_t keyword_size;       // 唤醒词拼音长度
    uint8_t send_pro_len;       // 发送协议长度
    uint8_t recv_pro_len;       // 接收协议长度
	uint8_t audio_size;         // 播放音频列表长度
    uint8_t play_type;          // 播报类型：0 主动，1 被动，2 强制
    uint8_t frozen;             // 是否常驻，不允许被删除
    int8_t keywords[ALGO_KEYWORD_MAX_LEN];     // 唤醒词拼音内容
    int8_t send_pro_buf[PROTOCOL_DATA_LEN_MAX];// 发送协议缓冲区
    int8_t recv_pro_buf[PROTOCOL_DATA_LEN_MAX];// 接收协议缓冲区
    uint8_t play_audio[PLAY_AUDIO_COUNT_MAX];  // 唤醒词提示音列表
} wakeup_switch_item_t;

// 多唤醒词切换总配置
typedef struct wakeup_switch_item
{
    uint8_t enable;                                 // 是否启用多唤醒词切换
    wakeup_switch_type_t sw_type;                   // 切换类型
    wakeup_switch_mode_t sw_mode;                   // 切换模式
    uint8_t switch_list_size;                       // 可切换唤醒词数量
    wakeup_switch_info_t switch_info;               // 切换命令配置
    wakeup_query_info_t query_info;                 // 查询命令配置
    wakeup_restore_info_t restore_info;             // 恢复默认命令配置
    wakeup_switch_item_t switch_list[WAKEWORD_SWITCH_LIST_MAX]; // 唤醒词切换列表
} multi_wakeup_item_t;

/**********************************************************/
/*********************  语音注册功能  **********************/
/**********************************************************/

// 语音注册意图类型
typedef enum {
    ACTION_TYPE_UNKNOW = -1,    // 未知动作
    ACTION_TYPE_STUDY_WAKE,     // 学习唤醒词
    ACTION_TYPE_STUDY_CMD,      // 学习命令词
    ACTION_TYPE_STUDY_NEXT,     // 学习下一个
    ACTION_TYPE_STUDY_RELEARN,  // 重新学习
    ACTION_TYPE_STUDY_QUIT,     // 退出学习
    ACTION_TYPE_DEL_WAKE,       // 删除唤醒词
    ACTION_TYPE_DEL_CMD,        // 删除命令词
    ACTION_TYPE_DEL_ALL_CMD,    // 删除所有命令词
    ACTION_TYPE_DEL_ALL,        // 删除所有词
    ACTION_TYPE_DEL_QUIT        // 退出删除
} action_type_t;

// 播报提示标签
typedef enum {
    REPLY_LABEL_TYPE_TIP_UNKNOW = -1,           // 未知提示标签
    REPLY_LABEL_TYPE_TIP_RECORD_SUCCESS,        // 录音成功
    REPLY_LABEL_TYPE_TIP_RECORD_FAILED,         // 录音失败
    REPLY_LABEL_TYPE_TIP_STUDY_SUCCESS,         // 学习成功
    REPLY_LABEL_TYPE_TIP_STUDY_COMPLETE,        // 学习完成
    REPLY_LABEL_TYPE_TIP_STUDY_FAILED,          // 学习失败
    REPLY_LABEL_TYPE_TIP_TEMPLATE_FULL,         // 模板数量已满
    REPLY_LABEL_TYPE_TIP_LENGTH_ERROR,          // 长度不合法
    REPLY_LABEL_TYPE_TIP_SPEED_ERROR,           // 语速异常
    REPLY_LABEL_TYPE_TIP_SIMILA_ERROR,          // 前后两次内容不一致
    REPLY_LABEL_TYPE_TIP_COMMAND_CONFLICT,      // 指令冲突
    REPLY_LABEL_TYPE_TIP_DELETE_FAILED,         // 删除失败
    REPLY_LABEL_TYPE_TIP_DELETE_WAKE_START,     // 开始删除唤醒词
    REPLY_LABEL_TYPE_TIP_DELETE_WAKE_SUCCESS,   // 删除唤醒词成功
    REPLY_LABEL_TYPE_TIP_DELETE_CMD_START,      // 开始删除命令词
    REPLY_LABEL_TYPE_TIP_DELETE_CMD_SUCCESS,    // 删除命令词成功
    REPLY_LABEL_TYPE_TIP_DELETE_ALL_START,      // 开始删除全部
    REPLY_LABEL_TYPE_TIP_DELETE_ALL_SUCCESS,    // 删除全部成功
    REPLY_LABEL_TYPE_TIP_ASR_OVV,               // 当前指令不支持学习或删除
    REPLY_LABEL_TYPE_TIP_DELETE_ALL_CMD_SUCCESS,// 全部命令词删除成功
    REPLY_LABEL_TYPE_TIP_ASR_START,             // 开始学习命令词
    REPLY_LABEL_TYPE_TIP_WAKEUP_START,          // 开始学习唤醒词
    REPLY_LABEL_TYPE_TIP_ASR_OVER_TAIL,         // 命令词后缀超限
    REPLY_LABEL_TYPE_TIP_ASR_OVER_HEAD,         // 命令词前缀超限
} reply_label_t;

// 语音注册用户配置项
typedef struct user_cfg_item
{
    uint8_t wake_sensitivity;   // 注册唤醒词灵敏度：0 低，1 中，2 高
    uint8_t wake_min_words;     // 注册唤醒词最小长度
    uint8_t wake_max_words;     // 注册唤醒词最大长度
    uint8_t wake_reg_max;       // 最大可注册唤醒词数量
    uint8_t wake_repeat_count;  // 注册唤醒词重复次数
    uint8_t wake_retry_count;   // 注册唤醒词错误重试次数
    uint8_t cmd_sensitivity;    // 注册命令词灵敏度：0 低，1 中，2 高
    uint8_t cmd_min_words;      // 注册命令词最小长度
    uint8_t cmd_max_words;      // 注册命令词最大长度
    uint8_t cmd_reg_max;        // 最大可注册命令词数量
    uint8_t cmd_repeat_count;   // 注册命令词重复次数
    uint8_t cmd_retry_count;    // 注册命令词错误重试次数
} user_cfg_item_t;

// 其他播报提示音项
typedef struct other_replys_item
{
    reply_label_t label_type;   // 播报提示标签
    uint8_t audio_size;         // 音频列表长度
    uint8_t play_audio[PLAY_AUDIO_COUNT_MAX];  // 提示音列表
} other_replys_item_t;

// 语音注册动作项
typedef struct control_cmd_item
{
	action_type_t action_type;  // 动作类型
    uint8_t keyword_size;       // 命令词拼音长度
	uint8_t audio_size;         // 音频列表长度
    int8_t keywords[ALGO_KEYWORD_MAX_LEN];     // 命令词拼音内容
    uint8_t play_audio[PLAY_AUDIO_COUNT_MAX];  // 提示音列表
} control_cmd_item_t;

// 语音注册词项
typedef struct reg_word_item
{
    uint8_t keyword_size;   // 词条拼音长度
	uint8_t audio_size;     // 注册成功提示音数量
	uint8_t del_audio_size; // 删除成功提示音数量
    int8_t keywords[ALGO_KEYWORD_MAX_LEN];     // 词条拼音内容
    uint8_t play_audio[PLAY_AUDIO_COUNT_MAX];  // 注册成功提示音列表
    uint32_t del_play_audio[PLAY_AUDIO_COUNT_MAX];  // 删除成功提示音列表
} reg_word_item_t;

// 语音注册总配置
typedef struct study_item
{
    uint8_t enable;                 // 是否使能语音注册功能
    uint8_t type;                   // 1 连续学习 2 指定学习
    uint8_t mode;                   // 模式字段，当前预留
    user_cfg_item_t user_cfg;       // 用户配置项
	uint8_t control_cmd_item_size;  // 语音注册动作数量
    uint8_t other_replys_item_size; // 其他播报提示音数量
    uint8_t reg_wakewords_item_size;// 注册唤醒词模板数量
    uint8_t reg_commands_item_size; // 注册命令词模板数量
	control_cmd_item_t control_cmd_items[VOICEREG_CONTROL_CMD_COUNT_MAX]; // 语音注册动作列表
	other_replys_item_t other_replys_items[VOICEREG_OTHER_REPLY_COUNT_MAX]; // 其他播报提示音列表
    reg_word_item_t reg_wakewords_items[VOICEREG_REGWORD_COUNT_MAX]; // 注册唤醒词列表
    reg_word_item_t reg_commands_items[VOICEREG_REGWORD_COUNT_MAX];  // 注册命令词列表
} study_item_t;

#endif