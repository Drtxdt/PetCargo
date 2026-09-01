#include "appinc.h"

#if CONFIG_DMAC_ENABLE
#define DMAC_EVT_STEP   XBIT(0)     // DMA_EVENT_TRANSFER_COMPLETE
#define DMAC_EVT_FULL   XBIT(1)     // DMA_EVENT_BLOCK_COMPLETE
#define DMAC_EVT_FAIL   XBIT(2)     // DMA_EVENT_ERROR
#define DMAC_EVT_MASK   XBITS(2,0)

typedef struct {
    x_event_t evts;
    uint8_t chn;
} obj_priv_t;

static obj_priv_t *priv = NULL;
////////////////////////////////////////////////////////////////////////////////////////////////////
static void dma_drv_evt(uint32_t event, uint32_t xsize, uint32_t user)
{
    x_event_set(priv->evts, event & DMAC_EVT_MASK);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
static int mod_trans(void *dst, const void *src, int size, x_msec_t msec)
{
    x_event_wait(priv->evts, DMAC_EVT_STEP | XEVT_AUTO_CLR, XINFINITE);
    int ret = dma_memcpy(priv->chn, (uint32_t)src, (uint32_t)dst, (uint32_t)size);
    ASSERT(CSK_DRIVER_OK == ret, "dmacp(%d)", ret);
    x_event_wait(priv->evts, DMAC_EVT_STEP, msec);
    return size;
}
static int mod_ctrl(uint32_t uarg, void *parg)
{
    return (int)x_event_wait(priv->evts, DMAC_EVT_STEP, (x_msec_t)uarg);
}
static void mod_setup(mod_inst_t self, void *user)
{
    // obj_priv_t *const 
    priv = user;
    priv->evts = x_event_new("dmac");
    ASSERT(priv->evts, "evts");
    ASSERT(!dma_channel_is_reserved(priv->chn), "dma:exist");
    priv->chn = dma_channel_reserve(priv->chn, dma_drv_evt, (uint32_t)priv, DMA_CACHE_SYNC_AUTO);
    ASSERT(DMA_CHANNEL_ANY != priv->chn, "dma:full");
    x_event_set(priv->evts, DMAC_EVT_STEP);
}

void x_dma_copy(void *dst, const void *src, size_t size, x_msec_t msec)
{
    if (size > 0) mod_trans(dst, src, size, 0);
    mod_ctrl(msec, NULL);
}
void x_dma_wait(x_msec_t msec)
{
    x_dma_copy(NULL, NULL, 0, msec);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
static obj_priv_t mod_priv = { .chn = 0 };
static mod_apis_t mod_apis = {
    .setup = mod_setup,
    .trans = mod_trans,
    .ctrl  = mod_ctrl,
};
__MAPI_DECLARE(CONFIG_DMAC_NAME, 0, 1, &mod_apis, &mod_priv);
#endif//CONFIG_DMAC_ENABLE
