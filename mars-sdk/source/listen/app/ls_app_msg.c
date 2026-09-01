#include <string.h>
#include "appinc.h"
#include "IOMuxManager.h"
#include "Driver_GPIO.h"
#include "ls_app_msg.h"
#include "lisa_mem.h"
#include "lisa_log.h"
#include "evs_utils.h"
#include "ls_app_client.h"

#define MSG_TAG "app_msg"

typedef struct msg_send_item
{
    LS_APP_MSG_TYPE type;
    uint8_t *data;
    uint32_t len;
} msg_send_item_t;

static void* GPIOA_Handler = NULL;
static bool io_high = false;
static uint8_t g_tag_sw = 0; 

static void csk_gpio_pin_write()
{
	if (GPIOA_Handler == NULL) {
 		GPIOA_Handler = GPIOA();
		IOMuxManager_PinConfigure(CSK_IOMUX_PAD_A, 10, CSK_IOMUX_FUNC_DEFAULT);
		GPIO_SetDir(GPIOA_Handler, CSK_GPIO_PIN10, CSK_GPIO_DIR_OUTPUT);
		// GPIO_Control(GPIOA_Handler, CSK_GPIO_DEBOUNCE_DISABLE, CSK_GPIO_PIN10);
	}
	if (io_high) {
		io_high = false;
	} else {
		io_high = true;
	}
 	GPIO_PinWrite(GPIOA_Handler, CSK_GPIO_PIN10, io_high?1:0); //1输出高电平 0输出低电平
}

static int _handle_app_msg_send(void *arg)
{
    msg_send_item_t *send_item = (msg_send_item_t *)arg;
    switch (send_item->type)
    {
    case MSG_TYPE_WAKEUP:
        listen_client_wakeup(send_item->data, send_item->len);
        break;
    case MSG_TYPE_PRE_WAKEUP:
        listen_client_pre_wakeup(send_item->data, send_item->len);
        break;
    case MSG_TYPE_WAKEUP_TIMEOUT:
        listen_client_wakeup_timeout();
        break;
    default:
        LISA_LOGE(MSG_TAG, "unknow type=%d", send_item->type);
        break;
    }

    if (send_item->data) lisa_mem_free(send_item->data);
    lisa_mem_free(send_item);
    return 0;
}

void ls_app_msg_send(LS_APP_MSG_TYPE type, const uint8_t *data, uint32_t len)
{
    if (g_tag_sw && type == MSG_TYPE_WAKEUP) {
        csk_gpio_pin_write();
    }

    LISA_LOGD(MSG_TAG, "type=%d len=%ld", type, len);
    uint8_t *result = NULL;
    uint32_t result_len = 0;
    if(data && len != 0) {
        result = lisa_mem_calloc(1, len + 1);
        if(NULL == result) return;
        memcpy(result, data, len);
        result_len = len;
    }
    
    msg_send_item_t *send_item = lisa_mem_alloc(sizeof(msg_send_item_t));
    send_item->type = type;
    send_item->data = result;
    send_item->len = result_len;

    if (evs_handler_post_runnable(_handle_app_msg_send, send_item) != 0) {
        lisa_mem_free(result);
        lisa_mem_free(send_item);
    }
}

void ls_app_msg_send_type(LS_APP_MSG_TYPE type)
{
    ls_app_msg_send(type, NULL, 0);
}

void shell_user_set_tag_sw(uint8_t sw)
{
	SHELL_ITEM_EXPORT("user.settag", shell_user_set_tag_sw, "lisa tag switch (0/1)");
	g_tag_sw = sw;
    // csk_gpio_pin_write();
}