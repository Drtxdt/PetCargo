#ifndef __LS_APPLICATION_CLIENT_H__
#define __LS_APPLICATION_CLIENT_H__

#include <stdint.h>

/**
 * @brief   app client创建
 *
 */
void listen_client_create(char *share_mem, char *voice_mem);

/**
 * @brief       本地唤醒 + 识别
 *
 * @param data  识别JSON消息
 * @param len   消息长度
 */
void listen_client_wakeup(const uint8_t *data, uint32_t len);

/**
 * @brief 	                预唤醒事件通知
 *
 * @param pre_wakeup_info   唤醒 JSON 信息
 */
void listen_client_pre_wakeup(const uint8_t *pre_wakeup_info, uint32_t len);

/**
 * @brief 	识别超时通知
 *
 */
void listen_client_wakeup_timeout();

void clear_voice_reg_flag(int reset_waked);

#endif