/* Executes production C with fake peripherals, not a CPU/hardware emulator. */
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "runtime.h"
#include "hal.h"
#include "music.h"
#include "devices.h"
#include "oled.h"
#include "protocol.h"
static uint32_t mock_now;
static uint16_t mock_hz,lost;
static uint8_t mock_k1,mock_k2,mock_nav=255,mock_light=10,mock_adc_ok=1;
static uint8_t rx[100],rx_n,rx_pos,send_ok=1,last_type,last_urgent;
static uint16_t motion_requests,bright_events;
uint32_t hal_millis(void){return mock_now;}
void hal_buzzer_start(uint16_t hz){mock_hz=hz;}
void hal_buzzer_stop(void){mock_hz=0;}
uint8_t hal_key1_down(void){return mock_k1;}
uint8_t hal_key2_down(void){return mock_k2;}
uint8_t hal_key3_down(uint8_t v){return nav_is_k3(v);}
uint8_t hal_adc_read8(uint8_t c){return c==7?mock_nav:(c==4?mock_light:128);}
uint8_t hal_adc_ok(void){return mock_adc_ok;}
int16_t hal_ntc_to_celsius_x10(uint8_t v){return 250;}
uint8_t hal_hall_near(void){return 0;}
uint8_t hal_vibration_active(void){return 0;}
uint8_t hal_uart1_read(uint8_t *v){return 0;}
uint8_t hal_uart2_read(uint8_t *v){if(rx_pos==rx_n)return 0;*v=rx[rx_pos++];return 1;}
uint16_t hal_uart1_overflows(void){return 0;}
uint16_t hal_uart2_overflows(void){return lost;}
uint8_t hal_ir_read(uint8_t *v){return 0;}
void hal_uart2_write(const uint8_t *p,uint8_t n){}
uint8_t hal_uart1_send(const uint8_t *p,uint8_t n,uint8_t urgent){last_type=p[3];last_urgent=urgent;if(send_ok&&last_type==MSG_MOTION_REQUEST)motion_requests++;if(last_type==MSG_EVENT&&p[6]==EVENT_BRIGHT_LIGHT)bright_events++;return send_ok;}
void hal_display_clear(void){}
void hal_display_char(uint8_t p,char c){}
void hal_display_uint(uint8_t p,uint16_t v,uint8_t w){}
void hal_display_commit(void){}
void hal_led_pattern(uint8_t p){}
void oled_init(void){}
void oled_draw_face(uint8_t f){}
void oled_service(void){}
uint8_t adxl345_init(void){return 0;}
void adxl345_read(accel_sample_t *s){s->available=0;s->shock=0;}
uint8_t rtc_read_hms(uint8_t*h,uint8_t*m,uint8_t*s){return 0;}
void persistence_load(persisted_settings_t*s){memset(s,0,sizeof(*s));}
void persistence_save(persisted_settings_t*s){}
void persistence_service(void){}
#include "../firmware/stc/src/petcargo.c"
static void test_buttons(void)
{
    button_state_t b={0};
    assert(!button_update(&b,1,1,0));assert(!button_update(&b,0,1,10));
    assert(!button_update(&b,1,1,20));assert(button_update(&b,1,1,50)==BUTTON_PRESS);
    assert(!button_update(&b,1,1,1249));assert(button_update(&b,1,1,1250)==BUTTON_LONG);
    assert(!button_update(&b,1,1,1300));assert(!button_update(&b,0,1,1310));assert(!button_update(&b,0,1,1340));
    memset(&b,0,sizeof(b));button_update(&b,1,1,2000);assert(button_update(&b,1,1,2030)==BUTTON_PRESS);
    button_update(&b,0,1,2100);assert(button_update(&b,0,1,2130)==BUTTON_SHORT);
    memset(&b,0,sizeof(b));button_update(&b,1,1,0);assert(!button_update(&b,1,0,30));assert(!button_update(&b,1,1,40));assert(button_update(&b,1,1,60)==BUTTON_PRESS);
    assert(nav_is_k3(0)&&nav_is_k3(28)&&!nav_is_k3(29)&&!nav_is_k3(255));
    puts("PASS button bounce, short/long, invalid ADC, K3 boundaries");
}
static void test_parser(void)
{
    csk_parser_t p={0};uint8_t c;unsigned i;
    assert(!csk_feed(&p,10,0));
    for(c=1;c<=12;c++){assert(!csk_feed(&p,0xA5,10));assert(!csk_feed(&p,0x5A,11));assert(!csk_feed(&p,c,12));assert(csk_feed(&p,c^255,13)==c);}
    assert(p.frames==12);csk_feed(&p,0xA5,20);csk_feed(&p,0x5A,21);csk_expire(&p,121);assert(p.state==0&&p.errors==1);
    {const uint8_t seq[]={0xA5,0x5A,10,0xA5,0x5A,8,0xF7};for(i=0;i<sizeof(seq);i++)c=csk_feed(&p,seq[i],200+i);assert(c==8&&p.frames==13&&p.errors==2);}
    puts("PASS 12 voice frames, standalone-byte rejection, timeout, resynchronization");
}
static void test_queue(void)
{
    tx_queue_t q={0};uint8_t out,normal[]={1,2,3},stale[]={4,5},stop[]={9,8};
    assert(tx_enqueue(&q,normal,3,0));assert(tx_next(&q,&out)&&out==1);
    assert(tx_enqueue(&q,stale,2,0));assert(tx_enqueue(&q,stale,2,0));assert(tx_enqueue(&q,stale,2,0));
    assert(!tx_enqueue(&q,stale,2,0)&&q.dropped==1);
    assert(tx_enqueue(&q,stop,2,1));assert(tx_next(&q,&out)&&out==2);assert(tx_next(&q,&out)&&out==3);
    assert(tx_next(&q,&out)&&out==9);assert(tx_next(&q,&out)&&out==8);assert(!tx_next(&q,&out));
    assert(!tx_enqueue(&q,normal,41,0));
    puts("PASS whole-frame queue congestion, reserved STOP, stale-request cancellation");
}
static void test_music(void)
{
    uint32_t total=0;uint16_t rem=0,ticks=0;unsigned i,j;
    for(i=0;i<petcargo_song_count;i++){assert(petcargo_song[i].ticks);ticks+=petcargo_song[i].ticks;}
    assert(ticks==162);
    for(j=0;j<1000;j++)for(i=0;i<petcargo_song_count;i++)total+=music_duration(petcargo_song[i].ticks,&rem);
    assert(total==(uint32_t)(162ULL*15000*1000/136));
    music_stop();assert(!music_start(0,1)&&!music_playing()&&!mock_hz);
    assert(music_start(0,0)&&mock_hz==1200);music_update(300);assert(!mock_hz);music_update(500);assert(mock_hz==523);
    for(i=501;i<60000;i++)music_update(i);assert(music_playing());music_stop();music_update(70000);assert(!mock_hz&&!music_playing());
    printf("PASS score %u notes / %u ticks, 1000-loop timing, locked start, stop\n",petcargo_song_count,ticks);
}
static void press_cycle(uint8_t which,uint32_t t)
{
    mock_now=t;if(which==1)mock_k1=1;else mock_nav=0;handle_inputs(t);handle_inputs(t+30);
    if(which==1)mock_k1=0;else mock_nav=255;handle_inputs(t+70);handle_inputs(t+100);
}
static void test_application(void)
{
    petcargo_init();mock_now=1000;voice_command(10);assert(music_playing());voice_command(1);assert(!music_playing()&&!app.emergency);
    press_cycle(1,1100);assert(app.display_page==1);
    voice_command(10);press_cycle(3,1300);assert(app.emergency&&!music_playing());voice_command(10);assert(!music_playing());
    mock_nav=0;handle_inputs(1500);handle_inputs(1530);handle_inputs(2730);assert(!app.emergency);mock_nav=255;handle_inputs(2800);handle_inputs(2830);
    mock_adc_ok=0;mock_nav=0;handle_inputs(2900);handle_inputs(3000);assert(!app.emergency);mock_adc_ok=1;mock_nav=255;
    voice_command(10);voice_command(7);assert(app.sleeping&&!music_playing());voice_command(2);assert(!app.motion_active);voice_command(8);assert(!app.sleeping);
    send_ok=0;voice_command(2);assert(!app.motion_active);send_ok=1;
    memcpy(rx,(uint8_t[]){0xA5,0x5A,10,0xF5},4);rx_n=4;rx_pos=0;mock_now=4000;poll_csk();assert(music_playing()&&app.last_voice_command==10);
    music_stop();rx_pos=0;lost++;poll_csk();assert(!music_playing());
    mock_light=19;sample_light(5000);sample_light(5500);assert(!bright_events);
    mock_light=20;sample_light(6000);sample_light(6199);assert(!bright_events);sample_light(6200);assert(bright_events==1&&app.fear>=80&&app.motion_active);
    sample_light(6500);assert(bright_events==1);mock_light=15;sample_light(7000);sample_light(8999);assert(!light_armed);sample_light(9000);assert(light_armed);
    send_stop(1);assert(last_type==MSG_STOP&&last_urgent);
    puts("PASS application: K1/K3, voice music/stop/sleep, locked rejection, RX overflow, raw L19/20/15");
}
int main(void){test_buttons();test_parser();test_queue();test_music();test_application();puts("ALL PRODUCTION C TESTS PASSED");return 0;}
