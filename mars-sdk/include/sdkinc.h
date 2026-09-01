#ifndef __SDK_COMMON_HEADER__
#define __SDK_COMMON_HEADER__

// standard c libraries header
#include "stdinc.h"

// auto configurations by kconfig
#include "autoconf.h"

// platform drivers header
#if CONFIG_MODULE_BSP && CONFIG_MODULE_DRIVER
#include "chip.h"
#include "chip_id.h"
#include "mars_base.h"
#include "mars.h"
#include "cache.h"
#include "dma.h"
#include "apc.h"
// #include "spi.h"
#include "systick.h"
#include "audio_codec.h"
#include "flash_cache.h"
#include "clock_config.h"
#include "ClockManager.h"
#include "PowerManager.h"
#include "IOMuxManager.h"
#include "Driver_ADC_PDM.h"
#include "Driver_AON_TIMER.h"
#include "Driver_AON_WDT.h"
#include "Driver_CLASSD.h"
#include "Driver_Common.h"
#include "Driver_Common_Audio.h"
#include "Driver_DUAL_TIMER.h"
#include "Driver_GPADC.h"
#include "Driver_GPIO.h"
#include "Driver_GPT_Common.h"
#include "Driver_GPT_IC.h"
#include "Driver_GPT_PWM.h"
#include "Driver_GPT_TIMER.h"
#include "Driver_I2C.h"
#include "Driver_I2S.h"
#include "Driver_IR.h"
#include "Driver_KEYSENSE.h"
#include "Driver_SPI.h"
#include "Driver_UART.h"
#endif//#if CONFIG_MODULE_BSP && CONFIG_MODULE_DRIVER

// modules: rtos
#if CONFIG_MODULE_FREERTOS
#include "FreeRTOS.h"
#include "portable.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "timers.h"
#include "event_groups.h"
#include "stream_buffer.h"
#endif//CONFIG_MODULE_FREERTOS

// modules
#if CONFIG_MODULE_SHELL
#include "shell.h"
#endif
#if CONFIG_MODULE_CJSON
#include "cJSON.h"
#endif
#if CONFIG_MODULE_CRC32
#include "crc32.h"
#endif
#if CONFIG_MODULE_BTRACE
#include "btrace.h"
#endif

#endif//__SDK_COMMON_HEADER__
