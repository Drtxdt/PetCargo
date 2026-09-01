#include "appinc.h"

#if CONFIG_SPI_ENABLE
#define SPI_EVT_XFER_DONE   XBIT(0) // CSK_SPI_EVENT_TRANSFER_COMPLETE
#define SPI_EVT_XFER_LOSE   XBIT(1) // CSK_SPI_EVENT_DATA_LOST
#define SPI_EVT_XFER_MASK   XBITS(1, 0)
#define SPI_EVT_SLV_CMD_R   XBIT(3) // CSK_SPI_EVENT_SLV_CMD_R
#define SPI_EVT_SLV_CMD_W   XBIT(4) // CSK_SPI_EVENT_SLV_CMD_W
#define SPI_EVT_SLV_CMD_S   XBIT(5) // CSK_SPI_EVENT_SLV_CMD_S
#define SPI_EVT_SLV_MASK    XBITS(5, 3)

typedef struct { 
    mod_inst_t self;
    x_handle_t hdrv;
    x_event_t xevt;
} obj_priv_t;

static void spi_drv_event(uint32_t event, uint32_t user)
{
    obj_priv_t *const priv = (void *)(uint32_t *)user;
    ASSERT(!(event & SPI_EVT_SLV_MASK), "SLVCMD");
    x_event_set(priv->xevt, event);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
static int mod_trans(void *dst, const void *src, int size, x_msec_t msec)
{
    obj_priv_t *const priv = __MAPI_PRIVTE(CONFIG_SPI_NAME);
    int ret = SPI_Transfer(priv->hdrv, src, dst, size);
    ASSERT(CSK_DRIVER_OK == ret, "trans(%d)", ret);
    x_evtbit_t xbits = x_event_wait(priv->xevt, SPI_EVT_XFER_MASK | XEVT_AUTO_CLR, msec);
    if (xbits & SPI_EVT_XFER_DONE) return 1;
    if (xbits & SPI_EVT_XFER_LOSE) return -1;
    return 0;
}

static void mod_setup(mod_inst_t self, void *user)
{
    obj_priv_t *const priv = user;
    priv->self = self;
    priv->hdrv = SPI0();
    priv->xevt = x_event_new(CONFIG_SPI_NAME);
    ASSERT(priv->xevt, "xevt");

    int ret = SPI_Initialize(priv->hdrv, spi_drv_event, (uint32_t)priv);
    ASSERT (CSK_DRIVER_OK == ret, "init(%d)", ret);

    ret = SPI_PowerControl(priv->hdrv, CSK_POWER_FULL);
    ASSERT (CSK_DRIVER_OK == ret, "power(%d)", ret);

    // 12M
    ret = I2S_Control(priv->hdrv, CSK_SPI_MODE_MASTER | CSK_SPI_TXIO_DMA | CSK_SPI_CPOL1_CPHA1 
            | CSK_SPI_RXIO_PIO | CSK_SPI_DATA_BITS(8) | CSK_SPI_MSB_LSB, 12000000);
    ASSERT (CSK_DRIVER_OK == ret, "ctrl(%d)", ret);
    int speed = SPI_Control(priv->hdrv, CSK_SPI_GET_BUS_SPEED, 0);
    ASSERT(speed < 100 * 1000 * 1000, "speed(%d)", speed);
    LOGD("SPI: %d", speed);
}

static obj_priv_t mod_priv = { NULL };
static mod_apis_t mod_apis = {
    .setup = mod_setup,
    .trans = mod_trans,
};
__MAPI_DECLARE(CONFIG_SPI_NAME, 2, 0, &mod_apis, &mod_priv);
#endif//CONFIG_SPI_ENABLE
