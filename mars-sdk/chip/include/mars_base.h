/*
 * mars_base.h
 *
 *  Created on: 
 *
 */

#ifndef INCLUDE_MARS_BASE_H_
#define INCLUDE_MARS_BASE_H_

/************************************************************************************
 * Memory map
 ************************************************************************************/

/************ SRAM **************************/
#define AP_ROM                 0x00000000UL  //0x0000_0000  0x0000_3FFF  16KB   AP ROM region: ROM (actual size : 16KB)
#define AP_RAM_G0              0x20000000UL  //0x2000_0000  0x2002_FFFF  192KB  AP RAM G0 region: RAM (actual size : 192KB)
#define AP_RAM_G1              0x20030000UL  //0x2003_0000  0x2003_7FFF  32KB   AP RAM G1 region: RAM (actual size : 32KB)
#define LUNA_RAM               0x20038000UL  //0x2003_8000  0x2003_FFFF  32KB   LUNA RAM region: RAM (actual size :32KB)
#define SYS_FLASH_BASE         0x30000000UL  //0x3000_0000  0x37FF_FFFF  128MB  FLASH region: FLASH (actual size:up to die)
#define SYS_FLASH_NC_BASE      0x38000000UL  //0x3800_0000  0x3FFF_FFFF  128MB  FLASH region: FLASH (actual size:up to die)

#define CMN_RAM                0x20000000UL  //0x2000_0000  0x2003_FFFF  256KB  CMN RAM region: RAM (actual size : 256KB)


/************ Peripheral ********************/
#define DMAC_BASE           0x40000000UL  //0x4000_0000  0x40FF_FFFF  16MB   DMAC CP-S
#define DMAC_Channel0_BASE  (DMAC_BASE + 0x0000)
#define DMAC_Channel1_BASE  (DMAC_BASE + 0x0058)
#define DMAC_Channel2_BASE  (DMAC_BASE + 0x00B0)
#define DMAC_Channel3_BASE  (DMAC_BASE + 0x0108)
#define DMAC_Channel4_BASE  (DMAC_BASE + 0x0160)
#define DMAC_Channel5_BASE  (DMAC_BASE + 0x01B8)
#define DMAC_Channel6_BASE  (DMAC_BASE + 0x0210)
#define DMAC_Channel7_BASE  (DMAC_BASE + 0x0268)

#define LUNA_BASE              0x41000000UL // 0x4100_0000  0x41FF_FFFF  16MB    LUNA REGISTERS
#define CMN_PERPH_BASE         0x48000000UL // 0x4800_0000  0x48FF_FFFF  16MB    CMN Peripheral Wrap
#define AP_PERPH_BASE          0x49000000UL // 0x4900_0000  0x49FF_FFFF  16MB    AP Peripheral Wrap
#define AON_PERPH_BASE         0x4F000000UL // 0x4F00_0000  0x4FFF_FFFF  16MB    AON Peripheral Wrap


/********** CMN Peripheral Wrap ***************/
#define CMN_SYSCFG_BASE        0x48000000UL //0x4800_0000  0x480F_FFFF  1MB  CMN_SYS_CFG
#define CMN_BUSCFG_BASE        0x48100000UL //0x4810_0000  0x481F_FFFF  1MB  CMN_BUS_CFG
#define CORE_IOMUX_BASE        0x48200000UL //0x4820_0000  0x482F_FFFF  1MB  CORE_IOMUX
#define FLASH_CACHE_BASE       0x48300000UL //0x4830_0000  0x483F_FFFF  1MB  FLASH_CACHE
#define FLASH_CTRL_BASE        0x48400000UL //0x4840_0000  0x484F_FFFF  1MB  FLASH_CTRL
#define FLASH_DL_BASE          0x48500000UL //0x4850_0000  0x485F_FFFF  1MB  FLASH_DL

#define UART0_BASE             0x49000000UL //0x4900_0000  0x490F_FFFF  1MB  UART0
#define UART1_BASE             0x49100000UL //0x4910_0000  0x491F_FFFF  1MB  UART1
#define I2C0_BASE              0x49200000UL //0x4920_0000  0x492F_FFFF  1MB  I2C0
#define SPI0_BASE              0x49300000UL //0x4930_0000  0x493F_FFFF  1MB  SPI0
#define IR_BASE                0x49400000UL //0x4940_0000  0x494F_FFFF  1MB  IR
#define DUALTIMERS0_BASE       0x49500000UL //0x4950_0000  0x495F_FFFF  1MB  DUALTIMERS0
#define GPIO0_BASE             0x49600000UL //0x4960_0000  0x496F_FFFF  1MB  GPIOA
#define GPT_BASE               0x49700000UL //0x4970_0000  0x497F_FFFF  1MB  GPT
#define GPADC_BASE             0x49900000UL //0x4990_0000  0x499F_FFFF  1MB  GPADC
#define APC_BASE               0x49A00000UL //0x49A0_0000  0x49AF_FFFF  1MB  APC APC
#define AUDIO_CODEC_BASE       0x49B00000UL //0x49B0_0000  0x49BF_FFFF  1MB  CODEC CODEC


/********** CMN CP AON Peripheral Wrap ***************/
#define AON_CTRL_BASE          0x4F000000UL //0x4F00_0000  0x4F0F_FFFF  1MB  AON_CTRL
#define KEYSENSE0_BASE         0x4F100000UL //0x4F20_0000  0x4F2F_FFFF  1MB  KEYSENSE0
#define AON_IOMUX_BASE         0x4F200000UL //0x4F10_0000  0x4F1F_FFFF  1MB  AON_IOMUX
#define AON_TIMER_BASE         0x4F300000UL //0x4F30_0000  0x4F3F_FFFF  1MB  AON_TIMER
#define AON_WDT_BASE           0x4F400000UL //0x4F40_0000  0x4F4F_FFFF  1MB  AON_WDT


#endif /* INCLUDE_MARS_BASE_H_ */
