#ifndef __TINY_NVS_MODULE_HEADER__
#define __TINY_NVS_MODULE_HEADER__

#include "xutils.h"

x_handle_t tnvs_mod_init(int blkid);
int tnvs_mod_free(x_handle_t tnvs);
int tnvs_save_item(x_handle_t tnvs, const void *src, int size, x_msec_t msec);
int tnvs_load_item(x_handle_t tnvs, void *dst, int size, x_msec_t msec);
int tnvs_erase_content(x_handle_t tnvs);
int tnvs_get_status(x_handle_t tnvs, uint8_t *sr1, uint8_t *sr2);
int tnvs_lock_content(x_handle_t tnvs, uint8_t *lockmsk);
int tnvs_dump_content(x_handle_t tnvs);
int tnvs_earse_content_by_addr(x_handle_t tnvs, uint32_t addr);
int tnvs_read_content_by_addr(x_handle_t tnvs, uint32_t addr, uint8_t *data, int size);
int tnvs_write_content_by_addr(x_handle_t tnvs, uint32_t addr, uint8_t *data, int size);

#endif//__TINY_NVS_MODULE_HEADER__
