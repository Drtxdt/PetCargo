#include "appinc.h"
#include "crc32.h"

#if CONFIG_RESMGR_ENABLE
typedef struct {
	mod_inst_t self;
	res_hdr_t xres;
} obj_priv_t;

static void res_mgr_init(obj_priv_t *priv)
{
#if CONFIG_RESMGR_AUTO_INCLUDED
	extern res_hdr_t __res_start, __res_end;
	priv->xres = (const void *)&__res_start;
	LOGW("RES: From %p to %p, Total %d bytes", &__res_start, &__res_end, &__res_end - &__res_start);
#else
	priv->xres = (const void *)CONFIG_RESMGR_FIXED_ADDRESS;
	LOGI("RES @ %p", priv->xres);
#endif
	
	ASSERT(priv->xres->hdrtag == RESMGR_TAGNAME, "TAG(%#lx)", priv->xres->hdrtag);
	uint32_t chk = crc32_calc(&priv->xres->prot_ver, 24 + priv->xres->itemcnt * sizeof(item_desc_t), 0);
	ASSERT(priv->xres->hdrcrc == chk, "CRC32(%#lx!=%#lx)", chk, priv->xres->hdrcrc);
#if RESMGR_DATCHK
	uint32_t size;
	for (uint32_t resid = 0; resid < priv->xres->itemcnt; resid++) {
		item_desc_t const *const item = &priv->xres->items[resid];
		void *const addr = res_item_by_id(resid, &size);
		chk = crc32_calc(addr, size, 0);
		ASSERT(item->crc32 == chk, "RES(%c%c%c%c) ADR(%p) LEN(%lx) CHK(%lx!=%lx)"
			, (char)(item->tagid & 0xFF), (char)((item->tagid >> 8) & 0xFF)
			, (char)((item->tagid >> 16) & 0xFF), (char)((item->tagid >> 24) & 0xFF)
			, addr, size, chk, item->crc32);
		LOGV("[ITEM]%03d: ID(%c%c%c%c) ADR(%p) LEN(%lx) CHK(%lx)", (int)resid
			, (char)(item->tagid & 0xFF), (char)((item->tagid >> 8) & 0xFF)
			, (char)((item->tagid >> 16) & 0xFF), (char)((item->tagid >> 24) & 0xFF)
			, addr, size, item->crc32);
	}
#endif
	LOGV("RESOK");
}

void res_init()
{
	obj_priv_t *const priv = __MAPI_PRIVTE(CONFIG_RESMGR_NAME);
	res_mgr_init(priv);
}

#if RESMGR_FINDBYID
void *res_item_by_id(uint32_t resid, uint32_t *psize)
{
	obj_priv_t *const priv = __MAPI_PRIVTE(CONFIG_RESMGR_NAME);
	if (!priv->xres || resid >= priv->xres->itemcnt) return NULL;
	item_desc_t const *const item = &priv->xres->items[resid];
	if (psize) *psize = item->size;
	return (uint8_t *)priv->xres + item->addr;
}
#endif

#if RESMGR_FINDBYTAG
const void *res_item_by_name(uint32_t tagid, uint32_t *psize)
{
	obj_priv_t *const priv = __MAPI_PRIVTE(CONFIG_RESMGR_NAME);
	if (!priv->xres) return NULL; 
	for (uint32_t resid = 0; resid < priv->xres->itemcnt; resid++) {
		item_desc_t const *const item = &priv->xres->items[resid];
		if (targid == item->tagid)
			return res_item_by_id(resid, psize);
	}
	return NULL;
}
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////
static void mod_setup(mod_inst_t self, void *user)
{
	obj_priv_t *const priv = user;
	priv->self = self;
	// res_mgr_init(priv);
}

static obj_priv_t mod_priv = { NULL };
static mod_apis_t mod_apis = { 
	.setup = mod_setup
};
__MAPI_DECLARE(CONFIG_RESMGR_NAME, 1, 1, &mod_apis, &mod_priv);

#endif//CONFIG_RESMGR_ENABLE
