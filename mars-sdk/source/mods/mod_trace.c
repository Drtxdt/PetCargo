#include "appinc.h"
#include "ls_app_cfg_mgr.h"

#if CONFIG_TRACE_ENABLE
#if CONFIG_TRACE_UART0
#elif CONFIG_TRACE_UART1
#else
#error "invalid trace device"
#endif

typedef struct {
    mod_inst_t self;
    x_handle_t hdrv;
    UART_RegDef *hreg;
    uint32_t rxfifo;
} obj_priv_t;

static UART_RegDef *get_config_uart_base()
{
    return (UART_RegDef *)(cfg_parser_get_uart_config().trace_uart == 0 ? IP_UART0 : IP_UART1);
}

static x_handle_t get_config_uart_inst()
{
    return (cfg_parser_get_uart_config().trace_uart == 0 ? UART0() : UART1());
}

static uint32_t get_config_trace_baud()
{
    uint32_t baud = CONFIG_TRACE_BAUD;
    uint32_t tmp_baud = cfg_parser_get_uart_config().trace_baud;
    if ((2400 <= tmp_baud && tmp_baud <= 921600)) {
        baud = tmp_baud;
    }

    return baud;
}

static void uart_putc(void *user, char chr)
{
    UART_RegDef *const ureg = user;
    ureg->REG_RXTX_BUFFER.all = chr;
    while (!ureg->REG_STATUS.bit.TX_FIFO_SPACE);
    if (chr == '\n') while (ureg->REG_STATUS.bit.TX_FIFO_SPACE < 16) {};
}
static void uart_puts(void *user, const char *str)
{
    while (*str) uart_putc(user, *str++);
}

__attribute__((format(printf, 1, 2)))
int printk(const char *__restrict fmt, ...)
{
    UART_RegDef *const ureg = get_config_uart_base();
	va_list ap;
	va_start(ap, fmt);
	int len = vsnprintf(NULL, 0, fmt, ap);
    if (len > 0) {
        if (len > CONFIG_SHELL_LINE_LIMIT - 1) len = CONFIG_SHELL_LINE_LIMIT - 1;
        char *const buf = x_malloc(len + 1, 1);
        vsnprintf(buf, len + 1, fmt, ap);
        uart_puts(ureg, buf);
        x_free(buf);
    }
    va_end(ap);
	return len;
}

void uart_prev_init(void)
{
    UART_RegDef *const ureg = get_config_uart_base();
    ureg->REG_IRQ_MASK.all = 0;
    ureg->REG_CTRL.all = 0;
    ureg->REG_CTRL.bit.DIVISOR_MODE = 0;        // [0]DIV=SRCLK/4 [1]DIV=SRCLK/16
    ureg->REG_CMD_SET.bit.TX_FIFO_RESET = 1;    // reset tx fifo
    ureg->REG_CMD_SET.bit.RX_FIFO_RESET = 1;    // reset rx fifo
    ureg->REG_CTRL.bit.DATA_BITS = 1;           // 8bits
    ureg->REG_CTRL.bit.ENABLE = 1;              // enable uart
    ureg->REG_STATUS.all = 1;                   // clear line error bits
    IP_CMN_IOMUX->REG_PAD_GPIOA_00.bit.PAD_GPIOA_00_FSEL = 2;
}

static void __fast__ uart_drv_event(uint32_t event, void *user)
{
    obj_priv_t *const priv = user;
    if (event & (CSK_UART_EVENT_RECEIVE_COMPLETE)) {
        __mapi_notify(priv->self, 1, &priv->rxfifo);
        UART_Receive(priv->hdrv, &priv->rxfifo, 1);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
#if CONFIG_TRACE_SEND
static int mod_write(const void *src, int size, x_msec_t msec)
{
    static obj_priv_t *priv = NULL;
    if (priv || (priv = __MAPI_PRIVTE(CONFIG_TRACE_NAME))) {
        for (int i = 0; i < size; i++) uart_putc(priv->hreg, ((const char *)src)[i]);
        return size;
    }
    return -1;
}
#else
#define mod_write NULL
#endif//CONFIG_TRACE_SEND

#if CONFIG_TRACE_RECV
int mod_read(void *dst, int size, x_msec_t msec)
{
    static obj_priv_t *priv = NULL;
    if (priv || (priv = __MAPI_PRIVTE(CONFIG_TRACE_NAME)));
    UART_Receive_IT(priv->hdrv, dst, size);
    return 0;
}
#else
#define mod_read NULL
#endif//CONFIG_TRACE_RECV

static void mod_setup(mod_inst_t self, void *user)
{
    obj_priv_t *const priv = user;
    priv->self = self;
	priv->hdrv = get_config_uart_inst();
    priv->hreg = get_config_uart_base();

    UART_Initialize(priv->hdrv, uart_drv_event, priv);
    UART_PowerControl(priv->hdrv, CSK_POWER_FULL);
    UART_Control(priv->hdrv, CSK_UART_MODE_ASYNCHRONOUS | CSK_UART_Function_CONTROL_Int
        | CSK_UART_DATA_BITS_8 | CSK_UART_STOP_BITS_1 | CSK_UART_PARITY_NONE | CSK_UART_FLOW_CONTROL_NONE
        , get_config_trace_baud());

#if CONFIG_TRACE_SEND
    UART_Control(priv->hdrv, CSK_UART_CONTROL_TX, true);
#endif

#if CONFIG_TRACE_RECV
    UART_Control(priv->hdrv, CSK_UART_CONTROL_RX, true);
    UART_Receive(priv->hdrv, &priv->rxfifo, 1);
#endif
}

static obj_priv_t mod_priv = { NULL };
static mod_apis_t mod_apis = {
    .setup = mod_setup,
    .write = mod_write,
    .read = mod_read,
};
__MAPI_DECLARE(CONFIG_TRACE_NAME, 0, 3, &mod_apis, &mod_priv);
#endif//CONFIG_TRACE_ENABLE
