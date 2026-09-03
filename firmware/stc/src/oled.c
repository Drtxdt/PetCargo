#include "stc15.h"
#include "hal.h"
#include "config.h"
#include "oled.h"

#define OLED_ADDRESS 0x78

/* ULN2003 inverts the MCU output. 0 releases the SM line, 1 pulls it low. */
static void oled_sda(uint8_t high) { PIN_SM_S2 = high ? 0 : 1; }
static void oled_scl(uint8_t high) { PIN_SM_S1 = high ? 0 : 1; }
/* Explicit setup/hold on BOTH clock phases: ULN2003 release is not a
 * push-pull rising edge. Do not rely on SDCC function-call overhead. */
static void oled_delay(void) { hal_delay_us(5); }
static void oled_start(void)
{
    oled_scl(0); oled_delay(); oled_sda(1); oled_delay();
    oled_scl(1); oled_delay(); oled_sda(0); oled_delay(); oled_scl(0); oled_delay();
}
static void oled_stop(void)
{
    oled_scl(0); oled_delay(); oled_sda(0); oled_delay();
    oled_scl(1); oled_delay(); oled_sda(1); oled_delay();
}
static void oled_write_byte(uint8_t value)
{
    uint8_t i;
    for (i = 0; i < 8; i++) {
        oled_sda(value & 0x80); oled_delay();
        oled_scl(1); oled_delay(); oled_scl(0); oled_delay(); value <<= 1;
    }
    oled_sda(1); oled_delay(); oled_scl(1); oled_delay();
    oled_scl(0); oled_delay(); /* ACK cannot be read through ULN2003. */
}
static void oled_command(uint8_t value)
{
    oled_start(); oled_write_byte(OLED_ADDRESS); oled_write_byte(0x00); oled_write_byte(value); oled_stop();
}
static void oled_position(uint8_t page, uint8_t column)
{
    oled_command((uint8_t)(0xB0 + page)); oled_command(column & 0x0F); oled_command((uint8_t)(0x10 | (column >> 4)));
}

/* Hand-authored 16x16 monochrome sprites, scaled 4x on the 128x64 panel. */
static const uint16_t __code sprites[12][16] = {
    {0x0810,0x1C38,0x366C,0x63C6,0xC003,0x8001,0x9999,0x8181,0x8001,0x8421,0x83C1,0x8001,0xC003,0x6006,0x3FFC,0x0000},
    {0x0810,0x1C38,0x366C,0x63C6,0xC003,0x8001,0xA5A5,0x9999,0x8001,0x8181,0x8241,0x8421,0xC183,0x6006,0x3FFC,0x0000},
    {0x0810,0x1C38,0x366C,0x63C6,0xC003,0x8001,0x9DB9,0xA5A5,0x8001,0x8181,0x8241,0x8241,0xC183,0x6006,0x3FFC,0x0000},
    {0x0810,0x1C38,0x366C,0x63C6,0xC003,0x8001,0xA5A5,0x9999,0x8001,0x9249,0x8921,0x9249,0xC003,0x6006,0x3FFC,0x0000},
    {0x0810,0x1C38,0x366C,0x63C6,0xC003,0x8001,0x8001,0x9FF9,0x8001,0x8181,0x83C1,0x8001,0xC003,0x6006,0x3FFC,0x0000},
    {0x0810,0x1C38,0x366C,0x63C6,0xC003,0x8001,0x9999,0x8181,0x8421,0x8241,0x8181,0x8421,0xC003,0x6006,0x3FFC,0x0240},
    {0x0810,0x1C38,0x366C,0x63C6,0xC003,0x8101,0x9989,0x8181,0x8001,0x8241,0x8421,0x8001,0xC403,0x6606,0x3FFC,0x0C18},
    {0x0180,0x03C0,0x07E0,0x0DB0,0x1980,0x3180,0x6180,0xFFFF,0xFFFF,0x6180,0x3180,0x1980,0x0DB0,0x07E0,0x03C0,0x0180},
    {0x0180,0x03C0,0x07E0,0x0DB0,0x0180,0x0180,0x0180,0xFFFF,0xFFFF,0x6186,0x318C,0x1998,0x0DB0,0x07E0,0x03C0,0x0180},
    {0x0180,0x0380,0x0780,0x0FFF,0x1FFF,0x3FFF,0x7FFF,0xFFFF,0xFFFF,0x7FFF,0x3FFF,0x1FFF,0x0FFF,0x0780,0x0380,0x0180},
    {0x0180,0x01C0,0x01E0,0xFFF0,0xFFF8,0xFFFC,0xFFFE,0xFFFF,0xFFFF,0xFFFE,0xFFFC,0xFFF8,0xFFF0,0x01E0,0x01C0,0x0180},
    {0x8001,0xC003,0x6006,0x300C,0x1818,0x0C30,0x0660,0x03C0,0x03C0,0x0660,0x0C30,0x1818,0x300C,0x6006,0xC003,0x8001}
};

static uint8_t requested_face,active_face,draw_page,draw_x,drawing;
static uint8_t init_index,ready;
static uint32_t power_ready_ms;
/* SSD1306 128x64, internal charge pump, page addressing. Explicitly reset
 * start line, contrast and RAM-display mode rather than relying on POR. */
static const uint8_t __code init_commands[]={
    0xAE,0xD5,0x80,0xA8,0x3F,0xD3,0x00,0x40,
    0x8D,0x14,0x20,0x02,0xA1,0xC8,0xDA,0x12,
    0x81,0x7F,0xD9,0xF1,0xDB,0x40,0xA4,0xA6
};
static uint8_t face_column(uint8_t face, uint8_t x, uint8_t page)
{
    uint16_t mask;uint8_t value=0;
    if(face==12)return (x==0||x==127)?0xFF:((page==0)?0x01:((page==7)?0x80:((x&8)?0xAA:0x55)));
    if (face >= 12 || x < 32 || x >= 96) return 0;
    mask=(uint16_t)(0x8000u>>((x-32)>>2));
    if(sprites[face][page*2]&mask)value=0x0F;
    if(sprites[face][page*2+1]&mask)value|=0xF0;
    return value;
}

void oled_init(void)
{
#if !PETCARGO_OLED_ENABLED
    return;
#endif
    oled_sda(1); oled_scl(1);
    power_ready_ms=hal_millis()+200;init_index=0;ready=0;
    active_face=0xFF;drawing=0;oled_draw_face(FACE_SMUG);
}

void oled_draw_face(uint8_t face)
{
    requested_face=face;
}
void oled_test_pattern(void){requested_face=12;}
void oled_service(void)
{
    uint8_t i;
#if !PETCARGO_OLED_ENABLED
    return;
#endif
    if(!ready){
        if((int32_t)(hal_millis()-power_ready_ms)<0)return;
        if(init_index==0){
            /* Release a slave left mid-byte by an MCU-only reset. */
            oled_sda(1);
            for(i=0;i<9;i++){oled_scl(0);oled_delay();oled_scl(1);oled_delay();}
            oled_stop();
        }
        oled_command(init_commands[init_index++]);
        if(init_index==sizeof(init_commands))ready=1;
        return;
    }
    if(!drawing){if(active_face==requested_face)return;active_face=requested_face;draw_page=draw_x=0;drawing=1;}
    if(draw_x==0)oled_position(draw_page,0);
    oled_start();oled_write_byte(OLED_ADDRESS);oled_write_byte(0x40);
    for(i=0;i<16;i++)oled_write_byte(face_column(active_face,draw_x++,draw_page));
    oled_stop();
    if(draw_x==128){draw_x=0;if(++draw_page==8){drawing=0;oled_command(0xAF);}}
}
