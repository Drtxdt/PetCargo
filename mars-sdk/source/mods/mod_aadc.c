#include "appinc.h"

#if CONFIG_AADC_ENABLE

#define AADC_QUE_SIZE       (18)
#define AADC_D_GAIN_DB_DEF   (6)

typedef struct {
	x_queue_t xque;
	aud_step_t *xbuf;
	uint32_t xpos;
} fifo_mrg_t;
typedef struct { 
    mod_inst_t self, i2so;
    x_handle_t hdrv;
    fifo_mrg_t xmgr;
    bool xfer;
} obj_priv_t;

static void aadc_drv_event(uint32_t event, uint32_t user)
{
    obj_priv_t *const priv = (void *)(uint32_t *)user;
    if (event & CSK_ADCPDM_EVENT_BLOCK_COMPLETE) {
        aud_step_t const *addr = &priv->xmgr.xbuf[priv->xmgr.xpos];
        if (++priv->xmgr.xpos >= AADC_QUE_SIZE) priv->xmgr.xpos = 0;
        int ipos = priv->xmgr.xpos + 1;
        if (ipos >= AADC_QUE_SIZE) ipos = 0; 

        uint8_t xcnt = 1;
        PIPO_IN_BLOCK pipo = {
            .sample_data = (uint32_t *)&priv->xmgr.xbuf[ipos],
            .sample_cnt = AUD_STEP_SAMPS,
            .flags = false
        };
        int ret = ADC_PDM_Receive_PiPo(priv->hdrv, &pipo, &xcnt, ADC_PDM_RX_FLAG_START_NOW);
        ASSERT(ret == CSK_DRIVER_OK, "recv(%d)", ret);
    #if 0
        __mapi_write(priv->i2so, addr, AUD_STEP_SAMPS, 0);
    #endif
        if (priv->xfer) {
            if (x_queue_spaces(priv->xmgr.xque) < 1) {
                TRACE("AADC:LOSE");
                x_queue_rst(priv->xmgr.xque);
            }
            ret = x_queue_send(priv->xmgr.xque, &addr, false, 0);
            ASSERT(ret, "AADC:FAIL");
        }
    }
    if (event & CSK_ADCPDM_EVENT_RECEIVE_COMPLETE) TRACE("AADC:COMPLETE");
    if (event & CSK_ADCPDM_EVENT_RX_FIFO_OVERRUN) TRACE("AADC:OVERUN");
    if (event & CSK_ADCPDM_EVENT_RX_FIFO_FULL) TRACE("AADC:FULL");
    ASSERT(!(event & CSK_ADCPDM_EVENT_OTHER_ERROR), "AADC:ERROR");
}

////////////////////////////////////////////////////////////////////////////////////////////////////
static int mod_read(void *dst, int size, x_msec_t msec)
{
    obj_priv_t *const priv = __MAPI_PRIVTE(CONFIG_AADC_NAME);
    ASSERT(priv, "aadc");
    return x_queue_recv(priv->xmgr.xque, dst, msec);
}

static int mod_ctrl(uint32_t uarg, void *parg)
{
    int ret = CSK_DRIVER_OK;
    obj_priv_t *const priv = __MAPI_PRIVTE(CONFIG_AADC_NAME);
    ASSERT(priv, "aadc");

    switch (uarg) {
    case MAPI_AADC_CTRL_REC_START:
        LOGD("AADC: START");
        if (!priv->xfer) {
            x_queue_rst(priv->xmgr.xque);
            priv->xfer = true;
        }
        break;
    case MAPI_AADC_CTRL_REC_STOP:
        LOGD("AADC: STOP");
        priv->xfer = false;
        break;
    case MAPI_AADC_CTRL_REC_RESET:
        LOGD("AADC: RESET");
        x_queue_rst(priv->xmgr.xque);
        break;
    case MAPI_AADC_CTRL_GET_SAMPS: {
        int *samp = (int *)parg;
        *samp = AUD_STEP_SAMPS;
        LOGD("AADC: STEPSAMP=%d", *samp);
        break;
    }
    case MAPI_AADC_CTRL_SET_GAIN: {
        int *gain = (int *)parg;
        int again = gain[0]; // set again, step:2
        int dgain = gain[1]; // set dgain, step:1
        if (again > ADC_PDM_GAIN_A_MAX_DB) again = ADC_PDM_GAIN_A_MAX_DB;
        if (again < ADC_PDM_GAIN_A_MIN_DB) again = ADC_PDM_GAIN_A_MIN_DB;
        if (dgain > ADC_PDM_GAIN_D_MAX_DB) dgain = ADC_PDM_GAIN_D_MAX_DB;
        if (dgain < ADC_PDM_GAIN_D_MIN_DB) dgain = ADC_PDM_GAIN_D_MIN_DB;
        LOGI("AADC: ANAGAIN=%ddB, DNAGAIN=%ddB", again, dgain);
        ret = ADC_PDM_SetVolume(priv->hdrv, ADC_PDM_GAIN_A_VAL(again),
                ADC_PDM_GAIN_D_VAL(dgain), ADC_PDM_VOL_FLAG_A_LEFT| ADC_PDM_VOL_FLAG_D_LEFT);
        ASSERT(CSK_DRIVER_OK == ret, "setvol(%d)", ret);
        break;
    }
    case MAPI_AADC_CTRL_SET_CAPLESS: {
        bool bcap = *(bool *)parg;
        LOGI("AADC: CAPLESS=%d", bcap);
        IP_AON_CTRL->REG_AON_TUNE2.bit.EN_CAPLESS_LDO_VA = bcap; /* 0:external; 1:internal */
        break;
    }
    case MAPI_AADC_CTRL_SET_PGATYPE:
    default:
        return -1;
    }
    return 0;
}

static void mod_setup(mod_inst_t self, void *user)
{
    obj_priv_t *const priv = user;
    priv->self = self;
    priv->xfer = false;
    priv->hdrv = ADC_PDM01();

#if 0
    priv->i2so = __MAPI_FIND(CONFIG_I2SO_NAME);
    ASSERT(priv->i2so, CONFIG_I2SO_NAME);
#endif

    priv->xmgr.xque = x_queue_new("xmgr", AADC_QUE_SIZE - 2, sizeof(uint32_t *));
    ASSERT(priv->xmgr.xque, "xque");

    // mono(left channel), 16bits
    ADC_PDM_DMA_CHS dmach = { .dma_ch_in_lr = DMA_CHANNEL_ANY };
    int ret = ADC_PDM_Initialize(priv->hdrv, aadc_drv_event, (uint32_t)priv
        , ADC_PDM_BMP_LEFT | ADC_PDM_BMP_FLAG_USE_16BITS, &dmach);
    ASSERT(CSK_DRIVER_OK == ret, "adc(%d)", ret);

    ret = ADC_PDM_PowerControl(priv->hdrv, CSK_POWER_FULL);
    ASSERT(CSK_DRIVER_OK == ret, "pwr(%d)", ret);

    // 16k + hpf + pga
#if CONFIG_AADC_CAPEX
    IP_AON_CTRL->REG_AON_TUNE2.bit.EN_CAPLESS_LDO_VA = false; /* 0:external/exthw */
#else
    IP_AON_CTRL->REG_AON_TUNE2.bit.EN_CAPLESS_LDO_VA = true;  /* 1:internal/onchip */
#endif
    LOGI("AADC: AGAIN=%ddB DGAIN=%ddB, CAPLESS=%d PGA=%s", CONFIG_AADC_GAIN, AADC_D_GAIN_DB_DEF
        , IP_AON_CTRL->REG_AON_TUNE2.bit.EN_CAPLESS_LDO_VA /* 0:external; 1:internal */
#if CONFIG_AADC_DIFF
        , "DIFF"
#else
        , "SEND"
#endif
    );

    ret = ADC_PDM_Control(priv->hdrv, CSK_ADCPDM_SR_16KHZ | CSK_ADCPDM_OSR_250, 0);
    ASSERT(CSK_DRIVER_OK == ret, "ctrl(%d)", ret);
#if CONFIG_AADC_HPF
    ret = ADC_PDM_Control(priv->hdrv, CSK_ADCPDM_HPF_SET
        , CSK_ADCPDM_ARG_HPF1_EN | CSK_ADCPDM_ARG_HPF2_EN | CSK_ADCPDM_ARG_HPF2_CUT(3));
    ASSERT(CSK_DRIVER_OK == ret, "sethpf(%d)", ret);
#endif
    ret = ADC_PDM_Control(priv->hdrv, CSK_ADCPDM_PGA_INPUT_SET
#if CONFIG_AADC_DIFF
        , CSK_ADCPDM_ARG_LPGA_INPUT_DIFFER
#else
        , CSK_ADCPDM_ARG_LPGA_INPUT_SINGLE
#endif
    );
    ASSERT(CSK_DRIVER_OK == ret, "setpga(%d)", ret);

    // volume: analog=-12~+36db, digital=0db
    ret = ADC_PDM_SetVolume(priv->hdrv, ADC_PDM_GAIN_A_VAL(CONFIG_AADC_GAIN)
        , ADC_PDM_GAIN_D_VAL(AADC_D_GAIN_DB_DEF), ADC_PDM_VOL_FLAG_A_LEFT | ADC_PDM_VOL_FLAG_D_LEFT);
    ASSERT(CSK_DRIVER_OK == ret, "setvol(%d)", ret);

    // unmute at first
    ret = ADC_PDM_SetMute(priv->hdrv, 0, ADC_PDM_BMP_LEFT);
    ASSERT(CSK_DRIVER_OK == ret, "mute(%d)", ret);

    priv->xmgr.xpos = 0;
    priv->xmgr.xbuf = (aud_step_t *)x_malloc(AADC_QUE_SIZE * sizeof(aud_step_t), sizeof(void *));
    ASSERT(priv->xmgr.xbuf, "mbuf");

    PIPO_IN_BLOCK pipo[] = {
        [0] = { .sample_data = (void *)priv->xmgr.xbuf[0], .sample_cnt = AUD_STEP_SAMPS },
        [1] = { .sample_data = (void *)priv->xmgr.xbuf[1], .sample_cnt = AUD_STEP_SAMPS },
    };
    uint8_t xcnt = __cntof(pipo);
    ret = ADC_PDM_Receive_PiPo(priv->hdrv, pipo, &xcnt, ADC_PDM_RX_FLAG_START_NOW);
    ASSERT(ret == CSK_DRIVER_OK, "recv(%d)", ret);
}

static obj_priv_t mod_priv = { NULL };
static mod_apis_t mod_apis = {
    .setup = mod_setup,
    .read = mod_read,
    .ctrl = mod_ctrl,
};
__MAPI_DECLARE(CONFIG_AADC_NAME, 2, 1, &mod_apis, &mod_priv);
#endif//CONFIG_AADC_ENABLE
