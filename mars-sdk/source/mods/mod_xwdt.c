#include "appinc.h"

#if CONFIG_XWDT_ENABLE
#define WDT_TIMER_PERIOD (64000)

typedef struct {
    mod_inst_t self;
    x_handle_t hdrv;
} obj_priv_t;

static void mod_setup(mod_inst_t self, void *user)
{
	obj_priv_t *const priv = user;
   	priv->self = self;
    priv->hdrv = AON_WDT();
	AON_WDT_Initialize(priv->hdrv, NULL, NULL);
	AON_WDT_PowerControl(priv->hdrv, CSK_POWER_FULL);
	AON_WDT_Control(priv->hdrv, HAL_AON_WDT_TIME_Msk, WDT_TIMER_PERIOD);
    AON_WDT_Control(priv->hdrv, HAL_AON_WDT_CTRL_RESET_MODE | HAL_AON_WDT_RST_CORE_DOMAIN, 0);
}

static int mod_ctrl(uint32_t uarg, void *parg)
{
	obj_priv_t *const priv = __MAPI_PRIVTE(CONFIG_XWDT_NAME);
    ASSERT(priv, "aadc inst is NULL");

    switch (uarg) {
	case MAPI_XWDT_CTRL_CLOSE:
       	AON_WDT_Disable(priv->hdrv);
   	 	AON_WDT_PowerControl(priv->hdrv, CSK_POWER_OFF);
   		AON_WDT_Uninitialize(priv->hdrv);
		LOGI("WDT_STOP");
        break;
	case MAPI_XWDT_CTRL_OPEN:
       	AON_WDT_Enable(priv->hdrv);
		LOGI("WDT_START");
        break;
	case MAPI_XWDT_CTRL_FEED:
       	AON_WDT_Refresh(priv->hdrv);
        break;
	default:
        return -1;
	}
	return 0;
}

static obj_priv_t mod_priv = { NULL };
static mod_apis_t mod_apis = {
    .setup = mod_setup,
    .ctrl = mod_ctrl,
};
__MAPI_DECLARE(CONFIG_XWDT_NAME, 2, 4, &mod_apis, &mod_priv);

#endif//CONFIG_XWDT_ENABLE
