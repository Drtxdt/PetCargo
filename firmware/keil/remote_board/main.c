#include "../vendor/inc/STC15F2K60S2.H"
#include "../vendor/inc/sys.H"
#include "../vendor/inc/displayer.h"
#include "../vendor/inc/adc.h"
#include "../vendor/inc/Key.H"
#include "../vendor/inc/uart1.h"

code unsigned long SysClock = 11059200UL;
code char decode_table[] = {
    0x3f,0x06,0x5b,0x4f,0x66,0x6d,0x7d,0x07,0x7f,0x6f,
    0x00,0x08,0x40,0x01,0x41,0x48,0xbf,0x86,0xdb,0xcf,
    0xe6,0xed,0xfd,0x87,0xff,0xef,0x76,0x38
};

#define CMD_IDLE 0
#define CMD_FORWARD 1
#define CMD_BACKWARD 2
#define CMD_LEFT 3
#define CMD_RIGHT 4
#define CMD_STOP 5

static unsigned char sequence;
static unsigned char active_command;
static unsigned char repeat_ticks;

static unsigned char checksum(unsigned char *p, unsigned char n)
{
    unsigned char v=0; while(n--)v^=*p++; return v;
}

static void send_command(unsigned char command)
{
    unsigned char packet[6]={0xA5,0x5A,0x02,0,0,0};
    if (GetUart1TxStatus()!=enumUart1TxFree) return;
    packet[3]=command; packet[4]=sequence++; packet[5]=checksum(packet,5);
    Uart1Print(packet,6);
}

static void show_command(unsigned char command)
{
    struct_ADC adc;
    unsigned int raw;
    adc=GetADC(); raw=adc.Nav; if(raw>999U)raw=999U;
    Seg7Print(10,command,10,10,10,(unsigned char)(raw/100),(unsigned char)((raw/10)%10),(unsigned char)(raw%10));
    switch(command){
    case CMD_FORWARD: LedPrint(0x18);break;
    case CMD_BACKWARD: LedPrint(0x81);break;
    case CMD_LEFT: LedPrint(0x0F);break;
    case CMD_RIGHT: LedPrint(0xF0);break;
    case CMD_STOP: LedPrint(0xFF);break;
    default: LedPrint(0);break;
    }
}

static void navigation_callback(void)
{
    unsigned char next=CMD_IDLE;
    if(GetAdcNavAct(enumAdcNavKeyUp)==enumKeyPress)next=CMD_FORWARD;
    else if(GetAdcNavAct(enumAdcNavKeyDown)==enumKeyPress)next=CMD_BACKWARD;
    else if(GetAdcNavAct(enumAdcNavKeyLeft)==enumKeyPress)next=CMD_LEFT;
    else if(GetAdcNavAct(enumAdcNavKeyRight)==enumKeyPress)next=CMD_RIGHT;
    else if(GetAdcNavAct(enumAdcNavKeyCenter)==enumKeyPress || GetAdcNavAct(enumAdcNavKey3)==enumKeyPress)next=CMD_STOP;
    else {
        if(GetAdcNavAct(enumAdcNavKeyUp)==enumKeyRelease || GetAdcNavAct(enumAdcNavKeyDown)==enumKeyRelease ||
           GetAdcNavAct(enumAdcNavKeyLeft)==enumKeyRelease || GetAdcNavAct(enumAdcNavKeyRight)==enumKeyRelease ||
           GetAdcNavAct(enumAdcNavKeyCenter)==enumKeyRelease || GetAdcNavAct(enumAdcNavKey3)==enumKeyRelease)
            next=CMD_IDLE;
        else return;
    }
    active_command=next; repeat_ticks=0; show_command(next); send_command(next);
}

static void repeat_callback(void)
{
    if(active_command!=CMD_IDLE && ++repeat_ticks>=1){repeat_ticks=0;send_command(active_command);}
}

static void bsp_init(void)
{
    DisplayerInit(); SetDisplayerArea(0,7); LedPrint(0);
    AdcInit(ADCexpEXT); Uart1Init(9600UL);
    SetEventCallBack(enumEventNav,navigation_callback);
    SetEventCallBack(enumEventSys100mS,repeat_callback);
    show_command(CMD_IDLE);
}

void main(void)
{
    bsp_init();
    MySTC_Init();
    while(1) MySTC_OS();
}
