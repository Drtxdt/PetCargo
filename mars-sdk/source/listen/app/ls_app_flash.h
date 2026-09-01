#ifndef __LS_APPLICATION_FLASH_H__
#define __LS_APPLICATION_FLASH_H__

#include <stdint.h>

int listen_flash_init();

int listen_flash_voice_read(void *data, int size);

int listen_flash_voice_write(void *data, int size);

// int listen_flash_erase();

int listen_flash_config_read(void *data, int size);

int listen_flash_config_write(const void *data, int size);

#endif