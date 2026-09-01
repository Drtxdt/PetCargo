#include "appinc.h"

#if CONFIG_CLSD_ENABLE
#define CLSD_QUE_SIZE   (2)

typedef struct { uint32_t *addr, size; } clsd_item_t;
typedef struct { 
    mod_inst_t self;
    x_handle_t hdrv;
    x_queue_t xque;
    x_event_t xevt;
    uint32_t *zero;
    bool mute;
} obj_priv_t;

static void clsd_drv_event(uint32_t event, uint32_t user)
{
    if (event & CSK_CLASSD_EVENT_BLOCK_COMPLETE) {
        obj_priv_t *const priv = (void *)(uint32_t *)user;
        clsd_item_t item;
        int ret = CSK_DRIVER_OK;
        uint8_t xcnt = 1;
        bool qret = x_queue_recv(priv->xque, &item, 0), play = qret && item.addr && item.size > 1;
        if (play) {
            ret = CLASSD_Send_PiPo(priv->hdrv, &(PIPO_OUT_BLOCK){
                    .sample_data = item.addr, .sample_cnt = item.size & ~1, .flags = false
                }, &xcnt, CLSD_TX_FLAG_START_NOW);
        } else {
            ret = CLASSD_Send_PiPo(priv->hdrv, &(PIPO_OUT_BLOCK){
                    .sample_data = priv->zero, .sample_cnt = AUD_STEP_SAMPS, .flags = qret
                }, &xcnt, CLSD_TX_FLAG_START_NOW);
        }
        ASSERT(CSK_DRIVER_OK == ret, "send(%d)", ret);
        if (priv->mute == play) {
            ret = CLASSD_SetMute(priv->hdrv, play ? 0 : CLSD_BMP_LEFT, CLSD_BMP_LEFT);
            ASSERT(CSK_DRIVER_OK == ret, "mute(%d)", ret);
            priv->mute = !play;
        }
    }
    if (event & CSK_CLASSD_EVENT_SEND_COMPLETE) TRACE("CLSD:COMPLETE");
    if (event & CSK_CLASSD_EVENT_TX_FIFO_UNDERRUN) TRACE("CLSD:UNDERUN");
    if (event & CSK_CLASSD_EVENT_TX_FIFO_EMPTY) TRACE("CLSD:EMPTY");
    ASSERT(!(event & CSK_CLASSD_EVENT_OTHER_ERROR), "CLSD:ERROR");
}

////////////////////////////////////////////////////////////////////////////////////////////////////
static int mod_write(const void *src, int size, x_msec_t msec)
{
    obj_priv_t *const priv = __MAPI_PRIVTE(CONFIG_CLSD_NAME);
    return x_queue_send(priv->xque, &(clsd_item_t){ .addr = (void *)src, .size = size }, false, msec);
}

static void mod_setup(mod_inst_t self, void *user)
{
    obj_priv_t *const priv = user;
    priv->self = self;
    priv->hdrv = CLASSD01();
    priv->xque = x_queue_new(CONFIG_CLSD_NAME, CLSD_QUE_SIZE, sizeof(clsd_item_t));
    ASSERT(priv->xque, "xque");
    priv->xevt = x_event_new(CONFIG_CLSD_NAME);
    ASSERT(priv->xque, "event");
    priv->zero = x_malloc(sizeof(aud_step_t), sizeof(uint32_t));
    memset(priv->zero, 0, x_msize(priv->zero));

    // mono(left channel), 16bits
    CLASSD_DMA_CHS dmach = { .dma_ch_out_lr = DMA_CHANNEL_ANY };
    int ret = CLASSD_Initialize(priv->hdrv, clsd_drv_event, (uint32_t)priv
        , CLSD_BMP_LEFT | CLSD_BMP_FLAG_DIS_MERGE_16BITS, &dmach);
    ASSERT (CSK_DRIVER_OK == ret, "init(%d)", ret);

    ret = CLASSD_PowerControl(priv->hdrv, CSK_POWER_FULL);
    ASSERT (CSK_DRIVER_OK == ret, "power(%d)", ret);

    // 16k
    ret = CLASSD_Control(priv->hdrv, CSK_CLASSD_SR_16KHZ | CSK_CLASSD_OSR_30, 0);
    ASSERT (CSK_DRIVER_OK == ret, "ctrl(%d)", ret);

    // mute at first
    ret = CLASSD_SetMute(priv->hdrv, CLSD_BMP_LEFT, CLSD_BMP_LEFT);
    ASSERT (CSK_DRIVER_OK == ret, "mute(%d)", ret);
    priv->mute = true;

    // start
    PIPO_OUT_BLOCK pipo[2] = {
        [0] = { .sample_data = priv->zero, .sample_cnt = AUD_STEP_SAMPS },
        [1] = { .sample_data = priv->zero, .sample_cnt = AUD_STEP_SAMPS },
    };
    uint8_t xcnt = __cntof(pipo);
    ret = CLASSD_Send_PiPo(priv->hdrv, pipo, &xcnt, CLSD_TX_FLAG_START_NOW);
    ASSERT(CSK_DRIVER_OK == ret || CSK_DRIVER_ERROR_BUSY == ret, "send(%d)", ret);
    ret = CLASSD_Enable(priv->hdrv, CLSD_BMP_LEFT, 0);
    ASSERT(CSK_DRIVER_OK == ret, "enable(%d)", ret);
}

static obj_priv_t mod_priv = { NULL };
static mod_apis_t mod_apis = {
    .setup = mod_setup,
    .write = mod_write
};
__MAPI_DECLARE(CONFIG_CLSD_NAME, 2, 2, &mod_apis, &mod_priv);

#endif//CONFIG_CLSD_ENABLE
