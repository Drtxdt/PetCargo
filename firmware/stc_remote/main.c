#include <stdint.h>
#include "stc15.h"

#define FOSC 11059200UL
#define TIMER0_RELOAD (65536UL-(FOSC/1000UL))
#define ADC_POWER 0x80
#define ADC_SPEED 0x20
#define ADC_FLAG 0x10
#define ADC_START 0x08

enum { CMD_IDLE=0,CMD_FORWARD=1,CMD_BACKWARD=2,CMD_LEFT=3,CMD_RIGHT=4,CMD_STOP=5 };
static volatile uint32_t system_ms;
static uint8_t display[8],scan,phase,led_pattern;

static void timer0_reload(void){TH0=(uint8_t)(TIMER0_RELOAD>>8);TL0=(uint8_t)TIMER0_RELOAD;}
void timer0_isr(void)__interrupt(1){timer0_reload();system_ms++;}

static uint8_t encode(char c)
{
    static const uint8_t __code d[10]={0x3F,0x06,0x5B,0x4F,0x66,0x6D,0x7D,0x07,0x7F,0x6F};
    if(c>='0'&&c<='9')return d[(uint8_t)(c-'0')];switch(c){case'A':return 0x77;case'D':return 0x5E;case'E':return 0x79;case'F':return 0x71;case'G':return 0x6F;case'I':return 0x06;case'L':return 0x38;case'N':return 0x54;case'P':return 0x73;case'R':return 0x50;case'S':return 0x6D;case'T':return 0x78;case'U':return 0x3E;case'W':return 0x2A;case'O':return 0x5C;case'-':return 0x40;default:return 0;}
}
static void set_text(const char*s){uint8_t i;for(i=0;i<8;i++)display[i]=s[i]?encode(s[i]):0;}
static void refresh(void)
{
    uint16_t spin=300;P0=0;if(phase==7){P2=0xF8;P0=led_pattern;phase=0;}else{P2=(uint8_t)(0xF0|scan);P0=display[scan];scan=(scan+1)&7;phase++;}while(spin--){__asm nop __endasm;}
}
static uint8_t adc_nav(void)
{
    uint16_t timeout=5000;P1M1|=0x80;P1M0&=(uint8_t)~0x80;ADC_CONTR=ADC_POWER|ADC_SPEED|7;ADC_CONTR|=ADC_START;
    while(!(ADC_CONTR&ADC_FLAG)&&--timeout){}if(!timeout)return 255;ADC_CONTR&=(uint8_t)~ADC_FLAG;return ADC_RES;
}
static uint8_t nav_command(uint8_t v)
{
    if(v<=28)return CMD_STOP;if(v<=76)return CMD_RIGHT;if(v<=116)return CMD_BACKWARD;if(v<=156)return CMD_STOP;if(v<=188)return CMD_LEFT;if(v<=228)return CMD_FORWARD;return CMD_IDLE;
}
static void wait_us(uint16_t us)
{
    while(us){uint16_t part=us>4000?4000:us;uint16_t reload=(uint16_t)(65536UL-((uint32_t)part*FOSC/1000000UL));TR1=0;TH1=(uint8_t)(reload>>8);TL1=(uint8_t)reload;TF1=0;TR1=1;while(!TF1){}TR1=0;TF1=0;us-=part;}
}
static void mark(uint16_t us)
{
    uint16_t cycles=(uint16_t)((us+12)/25);while(cycles--){PIN_IR_TX=1;wait_us(12);PIN_IR_TX=0;wait_us(13);}
}
static void send_byte(uint8_t value)
{
    uint8_t i;for(i=0;i<8;i++){mark(562);PIN_IR_TX=0;wait_us((value&1)?1687:562);value>>=1;}
}
static void ir_send(uint8_t command)
{
    mark(9000);PIN_IR_TX=0;wait_us(4500);send_byte(0x50);send_byte(0xAF);send_byte(command);send_byte((uint8_t)~command);mark(562);PIN_IR_TX=0;
}
static void show_command(uint8_t c)
{
    switch(c){case CMD_FORWARD:set_text("UP      ");led_pattern=0x18;break;case CMD_BACKWARD:set_text("DOWN    ");led_pattern=0x81;break;case CMD_LEFT:set_text("LEFT    ");led_pattern=0x0F;break;case CMD_RIGHT:set_text("RIGHT   ");led_pattern=0xF0;break;case CMD_STOP:set_text("STOP    ");led_pattern=0xFF;break;default:set_text("----    ");led_pattern=0;break;}
}
static void init(void)
{
    uint8_t i;EA=0;P0M1=0;P0M0=0xFF;P0=0;P2M1=0;P2M0=0x08;P2=0xF0;P3M1&=(uint8_t)~0x80;P3M0|=0x80;PIN_IR_TX=0;
    for(i=0;i<8;i++)display[i]=0;scan=phase=led_pattern=0;ADC_CONTR=ADC_POWER|ADC_SPEED;AUXR|=0xC0;TMOD=(TMOD&0x00)|0x11;timer0_reload();TF0=0;ET0=1;TR0=1;EA=1;show_command(CMD_IDLE);
}
void main(void)
{
    uint8_t candidate=CMD_IDLE,stable=CMD_IDLE,samples=0;uint32_t next_sample=0,next_send=0;init();
    while(1){uint32_t now=system_ms;refresh();if((int32_t)(now-next_sample)>=0){uint8_t c=nav_command(adc_nav());next_sample=now+10;if(c!=candidate){candidate=c;samples=1;}else if(samples<3)samples++;if(samples>=3&&stable!=candidate){stable=candidate;show_command(stable);if(stable==CMD_IDLE){ir_send(CMD_IDLE);ir_send(CMD_IDLE);}else{ir_send(stable);next_send=system_ms+100;}}}
        if(stable!=CMD_IDLE&&(int32_t)(system_ms-next_send)>=0){ir_send(stable);next_send=system_ms+100;}}
}
