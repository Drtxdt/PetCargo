#ifndef __FLASH_CACHE_H
#define __FLASH_CACHE_H

#include "mars.h"

//Flash_Cache Enable Flag
#define FLASH_CACHE_ENABLE			(1)
#define FLASH_CACHE_DISABLE			(0)

void flash_enableCache(void);

void flash_disableCache(void);

void flash_cache_invalidate_all(void);

int flash_cache_hit_cnt(void);

int flash_cache_miss_cnt(void);

void flash_cache_hit_clr(void);

void flash_cache_miss_clr(void);

#endif
