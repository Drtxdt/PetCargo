#include "appinc.h"
#include "ls_app_flash.h"

// #define APP_CONFIG_VOICE_REGISTER_ENABLE (1)

#define FLASH_OTP1_AREA_ADDR    (0x2000)
#define FLASH_OTP2_AREA_ADDR    (0x3000)
// #define APP_CONFIG_BASE_ADDR    (FLASH_OTP1_AREA_ADDR + 0x20)
#define APP_OTP_DEF_DATA_SIZE   (32)
#define APP_CONFIG_OPT1_SIZE    (1024)
#define APP_CONFIG_OPT2_SIZE    (1024)
#define APP_CONFIG_HDR_SIZE     (4)
#define APP_CONFIG_DAT_SIZE     (28)
#define APP_CONFIG_BLK_SIZE     (APP_CONFIG_HDR_SIZE + APP_CONFIG_DAT_SIZE)
// #define APP_CONFIG_BLK_CNTS     (APP_CONFIG_DEV_SIZE / APP_CONFIG_BLK_SIZE)
#define APP_CONFIG_HDR_TAGS     (0x5A)
#define APP_CONFIG_FMT_TAGS     (0xFF)

#define APP_CONFIG_BUFFER_SIZE  (APP_CONFIG_OPT1_SIZE + APP_CONFIG_OPT2_SIZE)

typedef union {
    uint8_t xraw[APP_CONFIG_BLK_SIZE];
    struct { uint8_t ftag, fchk, size, bkid, data[APP_CONFIG_DAT_SIZE]; };
} app_config_item_t;

extern int mod_tnvs_read(uint32_t addr, uint8_t *data, int size);
extern int mod_tnvs_write(uint32_t addr, uint8_t *data, int size);
extern int mod_tnvs_erase(uint32_t addr);

static uint8_t s_config_buffer[APP_CONFIG_BUFFER_SIZE] __attribute__((aligned(4)));

int listen_flash_init()
{
    mod_tnvs_read(FLASH_OTP1_AREA_ADDR, s_config_buffer, APP_CONFIG_OPT1_SIZE);
    mod_tnvs_read(FLASH_OTP2_AREA_ADDR, s_config_buffer + APP_CONFIG_OPT1_SIZE, APP_CONFIG_OPT2_SIZE);
    return 0;
}

int listen_flash_voice_read(void *data, int size)
{
    if (size <= (APP_CONFIG_BUFFER_SIZE - APP_CONFIG_BLK_SIZE - APP_OTP_DEF_DATA_SIZE)) {
        memcpy(data, s_config_buffer + APP_CONFIG_BLK_SIZE + APP_OTP_DEF_DATA_SIZE, size);
        return 0;
    }
    return -1;
}

int listen_flash_voice_write(void *data, int size)
{
    if (size <= (APP_CONFIG_BUFFER_SIZE - APP_CONFIG_BLK_SIZE - APP_OTP_DEF_DATA_SIZE)) {
        memcpy(s_config_buffer + APP_CONFIG_BLK_SIZE + APP_OTP_DEF_DATA_SIZE, data, size);
        if (mod_tnvs_erase(FLASH_OTP1_AREA_ADDR)) return -1;
        if (mod_tnvs_write(FLASH_OTP1_AREA_ADDR, s_config_buffer, APP_CONFIG_OPT1_SIZE)) return -2;
        if (mod_tnvs_erase(FLASH_OTP2_AREA_ADDR)) return -3;
        if (mod_tnvs_write(FLASH_OTP2_AREA_ADDR, s_config_buffer + APP_CONFIG_OPT1_SIZE, APP_CONFIG_OPT2_SIZE)) return -4;
        return 0;
    }
    return -5;
}

int listen_flash_config_read(void *data, int size)
{
    app_config_item_t *const item = x_malloc(APP_CONFIG_BLK_SIZE, sizeof(uint8_t));
    memset(item, 0, APP_CONFIG_BLK_SIZE);
    int rtsize = -2;
    memcpy((void *)item, s_config_buffer + APP_OTP_DEF_DATA_SIZE, APP_CONFIG_BLK_SIZE);
    if (APP_CONFIG_HDR_TAGS == item->ftag && item->size <= APP_CONFIG_DAT_SIZE) {
        uint8_t xchk = 0x00;
        for (int ipos = 1; ipos < APP_CONFIG_HDR_SIZE + item->size; xchk ^= item->xraw[ipos++]);
        if (!xchk) {
            rtsize = __minof(size, item->size);
            memcpy(data, item->data, rtsize);
        }
    }
    x_free(item);
    return rtsize;
}

int listen_flash_config_write(const void *data, int size)
{
    if (mod_tnvs_erase(FLASH_OTP1_AREA_ADDR)) return -3;

    int rtsize = __minof(size / 4 * 4, APP_CONFIG_DAT_SIZE);
    app_config_item_t *const item = x_malloc(APP_CONFIG_BLK_SIZE, sizeof(uint8_t));
    memcpy(item->data, data, rtsize);
    item->ftag = APP_CONFIG_HDR_TAGS;
    item->size = rtsize;
    item->bkid = 0;
    item->fchk = 0;
    for (int ipos = 2; ipos < APP_CONFIG_HDR_SIZE + rtsize; item->fchk ^= item->xraw[ipos++]);

    memcpy(s_config_buffer + APP_OTP_DEF_DATA_SIZE, item, APP_CONFIG_HDR_SIZE + rtsize);

    if (mod_tnvs_write(FLASH_OTP1_AREA_ADDR, s_config_buffer, APP_CONFIG_OPT1_SIZE))
        rtsize = -4;
    x_free(item);
    return rtsize;
}