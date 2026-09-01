#include "appinc.h"
#include "ls_app_cfg_mgr.h"

#if CONFIG_UPROT_ENABLE
#if CONFIG_UPROT_UART0
#elif CONFIG_UPROT_UART1
#else
#error "invalid protocol device"
#endif
#define UPROT_EVT_TX_DONE   BIT(0)
#define UPROT_RXFIFO_BLKS   (CONFIG_UPRPT_RXFIFO_BLKCNT)
#define UPROT_RXFIFO_CNTS   (CONFIG_UPROT_RXFIFO_DEPTH / sizeof(uint32_t))

typedef struct {
    mod_inst_t self;
    x_handle_t hdrv;
    x_event_t xevt;
    UART_RegDef *hreg;
    uint32_t baud;
#if CONFIG_UPROT_RECV
    uint32_t rxpos, rxfifo[UPROT_RXFIFO_BLKS][UPROT_RXFIFO_CNTS];
#endif
} obj_priv_t;

static UART_RegDef *get_config_uart_base()
{
    return (UART_RegDef *)(cfg_parser_get_uart_config().uport_uart == 0 ? IP_UART0 : IP_UART1);
}

static x_handle_t get_config_uart_inst()
{
    return (cfg_parser_get_uart_config().uport_uart == 0 ? UART0() : UART1());
}

static uint32_t get_config_uport_baud()
{
    uint32_t baud = CONFIG_UPROT_BAUD;
    uint32_t tmp_baud = cfg_parser_get_uart_config().uport_baud;
    if ((2400 <= tmp_baud && tmp_baud <= 921600)) {
        baud = tmp_baud;
    }

    return baud;
}

static int uart_apply_config(obj_priv_t *priv, uint32_t baud, int restart_rx)
{
    int ret = UART_Control(priv->hdrv, CSK_UART_MODE_ASYNCHRONOUS_TIMEOUT
        | CSK_UART_Function_CONTROL_Int | CSK_UART_GPIO_CONTROL_DEFAULT | CSK_UART_DATA_BITS_8
        | CSK_UART_STOP_BITS_1 | CSK_UART_PARITY_NONE | CSK_UART_FLOW_CONTROL_NONE
        , baud);
    ASSERT(CSK_DRIVER_OK == ret, "ctrl(%d)", ret);

#if CONFIG_UPROT_SEND
    ret = UART_Control(priv->hdrv, CSK_UART_CONTROL_TX, true);
    ASSERT(CSK_DRIVER_OK == ret, "tx.ctrl(%d)", ret);
    x_event_set(priv->xevt, UPROT_EVT_TX_DONE);
#endif

#if CONFIG_UPROT_RECV
    ret = UART_Control(priv->hdrv, CSK_UART_CONTROL_RX, true);
    ASSERT(CSK_DRIVER_OK == ret, "rx.ctrl(%d)", ret);
    if (restart_rx) {
        ret = UART_Receive_IT(priv->hdrv, &priv->rxfifo[(priv->rxpos = 0)], sizeof(priv->rxfifo[0]));
        ASSERT(CSK_DRIVER_OK == ret, "recv(%d)", ret);
    }
#endif

    priv->baud = baud;
    return ret;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
static void __fast__ uart_drv_event(uint32_t event, void *user)
{
    obj_priv_t *const priv = user;
#if CONFIG_UPROT_RECV
    if (event & (CSK_UART_EVENT_RECEIVE_COMPLETE | CSK_UART_EVENT_RX_TIMEOUT)) {
        uint32_t xcnt = UART_GetRxCount(priv->hdrv);
        int ret = CSK_DRIVER_OK;
        if (event & CSK_UART_EVENT_RX_TIMEOUT) {
            ret = UART_Control(priv->hdrv, CSK_UART_ABORT_RECEIVE, 0);
            ASSERT(CSK_DRIVER_OK == ret, "rx.ctrl(%d)", ret);
        }
        void *const recv = &priv->rxfifo[priv->rxpos];
        if (++priv->rxpos == CONFIG_UPRPT_RXFIFO_BLKCNT) priv->rxpos = 0;
        ret = UART_Receive_IT(priv->hdrv, &priv->rxfifo[priv->rxpos], sizeof(priv->rxfifo[0]));
        ASSERT(CSK_DRIVER_OK == ret, "recv(%d)", ret);
        __mapi_notify(priv->self, xcnt, recv);
    }
#endif//CONFIG_UPROT_RECV
#if CONFIG_UPROT_SEND
    if (event & CSK_UART_EVENT_SEND_COMPLETE)
        x_event_set(priv->xevt, UPROT_EVT_TX_DONE);
#endif//CONFIG_UPROT_SEND
}

////////////////////////////////////////////////////////////////////////////////////////////////////
#if CONFIG_UPROT_SEND
static int mod_write(const void *src, int size, x_msec_t msec)
{
    static obj_priv_t *priv = NULL;
    if (priv || (priv = __MAPI_PRIVTE(CONFIG_UPROT_NAME)));
    ASSERT(priv, "priv");

    if (!x_event_get(priv->xevt, UPROT_EVT_TX_DONE)) return 0;
    x_event_clr(priv->xevt, UPROT_EVT_TX_DONE);

    int ret = UART_Send_IT(priv->hdrv, src, size);
    ASSERT(CSK_DRIVER_OK == ret, "send(%d)", ret);
    x_event_wait(priv->xevt, UPROT_EVT_TX_DONE, msec);

    return size;
}
#endif//CONFIG_UPROT_SEND

static int mod_ctrl(uint32_t uarg, void *parg)
{
    obj_priv_t *const priv = __MAPI_PRIVTE(CONFIG_UPROT_NAME);
    ASSERT(priv, "priv");

    switch (uarg) {
    case MAPI_UPROT_CTRL_SET_BAUD: {
        uint32_t const baud = parg ? *(uint32_t *)parg : 0;
        if (baud < 2400 || baud > 921600) {
            return -1;
        }
        if (priv->baud == baud) {
            return 0;
        }
#if CONFIG_UPROT_SEND
        if (!x_event_wait(priv->xevt, UPROT_EVT_TX_DONE, 1000)) {
            return -1;
        }
#endif
#if CONFIG_UPROT_RECV
        int ret = UART_Control(priv->hdrv, CSK_UART_ABORT_RECEIVE, 0);
        ASSERT(CSK_DRIVER_OK == ret, "rx.abort(%d)", ret);
#endif
        return uart_apply_config(priv, baud, 1);
    }
    default:
        return -1;
    }
}

static void mod_setup(mod_inst_t self, void *user)
{
    obj_priv_t *const priv = user;
    priv->self = self;
    priv->hdrv = get_config_uart_inst();
    priv->hreg = get_config_uart_base();
    priv->xevt = x_event_new("uprot");
    ASSERT(priv->xevt, "uprot");
    x_event_set(priv->xevt, UPROT_EVT_TX_DONE);

    int ret = UART_Initialize(priv->hdrv, uart_drv_event, priv);
    ASSERT(CSK_DRIVER_OK == ret, "init(%d)", ret);

    ret = UART_PowerControl(priv->hdrv, CSK_POWER_FULL);
    ASSERT(CSK_DRIVER_OK == ret, "pwr(%d)", ret);

    ret = uart_apply_config(priv, get_config_uport_baud(), 1);
    ASSERT(CSK_DRIVER_OK == ret, "uart_apply_config(%d)", ret);
}

static obj_priv_t mod_priv = { NULL };
static mod_apis_t mod_apis = {
    .setup = mod_setup,
#if CONFIG_UPROT_SEND
    .write = mod_write,
#endif
    .ctrl = mod_ctrl,
};
__MAPI_DECLARE(CONFIG_UPROT_NAME, 1, 3, &mod_apis, &mod_priv);
#endif//CONFIG_UPROT_ENABLE
