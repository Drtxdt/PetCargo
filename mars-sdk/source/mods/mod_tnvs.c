#include "appinc.h"
#if CONFIG_TNVS_ENABLE
#include "tinynvs.h"

typedef struct {
    x_handle_t tnvs;
} obj_priv_t;

#if CONFIG_TNVS_SHELL
void shell_tnvs_dump(void)
{
    SHELL_ITEM_EXPORT("tnvs.dump", shell_tnvs_dump, "dump");
    obj_priv_t *const priv = __MAPI_PRIVTE(CONFIG_TNVS_NAME);
    tnvs_dump_content(priv->tnvs);
}

void shell_tnvs_erase(void)
{
    SHELL_ITEM_EXPORT("tnvs.erase", shell_tnvs_erase, "erase");
    obj_priv_t *const priv = __MAPI_PRIVTE(CONFIG_TNVS_NAME);
    int ret = tnvs_erase_content(priv->tnvs);
    if (ret < 0)
        LOGE("erase: fail(%d)", ret);
    else
        LOGI("erase: pass");
}

void shell_tnvs_status(void)
{
    SHELL_ITEM_EXPORT("tnvs.status", shell_tnvs_status, "tnvs.status");
    obj_priv_t *const priv = __MAPI_PRIVTE(CONFIG_TNVS_NAME);
    uint8_t sr1 = 0, sr2 = 0;
    int ret = tnvs_get_status(priv->tnvs, &sr1, &sr2);
    if (ret < 0)
        LOGE("status: fail(%d)", ret);
    else
        LOGI("status: pass(0x%02x%02x)", sr2, sr1);
}
#endif//CONFIG_TNVS_SHELL

static int mod_write(const void *src, int size, x_msec_t msec)
{
    obj_priv_t *const priv = __MAPI_PRIVTE(CONFIG_TNVS_NAME);
    return tnvs_save_item(priv->tnvs, src, size, msec);
}

static int mod_read(void *dst, int size, x_msec_t msec)
{
    obj_priv_t *const priv = __MAPI_PRIVTE(CONFIG_TNVS_NAME);
    return tnvs_load_item(priv->tnvs, dst, size, msec);
}

int mod_tnvs_read(uint32_t addr, uint8_t *data, int size)
{
    obj_priv_t *const priv = __MAPI_PRIVTE(CONFIG_TNVS_NAME);
    return tnvs_read_content_by_addr(priv->tnvs, addr, data, size);
}

int mod_tnvs_erase(uint32_t addr)
{
    obj_priv_t *const priv = __MAPI_PRIVTE(CONFIG_TNVS_NAME);
    return tnvs_earse_content_by_addr(priv->tnvs, addr);
}

int mod_tnvs_write(uint32_t addr, uint8_t *data, int size)
{
    obj_priv_t *const priv = __MAPI_PRIVTE(CONFIG_TNVS_NAME);
    return tnvs_write_content_by_addr(priv->tnvs, addr, data, size);
}

static int mod_ctrl(uint32_t uarg, void *parg)
{
    obj_priv_t *const priv = __MAPI_PRIVTE(CONFIG_TNVS_NAME);
    uint8_t ctrl = 0;
    int ret;
    switch (uarg) {
    case MAPI_TNVS_CTRL_OTP_LOCK_SET:
        ctrl = *(uint32_t *)parg;
    case MAPI_TNVS_CTRL_OTP_LOCK_GET:
        ret = tnvs_lock_content(priv->tnvs, &ctrl);
        *(uint32_t *)parg = ctrl;
        return ret;
    case MAPI_TNVS_CTRL_OTP_ERASE:
        return tnvs_erase_content(priv->tnvs);
    default:
        break;
    }
    return MAPI_CTRL_ECODE_FAIL;
}

static void mod_setup(mod_inst_t self, void *user)
{
    obj_priv_t *const priv = (void *)user;
    priv->tnvs = tnvs_mod_init(CONFIG_TNVS_DEFID);  // 0,1,2
}

static obj_priv_t mod_priv = { 0 };
static mod_apis_t mod_apis = {
    .setup = mod_setup,
    .write = mod_write,
    .read = mod_read,
    .ctrl = mod_ctrl,
};
__MAPI_DECLARE(CONFIG_TNVS_NAME, 0, 1, &mod_apis, &mod_priv);
#endif // CONFIG_TNVS_ENABLE
