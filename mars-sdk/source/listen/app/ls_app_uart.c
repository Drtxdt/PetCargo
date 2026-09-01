#include "ls_app_uart.h"
#include "lisa_log.h"
#include "ls_app_cfg_mgr.h"
#include "appinc.h"

#define LS_UART2_TAG "uart"
#define LS_UART_RECORD_BAUD             (921600U)
#define LS_UART_RECORD_FRAME_HEAD0      (0xFA)
#define LS_UART_RECORD_FRAME_HEAD1      (0xFB)
#define LS_UART_RECORD_FRAME_TAIL0      (0x5A)
#define LS_UART_RECORD_FRAME_TAIL1      (0x5B)
#define LS_UART_RECORD_PAYLOAD_MAX_LEN  (sizeof(aud_step_t))

typedef enum {
    LS_UART_RECORD_STATE_IDLE = 0,
    LS_UART_RECORD_STATE_ACTIVE,
    LS_UART_RECORD_STATE_RESTORING,
} ls_uart_record_state_t;

static ls_uart_receive_callback s_receive_cb = NULL;
static mod_inst_t s_uart_inst = NULL;
static volatile ls_uart_record_state_t s_uart_record_state = LS_UART_RECORD_STATE_IDLE;

static uint32_t ls_app_uart_get_normal_baud(void)
{
    uint32_t baud = CONFIG_UPROT_BAUD;
    uint32_t tmp_baud = cfg_parser_get_uart_config().uport_baud;
    if ((2400 <= tmp_baud) && (tmp_baud <= 921600)) {
        baud = tmp_baud;
    }
    return baud;
}

static int ls_app_uart_set_baud(uint32_t baud)
{
    return __mapi_ctrl(s_uart_inst, MAPI_UPROT_CTRL_SET_BAUD, &baud);
}

#if CONFIG_UPROT_ENABLE
static int ls_uart_on_recv(uint32_t size, void *data, void *user)
{
    if (s_uart_record_state != LS_UART_RECORD_STATE_IDLE) {
        return 0;
    }
	if (s_receive_cb) {
		s_receive_cb(data, size);
	}
    return 0;
}
#endif

#if CONFIG_UPROT_ENABLE
void ls_app_uart_record_send_frame(const uint8_t *data, uint16_t len)
{
    if (s_uart_record_state != LS_UART_RECORD_STATE_ACTIVE || !s_uart_inst || !data || len == 0) {
        return;
    }

    uint8_t frame[2 + 2 + LS_UART_RECORD_PAYLOAD_MAX_LEN + 1 + 2];
    uint32_t offset = 0;

    while (offset < len) {
        uint8_t checksum = 0;
        uint16_t payload_len = (uint16_t)((len - offset) > LS_UART_RECORD_PAYLOAD_MAX_LEN
            ? LS_UART_RECORD_PAYLOAD_MAX_LEN : (len - offset));

        frame[0] = LS_UART_RECORD_FRAME_HEAD0;
        frame[1] = LS_UART_RECORD_FRAME_HEAD1;
        frame[2] = (uint8_t)(payload_len & 0xFF);
        frame[3] = (uint8_t)((payload_len >> 8) & 0xFF);
        memcpy(&frame[4], data + offset, payload_len);
        for (uint32_t i = 0; i < payload_len; i++) {
            checksum += frame[4 + i];
        }
        frame[4 + payload_len] = checksum;
        frame[4 + payload_len + 1] = LS_UART_RECORD_FRAME_TAIL0;
        frame[4 + payload_len + 2] = LS_UART_RECORD_FRAME_TAIL1;

        __mapi_write(s_uart_inst, frame, 2 + 2 + payload_len + 1 + 2, XINFINITE);
        offset += payload_len;
    }
}

static int ls_app_uart_record_ctrl_internal(int enable)
{
    uint32_t target_baud = enable ? LS_UART_RECORD_BAUD : ls_app_uart_get_normal_baud();
    ls_uart_record_state_t prev_state = s_uart_record_state;

    if (enable) {
        if (prev_state != LS_UART_RECORD_STATE_IDLE) {
            LISA_LOGW(LS_UART2_TAG, "uart record is already active");
            return -1;
        }

        ls_app_uart_init();
        if (!s_uart_inst) {
            return -1;
        }
    } else if (prev_state == LS_UART_RECORD_STATE_IDLE) {
        LISA_LOGW(LS_UART2_TAG, "uart record is not active");
        return -1;
    } else {
        s_uart_record_state = LS_UART_RECORD_STATE_RESTORING;
    }

    if (ls_app_uart_set_baud(target_baud) != 0) {
        s_uart_record_state = prev_state;
        if (enable) {
            LISA_LOGE(LS_UART2_TAG, "set record baud failed");
        } else {
            LISA_LOGE(LS_UART2_TAG, "restore uart baud failed");
        }
        return -1;
    }

    s_uart_record_state = enable ? LS_UART_RECORD_STATE_ACTIVE : LS_UART_RECORD_STATE_IDLE;
    if (enable) {
        LISA_LOGI(LS_UART2_TAG, "uart record start, baud=%ld", target_baud);
    } else {
        LISA_LOGI(LS_UART2_TAG, "uart record stop");
    }
    return 0;
}

void shell_uart_record(int enable)
{
    SHELL_ITEM_EXPORT("uart.record", shell_uart_record, "uart.record <0|1>");
    ls_app_uart_record_ctrl_internal(enable);
}
#else
void ls_app_uart_record_send_frame(const uint8_t *data, uint16_t len)
{
    (void)data;
    (void)len;
}

void shell_uart_record(int enable)
{
    SHELL_ITEM_EXPORT("uart.record", shell_uart_record, "uart.record <0|1>");
    (void)enable;
    LISA_LOGE(LS_UART2_TAG, "uart record is unsupported");
}
#endif

void ls_app_uart_init()
{
	if (s_uart_inst != NULL) {
		return;
	}
	LISA_LOGD(LS_UART2_TAG, "ls_app_uart_init enter");
#if CONFIG_UPROT_ENABLE
	s_uart_inst = __MAPI_FIND(CONFIG_UPROT_NAME);
    __mapi_hook(s_uart_inst, ls_uart_on_recv, NULL);
#endif
	LISA_LOGD(LS_UART2_TAG, "ls_app_uart_init end");
}

void ls_app_uart_send(const uint8_t *data, uint32_t len)
{
    if (s_uart_record_state != LS_UART_RECORD_STATE_IDLE) {
        LISA_LOGW(LS_UART2_TAG, "uart record active, drop protocol tx len=%ld", len);
        return;
    }
	LISA_LOGB(LS_UART2_TAG, data, len, "[TX] [%d]: -> ", len);
	if (s_uart_inst) {
		__mapi_write(s_uart_inst, data, len, 1000);
	}
}

void ls_app_uart_receive_cb(ls_uart_receive_callback cb)
{
	s_receive_cb = cb;
}
