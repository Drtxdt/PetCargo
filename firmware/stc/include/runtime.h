#ifndef PETCARGO_RUNTIME_H
#define PETCARGO_RUNTIME_H
#include <stdint.h>
#ifndef __SDCC
#define __xdata
#define __code
#endif
#define BUTTON_PRESS 1
#define BUTTON_SHORT 2
#define BUTTON_LONG 4
typedef struct { uint8_t stable,candidate,long_sent; uint32_t changed_at,pressed_at; } button_state_t;
uint8_t button_update(button_state_t *b, uint8_t raw, uint8_t valid, uint32_t now);
uint8_t nav_is_k3(uint8_t raw);
typedef struct { uint8_t state,command,last[4],last_command; uint16_t bytes,frames,errors; uint32_t last_at; } csk_parser_t;
uint8_t csk_feed(csk_parser_t *p, uint8_t v, uint32_t now);
void csk_expire(csk_parser_t *p, uint32_t now);
#define TX_FRAME_MAX 40
#define TX_SLOTS 4
/* One active frame, three waiting frames, and a reserved urgent STOP frame. */
typedef struct {
    uint8_t data[TX_SLOTS][TX_FRAME_MAX],length[TX_SLOTS],head,tail;
    uint8_t active[TX_FRAME_MAX],pos,size,urgent[TX_FRAME_MAX],urgent_size;
    uint16_t dropped;
} tx_queue_t;
/* Caller serializes producer/consumer with a TX-only gate; RX stays enabled. */
uint8_t tx_enqueue(tx_queue_t *q,const uint8_t *data,uint8_t n,uint8_t urgent);
uint8_t tx_next(tx_queue_t *q,uint8_t *value);
typedef struct { uint16_t hz; uint8_t ticks,legato; } music_note_t;
extern const music_note_t __code petcargo_song[];
extern const uint8_t __code petcargo_song_count;
uint16_t music_duration(uint8_t ticks, uint16_t *remainder);
#endif
