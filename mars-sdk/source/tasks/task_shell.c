#include "appinc.h"

#if CONFIG_MODULE_SHELL
#define SEND_BUF_LEN	    (CONFIG_SHELL_TXBUF_SIZE)
#define RECV_BUF_LEN	    (CONFIG_SHELL_RXBUF_SIZE)
#define EVT_RX_RDY          XBIT(0)
#define EVT_TX_RDY          XBIT(1)

typedef struct {
    mod_inst_t self, mdrv;
	x_stmbuf_t rsb, ssb;
	x_mutex_t lock;
    x_event_t event;
    void *shell;
    char xbuf[SEND_BUF_LEN];
} shell_priv_t;

void *shell_malloc(int size)
{
    return pvPortMalloc(size);
}
void shell_free(void *ptr)
{
    return vPortFree(ptr);
}

static shell_priv_t *shell_priv = NULL;
static int shell_uart_writes(const void *data, int size, void *user);

int shell_task_printf(const char *text)
{
	const int size = strlen(text);
	if (!shell_priv || !shell_priv->shell) PRINTK("%s", text);
	else if (shell_priv->shell) shell_print_endl(shell_priv->shell, text, size);
	else shell_uart_writes(text, size, shell_priv);
	return size;
}

__attribute__((format(printf, 1, 2)))
int printf(const char *__restrict fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
    int len = vsnprintf(NULL, 0, fmt, ap);
    if (len > 0) {
        if (len > CONFIG_SHELL_LINE_LIMIT - 1) len = CONFIG_SHELL_LINE_LIMIT - 1;
        char *const buf = pvPortMalloc(len + 1);
        vsnprintf(buf, len + 1, fmt, ap);
        shell_task_printf(buf);
        vPortFree(buf);
    }
    va_end(ap);
	return len;
}

void shell_flush_cached(void)
{
    shell_priv_t *const priv = shell_priv;
    if (priv) {
        int xlen = 0;
        while ((xlen = x_stmbuf_recv(priv->ssb, priv->xbuf, sizeof(priv->xbuf), 0))) {
            priv->xbuf[xlen] = 0;
            PRINTK("%s" CRLF, priv->xbuf); 
        }
    }
}

static int shell_uart_writes(const void *data, int size, void *user)
{
	shell_priv_t *const priv = user;
	ASSERT(priv, "xshell");
	ASSERT(priv->ssb && data && size > 0, "data=%p,size=%d", data, size);

    x_mutex_lock(priv->lock, XINFINITE);
    int ret = x_stmbuf_send(priv->ssb, data, size, 100);
    x_mutex_unlock(priv->lock);

	return ret;
}

static int __fast__ shell_uart_cb(uint32_t uarg, void *parg, void *user)
{
    shell_priv_t *const priv = user;
    return x_stmbuf_send(priv->rsb, parg, uarg, 0);
}

static void shell_recv_task(void *param)
{
    shell_priv_t *const priv = param;
    priv->rsb = x_stmbuf_new("rsb", RECV_BUF_LEN, 1);
	ASSERT(priv->rsb, "rsb");
    x_event_sync(priv->event, EVT_RX_RDY, EVT_TX_RDY, XINFINITE);
    char recv_buf[RECV_BUF_LEN];
    __mapi_hook(priv->mdrv, shell_uart_cb, priv);
    while (true) {
        int xlen = x_stmbuf_recv(priv->rsb, recv_buf, sizeof(recv_buf), 30);
		for (int i = 0; i < xlen; i++) shell_recv_proc(priv->shell, recv_buf[i]);
    }

    x_stmbuf_del(priv->rsb);
    x_task_del(NULL);
}

static void shell_send_task(void *param)
{
    shell_priv_t *const priv = param;
	priv->ssb = x_stmbuf_new("ssb", SEND_BUF_LEN, 1);
	ASSERT(priv->ssb, "ssb");

    priv->mdrv = __MAPI_FIND(CONFIG_TRACE_NAME);
    ASSERT(priv->mdrv, "trace");

#if CONFIG_MODULE_SHELL
    shell_priv = priv;
	extern char _shell_items_start;
	priv->shell = shell_init(shell_uart_writes, &_shell_items_start, priv);
	ASSERT(priv->shell, "shell");
#endif
    LOGI("SDK: V%d.%d.%d, %s %s!", VER_MAJOR, VER_MINOR, VER_BUILD, __DATE__, __TIME__);

    int xlen = x_stmbuf_recv(priv->ssb, priv->xbuf, sizeof(priv->xbuf), 0);
    if (xlen > 0) __mapi_write(priv->mdrv, priv->xbuf, xlen, XINFINITE);
    x_event_sync(priv->event, EVT_TX_RDY, EVT_RX_RDY, XINFINITE);

    while (true) {
        int xlen = x_stmbuf_recv(priv->ssb, priv->xbuf, sizeof(priv->xbuf), 30);
		if (xlen > 0) __mapi_write(priv->mdrv, priv->xbuf, xlen, XINFINITE); 
    }

#if CONFIG_MODULE_SHELL
    shell_uninit();
#endif
    x_stmbuf_del(priv->ssb);
    x_task_del(NULL);
}

static void mod_setup(mod_inst_t self, void *user)
{
	shell_priv_t *const priv = x_malloc(sizeof(shell_priv_t), sizeof(uint32_t));
    memset(priv, 0, sizeof(shell_priv_t));

    priv->self = self;
    priv->event = x_event_new("shell.evts");
    ASSERT(priv->event, "shell.evts");
	priv->lock = x_mutex_new("shell.lock");
	ASSERT(priv->lock, "shell.lock");

    x_task_new("shtx", shell_send_task, OS_STACK_DEF * 2, OS_PRIO_DEF, priv);
    x_task_new("shrx", shell_recv_task, OS_STACK_DEF * 2, OS_PRIO_DEF, priv);
    x_event_wait(priv->event, EVT_TX_RDY | EVT_RX_RDY | XEVT_WAIT_ALL, XINFINITE);
}

static mod_apis_t mod_apis = {
    .setup = mod_setup
};
__MAPI_DECLARE(CONFIG_SHELL_NAME, 1, 0, &mod_apis, NULL);

#endif//CONFIG_MODULE_SHELL
