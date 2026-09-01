#ifndef PETCARGO_PROTOCOL_H
#define PETCARGO_PROTOCOL_H

#include <stdint.h>

#define PROTOCOL_VERSION 2
#define PROTOCOL_MAX_PAYLOAD 32

enum protocol_message_type {
    MSG_HELLO = 0x01,
    MSG_HEARTBEAT = 0x02,
    MSG_TELEMETRY = 0x10,
    MSG_EVENT = 0x11,
    MSG_MOTION_REQUEST = 0x20,
    MSG_MOTION_RESULT = 0x21,
    MSG_JOG_REQUEST = 0x22,
    MSG_STOP = 0x31,
    MSG_ACK = 0x7E,
    MSG_NACK = 0x7F
};

enum protocol_motion_kind { MOTION_LINEAR = 1, MOTION_ROTATE = 2, MOTION_LATERAL = 3 };
enum protocol_jog_direction { JOG_STOP = 0, JOG_FORWARD = 1, JOG_BACKWARD = 2, JOG_LEFT = 3, JOG_RIGHT = 4 };
enum protocol_event_code {
    EVENT_BRIGHT_LIGHT = 1,
    EVENT_SHAKE = 2,
    EVENT_FEED = 3,
    EVENT_TEMPERATURE = 4,
    EVENT_VOICE = 5,
    EVENT_BUTTON = 6,
    EVENT_FAULT = 8,
    EVENT_REMOTE = 9
};

typedef struct {
    uint8_t version;
    uint8_t type;
    uint8_t seq;
    uint8_t length;
    uint8_t payload[PROTOCOL_MAX_PAYLOAD];
} protocol_frame_t;

extern __xdata protocol_frame_t protocol_rx_frame;

void protocol_init(void);
uint8_t protocol_poll(void);
void protocol_send(uint8_t type, const uint8_t *payload, uint8_t length);
void protocol_send_ack(uint8_t type, uint8_t sequence, uint8_t code);
uint16_t protocol_crc16(const uint8_t *data, uint8_t length);
uint16_t protocol_crc_errors(void);
uint16_t protocol_length_errors(void);

void protocol_put_u16(uint8_t *target, uint16_t value);
void protocol_put_i16(uint8_t *target, int16_t value);
void protocol_put_u32(uint8_t *target, uint32_t value);
void protocol_put_i32(uint8_t *target, int32_t value);
uint16_t protocol_get_u16(const uint8_t *source);
int16_t protocol_get_i16(const uint8_t *source);
int32_t protocol_get_i32(const uint8_t *source);

#endif
