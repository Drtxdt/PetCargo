#include <stdint.h>
#include "hal.h"
#include "config.h"
#include "protocol.h"
#include "devices.h"
#include "oled.h"
#include "petcargo.h"

#define BUTTON_PRESS 1
#define BUTTON_SHORT 2
#define BUTTON_LONG 4
#define CSK_HEADER_1 0xA5
#define CSK_HEADER_2 0x5A
#define IR_JOG_SPEED 120u
#define IR_JOG_LEASE 300u

typedef struct { uint8_t stable,candidate,long_sent; uint32_t changed_at,pressed_at; } button_state_t;
typedef struct {
    uint8_t light_raw; int16_t temp_x10,temp_baseline_x10; accel_sample_t accel;
    uint8_t vibration,hall,happiness,fear,sleeping,emergency,display_page,adxl_ok,rtc_ok;
    uint8_t rtc_hour,rtc_minute,rtc_second,motion_active,motion_face,remote_direction;
    uint16_t motion_id,last_motion_id;
} app_state_t;
typedef struct { uint16_t hz,duration_ms; } music_note_t;

static const music_note_t __code song[] = {
    {659,250},{659,250},{587,250},{523,500},{523,250},{587,250},{659,500},{784,500},
    {659,250},{659,250},{587,250},{523,500},{523,250},{587,250},{659,500},{494,500},
    {523,250},{587,250},{659,250},{784,250},{880,500},{784,250},{659,250},{587,500},
    {523,250},{587,250},{659,250},{784,250},{659,500},{587,250},{523,250},{494,750}
};
#define SONG_NOTES ((uint8_t)(sizeof(song)/sizeof(song[0])))

static __xdata app_state_t app;
static __xdata persisted_settings_t settings;
static __xdata button_state_t button_k1,button_k2,button_k3;
static uint8_t k3_can_release,light_armed,hall_previous,csk_parser_state,csk_command,last_oled_face;
static uint8_t override_face,music_index,music_playing,remote_previous;
static __xdata uint32_t bright_since,dark_since,hot_since,last_temp_event,last_shake_event;
static __xdata uint32_t buzzer_deadline,override_deadline,music_next;
static __xdata uint32_t next_light,next_inputs,next_accel,next_temp,next_ui,next_telemetry,next_heartbeat,next_rtc,next_decay,motion_deadline;

static uint8_t due(uint32_t n,uint32_t t){return(int32_t)(n-t)>=0;}
static void music_stop(void);
static uint8_t add_sat(uint8_t v,uint8_t a){uint16_t r=v+a;return r>100?100:(uint8_t)r;}
static uint8_t sub_sat(uint8_t v,uint8_t a){return v>a?v-a:0;}
static void beep(uint16_t hz,uint16_t ms){hal_buzzer_start(hz);buzzer_deadline=hal_millis()+ms;}
static void send_event(uint8_t code,int16_t value){uint8_t p[3];p[0]=code;protocol_put_i16(&p[1],value);protocol_send(MSG_EVENT,p,3);}
static void send_stop(uint8_t code){uint8_t p[1];p[0]=code;protocol_send(MSG_STOP,p,1);}
static void cancel_motion(void){send_stop(2);app.motion_active=0;app.motion_face=FACE_SMUG;}
static void csk_send(uint8_t c){uint8_t d[4]={0x5A,0xA5,c,(uint8_t)(c^0xFF)};hal_uart2_write(d,4);}

static void request_motion(uint8_t kind,int16_t distance,int32_t angle,uint16_t speed,uint8_t face)
{
    uint8_t p[11];if(app.emergency||app.sleeping||app.motion_active)return;
    app.motion_id++;if(!app.motion_id)app.motion_id=1;protocol_put_u16(&p[0],app.motion_id);p[2]=kind;
    protocol_put_i16(&p[3],distance);protocol_put_i32(&p[5],angle);protocol_put_u16(&p[9],speed);
    protocol_send(MSG_MOTION_REQUEST,p,11);app.motion_active=1;app.last_motion_id=app.motion_id;app.motion_face=face;
    motion_deadline=hal_millis()+(kind==MOTION_ROTATE?16000UL:8000UL);
}

static uint8_t button_update(__xdata button_state_t*b,uint8_t raw,uint32_t now)
{
    uint8_t e=0;if(raw!=b->candidate){b->candidate=raw;b->changed_at=now;}
    if(b->candidate!=b->stable&&due(now,b->changed_at+30)){b->stable=b->candidate;if(b->stable){b->pressed_at=now;b->long_sent=0;e|=BUTTON_PRESS;}else if(!b->long_sent)e|=BUTTON_SHORT;}
    if(b->stable&&!b->long_sent&&due(now,b->pressed_at+1200)){b->long_sent=1;e|=BUTTON_LONG;}return e;
}
static void emergency_set(uint8_t engaged)
{
    if(app.emergency==engaged)return;app.emergency=engaged;app.motion_active=0;send_stop(engaged?1:0);
    if(engaged){music_stop();beep(2300,260);if(app.fear<75)app.fear=75;}else beep(950,100);
}
static void music_stop(void){music_playing=0;music_index=0;hal_buzzer_stop();buzzer_deadline=0;}
static void music_start(uint32_t now){music_stop();music_playing=1;music_next=now+1200;override_face=FACE_MUSIC;override_deadline=now+14000;}
static void music_update(uint32_t now)
{
    if(!music_playing||!due(now,music_next))return;if(music_index>=SONG_NOTES){music_stop();return;}
    hal_buzzer_start(song[music_index].hz);buzzer_deadline=now+song[music_index].duration_ms-35;music_next=now+song[music_index].duration_ms;music_index++;
}

static void voice_command(uint8_t c)
{
    uint32_t now=hal_millis();send_event(EVENT_VOICE,c);
    switch(c){
    case 1:cancel_motion();break;case 2:request_motion(MOTION_LINEAR,500,0,160,FACE_FORWARD);break;
    case 3:request_motion(MOTION_LINEAR,-500,0,160,FACE_BACKWARD);break;case 4:request_motion(MOTION_LATERAL,500,0,160,FACE_LEFT);break;
    case 5:request_motion(MOTION_LATERAL,-500,0,160,FACE_RIGHT);break;case 6:request_motion(MOTION_ROTATE,0,36000L,500,FACE_HAPPY);break;
    case 7:app.sleeping=1;cancel_motion();music_stop();break;case 8:app.sleeping=0;beep(1100,90);break;
    case 9:app.happiness=add_sat(app.happiness,5);override_face=FACE_HAPPY;override_deadline=now+2500;break;
    case 10:music_start(now);break;case 11:override_face=FACE_HAPPY;override_deadline=now+2500;break;
    case 12:app.display_page=0;override_face=FACE_SMUG;override_deadline=now+1800;break;default:break;}
}
static void poll_csk(void)
{
    uint8_t v;while(hal_uart2_read(&v)){if(v>=1&&v<=12&&csk_parser_state==0){voice_command(v);continue;}
        switch(csk_parser_state){case 0:if(v==CSK_HEADER_1)csk_parser_state=1;break;case 1:csk_parser_state=v==CSK_HEADER_2?2:0;break;
        case 2:csk_command=v;csk_parser_state=3;break;case 3:if(v==(uint8_t)(csk_command^0xFF)&&csk_command>=1&&csk_command<=12)voice_command(csk_command);csk_parser_state=0;break;default:csk_parser_state=0;break;}}
}

static void send_jog(uint8_t direction){uint8_t p[5];p[0]=direction;protocol_put_u16(&p[1],IR_JOG_SPEED);protocol_put_u16(&p[3],IR_JOG_LEASE);protocol_send(MSG_JOG_REQUEST,p,5);}
static uint8_t jog_face(uint8_t d){if(d==JOG_FORWARD)return FACE_FORWARD;if(d==JOG_BACKWARD)return FACE_BACKWARD;if(d==JOG_LEFT)return FACE_LEFT;if(d==JOG_RIGHT)return FACE_RIGHT;return FACE_SMUG;}
static void poll_ir(uint32_t now)
{
    uint8_t c;while(hal_ir_read(&c)){uint8_t d=c<=4?c:JOG_STOP;
        if(c==5||c==0){send_jog(JOG_STOP);cancel_motion();d=JOG_STOP;}
        else if(!app.sleeping&&!app.emergency){if(app.motion_active)cancel_motion();send_jog(d);app.motion_face=jog_face(d);override_face=app.motion_face;override_deadline=now+IR_JOG_LEASE+80;}
        app.remote_direction=d;if(c!=remote_previous){send_event(EVENT_REMOTE,c);remote_previous=c;}}
}

static void handle_protocol_frame(void)
{
    if(protocol_rx_frame.version!=PROTOCOL_VERSION){protocol_send_ack(protocol_rx_frame.type,protocol_rx_frame.seq,1);return;}
    switch(protocol_rx_frame.type){case MSG_HELLO:case MSG_HEARTBEAT:protocol_send_ack(protocol_rx_frame.type,protocol_rx_frame.seq,0);break;
    case MSG_MOTION_RESULT:if(protocol_rx_frame.length==9){uint16_t id=protocol_get_u16(&protocol_rx_frame.payload[0]);if(id==app.last_motion_id){app.motion_active=0;app.motion_face=FACE_SMUG;beep(protocol_rx_frame.payload[2]==0?1250:650,90);}protocol_send_ack(protocol_rx_frame.type,protocol_rx_frame.seq,0);}else protocol_send_ack(protocol_rx_frame.type,protocol_rx_frame.seq,2);break;
    case MSG_STOP:if(protocol_rx_frame.length<=1){uint8_t reason=protocol_rx_frame.length?protocol_rx_frame.payload[0]:1;if(reason==2){app.motion_active=0;}else{app.emergency=(reason!=0);if(app.emergency){music_stop();app.motion_active=0;}}protocol_send_ack(protocol_rx_frame.type,protocol_rx_frame.seq,0);}else protocol_send_ack(protocol_rx_frame.type,protocol_rx_frame.seq,2);break;
    case MSG_ACK:case MSG_NACK:break;default:protocol_send_ack(protocol_rx_frame.type,protocol_rx_frame.seq,3);break;}
}

static void trigger_bright_light(void)
{
    light_armed=0;bright_since=dark_since=0;app.sleeping=0;if(app.fear<80)app.fear=80;app.happiness=sub_sat(app.happiness,10);
    music_stop();send_event(EVENT_BRIGHT_LIGHT,app.light_raw);beep(2100,320);csk_send(0x80);request_motion(MOTION_LINEAR,-500,0,180,FACE_BACKWARD);
}
static void sample_light(uint32_t now)
{
    static uint8_t initialized;uint8_t raw=hal_adc_read8(ADC_CH_LIGHT);if(!hal_adc_ok())return;
    if(!initialized){app.light_raw=raw;initialized=1;}else app.light_raw=(uint8_t)(((uint16_t)app.light_raw*3u+raw)/4u);
    if(app.light_raw>=PETCARGO_LIGHT_TRIGGER_RAW){dark_since=0;if(!bright_since)bright_since=now;if(light_armed&&due(now,bright_since+200))trigger_bright_light();}
    else if(app.light_raw<=PETCARGO_LIGHT_RELEASE_RAW){bright_since=0;if(!dark_since)dark_since=now;if(!light_armed&&due(now,dark_since+2000))light_armed=1;}else bright_since=dark_since=0;
}

static void handle_inputs(uint32_t now)
{
    uint8_t nav=hal_adc_read8(ADC_CH_KEY3),e1=button_update(&button_k1,hal_key1_down(),now),e2=button_update(&button_k2,hal_key2_down(),now),e3=button_update(&button_k3,hal_key3_down(nav),now),hall=hal_hall_near();
    if(e1&BUTTON_SHORT){app.display_page=(app.display_page+1u)&3u;send_event(EVENT_BUTTON,1);}
    if(e2&BUTTON_LONG){if(music_playing)music_stop();else music_start(now);send_event(EVENT_BUTTON,20);}else if(e2&BUTTON_SHORT){app.sleeping=!app.sleeping;if(app.sleeping){cancel_motion();music_stop();}send_event(EVENT_BUTTON,2);}
    if(e3&BUTTON_PRESS){k3_can_release=app.emergency;if(!app.emergency){emergency_set(1);send_event(EVENT_BUTTON,3);}}if((e3&BUTTON_LONG)&&k3_can_release){emergency_set(0);k3_can_release=0;send_event(EVENT_BUTTON,30);}
    app.hall=hall;if(hall&&!hall_previous){settings.feed_count++;app.happiness=add_sat(app.happiness,35);app.fear=sub_sat(app.fear,20);send_event(EVENT_FEED,(int16_t)settings.feed_count);beep(1150,100);persistence_save(&settings);}hall_previous=hall;
}
static void sample_acceleration(uint32_t now)
{
    uint8_t shake;adxl345_read(&app.accel);shake=hal_vibration_active()||(app.accel.available&&app.accel.shock>PETCARGO_SHOCK_THRESHOLD_LSB);app.vibration=shake;
    if(shake&&due(now,last_shake_event+2000)){last_shake_event=now;if(app.fear<75)app.fear=75;send_event(EVENT_SHAKE,(int16_t)app.accel.shock);emergency_set(1);}
}
static void sample_temperature(uint32_t now)
{
    uint8_t raw=hal_adc_read8(ADC_CH_TEMP);if(!hal_adc_ok())return;app.temp_x10=hal_ntc_to_celsius_x10(raw);
    if(app.temp_x10>=app.temp_baseline_x10+20){if(!hot_since)hot_since=now;if(due(now,hot_since+2000)&&due(now,last_temp_event+10000)){last_temp_event=now;app.happiness=sub_sat(app.happiness,10);send_event(EVENT_TEMPERATURE,app.temp_x10);beep(1750,80);override_face=FACE_HOT;override_deadline=now+3000;}}else hot_since=0;
}
static int16_t accel_to_mg(int16_t raw){int32_t v=(int32_t)raw*1000L/256L;if(v>32767)v=32767;if(v<-32768L)v=-32768L;return(int16_t)v;}
static void send_telemetry(uint32_t now)
{
    uint8_t p[20],flags=0;if(app.sleeping)flags|=1;if(app.emergency)flags|=2;if(!app.adxl_ok||!hal_adc_ok())flags|=0x10;if(app.remote_direction)flags|=0x20;
    protocol_put_u32(&p[0],now);p[4]=app.light_raw;protocol_put_i16(&p[5],app.temp_x10);protocol_put_i16(&p[7],accel_to_mg(app.accel.x));protocol_put_i16(&p[9],accel_to_mg(app.accel.y));protocol_put_i16(&p[11],accel_to_mg(app.accel.z));
    p[13]=app.vibration;p[14]=app.hall;p[15]=app.happiness;p[16]=app.fear;p[17]=flags;protocol_put_u16(&p[18],settings.feed_count);protocol_send(MSG_TELEMETRY,p,20);
}
static uint8_t expression(uint32_t now)
{
    if(app.emergency)return FACE_STOP;if(override_deadline&&!due(now,override_deadline))return override_face;if(app.sleeping)return FACE_SLEEP;if(app.vibration)return FACE_DIZZY;if(app.motion_active)return app.motion_face;if(app.fear>=55)return FACE_FEAR;if(app.happiness>70)return FACE_HAPPY;return FACE_SMUG;
}
static void update_ui(uint32_t now)
{
    uint8_t face=expression(now),leds;hal_display_clear();if(app.emergency){hal_display_char(0,'S');hal_display_char(1,'T');hal_display_char(2,'O');hal_display_char(3,'P');}
    else if(app.display_page==0){hal_display_char(0,'L');hal_display_uint(1,app.light_raw,3);hal_display_char(4,'F');hal_display_uint(5,app.fear,3);}else if(app.display_page==1){hal_display_char(0,'C');hal_display_uint(1,(uint16_t)(app.temp_x10<0?0:app.temp_x10/10),3);hal_display_char(4,'H');hal_display_uint(5,app.happiness,3);}
    else if(app.display_page==2){if(app.rtc_ok){hal_display_uint(0,app.rtc_hour,2);hal_display_uint(2,app.rtc_minute,2);hal_display_uint(4,app.rtc_second,2);}hal_display_char(7,app.sleeping?'S':'A');}else{hal_display_char(0,'E');hal_display_uint(1,protocol_crc_errors()%1000u,3);hal_display_char(4,'U');hal_display_uint(5,hal_uart1_overflows()%1000u,3);}
    if(app.emergency)leds=(now&0x100)?0xFF:0;else if(app.fear>=55)leds=(now&0x100)?0xAA:0x55;else if(app.happiness>=75)leds=0x7E;else leds=0x18;hal_led_pattern(leds);if(face!=last_oled_face){oled_draw_face(face);last_oled_face=face;}
}
static void decay_emotions(void){app.fear=sub_sat(app.fear,3);if(app.happiness>50)app.happiness--;else if(app.happiness<50)app.happiness++;}

void petcargo_init(void)
{
    uint8_t hello[2]={0x7F,0};uint32_t now=hal_millis();protocol_init();persistence_load(&settings);if(!settings.valid){settings.valid=0;settings.sequence=0;settings.feed_count=0;}
    app.light_raw=hal_adc_read8(ADC_CH_LIGHT);app.temp_x10=hal_ntc_to_celsius_x10(hal_adc_read8(ADC_CH_TEMP));app.temp_baseline_x10=app.temp_x10;app.happiness=50;app.fear=0;app.motion_id=0;app.sleeping=app.emergency=app.display_page=app.motion_active=app.remote_direction=0;
    app.adxl_ok=adxl345_init();app.accel.available=0;app.rtc_ok=0;button_k1.stable=button_k1.candidate=button_k2.stable=button_k2.candidate=button_k3.stable=button_k3.candidate=0;
    light_armed=1;hall_previous=hal_hall_near();csk_parser_state=0;last_oled_face=0xFF;override_deadline=0;music_playing=0;remote_previous=0xFF;bright_since=dark_since=hot_since=last_temp_event=last_shake_event=buzzer_deadline=0;
    oled_init();protocol_send(MSG_HELLO,hello,2);next_light=next_inputs=next_accel=next_ui=now;next_temp=next_telemetry=next_heartbeat=next_rtc=next_decay=now;
}
void petcargo_run_once(void)
{
    uint32_t now=hal_millis();while(protocol_poll())handle_protocol_frame();poll_csk();poll_ir(now);if(buzzer_deadline&&due(now,buzzer_deadline)){hal_buzzer_stop();buzzer_deadline=0;}music_update(now);
    if(due(now,next_inputs)){next_inputs+=10;handle_inputs(now);}if(due(now,next_light)){next_light+=50;sample_light(now);}if(due(now,next_accel)){next_accel+=40;sample_acceleration(now);}if(due(now,next_temp)){next_temp+=500;sample_temperature(now);}
    if(due(now,next_rtc)){next_rtc+=1000;app.rtc_ok=rtc_read_hms(&app.rtc_hour,&app.rtc_minute,&app.rtc_second);}if(due(now,next_decay)){next_decay+=1000;decay_emotions();}if(due(now,next_ui)){next_ui+=100;update_ui(now);}if(due(now,next_telemetry)){next_telemetry+=200;send_telemetry(now);}if(due(now,next_heartbeat)){uint8_t p[4];next_heartbeat+=500;protocol_put_u32(p,now);protocol_send(MSG_HEARTBEAT,p,4);}
    if(app.motion_active&&due(now,motion_deadline)){app.motion_active=0;send_event(EVENT_FAULT,20);}if(override_deadline&&due(now,override_deadline)){override_deadline=0;if(app.remote_direction){app.remote_direction=0;send_jog(JOG_STOP);}}
}
