#include "hal.h"
#include "protocol.h"

enum parser_state {
    WAIT_AA, WAIT_55, READ_VERSION, READ_TYPE, READ_SEQ, READ_LENGTH,
    READ_PAYLOAD, READ_CRC_LO, READ_CRC_HI
};

__xdata protocol_frame_t protocol_rx_frame;
static __xdata uint8_t tx_buffer[40];
static uint8_t parser_state;
static uint8_t parser_index;
static uint16_t parser_crc_received;
static uint8_t tx_sequence;
static uint16_t crc_error_count;
static uint16_t length_error_count;

uint16_t protocol_crc16(const uint8_t *data, uint8_t length)
{
    uint16_t crc = 0xFFFF;
    uint8_t i;
    while (length--) {
        crc ^= (uint16_t)(*data++) << 8;
        for (i = 0; i < 8; i++) crc = (crc & 0x8000) ? (uint16_t)((crc << 1) ^ 0x1021) : (uint16_t)(crc << 1);
    }
    return crc;
}

void protocol_init(void)
{
    parser_state = WAIT_AA;
    parser_index = 0;
    tx_sequence = 0;
    crc_error_count = length_error_count = 0;
}

void protocol_put_u16(uint8_t *target, uint16_t value)
{
    target[0] = (uint8_t)value; target[1] = (uint8_t)(value >> 8);
}

void protocol_put_i16(uint8_t *target, int16_t value) { protocol_put_u16(target, (uint16_t)value); }

void protocol_put_u32(uint8_t *target, uint32_t value)
{
    target[0] = (uint8_t)value; target[1] = (uint8_t)(value >> 8);
    target[2] = (uint8_t)(value >> 16); target[3] = (uint8_t)(value >> 24);
}

void protocol_put_i32(uint8_t *target, int32_t value) { protocol_put_u32(target, (uint32_t)value); }

uint16_t protocol_get_u16(const uint8_t *source)
{
    return (uint16_t)source[0] | ((uint16_t)source[1] << 8);
}

int16_t protocol_get_i16(const uint8_t *source) { return (int16_t)protocol_get_u16(source); }

int32_t protocol_get_i32(const uint8_t *source)
{
    return (int32_t)((uint32_t)source[0] | ((uint32_t)source[1] << 8) |
                     ((uint32_t)source[2] << 16) | ((uint32_t)source[3] << 24));
}

void protocol_send(uint8_t type, const uint8_t *payload, uint8_t length)
{
    uint8_t i;
    uint16_t crc;
    if (length > PROTOCOL_MAX_PAYLOAD) return;
    tx_buffer[0] = 0xAA; tx_buffer[1] = 0x55;
    tx_buffer[2] = PROTOCOL_VERSION; tx_buffer[3] = type;
    tx_buffer[4] = ++tx_sequence; tx_buffer[5] = length;
    for (i = 0; i < length; i++) tx_buffer[6 + i] = payload[i];
    crc = protocol_crc16(&tx_buffer[2], (uint8_t)(4 + length));
    tx_buffer[6 + length] = (uint8_t)crc;
    tx_buffer[7 + length] = (uint8_t)(crc >> 8);
    hal_uart1_write(tx_buffer, (uint8_t)(8 + length));
}

void protocol_send_ack(uint8_t type, uint8_t sequence, uint8_t code)
{
    uint8_t data[3]; data[0] = type; data[1] = sequence; data[2] = code;
    protocol_send(code ? MSG_NACK : MSG_ACK, data, 3);
}

static uint8_t validate_frame(void)
{
    uint8_t i;
    uint16_t crc = 0xFFFF;
    uint8_t head[4];
    head[0] = protocol_rx_frame.version; head[1] = protocol_rx_frame.type;
    head[2] = protocol_rx_frame.seq; head[3] = protocol_rx_frame.length;
    crc = protocol_crc16(head, 4);
    /* Continue CRC without allocating a combined buffer. */
    for (i = 0; i < protocol_rx_frame.length; i++) {
        uint8_t bit;
        crc ^= (uint16_t)protocol_rx_frame.payload[i] << 8;
        for (bit = 0; bit < 8; bit++) crc = (crc & 0x8000) ? (uint16_t)((crc << 1) ^ 0x1021) : (uint16_t)(crc << 1);
    }
    return crc == parser_crc_received;
}

uint8_t protocol_poll(void)
{
    uint8_t value;
    while (hal_uart1_read(&value)) {
        switch (parser_state) {
        case WAIT_AA: if (value == 0xAA) parser_state = WAIT_55; break;
        case WAIT_55: parser_state = value == 0x55 ? READ_VERSION : (value == 0xAA ? WAIT_55 : WAIT_AA); break;
        case READ_VERSION: protocol_rx_frame.version = value; parser_state = READ_TYPE; break;
        case READ_TYPE: protocol_rx_frame.type = value; parser_state = READ_SEQ; break;
        case READ_SEQ: protocol_rx_frame.seq = value; parser_state = READ_LENGTH; break;
        case READ_LENGTH:
            protocol_rx_frame.length = value; parser_index = 0;
            if (value > PROTOCOL_MAX_PAYLOAD) { length_error_count++; parser_state = WAIT_AA; }
            else parser_state = value ? READ_PAYLOAD : READ_CRC_LO;
            break;
        case READ_PAYLOAD:
            protocol_rx_frame.payload[parser_index++] = value;
            if (parser_index >= protocol_rx_frame.length) parser_state = READ_CRC_LO;
            break;
        case READ_CRC_LO: parser_crc_received = value; parser_state = READ_CRC_HI; break;
        case READ_CRC_HI:
            parser_crc_received |= (uint16_t)value << 8; parser_state = WAIT_AA;
            if (validate_frame()) return 1;
            crc_error_count++;
            break;
        default: parser_state = WAIT_AA; break;
        }
    }
    return 0;
}

uint16_t protocol_crc_errors(void) { return crc_error_count; }
uint16_t protocol_length_errors(void) { return length_error_count; }
