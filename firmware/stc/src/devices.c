#include "stc15.h"
#include "hal.h"
#include "devices.h"

#define ADXL_WRITE 0xA6
#define ADXL_READ  0xA7
#define EEPROM_WRITE 0xA0
#define EEPROM_READ  0xA1
#define SETTINGS_SLOTS 8
#define SETTINGS_SIZE 12

static __xdata int16_t last_ax;
static __xdata int16_t last_ay;
static __xdata int16_t last_az;
static uint8_t accel_ready;
static uint8_t accel_has_sample;

static void i2c345_delay(void) { hal_delay_us(3); }
static void i2c345_start(void)
{
    PIN_345_SDA = 1; PIN_345_SCL = 1; i2c345_delay();
    PIN_345_SDA = 0; i2c345_delay(); PIN_345_SCL = 0;
}
static void i2c345_stop(void)
{
    PIN_345_SDA = 0; PIN_345_SCL = 1; i2c345_delay(); PIN_345_SDA = 1; i2c345_delay();
}
static uint8_t i2c345_write(uint8_t value)
{
    uint8_t i;
    for (i = 0; i < 8; i++) {
        PIN_345_SDA = (value & 0x80) ? 1 : 0; i2c345_delay();
        PIN_345_SCL = 1; i2c345_delay(); PIN_345_SCL = 0; value <<= 1;
    }
    PIN_345_SDA = 1; PIN_345_SCL = 1; i2c345_delay();
    i = PIN_345_SDA ? 0 : 1; PIN_345_SCL = 0; return i;
}
static uint8_t i2c345_read(uint8_t ack)
{
    uint8_t i, value = 0;
    PIN_345_SDA = 1;
    for (i = 0; i < 8; i++) {
        value <<= 1; PIN_345_SCL = 1; i2c345_delay();
        if (PIN_345_SDA) value |= 1; PIN_345_SCL = 0; i2c345_delay();
    }
    PIN_345_SDA = ack ? 0 : 1; PIN_345_SCL = 1; i2c345_delay(); PIN_345_SCL = 0; PIN_345_SDA = 1;
    return value;
}
static uint8_t adxl_write_register(uint8_t reg, uint8_t value)
{
    uint8_t ok; i2c345_start(); ok = i2c345_write(ADXL_WRITE); ok &= i2c345_write(reg); ok &= i2c345_write(value); i2c345_stop(); return ok;
}
static uint8_t adxl_read_register(uint8_t reg)
{
    uint8_t value; i2c345_start(); i2c345_write(ADXL_WRITE); i2c345_write(reg); i2c345_start(); i2c345_write(ADXL_READ); value = i2c345_read(0); i2c345_stop(); return value;
}

uint8_t adxl345_init(void)
{
    PIN_345_SCL = 1; PIN_345_SDA = 1; accel_ready = 0; accel_has_sample = 0;
    if (adxl_read_register(0x00) != 0xE5) return 0;
    if (!adxl_write_register(0x31, 0x08)) return 0; /* full resolution, +/-2 g */
    adxl_write_register(0x2C, 0x08);               /* 25 Hz */
    adxl_write_register(0x2D, 0x08);               /* measurement mode */
    last_ax = last_ay = last_az = 0; accel_ready = 1; return 1;
}

static uint16_t absolute16(int16_t value) { return value < 0 ? (uint16_t)(-value) : (uint16_t)value; }

void adxl345_read(accel_sample_t *sample)
{
    uint8_t data[6];
    uint8_t i;
    if (!accel_ready) { sample->available = 0; sample->shock = 0; return; }
    i2c345_start(); i2c345_write(ADXL_WRITE); i2c345_write(0x32);
    i2c345_start(); i2c345_write(ADXL_READ);
    for (i = 0; i < 6; i++) data[i] = i2c345_read(i != 5);
    i2c345_stop();
    sample->x = (int16_t)((uint16_t)data[0] | ((uint16_t)data[1] << 8));
    sample->y = (int16_t)((uint16_t)data[2] | ((uint16_t)data[3] << 8));
    sample->z = (int16_t)((uint16_t)data[4] | ((uint16_t)data[5] << 8));
    if (!accel_has_sample) sample->shock = 0;
    else {
        sample->shock = absolute16(sample->x - last_ax);
        if (absolute16(sample->y - last_ay) > sample->shock) sample->shock = absolute16(sample->y - last_ay);
        if (absolute16(sample->z - last_az) > sample->shock) sample->shock = absolute16(sample->z - last_az);
    }
    last_ax = sample->x; last_ay = sample->y; last_az = sample->z;
    sample->available = 1; accel_has_sample = 1;
}

static void rtc_write_bit(uint8_t value)
{
    PIN_RTC_IO = value; PIN_RTC_CLK = 1; hal_delay_us(2); PIN_RTC_CLK = 0;
}
static void rtc_write_byte(uint8_t value)
{
    uint8_t i; for (i = 0; i < 8; i++) { rtc_write_bit(value & 1); value >>= 1; }
}
static uint8_t rtc_read_byte(void)
{
    uint8_t i, value = 0; PIN_RTC_IO = 1;
    for (i = 0; i < 8; i++) { if (PIN_RTC_IO) value |= (uint8_t)(1u << i); PIN_RTC_CLK = 1; hal_delay_us(2); PIN_RTC_CLK = 0; hal_delay_us(2); }
    return value;
}
static uint8_t rtc_read_register(uint8_t command)
{
    uint8_t value;
    P1M1 &= (uint8_t)~0x60; P1M0 &= (uint8_t)~0x60;
    PIN_RTC_RST = 0; PIN_RTC_CLK = 0; PIN_RTC_RST = 1;
    rtc_write_byte(command); value = rtc_read_byte(); PIN_RTC_RST = 0; PIN_RTC_IO = 1;
    return value;
}
static uint8_t bcd_to_binary(uint8_t value) { return (uint8_t)((value >> 4) * 10 + (value & 0x0F)); }

uint8_t rtc_read_hms(uint8_t *hour, uint8_t *minute, uint8_t *second)
{
    uint8_t s = rtc_read_register(0x81);
    uint8_t m = rtc_read_register(0x83);
    uint8_t h = rtc_read_register(0x85);
    if ((s & 0x7F) > 0x59 || m > 0x59 || (h & 0x3F) > 0x23) return 0;
    *second = bcd_to_binary(s & 0x7F); *minute = bcd_to_binary(m); *hour = bcd_to_binary(h & 0x3F); return 1;
}

static void ee_delay(void) { hal_delay_us(4); }
static void ee_start(void) { PIN_EE_SDA = 1; PIN_EE_SCL = 1; ee_delay(); PIN_EE_SDA = 0; ee_delay(); PIN_EE_SCL = 0; }
static void ee_stop(void) { PIN_EE_SDA = 0; PIN_EE_SCL = 1; ee_delay(); PIN_EE_SDA = 1; ee_delay(); }
static uint8_t ee_write_byte(uint8_t value)
{
    uint8_t i;
    for (i = 0; i < 8; i++) { PIN_EE_SDA = (value & 0x80) ? 1 : 0; PIN_EE_SCL = 1; ee_delay(); PIN_EE_SCL = 0; value <<= 1; }
    PIN_EE_SDA = 1; PIN_EE_SCL = 1; ee_delay(); i = PIN_EE_SDA ? 0 : 1; PIN_EE_SCL = 0; return i;
}
static uint8_t ee_read_byte(uint8_t ack)
{
    uint8_t i, value = 0; PIN_EE_SDA = 1;
    for (i = 0; i < 8; i++) { value <<= 1; PIN_EE_SCL = 1; ee_delay(); if (PIN_EE_SDA) value |= 1; PIN_EE_SCL = 0; }
    PIN_EE_SDA = ack ? 0 : 1; PIN_EE_SCL = 1; ee_delay(); PIN_EE_SCL = 0; PIN_EE_SDA = 1; return value;
}
static uint8_t ee_read_address(uint8_t address)
{
    uint8_t value; ee_start(); if (!ee_write_byte(EEPROM_WRITE)) { ee_stop(); return 0xFF; }
    ee_write_byte(address); ee_start(); ee_write_byte(EEPROM_READ); value = ee_read_byte(0); ee_stop(); return value;
}
static void ee_write_address(uint8_t address, uint8_t value)
{
    ee_start(); if (!ee_write_byte(EEPROM_WRITE)) { ee_stop(); return; }
    ee_write_byte(address); ee_write_byte(value); ee_stop();
}
static __xdata uint8_t ee_pending[SETTINGS_SIZE];
static uint8_t ee_index,ee_base,ee_busy;
static uint32_t ee_deadline;
void persistence_service(void)
{
    uint8_t ready;
    if(!ee_busy)return;
    ee_start();ready=ee_write_byte(EEPROM_WRITE);ee_stop();
    if(!ready){if((int32_t)(hal_millis()-ee_deadline)>=0)ee_busy=0;return;}
    ee_write_address(ee_base+ee_index,ee_pending[ee_index]);
    ee_deadline=hal_millis()+20;
    if(++ee_index==SETTINGS_SIZE)ee_busy=0;
}
static uint8_t settings_crc(const uint8_t *data)
{
    uint8_t i, crc = 0x5A; for (i = 0; i < 10; i++) crc = (uint8_t)((crc << 1) | (crc >> 7)) ^ data[i]; return crc;
}

void persistence_load(__xdata persisted_settings_t *settings)
{
    uint8_t slot, i; uint8_t found = 0; uint16_t best = 0; uint8_t data[SETTINGS_SIZE];
    settings->valid = 0;
    for (slot = 0; slot < SETTINGS_SLOTS; slot++) {
        for (i = 0; i < SETTINGS_SIZE; i++) data[i] = ee_read_address((uint8_t)(slot * SETTINGS_SIZE + i));
        if (data[0] != 0xC7 || (data[1] != 1 && data[1] != 2) || data[10] != settings_crc(data) || data[11] != (uint8_t)~data[10]) continue;
        {
            uint16_t seq = (uint16_t)data[2] | ((uint16_t)data[3] << 8);
            if (!found || (int16_t)(seq - best) > 0) {
                found = 1; best = seq;
                if (data[1] == 1) settings->feed_count = (uint16_t)data[7] | ((uint16_t)data[8] << 8);
                else settings->feed_count = (uint16_t)data[4] | ((uint16_t)data[5] << 8);
            }
        }
    }
    settings->valid = found; settings->sequence = best;
}

void persistence_save(__xdata persisted_settings_t *settings)
{
    uint8_t data[SETTINGS_SIZE]; uint8_t i, slot;
    settings->sequence++; settings->valid = 1; slot = (uint8_t)(settings->sequence & (SETTINGS_SLOTS - 1));
    data[0] = 0xC7; data[1] = 2; data[2] = (uint8_t)settings->sequence; data[3] = (uint8_t)(settings->sequence >> 8);
    data[4] = (uint8_t)settings->feed_count; data[5] = (uint8_t)(settings->feed_count >> 8);
    data[6] = data[7] = data[8] = data[9] = 0;
    data[10] = settings_crc(data); data[11] = (uint8_t)~data[10];
    /* Coalesce a new feeding count into a new slot; never busy-wait for EEPROM. */
    for(i=0;i<SETTINGS_SIZE;i++)ee_pending[i]=data[i];
    ee_index=0;ee_base=slot*SETTINGS_SIZE;ee_busy=1;ee_deadline=hal_millis()+20;
}
