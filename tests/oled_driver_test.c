/* Execute the production driver; observe the inverted SM wire levels at
 * every explicit delay. This tests digital protocol, not electrical rise time. */
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#define PETCARGO_STC15_H
#define __code
static uint8_t PIN_SM_S1,PIN_SM_S2;
static uint32_t now;
static unsigned frames,data_bytes,commands,lit_bytes;
static uint8_t old_scl=1,old_sda=1,in_frame,bits,byte,frame[32],length;
uint32_t hal_millis(void){return now;}
void hal_delay_us(uint8_t count){
    uint8_t scl=!PIN_SM_S1,sda=!PIN_SM_S2;
    assert(count>=5);
    if(old_scl&&scl&&old_sda!=sda){
        if(!sda){assert(!in_frame);in_frame=1;bits=byte=length=0;}
        else if(in_frame){
            assert(length>=3&&frame[0]==0x78);
            if(frame[1]==0x40){
                assert(length==18);data_bytes+=16;
                for(unsigned j=2;j<length;j++)if(frame[j])lit_bytes++;
            }else{assert(frame[1]==0&&length==3);commands++;}
            frames++;in_frame=0;
        }
    }
    if(!old_scl&&scl&&in_frame){
        if(bits<8){byte=(uint8_t)((byte<<1)|sda);bits++;}
        else{assert(length<sizeof(frame));frame[length++]=byte;bits=byte=0;}
    }
    old_scl=scl;old_sda=sda;
}
#include "../firmware/stc/src/oled.c"
int main(void){
    oled_init();oled_test_pattern();
    for(now=0;now<200;now++)oled_service();
    assert(frames==0);
    for(unsigned j=0;j<100;j++)oled_service();
    assert(data_bytes==1024&&lit_bytes>0&&ready&&!drawing);
    assert(commands==sizeof(init_commands)+24+1);
    unsigned before=frames;
    for(unsigned j=0;j<100;j++)oled_service();
    assert(frames==before); /* no endless refresh when unchanged */
    oled_draw_face(FACE_HAPPY);
    for(unsigned j=0;j<64;j++)oled_service();
    assert(data_bytes==2048&&!drawing);
    puts("PASS production OLED inverted I2C frames, delayed init, 16-byte chunks and full frame");
    return 0;
}
