#ifndef __LISTEN_APP_UART_H__
#define __LISTEN_APP_UART_H__

#include "stdint.h"

#define LS_UART_MAG_MAX_LEN (32)

typedef struct ls_uart_msg
{
    uint8_t data[LS_UART_MAG_MAX_LEN];
    uint32_t len;
} ls_uart_msg_t;

typedef void (*ls_uart_receive_callback)(const uint8_t *data, uint32_t len);

/**
 * @brief       串口初始化
 * 
 */
void ls_app_uart_init();

/**
 * @brief       发送数据
 * 
 * @param data  发送的数据指针
 * @param len   发送的数据长度
 */
void ls_app_uart_send(const uint8_t *data, uint32_t len);

/**
 * @brief       设置串口接收回调
 * 
 * @param cb    callback
 */
void ls_app_uart_receive_cb(ls_uart_receive_callback cb);

#endif  // __LISTEN_APP_UART_H__