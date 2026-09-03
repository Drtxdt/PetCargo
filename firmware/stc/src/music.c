#include "runtime.h"
#include "hal.h"
#include "music.h"
static uint8_t playing,index,tone_on;
static uint16_t remainder;
static uint32_t next_note,tone_end;
static uint8_t due(uint32_t n,uint32_t t){return (int32_t)(n-t)>=0;}
uint8_t music_playing(void){return playing;}
void music_stop(void){playing=0;index=0;tone_on=0;hal_buzzer_stop();}
void music_beep(uint16_t hz,uint16_t ms,uint32_t now){hal_buzzer_start(hz);tone_on=1;tone_end=now+ms;}
uint8_t music_start(uint32_t now,uint8_t locked)
{
    if(locked)return 0;
    music_stop();playing=1;remainder=0;next_note=now+500;
    music_beep(1200,300,now);return 1;
}
void music_update(uint32_t now)
{
    uint16_t duration;uint8_t gap;
    if(tone_on&&due(now,tone_end)){hal_buzzer_stop();tone_on=0;}
    if(!playing||!due(now,next_note))return;
    duration=music_duration(petcargo_song[index].ticks,&remainder);
    gap=petcargo_song[index].legato?0:15;
    /* Accumulate score time, not loop execution time. */
    tone_end=next_note+duration-gap;next_note+=duration;
    hal_buzzer_start(petcargo_song[index].hz);tone_on=petcargo_song[index].hz!=0;
    if(++index==petcargo_song_count)index=0;
}
