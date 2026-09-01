#ifndef __MODULE_HELPER_HEADER__
#define __MODULE_HELPER_HEADER__

#include "mdef.h"

// module private structures
typedef const void *mod_inst_t;
typedef int (*mod_func_t)(uint32_t uarg, void *parg, void *user);
typedef struct {
    void (*setup)(mod_inst_t self, void *user);                         // mandatory
    int (*read)(void *dst, int size, x_msec_t msec);                    // optional
    int (*write)(const void *src, int size, x_msec_t msec);             // optional
    int (*trans)(void *dst, const void *src, int size, x_msec_t msec);  // optional
    int (*ctrl)(uint32_t uarg, void *parg);                             // optional
    mod_func_t func;                                                    // optional
    void *user;                                                         // optional
} mod_apis_t;
typedef struct __packed__ {
    uint32_t tags:8, hash:24;
    const char *name;
    mod_apis_t *apis;
    void *priv;
} mod_priv_t;

// helper macros
#define __PRIV_MTAGS        'M'
#define __PRIV_INIT(X)      XVANAME(__modinit_##X##_,__LINE__)
#define __PRIV_INST(X)      XVANAME(__modinst_##X##_,__LINE__)
#define __MAPI_FIND(NAME)   __mapi_find(__mapi_hash(NAME))
#define __MAPI_PRIVTE(NAME) (((mod_priv_t*const)__MAPI_FIND(NAME))->priv)
#define __MAPI_DECLARE(NAME, MAJOR, MINOR, APIS, PRIV)  \
    static mod_priv_t __attribute__((used,section(".mod."#MAJOR"."#MINOR),aligned(4))) \
    __PRIV_INST(MAJOR##MINOR)={.apis=APIS,.tags=__PRIV_MTAGS,.name=NAME,.priv=PRIV};

// called by module self, to notify event to application/user who hooks
int __mapi_notify(mod_inst_t hmod, uint32_t uarg, void *parg);

// called by module and application/user, to generate module hash value, for fast indexing
uint32_t __mapi_hash(const char *str);
mod_inst_t __mapi_find(uint32_t hash);

// called by applicaton/user, to send/recv data or control the module
int __mapi_hook(mod_inst_t hmod, mod_func_t func, void *user);
int __mapi_write(mod_inst_t hmod, const void *src, int size, x_msec_t msec);
int __mapi_read(mod_inst_t hmod, void *dst, int size, x_msec_t msec);
int __mapi_trans(mod_inst_t hmod, void *dst, const void *src, int size, x_msec_t msec);
int __mapi_ctrl(mod_inst_t hmod, uint32_t uarg, void *parg);

// called by system when power on
void __mapi_preinit(void);
void __mapi_setup(int stage);
#define __MAPI_STAGE_PREV_RTOS   0  // stage 0(stdc early but before rtos scheduling context)
                                    // modules that do not rely on rtos, can initialized in this 
                                    // stage, such as uart trace module
#define __MAPI_STAGE_POST_RTOS   1  // stage 1(rtos thread context), as rtos running already, so 
                                    // any modules can initialized in this stage

#endif//__MODULE_HELPER_HEADER__
