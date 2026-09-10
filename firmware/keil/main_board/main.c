#include "../vendor/inc/STC15F2K60S2.H"
#include "../vendor/inc/sys.H"
#include "../vendor/inc/displayer.h"
#include "../vendor/inc/Beep.h"
#include "../vendor/inc/music.h"
#include "../vendor/inc/Key.H"
#include "../vendor/inc/Vib.h"
#include "../vendor/inc/hall.H"
#include "../vendor/inc/adc.h"
#include "../vendor/inc/DS1302.h"
#include "../vendor/inc/M24C02.h"
#include "../vendor/inc/uart1.h"
#include "../vendor/inc/uart2.h"

/* Course/BSP build of PetCargo. The proven SDCC image remains the hardware image. */
code unsigned long SysClock = 11059200UL;
code char decode_table[] = {
    0x3f,0x06,0x5b,0x4f,0x66,0x6d,0x7d,0x07,0x7f,0x6f,
    0x00,0x08,0x40,0x01,0x41,0x48,0xbf,0x86,0xdb,0xcf,
    0xe6,0xed,0xfd,0x87,0xff,0xef,0x76,0x38
};

#define PROTOCOL_VERSION 2
#define MSG_HEARTBEAT 0x02
#define MSG_TELEMETRY 0x10
#define MSG_EVENT 0x11
#define MSG_MOTION_REQUEST 0x20
#define MSG_STOP 0x31
#define EVENT_BRIGHT_LIGHT 1
#define EVENT_SHAKE 2
#define EVENT_FEED 3
#define EVENT_VOICE 5
#define MOTION_LINEAR 1
#define MOTION_ROTATE 2
#define MOTION_LATERAL 3

static xdata unsigned char uart1_byte[1];
static xdata unsigned char uart2_frame[4];
static xdata unsigned char uart2_head[2];
static xdata unsigned char pc_frame[40];
static xdata unsigned char pc_pos;
static xdata unsigned char pc_need;
static xdata unsigned char tx_seq;
static xdata unsigned long uptime_ms;
static xdata unsigned int light_raw;
static xdata unsigned int feed_count;
static xdata unsigned char happiness;
static xdata unsigned char fear;
static xdata unsigned char sleeping;
static xdata unsigned char display_page;
static xdata unsigned char light_ticks;
static xdata unsigned char light_armed;
static xdata unsigned char rearm_ticks;
static xdata unsigned char music_note;
static xdata unsigned char music_playing;

static unsigned int crc16(unsigned char *bytes, unsigned char length)
{
    unsigned char i;
    unsigned int crc = 0xFFFFU;
    while (length--) {
        crc ^= (unsigned int)(*bytes++) << 8;
        for (i = 0; i < 8; ++i)
            crc = (crc & 0x8000U) ? (crc << 1) ^ 0x1021U : crc << 1;
    }
    return crc;
}

static void put16(unsigned char *p, unsigned int v)
{
    p[0] = (unsigned char)v;
    p[1] = (unsigned char)(v >> 8);
}

static void put32(unsigned char *p, unsigned long v)
{
    p[0]=(unsigned char)v; p[1]=(unsigned char)(v>>8);
    p[2]=(unsigned char)(v>>16); p[3]=(unsigned char)(v>>24);
}

static void send_frame(unsigned char type, unsigned char *payload, unsigned char length)
{
    unsigned char i;
    unsigned int crc;
    static xdata unsigned char frame[40];
    if (length > 32 || GetUart1TxStatus() != enumUart1TxFree) return;
    frame[0]=0xAA; frame[1]=0x55; frame[2]=PROTOCOL_VERSION;
    frame[3]=type; frame[4]=tx_seq++; frame[5]=length;
    for (i=0; i<length; ++i) frame[6+i]=payload[i];
    crc=crc16(frame+2, (unsigned char)(4+length));
    frame[6+length]=(unsigned char)crc;
    frame[7+length]=(unsigned char)(crc>>8);
    Uart1Print(frame, (unsigned int)(8+length));
}

static void send_event(unsigned char event, int value)
{
    unsigned char p[3]; p[0]=event; put16(p+1,(unsigned int)value);
    send_frame(MSG_EVENT,p,3);
}

static void send_motion(unsigned char kind, long amount, unsigned int speed)
{
    unsigned char p[8];
    p[0]=tx_seq; p[1]=kind; put32(p+2,(unsigned long)amount); put16(p+6,speed);
    send_frame(MSG_MOTION_REQUEST,p,8);
}

static void stop_all(void)
{
    unsigned char reason=0; music_playing=0; music_note=0;
    send_frame(MSG_STOP,&reason,1);
}

static void voice_action(unsigned char cmd)
{
    send_event(EVENT_VOICE,(int)cmd);
    switch (cmd) {
    case 1: stop_all(); break;
    case 2: send_motion(MOTION_LINEAR,500L,180); break;
    case 3: send_motion(MOTION_LINEAR,-500L,180); break;
    case 4: send_motion(MOTION_LATERAL,500L,180); break;
    case 5: send_motion(MOTION_LATERAL,-500L,180); break;
    case 6: send_motion(MOTION_ROTATE,360000L,500); break;
    case 7: sleeping=1; stop_all(); break;
    case 8: sleeping=0; break;
    case 9: happiness=(happiness<80)?80:happiness; break;
    case 10: music_note=0; music_playing=1; break;
    case 11: happiness=(happiness<70)?70:happiness; break;
    case 12: display_page=0; break;
    }
}

static void uart2_callback(void)
{
    unsigned char cmd=uart2_frame[2];
    if (uart2_frame[0]==0xA5 && uart2_frame[1]==0x5A &&
        cmd>=1 && cmd<=12 && uart2_frame[3]==(unsigned char)(cmd^0xFF))
        voice_action(cmd);
}

/* The BSP receives one PC byte per event; this parser deliberately keeps v2 framing. */
static void uart1_callback(void)
{
    unsigned char b=uart1_byte[0];
    if (pc_pos==0) { if (b==0xAA) pc_frame[pc_pos++]=b; return; }
    if (pc_pos==1) { if (b==0x55) pc_frame[pc_pos++]=b; else pc_pos=0; return; }
    pc_frame[pc_pos++]=b;
    if (pc_pos==6) {
        if (pc_frame[2]!=PROTOCOL_VERSION || pc_frame[5]>32) { pc_pos=0; return; }
        pc_need=(unsigned char)(8+pc_frame[5]);
    }
    if (pc_need && pc_pos>=pc_need) {
        unsigned int expected=(unsigned int)pc_frame[pc_need-2] | ((unsigned int)pc_frame[pc_need-1]<<8);
        if (crc16(pc_frame+2,(unsigned char)(pc_need-4))==expected && pc_frame[3]==MSG_STOP)
            stop_all();
        pc_pos=0; pc_need=0;
    }
}

static void key_callback(void)
{
    if (GetKeyAct(enumKey1)==enumKeyPress) display_page=(display_page+1)%3;
    if (GetKeyAct(enumKey2)==enumKeyPress) sleeping=!sleeping;
}

static void nav_callback(void)
{
    if (GetAdcNavAct(enumAdcNavKey3)==enumKeyPress) stop_all();
}

static void hall_callback(void)
{
    if (GetHallAct()==enumHallGetClose) {
        if (feed_count<65535U) ++feed_count;
        if (happiness<=80) happiness+=20; else happiness=100;
        fear=(fear>20)?fear-20:0;
        M24C02_Write(0,(unsigned char)feed_count);
        M24C02_Write(1,(unsigned char)(feed_count>>8));
        send_event(EVENT_FEED,(int)feed_count);
    }
}

static void vibration_callback(void)
{
    if (GetVibAct()==enumVibQuake) {
        if (fear<75) fear=75;
        stop_all(); send_event(EVENT_SHAKE,1);
    }
}

static void display_status(void)
{
    unsigned int v;
    struct_DS1302_RTC rtc;
    if (display_page==0) {
        v=light_raw; if(v>999U)v=999U;
        Seg7Print(27,10,(unsigned char)(v/100),(unsigned char)((v/10)%10),(unsigned char)(v%10),10,(unsigned char)(fear/10),(unsigned char)(fear%10));
    } else if (display_page==1) {
        Seg7Print(26,10,(unsigned char)(happiness/100),(unsigned char)((happiness/10)%10),(unsigned char)(happiness%10),10,10,10);
    } else {
        rtc=RTC_Read();
        Seg7Print((unsigned char)(rtc.hour>>4),(unsigned char)(rtc.hour&15),12,(unsigned char)(rtc.minute>>4),(unsigned char)(rtc.minute&15),12,(unsigned char)(rtc.second>>4),(unsigned char)(rtc.second&15));
    }
    LedPrint((unsigned char)((happiness>=80)?0xFF:((fear>=75)?0x81:0x18)));
}

static void music_step(void)
{
    static code unsigned int notes[8]={262,294,330,349,392,440,494,523};
    if (!music_playing || GetBeepStatus()!=enumBeepFree) return;
    if (music_note>=8) { music_playing=0; music_note=0; return; }
    SetBeep(notes[music_note++],22);
}

static void tick_100ms(void)
{
    unsigned char p[20];
    static unsigned char bright_reply[4]={0x5A,0xA5,0x80,0x7F};
    struct_ADC adc;
    adc=GetADC();
    light_raw=adc.Rop;
    if (light_raw>=20U && light_armed) {
        if (++light_ticks>=2) {
            light_armed=0; light_ticks=0; fear=(fear<80)?80:fear;
            if(GetUart2TxStatus()==enumUart2TxFree) Uart2Print(bright_reply,4);
            SetBeep(1200,20); send_event(EVENT_BRIGHT_LIGHT,(int)light_raw);
            send_motion(MOTION_LINEAR,-500L,180);
        }
    } else light_ticks=0;
    if (!light_armed && light_raw<=15U) { if(++rearm_ticks>=20){light_armed=1;rearm_ticks=0;} }
    else rearm_ticks=0;
    music_step(); display_status();
    if ((uptime_ms%500UL)==0 && GetUart1TxStatus()==enumUart1TxFree) {
        put32(p,uptime_ms); p[4]=(unsigned char)light_raw; put16(p+5,adc.Rt);
        put16(p+7,0);put16(p+9,0);put16(p+11,0);p[13]=0;p[14]=0;
        p[15]=happiness;p[16]=fear;p[17]=sleeping?1:0;put16(p+18,feed_count);
        send_frame(MSG_TELEMETRY,p,20);
    }
}

static void tick_1ms(void) { ++uptime_ms; }

static void bsp_init(void)
{
    struct_DS1302_RTC rtc={0x00,0x00,0x12,0x01,0x01,0x01,0x26};
    happiness=50; light_armed=1;
    uart2_head[0]=0xA5; uart2_head[1]=0x5A;
    DisplayerInit(); SetDisplayerArea(0,7); LedPrint(0);
    KeyInit(); AdcInit(ADCexpEXT); BeepInit(); MusicPlayerInit();
    HallInit(); VibInit(); DS1302Init(rtc);
    feed_count=(unsigned int)M24C02_Read(0)|((unsigned int)M24C02_Read(1)<<8);
    Uart1Init(9600UL); SetUart1Rxd(uart1_byte,1,0,0);
    Uart2Init(9600UL,Uart2UsedforEXT); SetUart2Rxd(uart2_frame,4,uart2_head,2);
    SetEventCallBack(enumEventSys1mS,tick_1ms);
    SetEventCallBack(enumEventSys100mS,tick_100ms);
    SetEventCallBack(enumEventKey,key_callback);
    SetEventCallBack(enumEventNav,nav_callback);
    SetEventCallBack(enumEventHall,hall_callback);
    SetEventCallBack(enumEventVib,vibration_callback);
    SetEventCallBack(enumEventUart1Rxd,uart1_callback);
    SetEventCallBack(enumEventUart2Rxd,uart2_callback);
}

void main(void)
{
    bsp_init();
    MySTC_Init();
    while (1) MySTC_OS();
}
