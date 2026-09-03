/* Standalone bench firmware. ASCII UART1 only; NEVER link to the ROS bridge. */
#include "runtime.h"
#include "hal.h"
#include "oled.h"
#include "music.h"
void timer0_isr(void) __interrupt(1);
void uart1_isr(void) __interrupt(4);
void uart2_isr(void) __interrupt(8);
void pca_isr(void) __interrupt(7);
static __xdata button_state_t keys[3];
static __xdata csk_parser_t parser;
static __xdata char line[40];
static uint8_t used,nav,nav_valid,k1,k2,locked,can_release,page,rejected;
static uint16_t events[3],last_lost;
static uint32_t next_keys,next_log,next_ui;
static void ch(char c){if(used<sizeof(line))line[used++]=c;}
static void str(const char *s){while(*s)ch(*s++);}
static void number(uint16_t v){uint16_t d=10000;uint8_t started=0;while(d){uint8_t digit=v/d;if(digit||started||d==1){ch('0'+digit);started=1;}v%=d;d/=10;}}
static void hex(uint8_t v){const char __code *digits="0123456789ABCDEF";ch(digits[v>>4]);ch(digits[v&15]);}
static void flush(void){ch('\r');ch('\n');hal_uart1_write((uint8_t*)line,used);used=0;}
static void report(void)
{
    uint8_t i;
    str("K1=");number(!k1);str(" K2=");number(!k2);str(" ADC7=");number(nav);str(" ok=");number(nav_valid);str(" lock=");number(locked);flush();
    str("RX=");number(hal_uart2_received());str(" F=");number(parser.frames);str(" E=");number(parser.errors);str(" O=");number(last_lost);flush();
    str("raw=");for(i=0;i<4;i++)hex(parser.last[i]);str(" C=");number(parser.last_command);str(" rej=");number(rejected);str(" M=");number(music_playing());flush();
    str("ev=");for(i=0;i<3;i++){number(events[i]%1000);ch('/');}str(" D=");number(hal_tx_dropped());flush();
}
void main(void)
{
    uint8_t v,c,e1,e2,e3,i;uint32_t now;
    hal_init();oled_init();oled_test_pattern();music_beep(1200,300,hal_millis());
    while(1){
        now=hal_millis();
        if((int32_t)(now-next_keys)>=0){
            next_keys=now+10;k1=hal_key1_down();k2=hal_key2_down();nav=hal_adc_read8(ADC_CH_KEY3);nav_valid=hal_adc_ok();
            e1=button_update(&keys[0],k1,1,now);e2=button_update(&keys[1],k2,1,now);e3=button_update(&keys[2],nav_is_k3(nav),nav_valid,now);
            if(e1)events[0]++;if(e2)events[1]++;if(e3)events[2]++;
            if(e1&BUTTON_SHORT)page^=1;
            if(e2&BUTTON_LONG){if(music_playing())music_stop();else music_start(now,locked);}
            if(e3&BUTTON_PRESS){can_release=locked;locked=1;music_stop();}
            if((e3&BUTTON_LONG)&&can_release){locked=0;can_release=0;}
        }
        if(last_lost!=hal_uart2_overflows()){last_lost=hal_uart2_overflows();parser.state=0;parser.errors++;while(hal_uart2_read(&v)){} }
        csk_expire(&parser,now);
        while(hal_uart2_read(&v)){
            c=csk_feed(&parser,v,now);if(!c)continue;rejected=0;
            if(c==10){if(!music_start(now,locked))rejected=1;}
            else if(c==1||c==7)music_stop();
            else if(c==8)music_beep(1100,90,now);
        }
        music_update(now);oled_service();
        if((int32_t)(now-next_ui)>=0){
            next_ui=now+50;hal_display_clear();
            if(!page){for(i=0;i<8;i++)hal_display_char(i,'1'+i);}
            else{hal_display_char(0,'U');hal_display_uint(1,parser.last_command,3);hal_display_char(4,'A');hal_display_uint(5,nav,3);}
            hal_led_pattern((locked?0x80:0)|(k1?1:0)|(k2?2:0)|(nav_valid&&nav_is_k3(nav)?4:0));hal_display_commit();
        }
        if((int32_t)(now-next_log)>=0){next_log=now+500;report();}
    }
}
