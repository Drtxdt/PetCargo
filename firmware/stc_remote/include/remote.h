#ifndef PETCARGO_REMOTE_H
#define PETCARGO_REMOTE_H
#include <stdint.h>
enum { CMD_IDLE=0,CMD_FORWARD=1,CMD_BACKWARD=2,CMD_LEFT=3,CMD_RIGHT=4,CMD_STOP=5 };
typedef struct {uint8_t candidate,stable,valid,release_frames;uint32_t changed_at,next_send;} remote_keys_t;
uint8_t remote_decode(uint8_t adc);
void remote_sample(remote_keys_t *state,uint8_t adc,uint8_t valid,uint32_t now);
uint8_t remote_next(remote_keys_t *state,uint32_t now);
uint8_t remote_checksum(const uint8_t *data,uint8_t length);
#endif
