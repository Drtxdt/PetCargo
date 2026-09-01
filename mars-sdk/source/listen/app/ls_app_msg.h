#ifndef __LS_APPLICATION_MSG_H__
#define __LS_APPLICATION_MSG_H__

#include <stdint.h>
#include "lisa_typedef.h"

typedef enum LS_APP_MSG_TYPE {
	MSG_TYPE_WAKEUP = 0,        // 唤醒事件
	MSG_TYPE_PRE_WAKEUP,        // 预唤醒事件
	MSG_TYPE_WAKEUP_TIMEOUT,	// 超时事件
	MSG_TYPE_COUNT_MAX,
} LS_APP_MSG_TYPE;

/**
 * @brief 		发送带数据的msg消息
 * 
 * @param type 	msg类型
 * @param data 	数据指针  data需由外面malloc，但是不能free，事件结束后会自动free
 * @param len 	数据长度
 * 
 */
void ls_app_msg_send(LS_APP_MSG_TYPE type, const uint8_t *data, uint32_t len);

/**
 * @brief 		发送不带数据的msg消息
 * 
 * @param type 	msg类型
 */
void ls_app_msg_send_type(LS_APP_MSG_TYPE type);

#endif // __LS_APPLICATION_MSG_H__