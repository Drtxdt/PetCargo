#ifndef PETCARGO_DEVICES_H
#define PETCARGO_DEVICES_H

#include <stdint.h>

typedef struct {
    int16_t x;
    int16_t y;
    int16_t z;
    uint16_t shock;
    uint8_t available;
} accel_sample_t;

typedef struct {
    uint8_t valid;
    uint16_t feed_count;
    uint16_t sequence;
} persisted_settings_t;

uint8_t adxl345_init(void);
void adxl345_read(accel_sample_t *sample);
uint8_t rtc_read_hms(uint8_t *hour, uint8_t *minute, uint8_t *second);
void persistence_load(__xdata persisted_settings_t *settings);
void persistence_save(__xdata persisted_settings_t *settings);

#endif
