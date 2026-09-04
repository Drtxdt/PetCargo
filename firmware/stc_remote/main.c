#include <stdint.h>
#include "stc15.h"
#include "remote.h"
#define FOSC 11059200UL
#define TIMER0_RELOAD (65536UL-(FOSC/1000UL))
static volatile __data uint32_t system_ms;
static volatile __data uint8_t front,pending,phase;
static volatile __xdata uint8_t frames[2][9];
static __xdata remote_keys_t keys;
static uint8_t adc_valid,sequence;
static uint32_t millis(void){uint32_t n;uint8_t saved=EA;EA=0;n=system_ms;EA=saved;return n;}
void timer0_isr(void)__interrupt(1){
    P0=0;if(phase==0&&pending){front=pending-1;pending=0;}
    P2&=0xF0;P2|=phase;P0=frames[front][phase];
    if(++phase==9)phase=0;system_ms++;
}
static uint8_t adc_nav(void){
    uint16_t timeout=5000;uint32_t selected_at;adc_valid=0;ADC_CONTR=0xA7;
    /* The resistor ladder needs acquisition time after selecting ADC7.  Match
       the course BSP's 1 ms settling delay; the display ISR keeps running. */
    selected_at=millis();while((int32_t)(millis()-selected_at)<1){}
    ADC_CONTR|=0x08;
    while(!(ADC_CONTR&0x10)&&--timeout){}
    if(!timeout)return 255;ADC_CONTR&=(uint8_t)~0x10;adc_valid=1;return ADC_RES;
}
static void uart_send(uint8_t direction){
    uint8_t i,packet[6]={0xA5,0x5A,0x02,direction,sequence++,0};
    packet[5]=remote_checksum(packet,5);
    for(i=0;i<6;i++){TI=0;SBUF=packet[i];while(!TI){}}
}
static void display_update(uint8_t raw){
    static const uint8_t __code digit[]={0x3F,0x06,0x5B,0x4F,0x66,0x6D,0x7D,0x07,0x7F,0x6F};
    static const uint8_t __code leds[]={0,0x18,0x81,0x0F,0xF0,0xFF};
    uint8_t back,i;if(pending)return;back=front^1;
    for(i=0;i<9;i++)frames[back][i]=0;
    frames[back][0]=0x39;frames[back][1]=digit[keys.stable]; /* Cn */
    frames[back][4]=adc_valid?0x77:0x79; /* Axxx or E001 */
    if(!adc_valid)raw=1;
    frames[back][5]=digit[raw/100];frames[back][6]=digit[(raw/10)%10];frames[back][7]=digit[raw%10];
    frames[back][8]=leds[keys.stable];pending=back+1;
}
static void init(void){
    uint16_t baud_reload=(uint16_t)(65536UL-(FOSC/4UL/9600UL));uint8_t i;
    EA=0;IE=0;IE2=0;IP=0;
    P0M1=0;P0M0=0xFF;P0=0;P2M1=0;P2M0=8;P2=0xF0;
    P1M1|=0x80;P1M0&=(uint8_t)~0x80;P1|=0x80;
    P3M1&=(uint8_t)~0x03;P3M0&=(uint8_t)~0x03;P3|=0x03;
    P_SW1&=(uint8_t)~0xC0;SCON=0x50;T2H=(uint8_t)(baud_reload>>8);T2L=(uint8_t)baud_reload;
    AUXR|=0x95; /* T0 1T; T2 1T/run; UART1 uses T2. */
    TMOD&=0xF0;TH0=(uint8_t)(TIMER0_RELOAD>>8);TL0=(uint8_t)TIMER0_RELOAD;
    for(i=0;i<9;i++)frames[0][i]=frames[1][i]=0;
    ADC_CONTR=0xA0;TF0=0;ET0=1;TR0=1;EA=1;
}
void main(void){
    uint32_t next_sample=10,now;uint8_t raw=255,command;
    init();while(1){now=millis();
        if((int32_t)(now-next_sample)>=0){next_sample=now+10;raw=adc_nav();remote_sample(&keys,raw,adc_valid,now);display_update(raw);}
        command=remote_next(&keys,now);if(command!=0xFF)uart_send(command);
    }
}
