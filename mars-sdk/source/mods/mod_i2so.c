#include "appinc.h"

#if CONFIG_I2SO_ENABLE
#define I2SO_QUE_SIZE   (18)

typedef struct { uint32_t *addr, size; } i2so_item_t;
typedef struct { 
    mod_inst_t self;
    x_handle_t hdrv;
    aud_step_t *xbuf;
    uint32_t xpos;
    x_queue_t xque;
    uint32_t *zero;
} obj_priv_t;

static void i2so_drv_event(uint32_t event, uint32_t user)
{
    if (event & CSK_I2S_EVENT_TX_BLOCK_COMPLETE) {
        obj_priv_t *const priv = (void *)(uint32_t *)user;
        i2so_item_t item;
        uint8_t xcnt = 1;
        int ret = CSK_DRIVER_OK;
        bool qret = x_queue_recv(priv->xque, &item, 0);
        if (qret && item.addr && item.size > 1) {
            ret = I2S_Send_PiPo(priv->hdrv, &(PIPO_OUT_BLOCK){
                    .sample_data = item.addr, .sample_cnt = item.size & ~1, .flags = false
                }, &xcnt, CLSD_TX_FLAG_START_NOW);
        } else {
            ret = I2S_Send_PiPo(priv->hdrv, &(PIPO_OUT_BLOCK){
                    .sample_data = priv->zero, .sample_cnt = AUD_STEP_SAMPS, .flags = qret
                }, &xcnt, CLSD_TX_FLAG_START_NOW);
        }
        ASSERT(CSK_DRIVER_OK == ret, "send(%d)", ret);
    }
    if (event & CSK_I2S_EVENT_TRANSMIT_COMPLETE) TRACE("I2SO:COMPLETE");
    if (event & CSK_I2S_EVENT_TX_FIFO_UNDERRUN) TRACE("I2SO:UNDERUN");
    if (event & CSK_I2S_EVENT_TX_FIFO_EMPTY) TRACE("I2SO:EMPTY");
    ASSERT(!(event & CSK_I2S_EVENT_CLOCK_ERROR), "I2SO:CLKER");
    ASSERT(!(event & CSK_I2S_EVENT_OTHER_ERROR), "I2SO:ERROR");
}

////////////////////////////////////////////////////////////////////////////////////////////////////
static int mod_write(const void *src, int size, x_msec_t msec)
{
    obj_priv_t *const priv = __MAPI_PRIVTE(CONFIG_I2SO_NAME);
    void *addr = priv->xbuf[priv->xpos];
    memcpy(addr, src, size<<1);
    if (++priv->xpos >= I2SO_QUE_SIZE) priv->xpos = 0;
    return x_queue_send(priv->xque, &(i2so_item_t){.addr=(void *)addr,.size=size}, false, msec);
}

static void mod_setup(mod_inst_t self, void *user)
{
    obj_priv_t *const priv = user;
    priv->self = self;
    priv->hdrv = I2S0();
    priv->xque = x_queue_new("i2s", I2SO_QUE_SIZE-2, sizeof(i2so_item_t));
    ASSERT(priv->xque, "xque");
    priv->xpos = 0;
    priv->zero = x_malloc(sizeof(aud_step_t), sizeof(uint32_t));
    memset(priv->zero, 0, x_msize(priv->zero));

    priv->xbuf = (aud_step_t *)x_malloc(I2SO_QUE_SIZE * sizeof(aud_step_t), sizeof(void *));
    // priv->xbuf = (aud_step_t *)(0x20038000 - 37*320);

    // mono(left channel), 16bits
    I2S_DMA_CHS dmach = { .dma_ch_out_lr = DMA_CHANNEL_ANY };
    int ret = I2S_Initialize(priv->hdrv, i2so_drv_event, (uint32_t)priv, I2S_BMP_FLAG_OUT_LEFT, &dmach);
    ASSERT (CSK_DRIVER_OK == ret, "init(%d)", ret);

    ret = I2S_PowerControl(priv->hdrv, CSK_POWER_FULL);
    ASSERT (CSK_DRIVER_OK == ret, "power(%d)", ret);

    // 16k
    ret = I2S_Control(priv->hdrv, CSK_I2S_MODE_MASTER | CSK_I2S_PROTO_PCMMODE_A | CSK_I2S_TDM_CHS(2)
        | CSK_I2S_DATA_FORMAT_DUAL_16BIT | CSK_I2S_TXCH_STEREO_SRC_MONO, 16000);
    ASSERT (CSK_DRIVER_OK == ret, "ctrl(%d)", ret);

    // pre insert some silence
    PIPO_OUT_BLOCK pipo[] = {
        [0] = { .sample_data = priv->zero, .sample_cnt = AUD_STEP_SAMPS },
        [1] = { .sample_data = priv->zero, .sample_cnt = AUD_STEP_SAMPS },
    };
    uint8_t xcnt = __cntof(pipo);
    ret = I2S_Send_PiPo(priv->hdrv, pipo, &xcnt, I2S_TX_FLAG_START_NOW);
    ASSERT(CSK_DRIVER_OK == ret, "send(%d)", ret);
}

static obj_priv_t mod_priv = { NULL };
static mod_apis_t mod_apis = {
    .setup = mod_setup,
    .write = mod_write,
};
__MAPI_DECLARE(CONFIG_I2SO_NAME, 2, 0, &mod_apis, &mod_priv);
#endif//CONFIG_I2SO_ENABLE
