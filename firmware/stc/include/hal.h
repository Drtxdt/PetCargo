#ifndef PETCARGO_HAL_H
#define PETCARGO_HAL_H

#include <stdint.h>

#ifndef FOSC
#define FOSC 11059200UL
#endif

#define ADC_CH_TEMP  3
#define ADC_CH_LIGHT 4
#define ADC_CH_KEY3  7

void hal_init(void);
uint32_t hal_millis(void);
void hal_delay_us(uint8_t count);

uint8_t hal_adc_read8(uint8_t channel);
uint8_t hal_adc_ok(void);
int16_t hal_ntc_to_celsius_x10(uint8_t raw);

uint8_t hal_key1_down(void);
uint8_t hal_key2_down(void);
uint8_t hal_key3_down(uint8_t nav_adc);
uint8_t hal_hall_near(void);
uint8_t hal_vibration_active(void);

void hal_display_clear(void);
void hal_display_char(uint8_t position, char value);
void hal_display_uint(uint8_t position, uint16_t value, uint8_t width);
void hal_led_pattern(uint8_t pattern);
void hal_display_commit(void);

void hal_buzzer_start(uint16_t hz);
void hal_buzzer_stop(void);

uint8_t hal_uart1_read(uint8_t *value);
uint8_t hal_uart2_read(uint8_t *value);
void hal_uart1_write(const uint8_t *data, uint8_t length);
uint8_t hal_uart1_send(const uint8_t *data,uint8_t length,uint8_t urgent);
void hal_uart2_write(const uint8_t *data, uint8_t length);
uint16_t hal_uart1_overflows(void);
uint16_t hal_uart2_overflows(void);
uint16_t hal_uart2_received(void);
uint16_t hal_tx_dropped(void);

uint8_t hal_ir_read(uint8_t *command);

#endif
