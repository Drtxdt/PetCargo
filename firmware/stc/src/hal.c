#include "stc15.h"
#include "hal.h"
#include "config.h"
#include "runtime.h"

#define ADC_POWER   0x80
#define ADC_SPEED_L 0x20
#define ADC_FLAG    0x10
#define ADC_START   0x08
#define ADC_TIMEOUT 5000u
#define UART_RX_SIZE 64u
#define TIMER0_RELOAD (65536UL - (FOSC / 1000UL))

static volatile __xdata uint32_t system_ms;
static volatile uint8_t uart1_head;
static volatile uint8_t uart1_tail;
static volatile uint8_t uart2_head;
static volatile uint8_t uart2_tail;
static volatile uint8_t uart1_tx_done;
static volatile uint8_t uart2_tx_done;
static volatile uint16_t uart1_lost;
static volatile uint16_t uart2_lost;
static __xdata uint8_t uart1_rx[UART_RX_SIZE];
static __xdata uint8_t uart2_rx[UART_RX_SIZE];
static uint8_t adc_ok;
static __xdata uint8_t display_buffer[8];
static volatile __xdata uint8_t display_frames[2][9];
static volatile __data uint8_t display_front,display_pending,ui_phase;
static uint8_t led_value;
static __xdata tx_queue_t tx1,tx2;
static volatile uint8_t tx_lock;
static volatile uint16_t uart2_received;
static uint16_t buzzer_hz;
static volatile uint8_t ir_command;
static volatile uint8_t ir_ready;
static uint16_t ir_last_capture;
static uint32_t ir_bits;
static uint8_t ir_bit_count;
static uint8_t ir_receiving;

static void timer0_reload(void)
{
    TH0 = (uint8_t)(TIMER0_RELOAD >> 8);
    TL0 = (uint8_t)TIMER0_RELOAD;
}

void timer0_isr(void) __interrupt (1)
{
    /* No delay, subroutine calls or port-pin readback in the scan ISR. */
    P0=0;
    if(ui_phase==0&&display_pending){display_front=display_pending-1;display_pending=0;}
    /* These compile to ANL/ORL direct (latch RMW), never MOV A,P2. */
    P2 &= 0xF0;
    P2 |= ui_phase;
    P0=display_frames[display_front][ui_phase];
    if(++ui_phase==9)ui_phase=0;
    system_ms++;
}

void uart1_isr(void) __interrupt (4)
{
    if (RI) {
        uint8_t next;
        uint8_t value;
        RI = 0;
        value = SBUF;
        next = (uart1_head + 1u) & (UART_RX_SIZE - 1u);
        if (next == uart1_tail) uart1_lost++;
        else { uart1_rx[uart1_head] = value; uart1_head = next; }
    }
    if (TI) { uint8_t value; TI = 0; if(!tx_lock&&tx_next(&tx1,&value)){uart1_tx_done=0;SBUF=value;}else uart1_tx_done=1; }
}

void uart2_isr(void) __interrupt (8)
{
    if (S2CON & 0x01) {
        uint8_t next;
        uint8_t value;
        S2CON &= (uint8_t)~0x01;
        value = S2BUF;
        uart2_received++;
        next = (uart2_head + 1u) & (UART_RX_SIZE - 1u);
        if (next == uart2_tail) uart2_lost++;
        else { uart2_rx[uart2_head] = value; uart2_head = next; }
    }
    if (S2CON & 0x02) { uint8_t value; S2CON &= (uint8_t)~0x02; if(!tx_lock&&tx_next(&tx2,&value)){uart2_tx_done=0;S2BUF=value;}else uart2_tx_done=1; }
}

void pca_isr(void) __interrupt (7)
{
    if (CCON & 0x02) {
        uint8_t low = CCAP1L;
        uint16_t captured = ((uint16_t)CCAP1H << 8) | low;
        uint16_t delta = captured - ir_last_capture;
        ir_last_capture = captured;
        CCON &= (uint8_t)~0x02;
        if (delta >= 11000u && delta <= 14000u) {
            ir_bits = 0; ir_bit_count = 0; ir_receiving = 1;
        } else if (ir_receiving) {
            if (delta >= 700u && delta <= 1400u) {
                ir_bit_count++;
            } else if (delta >= 1600u && delta <= 2500u) {
                ir_bits |= (uint32_t)1u << ir_bit_count; ir_bit_count++;
            } else {
                ir_receiving = 0; ir_bit_count = 0;
            }
            if (ir_bit_count == 32) {
                uint8_t address = (uint8_t)ir_bits;
                uint8_t address_inv = (uint8_t)(ir_bits >> 8);
                uint8_t command = (uint8_t)(ir_bits >> 16);
                uint8_t command_inv = (uint8_t)(ir_bits >> 24);
                if (address == 0x50 && address_inv == 0xAF && command_inv == (uint8_t)~command) {
                    ir_command = command; ir_ready = 1;
                }
                ir_receiving = 0;
            }
        }
    }
    if (CCON & 0x80) CCON &= (uint8_t)~0x80;
}

static void ir_init(void)
{
    P3M1 &= (uint8_t)~0x40; P3M0 &= (uint8_t)~0x40; PIN_IR_RX = 1;
    P_SW1 = (uint8_t)((P_SW1 & (uint8_t)~0x30) | 0x10); /* CCP1_2=P3.6. */
    CCON = 0; CMOD = 0x00; CCAPM1 = 0x11; /* SYSclk/12, falling-edge capture, interrupt. */
    ir_last_capture = 0; ir_bits = 0; ir_bit_count = ir_receiving = ir_ready = 0;
    CL = 0; CH = 0; CCON |= 0x40; /* ECCF1 in CCAPM1 enables capture IRQ, not IE.6! */
}

static void uart_init(void)
{
    uint16_t reload = (uint16_t)(65536UL - (FOSC / 4UL / PETCARGO_UART_BAUD));
    P_SW2 &= (uint8_t)~0x01;
    P_SW1 &= (uint8_t)~0xC0;
    P1M1 &= (uint8_t)~0x03; P1M0 &= (uint8_t)~0x03; P1 |= 0x03;
    SCON = 0x50;
    S2CON = 0x50;
    T2H = (uint8_t)(reload >> 8);
    T2L = (uint8_t)reload;
    AUXR |= 0x15; /* Timer2 run, 1T, UART1 also uses Timer2. */
    uart1_head = uart1_tail = uart2_head = uart2_tail = 0;
    uart1_tx_done = uart2_tx_done = 1;
    uart1_lost = uart2_lost = 0;
    uart2_received=0;
    ES = 1;
    IE2 |= 0x01;
}

static void timer0_init(void)
{
    AUXR |= 0x80; /* Timer0 1T. */
    TMOD &= 0xF0; /* STC15 mode 0 is 16-bit AUTO reload. */
    timer0_reload();
    TF0 = 0;
    ET0 = 1;
    IP |= 0x02; /* Display preempts the lower-priority UART/PCA handlers. */
    TR0 = 1;
}

void hal_init(void)
{
    uint8_t i;
    EA = 0;
    IE=0;IE2=0;IP=0;
    system_ms = 0;
    P0M1 = 0x00; P0M0 = 0xFF; P0 = 0x00;
    P1M1 = 0x00; P1M0 = 0x00; P1 = 0xFF;
    P2M1 = 0x00; P2M0 = 0x08; P2 = 0xF0;
    P3M1 = 0x00; P3M0 = 0x10; P3 |= 0x0C;
    P4M1 &= (uint8_t)~0x18; P4M0 |= 0x18; P4 &= (uint8_t)~0x18;
    P5M1 &= (uint8_t)~0x30; P5M0 &= (uint8_t)~0x30; P5 |= 0x30;
    PIN_KEY1 = 1; PIN_KEY2 = 1; PIN_HALL = 1; PIN_VIB = 1;
    PIN_345_SCL = 1; PIN_345_SDA = 1;
    PIN_RTC_CLK = 0; PIN_RTC_IO = 1; PIN_RTC_RST = 0;
    PIN_EE_SDA = 1; PIN_EE_SCL = 1;
    PIN_LED_SEL = 0;
    for (i = 0; i < 8; i++) display_buffer[i] = 0;
    display_front=display_pending=ui_phase=led_value=0;
    for(i=0;i<9;i++){display_frames[0][i]=0;display_frames[1][i]=0;}
    ADC_CONTR = ADC_POWER | ADC_SPEED_L;
    adc_ok = 0;
    hal_buzzer_stop();
    uart_init();
    ir_init();
    timer0_init();
    EA = 1;
}

uint32_t hal_millis(void)
{
    uint32_t value;
    uint8_t saved=EA; EA = 0; value = system_ms; EA = saved;
    return value;
}

void hal_delay_us(uint8_t count)
{
    while (count--) {
        __asm nop __endasm;
        __asm nop __endasm;
        __asm nop __endasm;
    }
}

uint8_t hal_adc_read8(uint8_t channel)
{
    uint8_t mask;
    uint16_t timeout = ADC_TIMEOUT;
    channel &= 7;
    mask = (uint8_t)(1u << channel);
    P1M1 |= mask;
    P1M0 &= (uint8_t)~mask;
    ADC_CONTR = (uint8_t)(ADC_POWER | ADC_SPEED_L | channel);
    hal_delay_us(20);
    ADC_CONTR |= ADC_START;
    while (!(ADC_CONTR & ADC_FLAG)) {
        if (--timeout == 0) {
            ADC_CONTR = ADC_POWER | ADC_SPEED_L;
            adc_ok = 0;
            return 0;
        }
    }
    ADC_CONTR &= (uint8_t)~ADC_FLAG;
    adc_ok = 1;
    return ADC_RES;
}

uint8_t hal_adc_ok(void) { return adc_ok; }

int16_t hal_ntc_to_celsius_x10(uint8_t raw)
{
    static const uint8_t __code adc_points[7] = {196,171,142,128,113,88,67};
    static const int16_t __code temp_points[7] = {0,100,200,250,300,400,500};
    uint8_t i;
    if (raw >= adc_points[0]) return 0;
    if (raw <= adc_points[6]) return 500;
    for (i = 0; i < 6; i++) {
        if (raw <= adc_points[i] && raw >= adc_points[i + 1]) {
            int16_t span_t = temp_points[i + 1] - temp_points[i];
            uint8_t span_adc = adc_points[i] - adc_points[i + 1];
            return temp_points[i] + (int16_t)((uint16_t)(adc_points[i] - raw) * span_t / span_adc);
        }
    }
    return 250;
}

uint8_t hal_key1_down(void) { return PIN_KEY1 == 0; }
uint8_t hal_key2_down(void) { return PIN_KEY2 == 0; }
uint8_t hal_key3_down(uint8_t nav_adc) { return nav_is_k3(nav_adc); }
uint8_t hal_hall_near(void) { return PIN_HALL == 0; }
uint8_t hal_vibration_active(void) { return PIN_VIB == 0; }

static uint8_t encode_char(char c)
{
    static const uint8_t __code digits[10] = {0x3F,0x06,0x5B,0x4F,0x66,0x6D,0x7D,0x07,0x7F,0x6F};
    if (c >= '0' && c <= '9') return digits[(uint8_t)(c - '0')];
    switch (c) {
    case 'A': case 'a': return 0x77; case 'B': case 'b': return 0x7C;
    case 'C': case 'c': return 0x39; case 'D': case 'd': return 0x5E;
    case 'E': case 'e': return 0x79; case 'F': case 'f': return 0x71;
    case 'H': case 'h': return 0x76; case 'L': case 'l': return 0x38;
    case 'O': case 'o': return 0x5C; case 'P': case 'p': return 0x73;
    case 'R': case 'r': return 0x50; case 'T': case 't': return 0x78;
    case 'S': case 's': return 0x6D;
    case 'U': case 'u': return 0x3E; case 'Y': case 'y': return 0x6E;
    case '-': return 0x40; case '_': return 0x08; default: return 0;
    }
}

void hal_display_clear(void)
{
    uint8_t i; for (i = 0; i < 8; i++) display_buffer[i] = 0;
}

void hal_display_char(uint8_t position, char value)
{
    if (position < 8) display_buffer[position] = encode_char(value);
}

void hal_display_uint(uint8_t position, uint16_t value, uint8_t width)
{
    uint8_t i;
    for (i = 0; i < width; i++) {
        uint8_t target = position + width - 1u - i;
        if (target < 8) display_buffer[target] = encode_char((char)('0' + value % 10u));
        value /= 10u;
    }
}

void hal_led_pattern(uint8_t pattern) { led_value = pattern; }

void hal_display_commit(void)
{
    uint8_t i,back;
    if(display_pending)return;
    back=display_front^1;
    for(i=0;i<8;i++)display_frames[back][i]=display_buffer[i];
    display_frames[back][8]=led_value;
    display_pending=back+1;
}

void hal_buzzer_start(uint16_t hz)
{
    uint32_t counts;
    uint16_t reload;
    if (!hz) { hal_buzzer_stop(); return; }
    if (hz == buzzer_hz && TR1) return;
    counts = FOSC / (2UL * hz);
    if (counts < 1) counts = 1;
    if (counts > 65535UL) counts = 65535UL;
    reload = (uint16_t)(65536UL - counts);
    TR1 = 0; ET1 = 0; INT_CLKO &= (uint8_t)~0x02;
    AUXR |= 0x40;
    TMOD &= 0x0F; /* STC15 mode 0: 16-bit auto-reload, required by T1CLKO. */
    TH1 = (uint8_t)(reload >> 8); TL1 = (uint8_t)reload; TF1 = 0;
    INT_CLKO |= 0x02; TR1 = 1; buzzer_hz = hz;
}

void hal_buzzer_stop(void)
{
    TR1 = 0; ET1 = 0; INT_CLKO &= (uint8_t)~0x02; PIN_BEEP = 0; buzzer_hz = 0;
}

uint8_t hal_uart1_read(uint8_t *value)
{
    if (uart1_head == uart1_tail) return 0;
    *value = uart1_rx[uart1_tail];
    uart1_tail = (uart1_tail + 1u) & (UART_RX_SIZE - 1u);
    return 1;
}

uint8_t hal_uart2_read(uint8_t *value)
{
    if (uart2_head == uart2_tail) return 0;
    *value = uart2_rx[uart2_tail];
    uart2_tail = (uart2_tail + 1u) & (UART_RX_SIZE - 1u);
    return 1;
}

void hal_uart1_write(const uint8_t *data, uint8_t length)
{
    (void)hal_uart1_send(data,length,0);
}

uint8_t hal_uart1_send(const uint8_t *data,uint8_t length,uint8_t urgent)
{
    uint8_t ok;
    /* RX interrupts remain live even during frame copies. TX ISR pauses only
     * its queue consumer while the main loop uses the non-reentrant helpers. */
    tx_lock=1;
    ok=tx_enqueue(&tx1,data,length,urgent);
    tx_lock=0;
    if(uart1_tx_done)TI=1;
    if(uart2_tx_done)S2CON|=0x02;
    return ok;
}

void hal_uart2_write(const uint8_t *data, uint8_t length)
{
    tx_lock=1;
    tx_enqueue(&tx2,data,length,0);
    tx_lock=0;
    if(uart1_tx_done)TI=1;
    if(uart2_tx_done)S2CON|=0x02;
}

uint16_t hal_uart2_received(void){uint16_t v;uint8_t saved=EA;EA=0;v=uart2_received;EA=saved;return v;}
uint16_t hal_tx_dropped(void){return tx1.dropped+tx2.dropped;}

uint16_t hal_uart1_overflows(void) { uint16_t v;uint8_t saved=EA;EA=0;v=uart1_lost;EA=saved;return v; }
uint16_t hal_uart2_overflows(void) { uint16_t v;uint8_t saved=EA;EA=0;v=uart2_lost;EA=saved;return v; }

uint8_t hal_ir_read(uint8_t *command)
{
    if (!ir_ready) return 0;
    {uint8_t saved=EA;EA = 0; *command = ir_command; ir_ready = 0; EA = saved;} return 1;
}
