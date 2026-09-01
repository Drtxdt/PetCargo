/**
  ******************************************************************************
  * @file    Driver_PMU.h
  * @author  ListenAI Application Team
  * @brief   Header file of PMU HAL module.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2021 ListenAI.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ListenAI under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */


/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __CSK_DRIVER_PMU_H
#define __CSK_DRIVER_PMU_H


#ifdef __cplusplus
 extern "C" {
#endif


/* Includes ------------------------------------------------------------------*/
#include "Driver_Common.h"
#include "chip.h"

/** @addtogroup CSK_HAL_Driver
  * @{
  */

/** @addtogroup PMU
  * @{
  */


/* Exported types ------------------------------------------------------------*/
/** @defgroup PMU_Exported_Types PMU Exported Types
  * @{
  */
 /**
  * @brief PMU sleep mode enumeration.
  *
  * This enum defines the possible Deep Sleep modes that can be used with the
  * HAL_PMU_EnterDeepSleepMode function.
  */
 typedef enum _pmu_sleepmode {
     PMU_SLEEPMODE_MODE1 = 0x1U,
     PMU_SLEEPMODE_MODE2,	//aon_sub module can wake up this sleep mode
     PMU_SLEEPMODE_MODE3,	//normal run
 }pmu_sleepmode_t;


typedef enum _pmu_wakeupsrc {
    PMU_WAKEUP_TIMER = 0x0U,
    PMU_WAKEUP_IWDT,
    PMU_WAKEUP_KEY,
    PMU_WAKEUP_GPIOB_00 = 0x8U,
    PMU_WAKEUP_GPIOB_01,
    PMU_WAKEUP_GPIOB_02,
    PMU_WAKEUP_GPIOB_03,
    PMU_WAKEUP_GPIOB_04,
    PMU_WAKEUP_GPIOB_05,
    PMU_WAKEUP_GPIOB_06,
    PMU_WAKEUP_GPIOB_07,
    PMU_WAKEUP_GPIOB_08,
    PMU_WAKEUP_GPIOB_09,
    PMU_WAKEUP_GPIOB_10,
    PMU_WAKEUP_GPIOB_11,
    PMU_WAKEUP_GPIOB_12,
    PMU_WAKEUP_NONE = 0xFFU,
}pmu_wakeupsrc_t;


typedef enum _pmu_rstsrc {
    PMU_RST_POR = 0x0U,
    PMU_RST_AON = 0x1U,
    PMU_RST_SYSCFG_P_AP = 0x10U,
    PMU_RST_SYSRSTREQ = 0x11,
    PMU_RST_NONE = 0xFFU,
}pmu_rstsrc_t;


typedef enum _pmu_gpio_src {
    PMU_POLARITY_GPIOB_00 = 0x0U,
    PMU_POLARITY_GPIOB_01,
    PMU_POLARITY_GPIOB_02,
    PMU_POLARITY_GPIOB_03,
    PMU_POLARITY_GPIOB_04,
    PMU_POLARITY_GPIOB_05,
    PMU_POLARITY_GPIOB_06,
    PMU_POLARITY_GPIOB_07,
    PMU_POLARITY_GPIOB_08,
    PMU_POLARITY_GPIOB_09,
    PMU_POLARITY_GPIOB_10,
    PMU_POLARITY_GPIOB_11,
    PMU_POLARITY_GPIOB_12,
	GPIOB_COUNT
}pmu_gpio_src_t;

/**
  * @}
  */

/* Exported constants --------------------------------------------------------*/

/** @defgroup PMU_Exported_Constants PMU Exported Constants
  * @{
  */
#define CSK_PMU_API_VERSION CSK_DRIVER_VERSION_MAJOR_MINOR(1,0)
#define CSK_PMU_DRV_VERSION CSK_DRIVER_VERSION_MAJOR_MINOR(1,1)


/*---------------------Control mode for application---------------------------------*/
/** @defgroup PMU_HOLD_mode_entry PMU SLEEP mode entry
  * @{
  */
#define PMU_HOLDENTRY_WFI              ((uint8_t)0x01)
#define PMU_HOLDENTRY_WFE              ((uint8_t)0x02)
/**
  * @}
  */

/** @defgroup PMU_LIGHTSLEEP_mode_entry PMU STOP mode entry
  * @{
  */
#define PMU_LIGHTSLEEPENTRY_WFI               ((uint8_t)0x01)
#define PMU_LIGHTSLEEPENTRY_WFE               ((uint8_t)0x02)
/**
  * @}
  */

/** @defgroup PMU_DEEPSLEEP_mode_entry PMU STOP mode entry
  * @{
  */
#define PMU_DEEPSLEEPENTRY_WFI               ((uint8_t)0x01)
#define PMU_DEEPSLEEPENTRY_WFE               ((uint8_t)0x02)
/**
  * @}
  */

/**
  * @}
  */

/* Exported macro ------------------------------------------------------------*/
/** @defgroup PMU_Exported_Macro PMU Exported Macro
  * @{
  */


/**
  * @}
  */


/* Exported functions --------------------------------------------------------*/
/** @addtogroup PMU_Exported_Functions PMU Exported Functions
  * @{
  */

/** @addtogroup PMU_Exported_Functions_Group2 Peripheral Control functions
  * @{
  */


/**
 * @brief Get the system reset cause.
 *
 * This function retrieves the system reset cause from the AON status register
 * and then clears the reset cause.
 *
 * @return The reset source as defined in pmu_rstsrc_t.
 */
pmu_rstsrc_t HAL_PMU_GetSysResetCause(void);


/**
 * @brief Get the wake-up cause.
 *
 * This function checks the PMU wake-up source register and returns the first
 * set wake-up source it finds.
 *
 * @return The wake-up source as defined in pmu_wakeupsrc_t.
 *         Returns PMU_WAKEUP_NONE if no source is found.
 */
pmu_wakeupsrc_t HAL_PMU_GetWakeUpCause(void);


/**
 * @brief Clear the system reset cause.
 *
 * This function clears the system reset cause in the AON status register.
 */
void HAL_PMU_ClearSysResetCause(void);


/**
 * @brief Clear the wake-up cause.
 *
 * This function clears the wake-up cause in the PMU wake-up IRQ clear register.
 */
void HAL_PMU_ClearWakeUpCause(void);


/**
 * @brief Enable a specific wake-up source.
 *
 * This function enables a specified wake-up source in the PMU enable wake-up register.
 *
 * @param WakeUpSrc The wake-up source to enable, as defined in pmu_wakeupsrc_t.
 */
void HAL_PMU_EnableWakeUpSrc(pmu_wakeupsrc_t WakeUpSrc);


/**
 * @brief Disable a specific wake-up source.
 *
 * This function disables a specified wake-up source in the PMU enable wake-up register.
 *
 * @param WakeUpSrc The wake-up source to disable.
 */
void HAL_PMU_DisableWakeUpSrc(pmu_wakeupsrc_t WakeUpSrc);


/**
 * @brief Enable IRQ for a specific wake-up source.
 *
 * This function enables the interrupt for a specified wake-up source in the
 * PMU enable wake-up IRQ register.
 *
 * @param WakeUpSrc The wake-up source for which to enable the IRQ, as defined in pmu_wakeupsrc_t.
 */
void HAL_PMU_EnableWakeUpSrcIrq(pmu_wakeupsrc_t WakeUpSrc);


/**
 * @brief Disable IRQ for a specific wake-up source.
 *
 * This function disables the interrupt for a specified wake-up source in the
 * PMU enable wake-up IRQ register.
 *
 * @param WakeUpSrc The wake-up source for which to enable the IRQ, as defined in pmu_wakeupsrc_t.
 */
void HAL_PMU_DisableWakeUpSrcIrq(pmu_wakeupsrc_t WakeUpSrc);


/**
 * @brief Select GPIO polarity for wake-up.
 *
 * This function sets the polarity for a specific GPIO used as a wake-up source.
 * The polarity can be set to either high or low.
 *
 * @param GpioPos The position of the GPIO in the wake-up source, as defined in pmu_gpio_src_t.
 * @param polarity The polarity to be set for the specified GPIO.
 *                - 0: Set the polarity to low.
 *                - 1: Set the polarity to high.
 */
void HAL_PMU_GPIOPolaritySelect(pmu_gpio_src_t GpioPos, uint8_t polarity);



/**
 * @brief Enter Hold Mode.
 *
 * This function puts the system into Hold mode. It chooses between Wait For Interrupt (WFI)
 * and Wait For Event (WFE) to enter sleep mode based on the SLEEPEntry parameter.
 *
 * @param SLEEPEntry Determines the method to enter sleep mode. Use PMU_HOLDENTRY_WFI for
 *        Wait For Interrupt, or other values for Wait For Event.
 */
void HAL_PMU_EnterHoldMode(uint8_t SLEEPEntry);


/**
 * @brief Enter Deep Sleep Mode.
 *
 * This function puts the system into a specified Deep Sleep mode. The sleep mode is selected
 * through the SleepMode parameter, and the method to enter sleep (WFI or WFE) is determined by
 * the SLEEPEntry parameter.
 *
 * @param SleepMode The deep sleep mode to enter, as defined by pmu_sleepmode_t.
 * @param SLEEPEntry Determines the method to enter sleep mode. Use PMU_HOLDENTRY_WFI for
 *        Wait For Interrupt, or other values for Wait For Event.
 */
void HAL_PMU_EnterDeepSleepMode(pmu_sleepmode_t SleepMode, uint8_t SLEEPEntry);

/**
 * @brief Enables RAM retention for the specified RAM bank.
 *
 * @note This function performs no action if an invalid RAM bank is specified. For CPU and Wi-Fi RAM banks,
 *       the function modifies the RAM_RETENTION_SEL_L bits in the RAM_RETENTION_SEL register. The PMU_RAMBANK_NONE
 *       value is considered invalid for enabling RAM retention.
 */
void HAL_PMU_EnableRamRetention();


/**
 * @brief Disables RAM retention for the specified RAM bank.
 *
 * @note This function performs no action if an invalid RAM bank is specified. For CPU and Wi-Fi RAM banks,
 *       the function modifies the RAM_RETENTION_SEL_L bits in the REG_PMU_CTRL3 register to clear the retention setting.
 *       For the Bluetooth RAM bank, it clears a specific bit (bit 28) in the REG_PMU_CTRL4 register. The PMU_RAMBANK_NONE
 *       value is considered invalid for disabling RAM retention.
 */
void HAL_PMU_DisableRamRetention();

void HAL_PMU_EntryAddress_Set(uint32_t EntryAddress);


/**
 * @brief Enable reset for UART0.
 *
 * This macro sets the UART0_RESET bit in REG_RST_CTRL0 register to 1,
 * which triggers a reset for UART0.
 */
#define __HAL_PMU_UART0_RST_ENABLE()    \
do { \
    IP_SYSCTRL->REG_RST_CTRL0.bit.UART0_RESET = 0x1; \
} while(0)


/**
 * @brief Enable reset for UART1.
 *
 * This macro sets the UART1_RESET bit in REG_RST_CTRL0 register to 1,
 * which triggers a reset for UART1.
 */
#define __HAL_PMU_UART1_RST_ENABLE()    \
do { \
    IP_SYSCTRL->REG_RST_CTRL0.bit.UART1_RESET = 0x1; \
} while(0)


/**
 * @brief Enable reset for IR.
 *
 * This macro sets the IR_RESET bit in REG_RST_CTRL0 register to 1,
 * which triggers a reset for IR.
 */
#define __HAL_PMU_IR_RST_ENABLE()    \
do { \
    IP_SYSCTRL->REG_RST_CTRL0.bit.IR_RESET = 0x1; \
} while(0)


/**
 * @brief Enable reset for SPI0.
 *
 * This macro sets the SPI0_RESET bit in REG_RST_CTRL0 register to 1,
 * which triggers a reset for SPI0.
 */
#define __HAL_PMU_SPI0_RST_ENABLE()    \
do { \
    IP_SYSCTRL->REG_RST_CTRL0.bit.SPI0_RESET = 0x1; \
} while(0)


/**
 * @brief Enable reset for I2C0.
 *
 * This macro sets the I2C0_RESET bit in REG_RST_CTRL0 register to 1,
 * which triggers a reset for I2C0.
 */
#define __HAL_PMU_I2C0_RST_ENABLE()    \
do { \
    IP_SYSCTRL->REG_RST_CTRL0.bit.I2C0_RESET = 0x1; \
} while(0)


/**
 * @brief Enable reset for GPIO0.
 *
 * This macro sets the GPIO0_RESET bit in REG_RST_CTRL0 register to 1,
 * which triggers a reset for GPIO0.
 */
#define __HAL_PMU_GPIO0_RST_ENABLE()    \
do { \
    IP_SYSCTRL->REG_RST_CTRL0.bit.GPIO0_RESET = 0x1; \
} while(0)


/**
 * @brief Enable reset for GPADC.
 *
 * This macro sets the GPADC_RESET bit in REG_RST_CTRL0 register to 1,
 * which triggers a reset for GPADC.
 */
#define __HAL_PMU_GPADC_RST_ENABLE()    \
do { \
	IP_SYSCTRL->REG_RST_CTRL0.bit.GPADC_RESET = 0x1; \
} while(0)


/**
 * @brief Enable reset for DMA.
 *
 * This macro sets the DMA_RESET bit in REG_RST_CTRL0 register to 1,
 * which triggers a reset for DMA.
 */
#define __HAL_PMU_DMA_RST_ENABLE()    \
do { \
	IP_SYSCTRL->REG_RST_CTRL0.bit.DMA_RESET = 0x1; \
} while(0)


/**
 * @brief Enable reset for FLASHC.
 *
 * This macro sets the FLASHC_RESET bit in REG_RST_CTRL0 register to 1,
 * which triggers a reset for FLASHC.
 */
#define __HAL_PMU_FLASHC_RST_ENABLE()    \
do { \
	IP_SYSCTRL->REG_RST_CTRL0.bit.FLASHC_RESET = 0x1; \
} while(0)


/**
 * @brief Enable reset for GPT.
 *
 * This macro sets the GPT_RESET bit in REG_RST_CTRL0 register to 1,
 * which triggers a reset for GPT.
 */
#define __HAL_PMU_GPT_RST_ENABLE()    \
do { \
	IP_SYSCTRL->REG_RST_CTRL0.bit.GPT_RESET = 0x1; \
} while(0)


/**
 * @brief Enable reset for I2S.
 *
 * This macro sets the I2S_RESET bit in REG_RST_CTRL0 register to 1,
 * which triggers a reset for I2S.
 */
#define __HAL_PMU_I2S_RST_ENABLE()    \
do { \
	IP_SYSCTRL->REG_RST_CTRL0.bit.I2S_RESET = 0x1; \
} while(0)


/**
 * @brief Enable reset for CODEC.
 *
 * This macro sets the CODEC_REG_RESET bit in REG_RST_CTRL0 register to 1,
 * which triggers a reset for CODEC.
 */
#define __HAL_PMU_CODEC_RST_ENABLE()    \
do { \
	IP_SYSCTRL->REG_RST_CTRL0.bit.CODEC_REG_RESET = 0x1; \
} while(0)


/**
 * @brief Enable reset for CODEC_ASYNC.
 *
 * This macro sets the CODEC_ASYNC_RESET bit in REG_RST_CTRL0 register to 1,
 * which triggers a reset for CODEC_ASYNC.
 */
#define __HAL_PMU_CODEC_ASYNC_RST_ENABLE()    \
do { \
	IP_SYSCTRL->REG_RST_CTRL0.bit.CODEC_ASYNC_RESET = 0x1; \
} while(0)


/**
 * @brief Enable reset for APC.
 *
 * This macro sets the APC_REG_RESET bit in REG_RST_CTRL0 register to 1,
 * which triggers a reset for APC.
 */
#define __HAL_PMU_APC_REG_RST_ENABLE()    \
do { \
	IP_SYSCTRL->REG_RST_CTRL0.bit.APC_REG_RESET = 0x1; \
} while(0)


/**
 * @brief Enable reset for APC_CORE.
 *
 * This macro sets the APC_CORE_RESET bit in REG_RST_CTRL0 register to 1,
 * which triggers a reset for APC_CORE.
 */
#define __HAL_PMU_APC_CORE_RST_ENABLE()    \
do { \
	IP_SYSCTRL->REG_RST_CTRL0.bit.APC_CORE_RESET = 0x1; \
} while(0)


/**
 * @brief Enable reset for MCU_CORE.
 *
 * This macro sets the MCU_CORE_RESET bit in REG_RST_CTRL0 register to 1,
 * which triggers a reset for MCU_CORE.
 */
#define __HAL_PMU_MCU_CORE_RST_ENABLE()    \
do { \
	IP_SYSCTRL->REG_RST_CTRL0.bit.MCU_CORE_RESET = 0x1; \
} while(0)


/**
 * @brief Enable reset for LUNA.
 *
 * This macro sets the LUNA_RESET bit in REG_RST_CTRL0 register to 1,
 * which triggers a reset for LUNA.
 */
#define __HAL_PMU_LUNA_RST_ENABLE()    \
do { \
	IP_SYSCTRL->REG_RST_CTRL0.bit.LUNA_RESET = 0x1; \
} while(0)


/**
 * @brief Enable reset for CORE and SubSystem.
 *
 * This macro sets the SYSRSTREQ2AP_RST_EN bit in REG_RST_CTRL0 register to 1,
 * Trigger SoftReset in Core.
 * which triggers a reset for CORE&SubSystem.
 */
#define __HAL_PMU_SYSRSTREQ_RST_ENABLE()    \
do { \
	IP_SYSCTRL->REG_RST_CTRL0.bit.SYSRSTREQ2AP_RST_EN = 0x1; \
	SysTimer_SoftwareReset(); \
} while(0)


/**
 * @brief Enable reset for AON and core subsystem.
 *
 * This macro sets the AON_RESET bit in REG_SW_RESET_AON register to 1,
 * which triggers a reset for AON and core subsystem.
 */
#define __HAL_PMU_SW_RST_AON_RST_ENABLE()    \
do { \
	IP_AON_CTRL->REG_AON_SW_RESET.all = 0xCAFE000A; \
} while(0)



/**
 * @brief Enable reset for AON TIMER.
 *
 * This macro sets the AON_TIMER_RESET bit in AON_CRM0 register to 1,
 * which triggers a reset for AON TIMER.
 */
#define __HAL_PMU_AON_TIMER_ENABLE()    \
do { \
	IP_AON_CTRL->REG_AON_RST_CTRL.bit.AON_TIMER_RESET = 1; \
} while(0)


/**
 * @brief Enable reset for Keysense.
 *
 * This macro sets the KEYSENSE_RESET bit in AON_CRM0 register to 1,
 * which triggers a reset for Keysense.
 */
#define __HAL_PMU_KEYSENSE_ENABLE()    \
do { \
	IP_AON_CTRL->REG_AON_RST_CTRL.bit.KEYSENSE_RESET = 1; \
} while(0)


/**
 * @brief Enable reset for AON IOMUX.
 *
 * This macro sets the AON_IOMUX_RESET bit in AON_CRM0 register to 1,
 * which triggers a reset for AON IOMUX.
 */
//#define __HAL_PMU_AON_IOMUX_ENABLE()    \
//do { \
//	IP_AON_CTRL->AON_CRM0.bit.AON_IOMUX_RESET = 1; \
//} while(0)


/**
 * @brief Enable LDO for the core.
 *
 * This macro enables the LDO for the core by setting the EN_LDO_CORE_FRCDATA and
 * EN_LDO_CORE_FRC bits in the REG_AON_FORCE_DATA and REG_AON_FORCE_CTRL registers.
 */
#define __HAL_PMU_LDO_CORE_ENABLE()    \
do { \
    IP_AON_CTRL->REG_AON_FORCE_DATA.bit.EN_LDO_CORE_FRCDATA = 1; \
    IP_AON_CTRL->REG_AON_FORCE_CTRL.bit.EN_LDO_CORE_FRC = 1; \
} while(0)


/**
 * @brief Disable LDO for the core.
 *
 * This macro disables the LDO for the core by resetting the EN_LDO_CORE_FRCDATA and
 * EN_LDO_CORE_FRC bits in the REG_AON_FORCE_DATA and REG_AON_FORCE_CTRL registers.
 */
#define __HAL_PMU_LDO_CORE_DISABLE()    \
do { \
    IP_AON_CTRL->REG_AON_FORCE_DATA.bit.EN_LDO_CORE_FRCDATA = 0; \
    IP_AON_CTRL->REG_AON_FORCE_CTRL.bit.EN_LDO_CORE_FRC = 0; \
} while(0)


/**
 * @brief Enable LDO for VRAM.
 *
 * This macro enables the LDO for VRAM by setting the EN_LDO_VRAM_FRCDATA and
 * EN_LDO_VRAM_FRC bits in the REG_AON_FORCE_DATA and REG_AON_FORCE_CTRL registers.
 */
#define __HAL_PMU_LDO_VRAM_ENABLE()    \
do { \
    IP_AON_CTRL->REG_AON_FORCE_DATA.bit.EN_LDO_VRAM_FRCDATA = 1; \
    IP_AON_CTRL->REG_AON_FORCE_CTRL.bit.EN_LDO_VRAM_FRC = 1; \
} while(0)


/**
 * @brief Disable LDO for VRAM.
 *
 * This macro disables the LDO for VRAM by resetting the EN_LDO_VRAM_FRCDATA and
 * EN_LDO_VRAM_FRC bits in the REG_AON_FORCE_DATA and REG_AON_FORCE_CTRL registers.
 */
#define __HAL_PMU_LDO_VRAM_DISABLE()    \
do { \
    IP_AON_CTRL->REG_AON_FORCE_DATA.bit.EN_LDO_VRAM_FRCDATA = 0; \
    IP_AON_CTRL->REG_AON_FORCE_CTRL.bit.EN_LDO_VRAM_FRC = 0; \
} while(0)


/**
 * @brief Enable UVLO for VIO.
 *
 * This macro enables the Under-Voltage Lock-Out (UVLO) for VIO by setting the
 * EN_UVLO_VIO_FRCDATA and EN_UVLO_VIO_FRC bits in the REG_AON_FORCE_DATA and
 * REG_AON_FORCE_CTRL registers.
 */
#define __HAL_PMU_UVLO_VIO_ENABLE()    \
do { \
    IP_AON_CTRL->REG_AON_FORCE_DATA.bit.EN_UVLO_VIO_FRCDATA = 1; \
    IP_AON_CTRL->REG_AON_FORCE_CTRL.bit.EN_UVLO_VIO_FRC = 1; \
} while(0)


/**
 * @brief Disable UVLO for VIO.
 *
 * This macro disables the Under-Voltage Lock-Out (UVLO) for VIO by resetting the
 * EN_UVLO_VIO_FRCDATA and EN_UVLO_VIO_FRC bits in the REG_AON_FORCE_DATA and
 * REG_AON_FORCE_CTRL registers.
 */
#define __HAL_PMU_UVLO_VIO_DISABLE()    \
do { \
    IP_AON_CTRL->REG_AON_FORCE_DATA.bit.EN_UVLO_VIO_FRCDATA = 0; \
    IP_AON_CTRL->REG_AON_FORCE_CTRL.bit.EN_UVLO_VIO_FRC = 0; \
} while(0)


/**
 * @brief Enable LDO for VA.
 *
 * This macro enables the LDO for VA by setting the EN_LDO_VA bit in the
 * REG_AON_TUNE2 register.
 */
#define __HAL_PMU_LDO_VA_ENABLE()    \
do { \
    IP_AON_CTRL->REG_AON_TUNE2.bit.EN_LDO_VA = 1; \
} while(0)


/**
 * @brief Disable LDO for VA.
 *
 * This macro disables the LDO for VA by resetting the EN_LDO_VA bit in the
 * REG_AON_TUNE2 register.
 */
#define __HAL_PMU_LDO_VA_DISABLE()    \
do { \
    IP_AON_CTRL->REG_AON_TUNE2.bit.EN_LDO_VA = 0; \
} while(0)


/**
 * @brief Enable Capless LDO for VA.
 *
 * This macro enables the Capless LDO for VA by setting the EN_CAPLESS_LDO_VA bit in the
 * REG_AON_TUNE2 register.
 */
#define __HAL_PMU_CAPLESS_LDO_VA_ENABLE()    \
do { \
    IP_AON_CTRL->REG_AON_TUNE2.bit.EN_CAPLESS_LDO_VA = 1; \
} while(0)


/**
 * @brief Disable Capless LDO for VA.
 *
 * This macro disables the Capless LDO for VA by resetting the EN_CAPLESS_LDO_VA bit in the
 * REG_AON_TUNE2 register.
 */
#define __HAL_PMU_CAPLESS_LDO_VA_DISABLE()    \
do { \
    IP_AON_CTRL->REG_AON_TUNE2.bit.EN_CAPLESS_LDO_VA = 0; \
} while(0)


/**
 * @brief Enable DET_VCC.
 *
 * This macro enables DET_VCC by setting the RESV_15_15 bit in the
 * REG_AON_TUNE2 register.
 */
#define __HAL_PMU_DET_VCC_ENABLE()    \
do { \
    IP_AON_CTRL->REG_AON_TUNE2.bit.RESV_15_15 = 1; \
} while(0)


/**
 * @brief Disable DET_VCC.
 *
 * This macro disables DET_VCC by resetting the RESV_15_15 bit in the
 * REG_AON_TUNE2 register.
 */
#define __HAL_PMU_DET_VCC_DISABLE()    \
do { \
    IP_AON_CTRL->REG_AON_TUNE2.bit.RESV_15_15 = 0; \
} while(0)


/**
 * @brief Enable XO24M.
 *
 * This macro enables the XO24M by setting the ENA_XO24M_SW bit in the
 * REG_XO24M_CTRL register.
 */
#define __HAL_PMU_XO24M_ENABLE()    \
do { \
    IP_AON_CTRL->REG_XO24M_CTRL.bit.ENA_XO24M_SW = 1; \
} while(0)


/**
 * @brief Disable XO24M.
 *
 * This macro disables the XO24M by resetting the ENA_XO24M_SW bit in the
 * REG_XO24M_CTRL register.
 */
#define __HAL_PMU_XO24M_DISABLE()    \
do { \
    IP_AON_CTRL->REG_XO24M_CTRL.bit.ENA_XO24M_SW = 0; \
} while(0)


/**
 * @brief Enable RC32K.
 *
 * This macro enables the RC32K oscillator by resetting the RCO32K_PD_FRCDATA and
 * RCO32K_PD_FRC bits in the REG_AON_FORCE_DATA and REG_AON_FORCE_CTRL registers.
 */
#define __HAL_PMU_RC32K_FORCE_ENABLE()    \
do { \
    IP_AON_CTRL->REG_AON_FORCE_DATA.bit.RCO32K_PD_FRCDATA = 0; \
    IP_AON_CTRL->REG_AON_FORCE_CTRL.bit.RCO32K_PD_FRC = 0; \
} while(0)

/**
 * @brief Disable RC32K.
 *
 * This macro disables the RC32K oscillator by setting the RCO32K_PD_FRCDATA and
 * RCO32K_PD_FRC bits in the REG_AON_FORCE_DATA and REG_AON_FORCE_CTRL registers.
 */
#define __HAL_PMU_RC32K_FORCE_DISABLE()    \
do { \
    IP_AON_CTRL->REG_AON_FORCE_DATA.bit.RCO32K_PD_FRCDATA = 1; \
    IP_AON_CTRL->REG_AON_FORCE_CTRL.bit.RCO32K_PD_FRC = 1; \
} while(0)

// Disable RC32K in sleep mode
#define __HAL_PMU_RC32K_DISABLE()    \
do { \
    IP_AON_CTRL->REG_SLEEP_MODE.bit.PW_MODE2_PD_RCO32K = 1; \
} while(0)

/**
  * @}
  */


/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */

#ifdef __cplusplus
}
#endif

#endif /* __CSK_DRIVER_PMU_H */
