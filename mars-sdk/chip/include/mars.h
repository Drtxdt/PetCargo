/******************************************************************************
 * @file     mars.h
 * @brief    NMSIS Core Peripheral Access Layer Header File for
 *           Nuclei Demo SoC which support Nuclei N/NX class cores
 * @version  V1.00
 * @date     22. Nov 2019
 ******************************************************************************/
/*
 * Copyright (c) 2019 Nuclei Limited. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the License); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef __MARS_H__
#define __MARS_H__

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/** @addtogroup Nuclei
  * @{
  */


/** @addtogroup mars
  * @{
  */


/** @addtogroup Configuration_of_NMSIS
  * @{
  */


/** \brief CPU Internal Region Information */
typedef struct IRegion_Info {
    unsigned long iregion_base;         /*!< Internal region base address */
    unsigned long eclic_base;           /*!< eclic base address */
    unsigned long systimer_base;        /*!< system timer base address */
    unsigned long smp_base;             /*!< smp base address */
    unsigned long idu_base;             /*!< idu base address */
} IRegion_Info_Type;


/* =========================================================================================================================== */
/* ================                                Interrupt Number Definition                                ================ */
/* =========================================================================================================================== */

/* mars's External IRQn ID is from the hard-wired persperctive, which has an offset mapped to the ECLIC IRQn.
   eg.: uart0's external interrupt id in mars is 32, while its ECLIC IRQn is 51 */
#define SOC_EXTERNAL_MAP_TO_ECLIC_IRQn_OFFSET      19
/* get mars's External IRQn from ECLIC external IRQn which indexs from 19 */
#define IRQn_MAP_TO_EXT_ID(IRQn)                   (IRQn - SOC_EXTERNAL_MAP_TO_ECLIC_IRQn_OFFSET)

typedef enum IRQn {
    /* =======================================  Nuclei Core Specific Interrupt Numbers  ======================================== */

	IRQ_Reserved0_VECTOR            =   0,              /*!<  Internal reserved */
	IRQ_Reserved1_VECTOR            =   1,              /*!<  Internal reserved */
	IRQ_Reserved2_VECTOR            =   2,              /*!<  Internal reserved */
	SysTimerSW_IRQn    				=   3,              /*!<  System Timer SW interrupt */
	IRQ_Reserved4_VECTOR            =   4,              /*!<  Internal reserved */
	IRQ_Reserved5_VECTOR            =   5,              /*!<  Internal reserved */
	IRQ_Reserved6_VECTOR            =   6,              /*!<  Internal reserved */
	SysTimer_IRQn       			=   7,              /*!<  System Timer Interrupt */
	IRQ_Reserved8_VECTOR            =   8,              /*!<  Internal reserved */
	IRQ_Reserved9_VECTOR            =   9,              /*!<  Internal reserved */
	IRQ_Reserved10_VECTOR           =  10,              /*!<  Internal reserved */
	IRQ_Reserved11_VECTOR           =  11,              /*!<  Internal reserved */
	IRQ_Reserved12_VECTOR           =  12,              /*!<  Internal reserved */
	IRQ_Reserved13_VECTOR           =  13,              /*!<  Internal reserved */
	IRQ_Reserved14_VECTOR           =  14,              /*!<  Internal reserved */
	IRQ_Reserved15_VECTOR           =  15,              /*!<  Internal reserved */
    InterCore_IRQn            		=  16,              /*!<  CIDU Inter Core Interrupt */
	IRQ_Reserved17_VECTOR           =  17,              /*!<  Internal reserved */
	IRQ_Reserved18_VECTOR           =  18,              /*!<  Internal reserved */

    /* ===========================================  mars Specific Interrupt Numbers  ========================================= */
    /* ToDo: add here your device specific external interrupt numbers. 19~1023 is reserved number for user. Maxmum interrupt supported
             could get from clicinfo.NUM_INTERRUPT. According the interrupt handlers defined in startup_Device.s
             eg.: Interrupt for Timer#1       eclic_tim1_handler   ->   TIM1_IRQn */
	IRQ_DMAC_VECTOR           = 19,                /*!< Device Interrupt */
	IRQ_FLASHC_VECTOR         = 20,                /*!< Device Interrupt */
	IRQ_LUNA_VECTOR           = 21,                /*!< Device Interrupt */
	IRQ_DUALTIMERS_VECTOR     = 22,                /*!< Device Interrupt */
	IRQ_GPT_VECTOR            = 23,                /*!< Device Interrupt */
	IRQ_GPADC_VECTOR          = 24,                /*!< Device Interrupt */
	IRQ_GPIO0_VECTOR          = 25,                /*!< Device Interrupt */
	IRQ_UART0_VECTOR          = 26,                /*!< Device Interrupt */
	IRQ_UART1_VECTOR          = 27,                /*!< Device Interrupt */
	IRQ_I2C0_VECTOR           = 28,                /*!< Device Interrupt */
	IRQ_IR_VECTOR             = 29,                /*!< Device Interrupt */
	IRQ_SPI0_VECTOR           = 30,                /*!< Device Interrupt */
	IRQ_APC_VECTOR            = 31,                /*!< Device Interrupt */
	IRQ_AON_KS0_VECTOR        = 32,                /*!< Device Interrupt */
	IRQ_AON_TIMER_VECTOR      = 33,                /*!< Device Interrupt */
	IRQ_AON_WDT_VECTOR        = 34,                /*!< Device Interrupt */
	IRQ_AON_WKUP_VECTOR       = 35,                /*!< Device Interrupt */
	IRQ_MAX,
} IRQn_Type;

//for compatible
#define IRQ_SysTimer_VECTOR SysTimer_IRQn


/* =========================================================================================================================== */
/* ================                                  Exception Code Definition                                ================ */
/* =========================================================================================================================== */

typedef enum EXCn {
    /* =======================================  Nuclei N/NX Specific Exception Code  ======================================== */
    InsUnalign_EXCn          =   0,              /*!<  Instruction address misaligned */
    InsAccFault_EXCn         =   1,              /*!<  Instruction access fault */
    IlleIns_EXCn             =   2,              /*!<  Illegal instruction */
    Break_EXCn               =   3,              /*!<  Beakpoint */
    LdAddrUnalign_EXCn       =   4,              /*!<  Load address misaligned */
    LdFault_EXCn             =   5,              /*!<  Load access fault */
    StAddrUnalign_EXCn       =   6,              /*!<  Store or AMO address misaligned */
    StAccessFault_EXCn       =   7,              /*!<  Store or AMO access fault */
    UmodeEcall_EXCn          =   8,              /*!<  Environment call from User mode */
    SmodeEcall_EXCn          =   9,              /*!<  Environment call from S-mode */
    MmodeEcall_EXCn          =  11,              /*!<  Environment call from Machine mode */
    InsPageFault_EXCn        =  12,              /*!<  Instruction page fault */
    LdPageFault_EXCn         =  13,              /*!<  Load page fault */
    StPageFault_EXCn         =  15,              /*!<  Store or AMO page fault */
    NMI_EXCn                 =  0xfff,           /*!<  NMI interrupt */
} EXCn_Type;

/* =========================================================================================================================== */
/* ================                           Processor and Core Peripheral Section                           ================ */
/* =========================================================================================================================== */

/* ToDo: set the defines according your Device */
/* ToDo: define the correct core revision */
#if __riscv_xlen == 32

#ifndef __NUCLEI_CORE_REV
#define __NUCLEI_N_REV            0x0104    /*!< Core Revision r1p4 */
#else
#define __NUCLEI_N_REV            __NUCLEI_CORE_REV
#endif

#elif __riscv_xlen == 64

#ifndef __NUCLEI_CORE_REV
#define __NUCLEI_NX_REV           0x0100    /*!< Core Revision r1p0 */
#else
#define __NUCLEI_NX_REV           __NUCLEI_CORE_REV
#endif

#endif /* __riscv_xlen == 64 */

extern volatile IRegion_Info_Type SystemIRegionInfo;

/* ToDo: define the correct core features for the mars */
#define __ECLIC_PRESENT           1                     /*!< Set to 1 if ECLIC is present */
#define __ECLIC_BASEADDR          SystemIRegionInfo.eclic_base          /*!< Set to ECLIC baseaddr of your device */

//#define __ECLIC_INTCTLBITS        3                     /*!< Set to 1 - 8, the number of hardware bits are actually implemented in the clicintctl registers. */
#define __ECLIC_INTNUM            51                    /*!< Set to 1 - 1024, total interrupt number of ECLIC Unit */
#define __SYSTIMER_PRESENT        1                     /*!< Set to 1 if System Timer is present */
#define __SYSTIMER_BASEADDR       SystemIRegionInfo.systimer_base          /*!< Set to SysTimer baseaddr of your device */

#define __CIDU_PRESENT            0                     /*!< Set to 1 if CIDU is present */
#define __CIDU_BASEADDR           SystemIRegionInfo.idu_base              /*!< Set to cidu baseaddr of your device */

/*!< Set to 0, 1, or 2, 0 not present, 1 single floating point unit present, 2 double floating point unit present */
#if !defined(__riscv_flen)
#define __FPU_PRESENT             0
#elif __riscv_flen == 32
#define __FPU_PRESENT             1
#else
#define __FPU_PRESENT             2
#endif

/* __riscv_bitmanip/__riscv_dsp/__riscv_vector is introduced
 * in nuclei gcc 10.2 when b/p/v extension compiler option is selected.
 * For example:
 * -march=rv32imacb -mabi=ilp32 : __riscv_bitmanip macro will be defined
 * -march=rv32imacp -mabi=ilp32 : __riscv_dsp macro will be defined
 * -march=rv64imacv -mabi=lp64 : __riscv_vector macro will be defined
 */
#if defined(__riscv_bitmanip)
#define __BITMANIP_PRESENT        1                     /*!< Set to 1 if Bitmainpulation extension is present */
#else
#define __BITMANIP_PRESENT        0                     /*!< Set to 1 if Bitmainpulation extension is present */
#endif
#if defined(__riscv_dsp)
#define __DSP_PRESENT             1                     /*!< Set to 1 if Partial SIMD(DSP) extension is present */
#else
#define __DSP_PRESENT             0                     /*!< Set to 1 if Partial SIMD(DSP) extension is present */
#endif
#if defined(__riscv_vector)
#define __VECTOR_PRESENT          1                     /*!< Set to 1 if Vector extension is present */
#else
#define __VECTOR_PRESENT          0                     /*!< Set to 1 if Vector extension is present */
#endif

#define __PMP_PRESENT             1                     /*!< Set to 1 if PMP is present */
#define __PMP_ENTRY_NUM           16                    /*!< Set to 8 or 16, the number of PMP entries */

#define __SPMP_PRESENT            0                     /*!< Set to 1 if SPMP is present */
#define __SPMP_ENTRY_NUM          16                    /*!< Set to 8 or 16, the number of SPMP entries */

#ifndef __TEE_PRESENT
#define __TEE_PRESENT             0                     /*!< Set to 1 if TEE is present */
#endif

#ifndef RUNMODE_CONTROL
#define __ICACHE_PRESENT          1                     /*!< Set to 1 if I-Cache is present */
#define __DCACHE_PRESENT          0                     /*!< Set to 1 if D-Cache is present */
#define __CCM_PRESENT             1                     /*!< Set to 1 if Cache Control and Mantainence Unit is present */
#endif

/* TEE feature depends on PMP */
#if defined(__TEE_PRESENT) && (__TEE_PRESENT == 1)
#if !defined(__PMP_PRESENT) || (__PMP_PRESENT != 1)
#error "__PMP_PRESENT must be defined as 1!"
#endif /* !defined(__PMP_PRESENT) || (__PMP_PRESENT != 1) */
#if !defined(__SPMP_PRESENT) || (__SPMP_PRESENT != 1)
#error "__SPMP_PRESENT must be defined as 1!"
#endif /* !defined(__SPMP_PRESENT) || (__SPMP_PRESENT != 1) */
#endif /* defined(__TEE_PRESENT) && (__TEE_PRESENT == 1) */

#ifndef __INC_INTRINSIC_API
#define __INC_INTRINSIC_API       0                     /*!< Set to 1 if intrinsic api header files need to be included */
#endif

#define __Vendor_SysTickConfig    0                     /*!< Set to 1 if different SysTick Config is used */
#define __Vendor_EXCEPTION        0                     /*!< Set to 1 if vendor exception hander is present */

/** @} */ /* End of group Configuration_of_NMSIS */


/* Define boot hart id */
#ifndef BOOT_HARTID
#define BOOT_HARTID               0                     /*!< Choosen boot hart id in current cluster when in soc system, need to align with the value defined in startup_<Device>.S, should start from 0, taken the mhartid bit 0-7 value */
#endif

#include <nmsis_core.h>                         /*!< Nuclei N/NX class processor and core peripherals */
/* ToDo: include your system_mars.h file
         replace 'Device' with your device name */
#include "system_RISCVN300.h"                    /*!< riscv N300 System */

/* ========================================  Start of section using anonymous unions  ======================================== */
#if   defined (__GNUC__)
/* anonymous unions are enabled by default */
#else
#warning Not supported compiler type
#endif

#define RTC_FREQ                    32768
// The TIMER frequency is just the RTC frequency
#define SOC_TIMER_FREQ              1000000





/* =========================================================================================================================== */
/* ================                            Device Specific Peripheral Section                             ================ */
/* =========================================================================================================================== */



/*****************************************************************************
 * Header for All peripherals
 ****************************************************************************/
#include "mars_base.h"

#include "aon_ctrl_reg.h"
#include "aon_iomux_reg.h"
#include "aon_timer_reg.h"
#include "aon_wdt_reg.h"
#include "cmn_buscfg_reg.h"
#include "cmn_syscfg_reg.h"
#include "core_iomux_reg.h"
#include "dualtimers_reg.h"
#include "gpadc_reg.h"
#include "gpio_reg.h"
#include "gpt_reg.h"
#include "i2c_reg.h"
#include "ir_reg.h"
#include "keysense_reg.h"
#include "spi_reg.h"
#include "uart_reg.h"
#include "apc_reg.h"
#include "audio_codec_reg.h"




/************************************************************************************
 * Linker definitions
 ************************************************************************************/

//fast text/data sections in fast memory
#define _FAST_TEXT_SEC              ".fast_text" // ".itcm"
#define _FAST_DATA_SEC              ".fast_data" // ".dtcm"
#define _FAST_BSS_SEC               ".fast_bss" //
#define _RAM_DATA_SEC               ".ram_data"

//fast function
#define _FAST_TEXT              __attribute__ ((section (_FAST_TEXT_SEC)))
#define _FAST_TEXT_TAG(tag)     __attribute__ ((section (_FAST_TEXT_SEC"."#tag)))
#define _FAST_FUNC_RO           //_FAST_TEXT

//fast data (initialized)
#define _FAST_DATA              __attribute__ ((section (_FAST_DATA_SEC)))
#define _FAST_DATA_TAG(tag)     __attribute__ ((section (_FAST_DATA_SEC"."#tag)))
#define _FAST_DATA_VI           //_FAST_DATA

//fast data (zero initialized)
#define _FAST_BSS               __attribute__ ((section (_FAST_BSS_SEC)))
#define _FAST_BSS_TAG(tag)      __attribute__ ((section (_FAST_BSS_SEC"."#tag)))
#define _FAST_DATA_ZI           //_FAST_BSS

#define _DMA                    _FAST_BSS //_FAST_DATA_ZI
#define _DMA_PRAM               __attribute__((aligned(32)))

#define _RAM_DATA               __attribute__ ((section (_RAM_DATA_SEC)))
#define _RAM_DATA_TAG(tag)      __attribute__ ((section (_RAM_DATA_SEC"."#tag)))


/************************************************************************************
 * Cache definitions
 ************************************************************************************/
enum cache_t { ICACHE, DCACHE };

static inline unsigned long ICACHE_WAY(enum cache_t cache)
{
    return 2;
}

static inline unsigned long DCACHE_WAY(enum cache_t cache)
{
    return 4;
}

static inline unsigned long CACHE_LINE_SIZE(enum cache_t cache)
{
    return 32;
}

/** @addtogroup Device_Peripheral_peripherals
  * @{
  */

/****************************************************************************
 * Platform definitions
 *****************************************************************************/

#define IP_SYSCTRL             ((CMN_SYSCFG_RegDef *) CMN_SYSCFG_BASE)
#define IP_SYSNODEF            ((CMN_BUSCFG_RegDef *) CMN_BUSCFG_BASE)
#define IP_CMN_IOMUX           ((CORE_IOMUX_RegDef *) CORE_IOMUX_BASE)
#define IP_AON_IOMUX           ((AON_IOMUX_RegDef *) AON_IOMUX_BASE)
#define IP_UART0               ((UART_RegDef *) UART0_BASE)
#define IP_UART1               ((UART_RegDef *) UART1_BASE)
#define IP_I2C0                ((I2C_RegDef *) I2C0_BASE)
#define IP_SPI0                ((SPI_RegDef *) SPI0_BASE)
#define IP_FLASH_CTRL          ((FLASHC_RegDef *) FLASH_CTRL_BASE)
#define IP_AUDIO_APC           ((APC_RegDef *) APC_BASE)
#define IP_AUDIO_CODEC         ((AUDIO_CODEC_RegDef *) AUDIO_CODEC_BASE)
#define IP_GPIO                ((GPIO_RegDef *) GPIO0_BASE)
#define IP_AON_CTRL            ((AON_CTRL_RegDef *) AON_CTRL_BASE)
#define IP_GPADC               ((GPADC_RegDef *) GPADC_BASE)
#define IP_IR                  ((IR_RegDef *) IR_BASE)
#define IP_AON_TIMER           ((AON_TIMER_RegDef*) AON_TIMER_BASE)
#define IP_AON_WDT             ((AON_WDT_RegDef*) AON_WDT_BASE)

#define IP_KEYSENSE0           ((KEYSENSE_RegDef *) KEYSENSE0_BASE)
#define IP_DUALTIMERS0         ((DUALTIMERS_RegDef *) DUALTIMERS0_BASE)
#define IP_GPT                 ((GPT_RegDef *) GPT_BASE)

#define AP_DMA_Channel0        ((DMA_Channel_TypeDef *) AP_DMAC_Channel0_BASE)
#define AP_DMA_Channel1        ((DMA_Channel_TypeDef *) AP_DMAC_Channel1_BASE)
#define AP_DMA_Channel2        ((DMA_Channel_TypeDef *) AP_DMAC_Channel2_BASE)
#define AP_DMA_Channel3        ((DMA_Channel_TypeDef *) AP_DMAC_Channel3_BASE)
#define AP_DMA_Channel4        ((DMA_Channel_TypeDef *) AP_DMAC_Channel4_BASE)
#define AP_DMA_Channel5        ((DMA_Channel_TypeDef *) AP_DMAC_Channel5_BASE)
#define AP_DMA_Channel6        ((DMA_Channel_TypeDef *) AP_DMAC_Channel6_BASE)
#define AP_DMA_Channel7        ((DMA_Channel_TypeDef *) AP_DMAC_Channel7_BASE)
/*@}*/ /* end of group mars_Peripherals */

/*****************************************************************************
 * System clock
 ****************************************************************************/

// IC_BOARD == 1 ---> ASIC
// IC_BOARD == 0 ---> FPGA

#if IC_BOARD // ASIC

#define DEF_MAIN_FREQUENCE    (300000000) // 300 MHz
//#define DEF_MAIN_FREQUENCE    (24000000) // 24 MHz

static inline uint32_t CPUFREQ() {
    extern uint32_t CRM_GetCoreFreq();
    return CRM_GetCoreFreq();
}

static inline uint32_t HCLKFREQ() {
    extern uint32_t CRM_GetHclkFreq();
    return CRM_GetHclkFreq();
}

static inline uint32_t PCLKFREQ() {
    extern uint32_t CRM_GetPeri_pclkFreq();
    return CRM_GetPeri_pclkFreq();
}

#else // FPGA

#define DEF_MAIN_FREQUENCE    (24000000) // 24MHZ

static inline uint32_t CPUFREQ() {
    return (DEF_MAIN_FREQUENCE);
}

static inline uint32_t HCLKFREQ() {
    return (DEF_MAIN_FREQUENCE);
}

static inline uint32_t PCLKFREQ() {
    return (DEF_MAIN_FREQUENCE);
}

#endif // IC_BOARD


// Is Global Interrupt enabled? 0 = disabled, 1 = enabled
static inline uint8_t GINT_enabled()
{
    uint32_t ret = __RV_CSR_READ(CSR_MSTATUS);
    return (ret == MSTATUS_MIE ? 0 : 1);
}

// Enable GINT
static inline void enable_GINT()
{
    __enable_irq();
}

// Disable GINT
static inline void disable_GINT()
{
    __disable_irq();
}

// ISR function prototype
typedef void (*ISR)(void);

// Register ISR into Interrupt Vector Table
void register_ISR(uint32_t irq_no, ISR isr, ISR* isr_old);


static inline uint8_t IRQ_enabled(uint32_t irq_no)
{
    return (uint8_t)(ECLIC_GetEnableIRQ(irq_no));
}

static inline void enable_IRQ(uint32_t irq_no)
{
    ECLIC_EnableIRQ(irq_no);
}

static inline void disable_IRQ(uint32_t irq_no)
{
    ECLIC_DisableIRQ(irq_no);
}

static inline void clear_IRQ(uint32_t irq_no)
{
    ECLIC_ClearPendingIRQ(irq_no);
}

#define inw(reg)               (*((volatile unsigned int *) (reg)))
#define outw(reg, data)        ((*((volatile unsigned int *)(reg)))=(unsigned int)(data))
#define inb(reg)               (*((volatile unsigned char *) (reg)))
#define outb(reg, data)        ((*((volatile unsigned char *)(reg)))=(unsigned char)(data))


/** @addtogroup Device_Peripheral_declaration
  * @{
  */
extern uint32_t get_cpu_freq(void);
extern void delay_1ms(uint32_t count);

/** @} */ /* End of group mars */

/** @} */ /* End of group Nuclei */

#ifdef __cplusplus
}
#endif









/************************************************************************************
 * DMA definitions
 ************************************************************************************/
// TODO: modify DMA definitions accordingly

// Hardware handshaking ID of each peripheral with DMA Controller
// default configuration (ap_dma_hs_sel_x = 0)
#define DMA_HSID_UART0_RX       0
#define DMA_HSID_UART0_TX       1
#define DMA_HSID_UART1_RX       2
#define DMA_HSID_UART1_TX       3
#define DMA_HSID_SPI0_RX        4
#define DMA_HSID_SPI0_TX        5
#define DMA_HSID_APC_RX0        6
#define DMA_HSID_APC_TX0        7
#define DMA_HSID_APC_RX1        8
#define DMA_HSID_APC_TX1        9
#define DMA_HSID_IR_RX          10
#define DMA_HSID_IR_TX          11
#define DMA_HSID_GPADC          12
#define DMA_HSID_I2C0           13
#define DMA_HSID_GPT_RX0        14
#define DMA_HSID_GPT_RX1        15

// Number of DMA channels & FIFO depth of each channel
#define DMA_NUMBER_OF_CHANNELS           ((uint8_t) 4)
static const uint8_t DMA_CHANNELS_FIFO_DEPTH[DMA_NUMBER_OF_CHANNELS] = {64, 64, 64, 64}; //in bytes

//default or preferred DMA channel definition
#define DMA_CH_UART_TX_DEF          ((uint8_t) 2)   // UART
#define DMA_CH_UART_RX_DEF          ((uint8_t) 1)   // UART
#define DMA_CH_SPI_TX_DEF           ((uint8_t) 2)   // SPI
#define DMA_CH_SPI_RX_DEF           ((uint8_t) 1)   // SPI
#define DMA_CH_AUD_TX_DEF           ((uint8_t) 2)   // CLASSD/I2S TX
#define DMA_CH_AUD_RX_DEF           ((uint8_t) 1)   // ADC/DMIC/I2S RX
#define DMA_CH_AUD_ECHO_DEF         ((uint8_t) 3)   // CLASSD/I2S TX LOOPBACK

// AHB master interface of memory that stores LLI (Linked List Item) for channel
#define DMAH_CH_LMS     0

#endif  /* __MARS_H__ */
