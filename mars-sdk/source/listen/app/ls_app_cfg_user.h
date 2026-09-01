/*
 * 由 json_to_c_header.py 自动生成
 * 源文件: config/cfg.json
 * 如需修改，请调整 JSON 后重新生成，不建议直接手改此文件。
 */
#ifndef __LISTENAI_CONFIG_USER_H__
#define __LISTENAI_CONFIG_USER_H__

#include "ls_app_cfg_def.h"
#include "../tone/tone.h"

/* PA 配置：功放开关、控制引脚和保持时间 */
const pa_config_item_t pa_config = {
    .enable = 0,                  /* 1: 使能PA, 0: 关闭 */
    .ctl_io_pad = CONTROL_IO_PAD_B,        /* 控制PAD，JSON: PB */
    .ctl_io_num = 11,          /* 控制引脚编号 */
    .enable_level = 1,                      /* 有效电平，JSON: high */
    .hold_time = 20000              /* 保持时间，单位us */
};

/* 串口配置：日志串口与协议串口参数 */
const uart_config_t uart_config = {
    .trace_uart = 0,         /* 日志串口号 */
    .uport_uart = 1,         /* 协议串口号 */
    .trace_baud = 115200,    /* 日志串口波特率 */
    .uport_baud = 9600       /* 协议串口波特率 */
};

/* 通用配置：版本、日志、持久化与麦克风增益 */
const general_item_t general_config = {
    .version = "V-2026.09.01_23.",          /* 配置版本字符串 */
    .log_level = 1,            /* 日志等级 */
    .persisted_volume = 0,   /* 音量是否持久化 */
    .persisted_voice = 1,     /* 语音开关是否持久化 */
    .persisted_wakeup = 1,   /* 唤醒词是否持久化 */
    .reply_switch = REPLY_SWITCH_ON,        /* 回复开关，JSON: 1 */
    .adc_again = 36,               /* 模拟增益 */
    .adc_dgain = 6                /* 数字增益 */
};

/* 欢迎语配置（空）：JSON 未配置 welcome */
const welcome_item_t welcome = {};

/* 超时配置（空）：JSON 未配置 timeout */
const timeout_item_t timeout = {};

/* 唤醒词配置数组：每项包含关键词、发送协议和播报音 */
const wakeup_item_t wakeup_items[] = {
    {
        .keyword_size = 16,                  /* 关键词长度 */
        .send_pro_len = 0,            /* 发送协议字节数 */
        .audio_size = 1,              /* 播报音频数量 */
        .play_type = 0,                /* 播报方式 */
        .keywords = "xiao ai tong xue",                 /* 对应 JSON keywords */
        .send_pro_buf = {}, /* 对应 JSON pro_buffer */
        .play_audio = {TONE_ID_0} /* 对应 JSON play_id */
    },
};

/* 命令词配置数组：每项包含关键词、收发协议和播报音 */
const command_item_t command_items[] = {
    {
        .keyword_size = 12,                    /* 关键词长度 */
        .send_pro_len = 4,              /* 发送协议字节数 */
        .recv_pro_len = 0,              /* 接收协议字节数 */
        .audio_size = 1,                /* 播报音频数量 */
        .play_type = 0,                  /* 播报方式 */
        .keywords = "ting xia lai",                   /* 对应 JSON keywords */
        .send_pro_buf = {0xA5, 0x5A, 0x01, 0xFE}, /* 对应 JSON pro_buffer */
        .recv_pro_buf = {}, /* 对应 JSON recv_pro_buffer */
        .play_audio = {TONE_ID_1}   /* 对应 JSON play_id */
    },
    {
        .keyword_size = 13,                    /* 关键词长度 */
        .send_pro_len = 4,              /* 发送协议字节数 */
        .recv_pro_len = 0,              /* 接收协议字节数 */
        .audio_size = 1,                /* 播报音频数量 */
        .play_type = 0,                  /* 播报方式 */
        .keywords = "zhuan ge quan",                   /* 对应 JSON keywords */
        .send_pro_buf = {0xA5, 0x5A, 0x06, 0xF9}, /* 对应 JSON pro_buffer */
        .recv_pro_buf = {}, /* 对应 JSON recv_pro_buffer */
        .play_audio = {TONE_ID_2}   /* 对应 JSON play_id */
    },
    {
        .keyword_size = 9,                    /* 关键词长度 */
        .send_pro_len = 4,              /* 发送协议字节数 */
        .recv_pro_len = 0,              /* 接收协议字节数 */
        .audio_size = 1,                /* 播报音频数量 */
        .play_type = 0,                  /* 播报方式 */
        .keywords = "shui jiao",                   /* 对应 JSON keywords */
        .send_pro_buf = {0xA5, 0x5A, 0x07, 0xF8}, /* 对应 JSON pro_buffer */
        .recv_pro_buf = {}, /* 对应 JSON recv_pro_buffer */
        .play_audio = {TONE_ID_3}   /* 对应 JSON play_id */
    },
    {
        .keyword_size = 14,                    /* 关键词长度 */
        .send_pro_len = 4,              /* 发送协议字节数 */
        .recv_pro_len = 0,              /* 接收协议字节数 */
        .audio_size = 1,                /* 播报音频数量 */
        .play_type = 0,                  /* 播报方式 */
        .keywords = "xiang qian zou",                   /* 对应 JSON keywords */
        .send_pro_buf = {0xA5, 0x5A, 0x02, 0xFD}, /* 对应 JSON pro_buffer */
        .recv_pro_buf = {}, /* 对应 JSON recv_pro_buffer */
        .play_audio = {TONE_ID_4}   /* 对应 JSON play_id */
    },
    {
        .keyword_size = 13,                    /* 关键词长度 */
        .send_pro_len = 4,              /* 发送协议字节数 */
        .recv_pro_len = 0,              /* 接收协议字节数 */
        .audio_size = 1,                /* 播报音频数量 */
        .play_type = 0,                  /* 播报方式 */
        .keywords = "xiang hou zou",                   /* 对应 JSON keywords */
        .send_pro_buf = {0xA5, 0x5A, 0x03, 0xFC}, /* 对应 JSON pro_buffer */
        .recv_pro_buf = {}, /* 对应 JSON recv_pro_buffer */
        .play_audio = {TONE_ID_5}   /* 对应 JSON play_id */
    },
    {
        .keyword_size = 13,                    /* 关键词长度 */
        .send_pro_len = 4,              /* 发送协议字节数 */
        .recv_pro_len = 0,              /* 接收协议字节数 */
        .audio_size = 1,                /* 播报音频数量 */
        .play_type = 0,                  /* 播报方式 */
        .keywords = "xiang zuo zou",                   /* 对应 JSON keywords */
        .send_pro_buf = {0xA5, 0x5A, 0x04, 0xFB}, /* 对应 JSON pro_buffer */
        .recv_pro_buf = {}, /* 对应 JSON recv_pro_buffer */
        .play_audio = {TONE_ID_6}   /* 对应 JSON play_id */
    },
    {
        .keyword_size = 13,                    /* 关键词长度 */
        .send_pro_len = 4,              /* 发送协议字节数 */
        .recv_pro_len = 0,              /* 接收协议字节数 */
        .audio_size = 1,                /* 播报音频数量 */
        .play_type = 0,                  /* 播报方式 */
        .keywords = "xiang you zou",                   /* 对应 JSON keywords */
        .send_pro_buf = {0xA5, 0x5A, 0x05, 0xFA}, /* 对应 JSON pro_buffer */
        .recv_pro_buf = {}, /* 对应 JSON recv_pro_buffer */
        .play_audio = {TONE_ID_7}   /* 对应 JSON play_id */
    },
    {
        .keyword_size = 9,                    /* 关键词长度 */
        .send_pro_len = 4,              /* 发送协议字节数 */
        .recv_pro_len = 0,              /* 接收协议字节数 */
        .audio_size = 1,                /* 播报音频数量 */
        .play_type = 0,                  /* 播报方式 */
        .keywords = "qi chuang",                   /* 对应 JSON keywords */
        .send_pro_buf = {0xA5, 0x5A, 0x08, 0xF7}, /* 对应 JSON pro_buffer */
        .recv_pro_buf = {}, /* 对应 JSON recv_pro_buffer */
        .play_audio = {TONE_ID_8}   /* 对应 JSON play_id */
    },
    {
        .keyword_size = 11,                    /* 关键词长度 */
        .send_pro_len = 4,              /* 发送协议字节数 */
        .recv_pro_len = 0,              /* 接收协议字节数 */
        .audio_size = 1,                /* 播报音频数量 */
        .play_type = 0,                  /* 播报方式 */
        .keywords = "mai ge meng",                   /* 对应 JSON keywords */
        .send_pro_buf = {0xA5, 0x5A, 0x09, 0xF6}, /* 对应 JSON pro_buffer */
        .recv_pro_buf = {}, /* 对应 JSON recv_pro_buffer */
        .play_audio = {TONE_ID_9}   /* 对应 JSON play_id */
    },
    {
        .keyword_size = 13,                    /* 关键词长度 */
        .send_pro_len = 4,              /* 发送协议字节数 */
        .recv_pro_len = 0,              /* 接收协议字节数 */
        .audio_size = 1,                /* 播报音频数量 */
        .play_type = 0,                  /* 播报方式 */
        .keywords = "chang shou ge",                   /* 对应 JSON keywords */
        .send_pro_buf = {0xA5, 0x5A, 0x0A, 0xF5}, /* 对应 JSON pro_buffer */
        .recv_pro_buf = {}, /* 对应 JSON recv_pro_buffer */
        .play_audio = {TONE_ID_10}   /* 对应 JSON play_id */
    },
    {
        .keyword_size = 13,                    /* 关键词长度 */
        .send_pro_len = 4,              /* 发送协议字节数 */
        .recv_pro_len = 0,              /* 接收协议字节数 */
        .audio_size = 1,                /* 播报音频数量 */
        .play_type = 0,                  /* 播报方式 */
        .keywords = "ni kai xin ma",                   /* 对应 JSON keywords */
        .send_pro_buf = {0xA5, 0x5A, 0x0B, 0xF4}, /* 对应 JSON pro_buffer */
        .recv_pro_buf = {}, /* 对应 JSON recv_pro_buffer */
        .play_audio = {TONE_ID_11}   /* 对应 JSON play_id */
    },
    {
        .keyword_size = 18,                    /* 关键词长度 */
        .send_pro_len = 4,              /* 发送协议字节数 */
        .recv_pro_len = 0,              /* 接收协议字节数 */
        .audio_size = 1,                /* 播报音频数量 */
        .play_type = 0,                  /* 播报方式 */
        .keywords = "bao gao zhuang tai",                   /* 对应 JSON keywords */
        .send_pro_buf = {0xA5, 0x5A, 0x0C, 0xF3}, /* 对应 JSON pro_buffer */
        .recv_pro_buf = {}, /* 对应 JSON recv_pro_buffer */
        .play_audio = {TONE_ID_12}   /* 对应 JSON play_id */
    },
    {
        .keyword_size = 0,                    /* 关键词长度 */
        .send_pro_len = 0,              /* 发送协议字节数 */
        .recv_pro_len = 4,              /* 接收协议字节数 */
        .audio_size = 1,                /* 播报音频数量 */
        .play_type = 1,                  /* 播报方式 */
        .keywords = "",                   /* 对应 JSON keywords */
        .send_pro_buf = {}, /* 对应 JSON pro_buffer */
        .recv_pro_buf = {0x5A, 0xA5, 0x80, 0x7F}, /* 对应 JSON recv_pro_buffer */
        .play_audio = {TONE_ID_16}   /* 对应 JSON play_id */
    },
};

/* 音量配置：音量表、默认值及音量控制命令 */
const volume_item_t volume = {
    .level = {10, 37, 58, 79, 100},            /* 音量等级表 */
    .level_size = 5,                    /* 音量等级数量 */
    .default_vol = 4,             /* 默认音量等级 */
    .adjmax_play_audio = {TONE_ID_17},  /* 最大音量提示音 */
    .adjmax_play_audio_size = 1,            /* 最大音量提示音数量 */
    .adjmin_play_audio = {TONE_ID_18},  /* 最小音量提示音 */
    .adjmin_play_audio_size = 1,            /* 最小音量提示音数量 */
    .vol_cmd_items = {
        {
            .keyword_size = 13,                  /* 关键词长度 */
            .send_pro_len = 0,            /* 发送协议字节数 */
            .recv_pro_len = 0,            /* 接收协议字节数 */
            .audio_size = 1,              /* 播报音频数量 */
            .ctrl_type = CTRL_TURN_UP_VOLUME,          /* 控制类型，JSON: 3 */
            .play_type = 0,                /* 播报方式 */
            .keywords = "da dian sheng",                 /* 对应 JSON keywords */
            .send_pro_buf = {}, /* 对应 JSON pro_buffer */
            .recv_pro_buf = {}, /* 对应 JSON recv_pro_buffer */
            .play_audio = {TONE_ID_14} /* 对应 JSON play_id */
        },
        {
            .keyword_size = 15,                  /* 关键词长度 */
            .send_pro_len = 0,            /* 发送协议字节数 */
            .recv_pro_len = 0,            /* 接收协议字节数 */
            .audio_size = 1,              /* 播报音频数量 */
            .ctrl_type = CTRL_TURN_DOWN_VOLUME,          /* 控制类型，JSON: 4 */
            .play_type = 0,                /* 播报方式 */
            .keywords = "xiao dian sheng",                 /* 对应 JSON keywords */
            .send_pro_buf = {}, /* 对应 JSON pro_buffer */
            .recv_pro_buf = {}, /* 对应 JSON recv_pro_buffer */
            .play_audio = {TONE_ID_15} /* 对应 JSON play_id */
        },
    },
    .vol_cmd_item_size = 2                 /* 音量控制命令数量 */
};

/* 退出识别模式配置数组：识别态下的退出命令 */
const quit_asr_item_t quit_asr_items[] = {
    {
        .keyword_size = 15,                  /* 关键词长度 */
        .audio_size = 1,              /* 播报音频数量 */
        .send_pro_len = 0,            /* 发送协议字节数 */
        .recv_pro_len = 0,            /* 接收协议字节数 */
        .play_type = 0,                /* 播报方式 */
        .keywords = "tui chu shi bie",                 /* 对应 JSON keywords */
        .play_audio = {TONE_ID_13}, /* 对应 JSON play_id */
        .send_pro_buf = {}, /* 对应 JSON pro_buffer */
        .recv_pro_buf = {} /* 对应 JSON recv_pro_buffer */
    },
};

/* 恢复出厂设置配置数组（空）：JSON 未配置 reset */
const reset_item_t reset_items[] = {};

/* 语音功能配置数组（空）：JSON 未配置 voice_function */
const voice_item_t voice_items[] = {};

/* 心跳配置数组（空）：JSON 未配置 heartbeat */
const heartbeat_item_t heartbeat_items[] = {};

/* 多唤醒词切换配置：切换方式、查询/恢复命令与唤醒词列表 */
const multi_wakeup_item_t multi_wakeup = {
    .enable = 0,                           /* 1: 使能多唤醒词切换 */
    .sw_type = (0),      /* 切换类型，JSON: 0 */
    .sw_mode = (0),       /* 切换模式，JSON: 0 */
    .switch_info = {
        .keyword_size = 0,                    /* 关键词长度 */
        .send_pro_len = 0,                 /* 发送协议字节数 */
        .recv_pro_len = 0,                 /* 接收协议字节数 */
        .audio_size = 0,                   /* 播报音频数量 */
        .play_type = 0,                /* 播报方式 */
        .keywords = "",                /* 对应 JSON keywords */
        .send_pro_buf = {}, /* 对应 JSON pro_buffer */
        .recv_pro_buf = {}, /* 对应 JSON recv_pro_buffer */
        .play_audio = {}   /* 对应 JSON play_id */
    },
    .query_info = {
        .keyword_size = 0,                    /* 关键词长度 */
        .send_pro_len = 0,                 /* 发送协议字节数 */
        .recv_pro_len = 0,                 /* 接收协议字节数 */
        .audio_size = 0,                   /* 播报音频数量 */
        .play_type = 0,                /* 播报方式 */
        .keywords = "",                /* 对应 JSON keywords */
        .send_pro_buf = {}, /* 对应 JSON pro_buffer */
        .recv_pro_buf = {}, /* 对应 JSON recv_pro_buffer */
        .play_audio = {}   /* 对应 JSON play_id */
    },
    .restore_info = {
        .keyword_size = 0,                    /* 关键词长度 */
        .send_pro_len = 0,                 /* 发送协议字节数 */
        .recv_pro_len = 0,                 /* 接收协议字节数 */
        .audio_size = 0,                   /* 播报音频数量 */
        .play_type = 0,                /* 播报方式 */
        .keywords = "",                /* 对应 JSON keywords */
        .send_pro_buf = {}, /* 对应 JSON pro_buffer */
        .recv_pro_buf = {}, /* 对应 JSON recv_pro_buffer */
        .play_audio = {}   /* 对应 JSON play_id */
    },
    .switch_list_size = 0,            /* 可切换唤醒词数量 */
    .switch_list = {
    }
};

/* 语音注册配置：学习模式、控制命令与可注册模板 */
const study_item_t study_config = {
    .enable = 0,                           /* 1: 使能语音注册 */
    .type = 0,                          /* 学习类型，JSON:  */
    .mode = 0,                          /* 学习模式，JSON:  */
    .user_cfg = {
        .wake_sensitivity = 1,           /* 唤醒词灵敏度，JSON: 中 */
        .wake_min_words = 0,      /* 唤醒词最少字数 */
        .wake_max_words = 0,      /* 唤醒词最多字数 */
        .wake_reg_max = 0,       /* 最大可注册唤醒词数量 */
        .wake_repeat_count = 0,  /* 唤醒词重复次数 */
        .wake_retry_count = 0,   /* 唤醒词重试次数 */
        .cmd_sensitivity = 1,            /* 命令词灵敏度，JSON: 中 */
        .cmd_min_words = 0,        /* 命令词最少字数 */
        .cmd_max_words = 0,        /* 命令词最多字数 */
        .cmd_reg_max = 0,          /* 最大可注册命令词数量 */
        .cmd_repeat_count = 0,      /* 命令词重复次数 */
        .cmd_retry_count = 0        /* 命令词重试次数 */
    },
    .control_cmd_item_size = 0,        /* 控制命令数量 */
    .other_replys_item_size = 0,       /* 提示播报数量 */
    .reg_wakewords_item_size = 0,          /* 可注册唤醒词模板数量 */
    .reg_commands_item_size = 0,            /* 可注册命令词模板数量 */
    .control_cmd_items = {
    },
    .other_replys_items = {
    },
    .reg_wakewords_items = {
    },
    .reg_commands_items = {
    }
};

#endif /* __LISTENAI_CONFIG_USER_H__ */