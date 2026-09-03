#ifndef PETCARGO_MUSIC_H
#define PETCARGO_MUSIC_H
#include <stdint.h>
uint8_t music_playing(void);
uint8_t music_start(uint32_t now,uint8_t locked);
void music_stop(void);
void music_update(uint32_t now);
void music_beep(uint16_t hz,uint16_t ms,uint32_t now);
#endif
