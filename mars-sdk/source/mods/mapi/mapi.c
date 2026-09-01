#include "appinc.h"

extern mod_priv_t __mod_stdc, __mod_rtos, __mod_ends;

uint32_t __mapi_hash(const char *str)
{
	const char *ptr = str;
    uint32_t hash = 0;
    while (*ptr) hash = (hash << 7) + (hash << 2) - hash + *ptr++;
    return hash >> 8;
}

void __mapi_preinit(void)
{
    for (mod_priv_t *priv = &__mod_stdc; priv < &__mod_ends; priv++) {
        if (priv) {
            priv->hash = __mapi_hash(priv->name);
        #if 0
           TRACE("%6s: %02X%06X / %c%c%c%c%c"
               , priv->name, priv->tags, priv->hash
               , priv->apis && priv->apis->setup  ? 'S' : '-'
               , priv->apis && priv->apis->read   ? 'R' : '-'
               , priv->apis && priv->apis->write  ? 'W' : '-'
               , priv->apis && priv->apis->ctrl   ? 'C' : '-'
               , priv->apis && priv->apis->func   ? 'F' : '-'
               , priv->priv                       ? 'P' : '-');
        #endif
        }
    }
}

void __mapi_setup(int stage)
{
    mod_priv_t *base = NULL, *end = NULL;
    switch (stage) {
    case __MAPI_STAGE_PREV_RTOS: base = &__mod_stdc, end = &__mod_rtos; break;
    case __MAPI_STAGE_POST_RTOS: base = &__mod_rtos, end = &__mod_ends; break;
    default: return;
    }
    for (mod_priv_t *priv = base; priv < end; priv++)
        if (priv && priv->apis && priv->apis->setup)
            priv->apis->setup(priv, priv->priv);
}

mod_inst_t __mapi_find(uint32_t hash)
{
    for (const mod_priv_t *priv = &__mod_stdc; priv < &__mod_ends; priv++)
        if (priv && __PRIV_MTAGS == priv->tags && priv->hash == hash) return priv;
    return NULL;
}

int __mapi_write(mod_inst_t hmod, const void *src, int size, x_msec_t msec)
{
    mod_priv_t const *const priv = hmod;
    if (priv && priv->apis && priv->apis->write && __PRIV_MTAGS == priv->tags)
        return priv->apis->write(src, size, msec);
    return -1;
}

int __mapi_read(mod_inst_t hmod, void *dst, int size, x_msec_t msec)
{
    mod_priv_t const *const priv = hmod;
    if (priv && priv->apis && priv->apis->read && __PRIV_MTAGS == priv->tags)
        return priv->apis->read(dst, size, msec);
    return -1;
}

int __mapi_trans(mod_inst_t hmod, void *dst, const void *src, int size, x_msec_t msec)
{
    mod_priv_t const *const priv = hmod;
    if (priv && priv->apis && priv->apis->read && __PRIV_MTAGS == priv->tags)
        return priv->apis->trans(dst, src, size, msec);
    return -1;
}

int __mapi_ctrl(mod_inst_t hmod, uint32_t uarg, void *parg)
{
    mod_priv_t const *const priv = hmod;
    if (priv && priv->apis && priv->apis->ctrl && __PRIV_MTAGS == priv->tags)
        return priv->apis->ctrl(uarg, parg);
    return -1;
}

int __mapi_hook(mod_inst_t hmod, mod_func_t func, void *user)
{
    mod_priv_t const *const priv = hmod;
    if (priv && priv->apis && __PRIV_MTAGS == priv->tags) {
        priv->apis->user = user;
        priv->apis->func = func;
        return 0;
    }
    return -1;
}

int __mapi_notify(mod_inst_t hmod, uint32_t uarg, void *parg)
{
    mod_priv_t const *const priv = hmod;
    if (priv && priv->apis && priv->apis->func)
        return priv->apis->func(uarg, parg, priv->apis->user);
    return -1;
}
