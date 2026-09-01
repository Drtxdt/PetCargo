
/**
  ******************************************************************************
  * @file    ClockManager.h
  * @author  ListenAI Application Team
  * @brief   Header file of CRM HAL module
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
#ifndef INCLUDE_DRIVER_CLOCKMANAGER_H_
#define INCLUDE_DRIVER_CLOCKMANAGER_H_

/* Includes ------------------------------------------------------------------*/
#include "Driver_Common.h"
#include "clock_config.h"
#include "chip.h"

/**
 * @enum _clock_src_name
 * @brief Enumeration of available clock sources in the system.
 *
 * This enumeration defines the different clock sources that can be used in the system.
 * It is used by various functions to select or identify the clock source for specific
 * operations or configurations. Each enumerator represents a unique clock source.
 */
typedef enum _clock_src_name {
    CRM_IpSrcInvalide       = 0x0U,

    CRM_IpSrcCoreClk,

    CRM_IpSrcXtalClk,

    CRM_IpSrcOscClk,

    CRM_IpSrcPeriClk,

    CRM_IpSrcFlashClk,

    CRM_IpSrcAudClk,

    CRM_IpSrcCmn32kClk,

    CRM_IpSrcAon32kClk,

}clock_src_name_t;

typedef enum _clock_src_core_div {
    
    CRM_IpCore_300MHz = 0,
    
    CRM_IpCore_240MHz = 1,
    
    CRM_IpCore_200MHz = 2,
    
    CRM_IpCore_150MHz = 3,
    
    CRM_IpCore_133MHz = 4,
    
    CRM_IpCore_120MHz = 5,
    
    CRM_IpCore_100MHz = 6,
    
} clock_src_core_div_t;

typedef enum _clock_src_flash_div {
    
    CRM_IpFlash_120MHz = 0,
    
    CRM_IpFlash_100MHz = 1,
    
    CRM_IpFlash_86MHz = 2,
    
    CRM_IpFlash_80MHz = 3,
    
} clock_src_flash_div_t;

/**********************************DEVICE************************************/

/** @defgroup _CRM_UART0 UART0_CLK_FUNC
  * @brief UART0 clock control function which can enable, disable or get status from corresponding
  * device, set ip clock source, set ip clock divider, or get ip clock configuration, help you calculate the ip divider
  * parameter when having ip reference clock and desire clock
  * @{
  */

/**
 * @brief Sets the clock source for Uart0.
 *
 * This function configures Uart0 to use a specific clock source as defined by the 'src' parameter.
 * The 'src' parameter should be one of the values defined in the 'clock_src_name_t' enumeration,
 * representing the various clock sources available in the system. This allows for flexible
 * configuration of the Uart0 clocking, depending on the system's requirements and the available
 * clock sources.
 *
 * @param src The desired clock source for Uart0. This should be a value from the
 *            'clock_src_name_t' enumeration that specifies which clock source to use that Can choose this source -> CRM_IpSrcXtalClk, CRM_IpSrcPeriClk.
 *
 * @return uint32_t Returns 0 on success, or a non-zero error code on failure. The error code
 *                  can indicate issues such as an invalid clock source or a failure in applying
 *                  the new clock source setting.
 *
 * @note The function's ability to change the clock source may depend on the current state of Uart0
 *       and the system's clock configuration. It is advisable to ensure that Uart0 is not actively
 *       transmitting data when changing its clock source.
 *
 * @warning Using an incorrect or unsupported clock source for Uart0 can lead to communication
 *          failures or system instability. Ensure that the selected clock source is compatible
 *          with Uart0's operational requirements.
 */
uint32_t HAL_CRM_SetUart0ClkSrc(clock_src_name_t src);

/**
 * @brief Sets the clock divider for Uart0.
 *
 * This function configures the clock division for Uart0 by setting the divider ratios
 * to the specified values. The division is defined by 'div_n' and 'div_m'.
 * These parameters determine how the input clock frequency is divided to derive the
 * desired Uart0 clock frequency.
 *
 * @param div_n The numerator part of the clock division ratio. Specifies the upper part
 *              of the division ratio, The div_n select range from [1 - 1023].
 *
 *
 * @param div_m The denominator part of the clock division ratio. Specifies the lower part
 *              of the division ratio, The div_m select range from [1 - 1023].
 *
 *
 * @return int32_t Returns 0 on success, or a non-zero error code on failure. The error code
 *                 typically indicates what went wrong during the configuration process.
 *
 * @note The exact behavior and limitations of the division ratio depend on the specific
 *       hardware capabilities and clock configuration. Ensure that the values of 'div_n' and 'div_m'
 *       are within the valid range for your hardware.
 *
 * @warning Improper configuration of the clock divider might disrupt UART0 communication.
 *          It's important to ensure that 'div_n' and 'div_m' are set to values that are
 *          compatible with the Uart0 specifications and the overall system clock settings.
 */
int32_t HAL_CRM_SetUart0ClkDiv(uint32_t div_n, uint32_t div_m);

/**
 * @brief Retrieves the clock configuration for UART0.
 *
 * This inline static function obtains the current clock source and division factors for the
 * UART0 module. The clock source and divider values are returned
 * through the pointer parameters 'src', 'div_n', and'div_m'.
 *
 * @param[out] src Pointer to a 'clock_src_name_t' variable where the clock source will be stored.
 *                 The clock source is indicated as an enumeration value of type 'clock_src_name_t'.
 *
 *
 * @param[out] div_n Pointer to a 'uint32_t' variable where the numerator of the clock division
 *                   ratio will be stored.
 *
 *
 * @param[out] div_m Pointer to a 'uint32_t' variable where the denominator of the clock division
 *                   ratio will be stored.
 *
 * @note Being an inline function, 'HAL_CRM_GetUart0ClkConfig()' is expanded at each point of call,
 *       which can increase code size but typically reduces execution time by avoiding a function
 *       call overhead. Use this function judiciously where performance is critical.
 *
 * @warning Ensure that the pointers passed to this function are valid and point to appropriate
 *          memory locations. Passing invalid pointers may lead to undefined behavior, including
 *          crashes. Also, consider the potential for race conditions if the clock configuration
 *          can be changed by other parts of the program while this function is being executed.
 */
inline static void HAL_CRM_GetUart0ClkConfig(clock_src_name_t* src, uint32_t* div_n, uint32_t* div_m){
    
    uint32_t src_t;
    src_t = IP_SYSCTRL->REG_PERI_CLK_CFG0.bit.SEL_UART0_CLK;
    
    if (src_t == 0){
        *src = CRM_IpSrcXtalClk;
    }
    
    if (src_t == 1){
        *src = CRM_IpSrcPeriClk;
    }
    
    
    
    *div_n = IP_SYSCTRL->REG_PERI_CLK_CFG0.bit.DIV_UART0_CLK_N;
    
    
    *div_m = IP_SYSCTRL->REG_PERI_CLK_CFG0.bit.DIV_UART0_CLK_M;
    
}

/**
 * @brief Retrieves the current operating frequency of UART0.
 *
 * This function returns the frequency (in Hz) at which the UART0
 * is currently operating. The frequency is calculated based on the current configuration of
 * the system's clock sources and the UART0 clock divider settings.
 *
 * @return uint32_t The operating frequency of UART0 in Hertz. If the frequency cannot be
 *                  determined, or if UART0 is not properly configured, the function may return
 *                  0.
 *
 * @note The returned frequency value is dependent on the current state of the system's clock
 *       configuration and the UART0 divider settings. Changes in these parameters can affect
 *       the UART0 frequency.
 *
 * @warning Ensure that UART0 and its clock sources are properly configured before calling this
 *          function. Calling this function without proper initialization may lead to undefined
 *          behavior or incorrect frequency values.
 */
uint32_t CRM_GetUart0Freq();
/**
  * @}
  */

/** @defgroup _CRM_UART1 UART1_CLK_FUNC
  * @brief UART1 clock control function which can enable, disable or get status from corresponding
  * device, set ip clock source, set ip clock divider, or get ip clock configuration, help you calculate the ip divider
  * parameter when having ip reference clock and desire clock
  * @{
  */

/**
 * @brief Sets the clock source for Uart1.
 *
 * This function configures Uart1 to use a specific clock source as defined by the 'src' parameter.
 * The 'src' parameter should be one of the values defined in the 'clock_src_name_t' enumeration,
 * representing the various clock sources available in the system. This allows for flexible
 * configuration of the Uart1 clocking, depending on the system's requirements and the available
 * clock sources.
 *
 * @param src The desired clock source for Uart1. This should be a value from the
 *            'clock_src_name_t' enumeration that specifies which clock source to use that Can choose this source -> CRM_IpSrcXtalClk, CRM_IpSrcPeriClk.
 *
 * @return uint32_t Returns 0 on success, or a non-zero error code on failure. The error code
 *                  can indicate issues such as an invalid clock source or a failure in applying
 *                  the new clock source setting.
 *
 * @note The function's ability to change the clock source may depend on the current state of Uart1
 *       and the system's clock configuration. It is advisable to ensure that Uart1 is not actively
 *       transmitting data when changing its clock source.
 *
 * @warning Using an incorrect or unsupported clock source for Uart1 can lead to communication
 *          failures or system instability. Ensure that the selected clock source is compatible
 *          with Uart1's operational requirements.
 */
uint32_t HAL_CRM_SetUart1ClkSrc(clock_src_name_t src);

/**
 * @brief Sets the clock divider for Uart1.
 *
 * This function configures the clock division for Uart1 by setting the divider ratios
 * to the specified values. The division is defined by 'div_n' and 'div_m'.
 * These parameters determine how the input clock frequency is divided to derive the
 * desired Uart1 clock frequency.
 *
 * @param div_n The numerator part of the clock division ratio. Specifies the upper part
 *              of the division ratio, The div_n select range from [1 - 1023].
 *
 *
 * @param div_m The denominator part of the clock division ratio. Specifies the lower part
 *              of the division ratio, The div_m select range from [1 - 1023].
 *
 *
 * @return int32_t Returns 0 on success, or a non-zero error code on failure. The error code
 *                 typically indicates what went wrong during the configuration process.
 *
 * @note The exact behavior and limitations of the division ratio depend on the specific
 *       hardware capabilities and clock configuration. Ensure that the values of 'div_n' and 'div_m'
 *       are within the valid range for your hardware.
 *
 * @warning Improper configuration of the clock divider might disrupt UART0 communication.
 *          It's important to ensure that 'div_n' and 'div_m' are set to values that are
 *          compatible with the Uart1 specifications and the overall system clock settings.
 */
int32_t HAL_CRM_SetUart1ClkDiv(uint32_t div_n, uint32_t div_m);

/**
 * @brief Retrieves the clock configuration for UART1.
 *
 * This inline static function obtains the current clock source and division factors for the
 * UART1 module. The clock source and divider values are returned
 * through the pointer parameters 'src', 'div_n', and'div_m'.
 *
 * @param[out] src Pointer to a 'clock_src_name_t' variable where the clock source will be stored.
 *                 The clock source is indicated as an enumeration value of type 'clock_src_name_t'.
 *
 *
 * @param[out] div_n Pointer to a 'uint32_t' variable where the numerator of the clock division
 *                   ratio will be stored.
 *
 *
 * @param[out] div_m Pointer to a 'uint32_t' variable where the denominator of the clock division
 *                   ratio will be stored.
 *
 * @note Being an inline function, 'HAL_CRM_GetUart1ClkConfig()' is expanded at each point of call,
 *       which can increase code size but typically reduces execution time by avoiding a function
 *       call overhead. Use this function judiciously where performance is critical.
 *
 * @warning Ensure that the pointers passed to this function are valid and point to appropriate
 *          memory locations. Passing invalid pointers may lead to undefined behavior, including
 *          crashes. Also, consider the potential for race conditions if the clock configuration
 *          can be changed by other parts of the program while this function is being executed.
 */
inline static void HAL_CRM_GetUart1ClkConfig(clock_src_name_t* src, uint32_t* div_n, uint32_t* div_m){
    
    uint32_t src_t;
    src_t = IP_SYSCTRL->REG_PERI_CLK_CFG1.bit.SEL_UART1_CLK;
    
    if (src_t == 0){
        *src = CRM_IpSrcXtalClk;
    }
    
    if (src_t == 1){
        *src = CRM_IpSrcPeriClk;
    }
    
    
    
    *div_n = IP_SYSCTRL->REG_PERI_CLK_CFG1.bit.DIV_UART1_CLK_N;
    
    
    *div_m = IP_SYSCTRL->REG_PERI_CLK_CFG1.bit.DIV_UART1_CLK_M;
    
}

/**
 * @brief Retrieves the current operating frequency of UART1.
 *
 * This function returns the frequency (in Hz) at which the UART1
 * is currently operating. The frequency is calculated based on the current configuration of
 * the system's clock sources and the UART1 clock divider settings.
 *
 * @return uint32_t The operating frequency of UART1 in Hertz. If the frequency cannot be
 *                  determined, or if UART1 is not properly configured, the function may return
 *                  0.
 *
 * @note The returned frequency value is dependent on the current state of the system's clock
 *       configuration and the UART1 divider settings. Changes in these parameters can affect
 *       the UART1 frequency.
 *
 * @warning Ensure that UART1 and its clock sources are properly configured before calling this
 *          function. Calling this function without proper initialization may lead to undefined
 *          behavior or incorrect frequency values.
 */
uint32_t CRM_GetUart1Freq();
/**
  * @}
  */

/** @defgroup _CRM_SPI0 SPI0_CLK_FUNC
  * @brief SPI0 clock control function which can enable, disable or get status from corresponding
  * device, set ip clock source, set ip clock divider, or get ip clock configuration, help you calculate the ip divider
  * parameter when having ip reference clock and desire clock
  * @{
  */

/**
 * @brief Macro to enable the clock for SPI0.
 *
 * This macro enables the clock for the SPI0 module.
 * It typically modifies a specific bit in a hardware register to provide the clock
 * to SPI0, allowing the module to operate. This macro should be called before
 * initializing or using SPI0 to ensure that the hardware is properly powered and
 * ready for operation.
 *
 * Usage:
 *      __HAL_CRM_SPI0_CLK_ENABLE();
 *
 * @note This macro directly interacts with hardware registers, and its effects are
 *       immediate. Ensure that the system is in a state where enabling the SPI0 clock
 *       is safe and appropriate.
 *
 * @warning Incorrect use of this macro, such as enabling the clock without proper
 *          configuration of the SPI0 module, may lead to unexpected behavior or
 *          system instability. Always ensure that the peripheral is configured
 *          correctly before enabling its clock.
 */
#define __HAL_CRM_SPI0_CLK_ENABLE()    \
do { \
	IP_SYSCTRL->REG_CLK_EN_CTRL0.bit.ENA_SPI0_CLK = 0x1; \
} while(0)
/**
 * @brief Macro to disable the clock for SPI0.
 *
 * This macro disables the clock for the SPI0 module.
 * Disabling the clock can be useful in power-saving modes or when the SPI0 module is
 * not in use. This macro typically modifies a specific bit in a hardware register to
 * stop the clock supply to SPI0.
 *
 * Usage:
 *      __HAL_CRM_SPI0_CLK_DISABLE();
 *
 * @note Disabling the clock to a module while it is in use can lead to incomplete or
 *       corrupted data transfers and should be done with caution. Ensure that SPI0
 *       is not actively transmitting or receiving data before calling this macro.
 *
 * @warning Improper use of this macro, such as disabling the clock during an active
 *          SPI0 operation, may result in system instability or data corruption. Always
 *          make sure that the peripheral is idle or powered off before disabling its clock.
 */
#define __HAL_CRM_SPI0_CLK_DISABLE()    \
do { \
	IP_SYSCTRL->REG_CLK_EN_CTRL0.bit.ENA_SPI0_CLK = 0x0; \
} while(0)
/**
 * @brief Checks if the SPI0 clock is enabled.
 *
 * This inline static function determines whether the clock for the SPI0 module is currently
 * enabled. It typically checks a specific bit in a control register and returns the status.
 * This function can be used to verify the clock state of SPI0 before performing operations
 * that require the clock to be active.
 *
 * @return uint32_t Returns 1 if the SPI0 clock is enabled, and 0 if it is disabled.
 *
 * @note Since this is an inline function, it is expanded at the point of each call, which can
 *       lead to increased code size if used frequently. However, inlining often results in
 *       faster execution, as the overhead of a function call is eliminated.
 *
 * @warning This function should be used with the understanding that the state of the clock could
 *          change immediately after the function call, especially in multi-threaded or
 *          interrupt-driven environments. Additional synchronization mechanisms may be needed
 *          in such cases.
 */
inline static uint32_t HAL_CRM_Spi0ClkIsEnabled(){
    return IP_SYSCTRL->REG_CLK_EN_CTRL0.bit.ENA_SPI0_CLK;
}

/**
 * @brief Sets the clock source for Spi0.
 *
 * This function configures Spi0 to use a specific clock source as defined by the 'src' parameter.
 * The 'src' parameter should be one of the values defined in the 'clock_src_name_t' enumeration,
 * representing the various clock sources available in the system. This allows for flexible
 * configuration of the Spi0 clocking, depending on the system's requirements and the available
 * clock sources.
 *
 * @param src The desired clock source for Spi0. This should be a value from the
 *            'clock_src_name_t' enumeration that specifies which clock source to use that Can choose this source -> CRM_IpSrcXtalClk, CRM_IpSrcPeriClk.
 *
 * @return uint32_t Returns 0 on success, or a non-zero error code on failure. The error code
 *                  can indicate issues such as an invalid clock source or a failure in applying
 *                  the new clock source setting.
 *
 * @note The function's ability to change the clock source may depend on the current state of Spi0
 *       and the system's clock configuration. It is advisable to ensure that Spi0 is not actively
 *       transmitting data when changing its clock source.
 *
 * @warning Using an incorrect or unsupported clock source for Spi0 can lead to communication
 *          failures or system instability. Ensure that the selected clock source is compatible
 *          with Spi0's operational requirements.
 */
uint32_t HAL_CRM_SetSpi0ClkSrc(clock_src_name_t src);

/**
 * @brief Sets the clock divider for Spi0.
 *
 * This function configures the clock division for Spi0 by setting the divider ratios
 * to the specified values. The division is defined by 'div_n' and 'div_m'.
 * These parameters determine how the input clock frequency is divided to derive the
 * desired Spi0 clock frequency.
 *
 * @param div_n The numerator part of the clock division ratio. Specifies the upper part
 *              of the division ratio, The div_n select range from [1 - 7].
 *
 *
 * @param div_m The denominator part of the clock division ratio. Specifies the lower part
 *              of the division ratio, The div_m select range from [1 - 7].
 *
 *
 * @return int32_t Returns 0 on success, or a non-zero error code on failure. The error code
 *                 typically indicates what went wrong during the configuration process.
 *
 * @note The exact behavior and limitations of the division ratio depend on the specific
 *       hardware capabilities and clock configuration. Ensure that the values of 'div_n' and 'div_m'
 *       are within the valid range for your hardware.
 *
 * @warning Improper configuration of the clock divider might disrupt UART0 communication.
 *          It's important to ensure that 'div_n' and 'div_m' are set to values that are
 *          compatible with the Spi0 specifications and the overall system clock settings.
 */
int32_t HAL_CRM_SetSpi0ClkDiv(uint32_t div_n, uint32_t div_m);

/**
 * @brief Retrieves the clock configuration for SPI0.
 *
 * This inline static function obtains the current clock source and division factors for the
 * SPI0 module. The clock source and divider values are returned
 * through the pointer parameters 'src', 'div_n', and'div_m'.
 *
 * @param[out] src Pointer to a 'clock_src_name_t' variable where the clock source will be stored.
 *                 The clock source is indicated as an enumeration value of type 'clock_src_name_t'.
 *
 *
 * @param[out] div_n Pointer to a 'uint32_t' variable where the numerator of the clock division
 *                   ratio will be stored.
 *
 *
 * @param[out] div_m Pointer to a 'uint32_t' variable where the denominator of the clock division
 *                   ratio will be stored.
 *
 * @note Being an inline function, 'HAL_CRM_GetSpi0ClkConfig()' is expanded at each point of call,
 *       which can increase code size but typically reduces execution time by avoiding a function
 *       call overhead. Use this function judiciously where performance is critical.
 *
 * @warning Ensure that the pointers passed to this function are valid and point to appropriate
 *          memory locations. Passing invalid pointers may lead to undefined behavior, including
 *          crashes. Also, consider the potential for race conditions if the clock configuration
 *          can be changed by other parts of the program while this function is being executed.
 */
inline static void HAL_CRM_GetSpi0ClkConfig(clock_src_name_t* src, uint32_t* div_n, uint32_t* div_m){
    
    uint32_t src_t;
    src_t = IP_SYSCTRL->REG_PERI_CLK_CFG0.bit.SEL_SPI0_CLK;
    
    if (src_t == 0){
        *src = CRM_IpSrcXtalClk;
    }
    
    if (src_t == 1){
        *src = CRM_IpSrcPeriClk;
    }
    
    
    
    *div_n = IP_SYSCTRL->REG_PERI_CLK_CFG0.bit.DIV_SPI0_CLK_N;
    
    
    *div_m = IP_SYSCTRL->REG_PERI_CLK_CFG0.bit.DIV_SPI0_CLK_M;
    
}

/**
 * @brief Retrieves the current operating frequency of SPI0.
 *
 * This function returns the frequency (in Hz) at which the SPI0
 * is currently operating. The frequency is calculated based on the current configuration of
 * the system's clock sources and the SPI0 clock divider settings.
 *
 * @return uint32_t The operating frequency of SPI0 in Hertz. If the frequency cannot be
 *                  determined, or if SPI0 is not properly configured, the function may return
 *                  0.
 *
 * @note The returned frequency value is dependent on the current state of the system's clock
 *       configuration and the SPI0 divider settings. Changes in these parameters can affect
 *       the SPI0 frequency.
 *
 * @warning Ensure that SPI0 and its clock sources are properly configured before calling this
 *          function. Calling this function without proper initialization may lead to undefined
 *          behavior or incorrect frequency values.
 */
uint32_t CRM_GetSpi0Freq();
/**
  * @}
  */

/** @defgroup _CRM_GPADC GPADC_CLK_FUNC
  * @brief GPADC clock control function which can enable, disable or get status from corresponding
  * device, set ip clock source, set ip clock divider, or get ip clock configuration, help you calculate the ip divider
  * parameter when having ip reference clock and desire clock
  * @{
  */

/**
 * @brief Macro to enable the clock for GPADC.
 *
 * This macro enables the clock for the GPADC module.
 * It typically modifies a specific bit in a hardware register to provide the clock
 * to GPADC, allowing the module to operate. This macro should be called before
 * initializing or using GPADC to ensure that the hardware is properly powered and
 * ready for operation.
 *
 * Usage:
 *      __HAL_CRM_GPADC_CLK_ENABLE();
 *
 * @note This macro directly interacts with hardware registers, and its effects are
 *       immediate. Ensure that the system is in a state where enabling the GPADC clock
 *       is safe and appropriate.
 *
 * @warning Incorrect use of this macro, such as enabling the clock without proper
 *          configuration of the GPADC module, may lead to unexpected behavior or
 *          system instability. Always ensure that the peripheral is configured
 *          correctly before enabling its clock.
 */
#define __HAL_CRM_GPADC_CLK_ENABLE()    \
do { \
	IP_SYSCTRL->REG_CLK_EN_CTRL0.bit.ENA_GPADC_CLK = 0x1; \
} while(0)
/**
 * @brief Macro to disable the clock for GPADC.
 *
 * This macro disables the clock for the GPADC module.
 * Disabling the clock can be useful in power-saving modes or when the GPADC module is
 * not in use. This macro typically modifies a specific bit in a hardware register to
 * stop the clock supply to GPADC.
 *
 * Usage:
 *      __HAL_CRM_SPI0_CLK_DISABLE();
 *
 * @note Disabling the clock to a module while it is in use can lead to incomplete or
 *       corrupted data transfers and should be done with caution. Ensure that GPADC
 *       is not actively transmitting or receiving data before calling this macro.
 *
 * @warning Improper use of this macro, such as disabling the clock during an active
 *          GPADC operation, may result in system instability or data corruption. Always
 *          make sure that the peripheral is idle or powered off before disabling its clock.
 */
#define __HAL_CRM_GPADC_CLK_DISABLE()    \
do { \
	IP_SYSCTRL->REG_CLK_EN_CTRL0.bit.ENA_GPADC_CLK = 0x0; \
} while(0)
/**
 * @brief Checks if the GPADC clock is enabled.
 *
 * This inline static function determines whether the clock for the GPADC module is currently
 * enabled. It typically checks a specific bit in a control register and returns the status.
 * This function can be used to verify the clock state of GPADC before performing operations
 * that require the clock to be active.
 *
 * @return uint32_t Returns 1 if the GPADC clock is enabled, and 0 if it is disabled.
 *
 * @note Since this is an inline function, it is expanded at the point of each call, which can
 *       lead to increased code size if used frequently. However, inlining often results in
 *       faster execution, as the overhead of a function call is eliminated.
 *
 * @warning This function should be used with the understanding that the state of the clock could
 *          change immediately after the function call, especially in multi-threaded or
 *          interrupt-driven environments. Additional synchronization mechanisms may be needed
 *          in such cases.
 */
inline static uint32_t HAL_CRM_GpadcClkIsEnabled(){
    return IP_SYSCTRL->REG_CLK_EN_CTRL0.bit.ENA_GPADC_CLK;
}

/**
 * @brief Sets the clock divider for Gpadc.
 *
 * This function configures the clock division for Gpadc by setting the divider ratios
 * to the specified values. The division is defined by  and 'div_m'.
 * These parameters determine how the input clock frequency is divided to derive the
 * desired Gpadc clock frequency.
 *
 *
 * @param div_m The denominator part of the clock division ratio. Specifies the lower part
 *              of the division ratio, The div_m select range from [1 - 1023].
 *
 *
 * @return int32_t Returns 0 on success, or a non-zero error code on failure. The error code
 *                 typically indicates what went wrong during the configuration process.
 *
 * @note The exact behavior and limitations of the division ratio depend on the specific
 *       hardware capabilities and clock configuration. Ensure that the values of  and 'div_m'
 *       are within the valid range for your hardware.
 *
 * @warning Improper configuration of the clock divider might disrupt UART0 communication.
 *          It's important to ensure that  and 'div_m' are set to values that are
 *          compatible with the Gpadc specifications and the overall system clock settings.
 */
int32_t HAL_CRM_SetGpadcClkDiv(uint32_t div_m);

/**
 * @brief Retrieves the clock configuration for GPADC.
 *
 * This inline static function obtains the current clock source and division factors for the
 * GPADC module. The clock source and divider values are returned
 * through the pointer parameters  'div_m'.
 *
 *
 *
 * @param[out] div_m Pointer to a 'uint32_t' variable where the denominator of the clock division
 *                   ratio will be stored.
 *
 * @note Being an inline function, 'HAL_CRM_GetGpadcClkConfig()' is expanded at each point of call,
 *       which can increase code size but typically reduces execution time by avoiding a function
 *       call overhead. Use this function judiciously where performance is critical.
 *
 * @warning Ensure that the pointers passed to this function are valid and point to appropriate
 *          memory locations. Passing invalid pointers may lead to undefined behavior, including
 *          crashes. Also, consider the potential for race conditions if the clock configuration
 *          can be changed by other parts of the program while this function is being executed.
 */
inline static void HAL_CRM_GetGpadcClkConfig(uint32_t* div_m){
    
    
    
    *div_m = IP_SYSCTRL->REG_PERI_CLK_CFG2.bit.DIV_GPADC_CLK_M;
    
}

/**
 * @brief Retrieves the current operating frequency of GPADC.
 *
 * This function returns the frequency (in Hz) at which the GPADC
 * is currently operating. The frequency is calculated based on the current configuration of
 * the system's clock sources and the GPADC clock divider settings.
 *
 * @return uint32_t The operating frequency of GPADC in Hertz. If the frequency cannot be
 *                  determined, or if GPADC is not properly configured, the function may return
 *                  0.
 *
 * @note The returned frequency value is dependent on the current state of the system's clock
 *       configuration and the GPADC divider settings. Changes in these parameters can affect
 *       the GPADC frequency.
 *
 * @warning Ensure that GPADC and its clock sources are properly configured before calling this
 *          function. Calling this function without proper initialization may lead to undefined
 *          behavior or incorrect frequency values.
 */
uint32_t CRM_GetGpadcFreq();
/**
  * @}
  */

/** @defgroup _CRM_IR_TX IR_TX_CLK_FUNC
  * @brief IR_TX clock control function which can enable, disable or get status from corresponding
  * device, set ip clock source, set ip clock divider, or get ip clock configuration, help you calculate the ip divider
  * parameter when having ip reference clock and desire clock
  * @{
  */

/**
 * @brief Sets the clock divider for Ir_tx.
 *
 * This function configures the clock division for Ir_tx by setting the divider ratios
 * to the specified values. The division is defined by  and 'div_m'.
 * These parameters determine how the input clock frequency is divided to derive the
 * desired Ir_tx clock frequency.
 *
 *
 * @param div_m The denominator part of the clock division ratio. Specifies the lower part
 *              of the division ratio, The div_m select range from [1 - 63].
 *
 *
 * @return int32_t Returns 0 on success, or a non-zero error code on failure. The error code
 *                 typically indicates what went wrong during the configuration process.
 *
 * @note The exact behavior and limitations of the division ratio depend on the specific
 *       hardware capabilities and clock configuration. Ensure that the values of  and 'div_m'
 *       are within the valid range for your hardware.
 *
 * @warning Improper configuration of the clock divider might disrupt UART0 communication.
 *          It's important to ensure that  and 'div_m' are set to values that are
 *          compatible with the Ir_tx specifications and the overall system clock settings.
 */
int32_t HAL_CRM_SetIr_txClkDiv(uint32_t div_m);

/**
 * @brief Retrieves the clock configuration for IR_TX.
 *
 * This inline static function obtains the current clock source and division factors for the
 * IR_TX module. The clock source and divider values are returned
 * through the pointer parameters  'div_m'.
 *
 *
 *
 * @param[out] div_m Pointer to a 'uint32_t' variable where the denominator of the clock division
 *                   ratio will be stored.
 *
 * @note Being an inline function, 'HAL_CRM_GetIr_txClkConfig()' is expanded at each point of call,
 *       which can increase code size but typically reduces execution time by avoiding a function
 *       call overhead. Use this function judiciously where performance is critical.
 *
 * @warning Ensure that the pointers passed to this function are valid and point to appropriate
 *          memory locations. Passing invalid pointers may lead to undefined behavior, including
 *          crashes. Also, consider the potential for race conditions if the clock configuration
 *          can be changed by other parts of the program while this function is being executed.
 */
inline static void HAL_CRM_GetIr_txClkConfig(uint32_t* div_m){
    
    
    
    *div_m = IP_SYSCTRL->REG_PERI_CLK_CFG1.bit.DIV_IR_TXCLK_M;
    
}

/**
 * @brief Retrieves the current operating frequency of IR_TX.
 *
 * This function returns the frequency (in Hz) at which the IR_TX
 * is currently operating. The frequency is calculated based on the current configuration of
 * the system's clock sources and the IR_TX clock divider settings.
 *
 * @return uint32_t The operating frequency of IR_TX in Hertz. If the frequency cannot be
 *                  determined, or if IR_TX is not properly configured, the function may return
 *                  0.
 *
 * @note The returned frequency value is dependent on the current state of the system's clock
 *       configuration and the IR_TX divider settings. Changes in these parameters can affect
 *       the IR_TX frequency.
 *
 * @warning Ensure that IR_TX and its clock sources are properly configured before calling this
 *          function. Calling this function without proper initialization may lead to undefined
 *          behavior or incorrect frequency values.
 */
uint32_t CRM_GetIr_txFreq();
/**
  * @}
  */

/** @defgroup _CRM_IR IR_CLK_FUNC
  * @brief IR clock control function which can enable, disable or get status from corresponding
  * device, set ip clock source, set ip clock divider, or get ip clock configuration, help you calculate the ip divider
  * parameter when having ip reference clock and desire clock
  * @{
  */

/**
 * @brief Macro to enable the clock for IR.
 *
 * This macro enables the clock for the IR module.
 * It typically modifies a specific bit in a hardware register to provide the clock
 * to IR, allowing the module to operate. This macro should be called before
 * initializing or using IR to ensure that the hardware is properly powered and
 * ready for operation.
 *
 * Usage:
 *      __HAL_CRM_IR_CLK_ENABLE();
 *
 * @note This macro directly interacts with hardware registers, and its effects are
 *       immediate. Ensure that the system is in a state where enabling the IR clock
 *       is safe and appropriate.
 *
 * @warning Incorrect use of this macro, such as enabling the clock without proper
 *          configuration of the IR module, may lead to unexpected behavior or
 *          system instability. Always ensure that the peripheral is configured
 *          correctly before enabling its clock.
 */
#define __HAL_CRM_IR_CLK_ENABLE()    \
do { \
	IP_SYSCTRL->REG_CLK_EN_CTRL0.bit.ENA_IR_CLK = 0x1; \
} while(0)
/**
 * @brief Macro to disable the clock for IR.
 *
 * This macro disables the clock for the IR module.
 * Disabling the clock can be useful in power-saving modes or when the IR module is
 * not in use. This macro typically modifies a specific bit in a hardware register to
 * stop the clock supply to IR.
 *
 * Usage:
 *      __HAL_CRM_SPI0_CLK_DISABLE();
 *
 * @note Disabling the clock to a module while it is in use can lead to incomplete or
 *       corrupted data transfers and should be done with caution. Ensure that IR
 *       is not actively transmitting or receiving data before calling this macro.
 *
 * @warning Improper use of this macro, such as disabling the clock during an active
 *          IR operation, may result in system instability or data corruption. Always
 *          make sure that the peripheral is idle or powered off before disabling its clock.
 */
#define __HAL_CRM_IR_CLK_DISABLE()    \
do { \
	IP_SYSCTRL->REG_CLK_EN_CTRL0.bit.ENA_IR_CLK = 0x0; \
} while(0)
/**
 * @brief Checks if the IR clock is enabled.
 *
 * This inline static function determines whether the clock for the IR module is currently
 * enabled. It typically checks a specific bit in a control register and returns the status.
 * This function can be used to verify the clock state of IR before performing operations
 * that require the clock to be active.
 *
 * @return uint32_t Returns 1 if the IR clock is enabled, and 0 if it is disabled.
 *
 * @note Since this is an inline function, it is expanded at the point of each call, which can
 *       lead to increased code size if used frequently. However, inlining often results in
 *       faster execution, as the overhead of a function call is eliminated.
 *
 * @warning This function should be used with the understanding that the state of the clock could
 *          change immediately after the function call, especially in multi-threaded or
 *          interrupt-driven environments. Additional synchronization mechanisms may be needed
 *          in such cases.
 */
inline static uint32_t HAL_CRM_IrClkIsEnabled(){
    return IP_SYSCTRL->REG_CLK_EN_CTRL0.bit.ENA_IR_CLK;
}

/**
 * @brief Retrieves the current operating frequency of IR.
 *
 * This function returns the frequency (in Hz) at which the IR
 * is currently operating. The frequency is calculated based on the current configuration of
 * the system's clock sources and the IR clock divider settings.
 *
 * @return uint32_t The operating frequency of IR in Hertz. If the frequency cannot be
 *                  determined, or if IR is not properly configured, the function may return
 *                  0.
 *
 * @note The returned frequency value is dependent on the current state of the system's clock
 *       configuration and the IR divider settings. Changes in these parameters can affect
 *       the IR frequency.
 *
 * @warning Ensure that IR and its clock sources are properly configured before calling this
 *          function. Calling this function without proper initialization may lead to undefined
 *          behavior or incorrect frequency values.
 */
uint32_t CRM_GetIrFreq();
/**
  * @}
  */

/** @defgroup _CRM_DMA DMA_CLK_FUNC
  * @brief DMA clock control function which can enable, disable or get status from corresponding
  * device, set ip clock source, set ip clock divider, or get ip clock configuration, help you calculate the ip divider
  * parameter when having ip reference clock and desire clock
  * @{
  */

/**
 * @brief Macro to enable the clock for DMA.
 *
 * This macro enables the clock for the DMA module.
 * It typically modifies a specific bit in a hardware register to provide the clock
 * to DMA, allowing the module to operate. This macro should be called before
 * initializing or using DMA to ensure that the hardware is properly powered and
 * ready for operation.
 *
 * Usage:
 *      __HAL_CRM_DMA_CLK_ENABLE();
 *
 * @note This macro directly interacts with hardware registers, and its effects are
 *       immediate. Ensure that the system is in a state where enabling the DMA clock
 *       is safe and appropriate.
 *
 * @warning Incorrect use of this macro, such as enabling the clock without proper
 *          configuration of the DMA module, may lead to unexpected behavior or
 *          system instability. Always ensure that the peripheral is configured
 *          correctly before enabling its clock.
 */
#define __HAL_CRM_DMA_CLK_ENABLE()    \
do { \
	IP_SYSCTRL->REG_CLK_EN_CTRL0.bit.ENA_DMA_CLK = 0x1; \
} while(0)
/**
 * @brief Macro to disable the clock for DMA.
 *
 * This macro disables the clock for the DMA module.
 * Disabling the clock can be useful in power-saving modes or when the DMA module is
 * not in use. This macro typically modifies a specific bit in a hardware register to
 * stop the clock supply to DMA.
 *
 * Usage:
 *      __HAL_CRM_SPI0_CLK_DISABLE();
 *
 * @note Disabling the clock to a module while it is in use can lead to incomplete or
 *       corrupted data transfers and should be done with caution. Ensure that DMA
 *       is not actively transmitting or receiving data before calling this macro.
 *
 * @warning Improper use of this macro, such as disabling the clock during an active
 *          DMA operation, may result in system instability or data corruption. Always
 *          make sure that the peripheral is idle or powered off before disabling its clock.
 */
#define __HAL_CRM_DMA_CLK_DISABLE()    \
do { \
	IP_SYSCTRL->REG_CLK_EN_CTRL0.bit.ENA_DMA_CLK = 0x0; \
} while(0)
/**
 * @brief Checks if the DMA clock is enabled.
 *
 * This inline static function determines whether the clock for the DMA module is currently
 * enabled. It typically checks a specific bit in a control register and returns the status.
 * This function can be used to verify the clock state of DMA before performing operations
 * that require the clock to be active.
 *
 * @return uint32_t Returns 1 if the DMA clock is enabled, and 0 if it is disabled.
 *
 * @note Since this is an inline function, it is expanded at the point of each call, which can
 *       lead to increased code size if used frequently. However, inlining often results in
 *       faster execution, as the overhead of a function call is eliminated.
 *
 * @warning This function should be used with the understanding that the state of the clock could
 *          change immediately after the function call, especially in multi-threaded or
 *          interrupt-driven environments. Additional synchronization mechanisms may be needed
 *          in such cases.
 */
inline static uint32_t HAL_CRM_DmaClkIsEnabled(){
    return IP_SYSCTRL->REG_CLK_EN_CTRL0.bit.ENA_DMA_CLK;
}

/**
 * @brief Retrieves the current operating frequency of DMA.
 *
 * This function returns the frequency (in Hz) at which the DMA
 * is currently operating. The frequency is calculated based on the current configuration of
 * the system's clock sources and the DMA clock divider settings.
 *
 * @return uint32_t The operating frequency of DMA in Hertz. If the frequency cannot be
 *                  determined, or if DMA is not properly configured, the function may return
 *                  0.
 *
 * @note The returned frequency value is dependent on the current state of the system's clock
 *       configuration and the DMA divider settings. Changes in these parameters can affect
 *       the DMA frequency.
 *
 * @warning Ensure that DMA and its clock sources are properly configured before calling this
 *          function. Calling this function without proper initialization may lead to undefined
 *          behavior or incorrect frequency values.
 */
uint32_t CRM_GetDmaFreq();
/**
  * @}
  */

/** @defgroup _CRM_FLASH FLASH_CLK_FUNC
  * @brief FLASH clock control function which can enable, disable or get status from corresponding
  * device, set ip clock source, set ip clock divider, or get ip clock configuration, help you calculate the ip divider
  * parameter when having ip reference clock and desire clock
  * @{
  */

/**
 * @brief Macro to enable the clock for FLASH.
 *
 * This macro enables the clock for the FLASH module.
 * It typically modifies a specific bit in a hardware register to provide the clock
 * to FLASH, allowing the module to operate. This macro should be called before
 * initializing or using FLASH to ensure that the hardware is properly powered and
 * ready for operation.
 *
 * Usage:
 *      __HAL_CRM_FLASH_CLK_ENABLE();
 *
 * @note This macro directly interacts with hardware registers, and its effects are
 *       immediate. Ensure that the system is in a state where enabling the FLASH clock
 *       is safe and appropriate.
 *
 * @warning Incorrect use of this macro, such as enabling the clock without proper
 *          configuration of the FLASH module, may lead to unexpected behavior or
 *          system instability. Always ensure that the peripheral is configured
 *          correctly before enabling its clock.
 */
#define __HAL_CRM_FLASH_CLK_ENABLE()    \
do { \
	IP_SYSCTRL->REG_CLK_EN_CTRL0.bit.ENA_FLASHC_CLK = 0x1; \
} while(0)
/**
 * @brief Macro to disable the clock for FLASH.
 *
 * This macro disables the clock for the FLASH module.
 * Disabling the clock can be useful in power-saving modes or when the FLASH module is
 * not in use. This macro typically modifies a specific bit in a hardware register to
 * stop the clock supply to FLASH.
 *
 * Usage:
 *      __HAL_CRM_SPI0_CLK_DISABLE();
 *
 * @note Disabling the clock to a module while it is in use can lead to incomplete or
 *       corrupted data transfers and should be done with caution. Ensure that FLASH
 *       is not actively transmitting or receiving data before calling this macro.
 *
 * @warning Improper use of this macro, such as disabling the clock during an active
 *          FLASH operation, may result in system instability or data corruption. Always
 *          make sure that the peripheral is idle or powered off before disabling its clock.
 */
#define __HAL_CRM_FLASH_CLK_DISABLE()    \
do { \
	IP_SYSCTRL->REG_CLK_EN_CTRL0.bit.ENA_FLASHC_CLK = 0x0; \
} while(0)
/**
 * @brief Checks if the FLASH clock is enabled.
 *
 * This inline static function determines whether the clock for the FLASH module is currently
 * enabled. It typically checks a specific bit in a control register and returns the status.
 * This function can be used to verify the clock state of FLASH before performing operations
 * that require the clock to be active.
 *
 * @return uint32_t Returns 1 if the FLASH clock is enabled, and 0 if it is disabled.
 *
 * @note Since this is an inline function, it is expanded at the point of each call, which can
 *       lead to increased code size if used frequently. However, inlining often results in
 *       faster execution, as the overhead of a function call is eliminated.
 *
 * @warning This function should be used with the understanding that the state of the clock could
 *          change immediately after the function call, especially in multi-threaded or
 *          interrupt-driven environments. Additional synchronization mechanisms may be needed
 *          in such cases.
 */
inline static uint32_t HAL_CRM_FlashClkIsEnabled(){
    return IP_SYSCTRL->REG_CLK_EN_CTRL0.bit.ENA_FLASHC_CLK;
}

/**
 * @brief Sets the clock source for Flash.
 *
 * This function configures Flash to use a specific clock source as defined by the 'src' parameter.
 * The 'src' parameter should be one of the values defined in the 'clock_src_name_t' enumeration,
 * representing the various clock sources available in the system. This allows for flexible
 * configuration of the Flash clocking, depending on the system's requirements and the available
 * clock sources.
 *
 * @param src The desired clock source for Flash. This should be a value from the
 *            'clock_src_name_t' enumeration that specifies which clock source to use that Can choose this source -> CRM_IpSrcXtalClk, CRM_IpSrcFlashClk.
 *
 * @return uint32_t Returns 0 on success, or a non-zero error code on failure. The error code
 *                  can indicate issues such as an invalid clock source or a failure in applying
 *                  the new clock source setting.
 *
 * @note The function's ability to change the clock source may depend on the current state of Flash
 *       and the system's clock configuration. It is advisable to ensure that Flash is not actively
 *       transmitting data when changing its clock source.
 *
 * @warning Using an incorrect or unsupported clock source for Flash can lead to communication
 *          failures or system instability. Ensure that the selected clock source is compatible
 *          with Flash's operational requirements.
 */
uint32_t HAL_CRM_SetFlashClkSrc(clock_src_name_t src);

/**
 * @brief Sets the clock divider for Flash.
 *
 * This function configures the clock division for Flash by setting the divider ratios
 * to the specified values. The division is defined by  and 'div_m'.
 * These parameters determine how the input clock frequency is divided to derive the
 * desired Flash clock frequency.
 *
 *
 * @param div_m The denominator part of the clock division ratio. Specifies the lower part
 *              of the division ratio, The div_m select range from [1 - 3].
 *
 *
 * @return int32_t Returns 0 on success, or a non-zero error code on failure. The error code
 *                 typically indicates what went wrong during the configuration process.
 *
 * @note The exact behavior and limitations of the division ratio depend on the specific
 *       hardware capabilities and clock configuration. Ensure that the values of  and 'div_m'
 *       are within the valid range for your hardware.
 *
 * @warning Improper configuration of the clock divider might disrupt UART0 communication.
 *          It's important to ensure that  and 'div_m' are set to values that are
 *          compatible with the Flash specifications and the overall system clock settings.
 */
int32_t HAL_CRM_SetFlashClkDiv(uint32_t div_m);

/**
 * @brief Retrieves the clock configuration for FLASH.
 *
 * This inline static function obtains the current clock source and division factors for the
 * FLASH module. The clock source and divider values are returned
 * through the pointer parameters 'src', 'div_m'.
 *
 * @param[out] src Pointer to a 'clock_src_name_t' variable where the clock source will be stored.
 *                 The clock source is indicated as an enumeration value of type 'clock_src_name_t'.
 *
 *
 *
 * @param[out] div_m Pointer to a 'uint32_t' variable where the denominator of the clock division
 *                   ratio will be stored.
 *
 * @note Being an inline function, 'HAL_CRM_GetFlashClkConfig()' is expanded at each point of call,
 *       which can increase code size but typically reduces execution time by avoiding a function
 *       call overhead. Use this function judiciously where performance is critical.
 *
 * @warning Ensure that the pointers passed to this function are valid and point to appropriate
 *          memory locations. Passing invalid pointers may lead to undefined behavior, including
 *          crashes. Also, consider the potential for race conditions if the clock configuration
 *          can be changed by other parts of the program while this function is being executed.
 */
inline static void HAL_CRM_GetFlashClkConfig(clock_src_name_t* src, uint32_t* div_m){
    
    uint32_t src_t;
    src_t = IP_SYSCTRL->REG_PERI_CLK_CFG2.bit.SEL_FLASHC_CLK;
    
    if (src_t == 0){
        *src = CRM_IpSrcXtalClk;
    }
    
    if (src_t == 1){
        *src = CRM_IpSrcFlashClk;
    }
    
    
    
    
    *div_m = IP_SYSCTRL->REG_PERI_CLK_CFG2.bit.DIV_FLASHC_CLK_M;
    
}

/**
 * @brief Retrieves the current operating frequency of FLASH.
 *
 * This function returns the frequency (in Hz) at which the FLASH
 * is currently operating. The frequency is calculated based on the current configuration of
 * the system's clock sources and the FLASH clock divider settings.
 *
 * @return uint32_t The operating frequency of FLASH in Hertz. If the frequency cannot be
 *                  determined, or if FLASH is not properly configured, the function may return
 *                  0.
 *
 * @note The returned frequency value is dependent on the current state of the system's clock
 *       configuration and the FLASH divider settings. Changes in these parameters can affect
 *       the FLASH frequency.
 *
 * @warning Ensure that FLASH and its clock sources are properly configured before calling this
 *          function. Calling this function without proper initialization may lead to undefined
 *          behavior or incorrect frequency values.
 */
uint32_t CRM_GetFlashFreq();
/**
  * @}
  */

/** @defgroup _CRM_GPIO0 GPIO0_CLK_FUNC
  * @brief GPIO0 clock control function which can enable, disable or get status from corresponding
  * device, set ip clock source, set ip clock divider, or get ip clock configuration, help you calculate the ip divider
  * parameter when having ip reference clock and desire clock
  * @{
  */

/**
 * @brief Macro to enable the clock for GPIO0.
 *
 * This macro enables the clock for the GPIO0 module.
 * It typically modifies a specific bit in a hardware register to provide the clock
 * to GPIO0, allowing the module to operate. This macro should be called before
 * initializing or using GPIO0 to ensure that the hardware is properly powered and
 * ready for operation.
 *
 * Usage:
 *      __HAL_CRM_GPIO0_CLK_ENABLE();
 *
 * @note This macro directly interacts with hardware registers, and its effects are
 *       immediate. Ensure that the system is in a state where enabling the GPIO0 clock
 *       is safe and appropriate.
 *
 * @warning Incorrect use of this macro, such as enabling the clock without proper
 *          configuration of the GPIO0 module, may lead to unexpected behavior or
 *          system instability. Always ensure that the peripheral is configured
 *          correctly before enabling its clock.
 */
#define __HAL_CRM_GPIO0_CLK_ENABLE()    \
do { \
	IP_SYSCTRL->REG_CLK_EN_CTRL0.bit.ENA_GPIO0_CLK = 0x1; \
} while(0)
/**
 * @brief Macro to disable the clock for GPIO0.
 *
 * This macro disables the clock for the GPIO0 module.
 * Disabling the clock can be useful in power-saving modes or when the GPIO0 module is
 * not in use. This macro typically modifies a specific bit in a hardware register to
 * stop the clock supply to GPIO0.
 *
 * Usage:
 *      __HAL_CRM_SPI0_CLK_DISABLE();
 *
 * @note Disabling the clock to a module while it is in use can lead to incomplete or
 *       corrupted data transfers and should be done with caution. Ensure that GPIO0
 *       is not actively transmitting or receiving data before calling this macro.
 *
 * @warning Improper use of this macro, such as disabling the clock during an active
 *          GPIO0 operation, may result in system instability or data corruption. Always
 *          make sure that the peripheral is idle or powered off before disabling its clock.
 */
#define __HAL_CRM_GPIO0_CLK_DISABLE()    \
do { \
	IP_SYSCTRL->REG_CLK_EN_CTRL0.bit.ENA_GPIO0_CLK = 0x0; \
} while(0)
/**
 * @brief Checks if the GPIO0 clock is enabled.
 *
 * This inline static function determines whether the clock for the GPIO0 module is currently
 * enabled. It typically checks a specific bit in a control register and returns the status.
 * This function can be used to verify the clock state of GPIO0 before performing operations
 * that require the clock to be active.
 *
 * @return uint32_t Returns 1 if the GPIO0 clock is enabled, and 0 if it is disabled.
 *
 * @note Since this is an inline function, it is expanded at the point of each call, which can
 *       lead to increased code size if used frequently. However, inlining often results in
 *       faster execution, as the overhead of a function call is eliminated.
 *
 * @warning This function should be used with the understanding that the state of the clock could
 *          change immediately after the function call, especially in multi-threaded or
 *          interrupt-driven environments. Additional synchronization mechanisms may be needed
 *          in such cases.
 */
inline static uint32_t HAL_CRM_Gpio0ClkIsEnabled(){
    return IP_SYSCTRL->REG_CLK_EN_CTRL0.bit.ENA_GPIO0_CLK;
}

/**
 * @brief Retrieves the current operating frequency of GPIO0.
 *
 * This function returns the frequency (in Hz) at which the GPIO0
 * is currently operating. The frequency is calculated based on the current configuration of
 * the system's clock sources and the GPIO0 clock divider settings.
 *
 * @return uint32_t The operating frequency of GPIO0 in Hertz. If the frequency cannot be
 *                  determined, or if GPIO0 is not properly configured, the function may return
 *                  0.
 *
 * @note The returned frequency value is dependent on the current state of the system's clock
 *       configuration and the GPIO0 divider settings. Changes in these parameters can affect
 *       the GPIO0 frequency.
 *
 * @warning Ensure that GPIO0 and its clock sources are properly configured before calling this
 *          function. Calling this function without proper initialization may lead to undefined
 *          behavior or incorrect frequency values.
 */
uint32_t CRM_GetGpio0Freq();
/**
  * @}
  */

/** @defgroup _CRM_I2C0 I2C0_CLK_FUNC
  * @brief I2C0 clock control function which can enable, disable or get status from corresponding
  * device, set ip clock source, set ip clock divider, or get ip clock configuration, help you calculate the ip divider
  * parameter when having ip reference clock and desire clock
  * @{
  */

/**
 * @brief Macro to enable the clock for I2C0.
 *
 * This macro enables the clock for the I2C0 module.
 * It typically modifies a specific bit in a hardware register to provide the clock
 * to I2C0, allowing the module to operate. This macro should be called before
 * initializing or using I2C0 to ensure that the hardware is properly powered and
 * ready for operation.
 *
 * Usage:
 *      __HAL_CRM_I2C0_CLK_ENABLE();
 *
 * @note This macro directly interacts with hardware registers, and its effects are
 *       immediate. Ensure that the system is in a state where enabling the I2C0 clock
 *       is safe and appropriate.
 *
 * @warning Incorrect use of this macro, such as enabling the clock without proper
 *          configuration of the I2C0 module, may lead to unexpected behavior or
 *          system instability. Always ensure that the peripheral is configured
 *          correctly before enabling its clock.
 */
#define __HAL_CRM_I2C0_CLK_ENABLE()    \
do { \
	IP_SYSCTRL->REG_CLK_EN_CTRL0.bit.ENA_I2C0_CLK = 0x1; \
} while(0)
/**
 * @brief Macro to disable the clock for I2C0.
 *
 * This macro disables the clock for the I2C0 module.
 * Disabling the clock can be useful in power-saving modes or when the I2C0 module is
 * not in use. This macro typically modifies a specific bit in a hardware register to
 * stop the clock supply to I2C0.
 *
 * Usage:
 *      __HAL_CRM_SPI0_CLK_DISABLE();
 *
 * @note Disabling the clock to a module while it is in use can lead to incomplete or
 *       corrupted data transfers and should be done with caution. Ensure that I2C0
 *       is not actively transmitting or receiving data before calling this macro.
 *
 * @warning Improper use of this macro, such as disabling the clock during an active
 *          I2C0 operation, may result in system instability or data corruption. Always
 *          make sure that the peripheral is idle or powered off before disabling its clock.
 */
#define __HAL_CRM_I2C0_CLK_DISABLE()    \
do { \
	IP_SYSCTRL->REG_CLK_EN_CTRL0.bit.ENA_I2C0_CLK = 0x0; \
} while(0)
/**
 * @brief Checks if the I2C0 clock is enabled.
 *
 * This inline static function determines whether the clock for the I2C0 module is currently
 * enabled. It typically checks a specific bit in a control register and returns the status.
 * This function can be used to verify the clock state of I2C0 before performing operations
 * that require the clock to be active.
 *
 * @return uint32_t Returns 1 if the I2C0 clock is enabled, and 0 if it is disabled.
 *
 * @note Since this is an inline function, it is expanded at the point of each call, which can
 *       lead to increased code size if used frequently. However, inlining often results in
 *       faster execution, as the overhead of a function call is eliminated.
 *
 * @warning This function should be used with the understanding that the state of the clock could
 *          change immediately after the function call, especially in multi-threaded or
 *          interrupt-driven environments. Additional synchronization mechanisms may be needed
 *          in such cases.
 */
inline static uint32_t HAL_CRM_I2c0ClkIsEnabled(){
    return IP_SYSCTRL->REG_CLK_EN_CTRL0.bit.ENA_I2C0_CLK;
}

/**
 * @brief Retrieves the current operating frequency of I2C0.
 *
 * This function returns the frequency (in Hz) at which the I2C0
 * is currently operating. The frequency is calculated based on the current configuration of
 * the system's clock sources and the I2C0 clock divider settings.
 *
 * @return uint32_t The operating frequency of I2C0 in Hertz. If the frequency cannot be
 *                  determined, or if I2C0 is not properly configured, the function may return
 *                  0.
 *
 * @note The returned frequency value is dependent on the current state of the system's clock
 *       configuration and the I2C0 divider settings. Changes in these parameters can affect
 *       the I2C0 frequency.
 *
 * @warning Ensure that I2C0 and its clock sources are properly configured before calling this
 *          function. Calling this function without proper initialization may lead to undefined
 *          behavior or incorrect frequency values.
 */
uint32_t CRM_GetI2c0Freq();
/**
  * @}
  */

/** @defgroup _CRM_GPT_S GPT_S_CLK_FUNC
  * @brief GPT_S clock control function which can enable, disable or get status from corresponding
  * device, set ip clock source, set ip clock divider, or get ip clock configuration, help you calculate the ip divider
  * parameter when having ip reference clock and desire clock
  * @{
  */

/**
 * @brief Macro to enable the clock for GPT_S.
 *
 * This macro enables the clock for the GPT_S module.
 * It typically modifies a specific bit in a hardware register to provide the clock
 * to GPT_S, allowing the module to operate. This macro should be called before
 * initializing or using GPT_S to ensure that the hardware is properly powered and
 * ready for operation.
 *
 * Usage:
 *      __HAL_CRM_GPT_S_CLK_ENABLE();
 *
 * @note This macro directly interacts with hardware registers, and its effects are
 *       immediate. Ensure that the system is in a state where enabling the GPT_S clock
 *       is safe and appropriate.
 *
 * @warning Incorrect use of this macro, such as enabling the clock without proper
 *          configuration of the GPT_S module, may lead to unexpected behavior or
 *          system instability. Always ensure that the peripheral is configured
 *          correctly before enabling its clock.
 */
#define __HAL_CRM_GPT_S_CLK_ENABLE()    \
do { \
	IP_SYSCTRL->REG_CLK_EN_CTRL0.bit.ENA_GPT_S_CLK = 0x1; \
} while(0)
/**
 * @brief Macro to disable the clock for GPT_S.
 *
 * This macro disables the clock for the GPT_S module.
 * Disabling the clock can be useful in power-saving modes or when the GPT_S module is
 * not in use. This macro typically modifies a specific bit in a hardware register to
 * stop the clock supply to GPT_S.
 *
 * Usage:
 *      __HAL_CRM_SPI0_CLK_DISABLE();
 *
 * @note Disabling the clock to a module while it is in use can lead to incomplete or
 *       corrupted data transfers and should be done with caution. Ensure that GPT_S
 *       is not actively transmitting or receiving data before calling this macro.
 *
 * @warning Improper use of this macro, such as disabling the clock during an active
 *          GPT_S operation, may result in system instability or data corruption. Always
 *          make sure that the peripheral is idle or powered off before disabling its clock.
 */
#define __HAL_CRM_GPT_S_CLK_DISABLE()    \
do { \
	IP_SYSCTRL->REG_CLK_EN_CTRL0.bit.ENA_GPT_S_CLK = 0x0; \
} while(0)
/**
 * @brief Checks if the GPT_S clock is enabled.
 *
 * This inline static function determines whether the clock for the GPT_S module is currently
 * enabled. It typically checks a specific bit in a control register and returns the status.
 * This function can be used to verify the clock state of GPT_S before performing operations
 * that require the clock to be active.
 *
 * @return uint32_t Returns 1 if the GPT_S clock is enabled, and 0 if it is disabled.
 *
 * @note Since this is an inline function, it is expanded at the point of each call, which can
 *       lead to increased code size if used frequently. However, inlining often results in
 *       faster execution, as the overhead of a function call is eliminated.
 *
 * @warning This function should be used with the understanding that the state of the clock could
 *          change immediately after the function call, especially in multi-threaded or
 *          interrupt-driven environments. Additional synchronization mechanisms may be needed
 *          in such cases.
 */
inline static uint32_t HAL_CRM_Gpt_sClkIsEnabled(){
    return IP_SYSCTRL->REG_CLK_EN_CTRL0.bit.ENA_GPT_S_CLK;
}

/**
 * @brief Sets the clock divider for Gpt_s.
 *
 * This function configures the clock division for Gpt_s by setting the divider ratios
 * to the specified values. The division is defined by  and 'div_m'.
 * These parameters determine how the input clock frequency is divided to derive the
 * desired Gpt_s clock frequency.
 *
 *
 * @param div_m The denominator part of the clock division ratio. Specifies the lower part
 *              of the division ratio, The div_m select range from [1 - 15].
 *
 *
 * @return int32_t Returns 0 on success, or a non-zero error code on failure. The error code
 *                 typically indicates what went wrong during the configuration process.
 *
 * @note The exact behavior and limitations of the division ratio depend on the specific
 *       hardware capabilities and clock configuration. Ensure that the values of  and 'div_m'
 *       are within the valid range for your hardware.
 *
 * @warning Improper configuration of the clock divider might disrupt UART0 communication.
 *          It's important to ensure that  and 'div_m' are set to values that are
 *          compatible with the Gpt_s specifications and the overall system clock settings.
 */
int32_t HAL_CRM_SetGpt_sClkDiv(uint32_t div_m);

/**
 * @brief Retrieves the clock configuration for GPT_S.
 *
 * This inline static function obtains the current clock source and division factors for the
 * GPT_S module. The clock source and divider values are returned
 * through the pointer parameters  'div_m'.
 *
 *
 *
 * @param[out] div_m Pointer to a 'uint32_t' variable where the denominator of the clock division
 *                   ratio will be stored.
 *
 * @note Being an inline function, 'HAL_CRM_GetGpt_sClkConfig()' is expanded at each point of call,
 *       which can increase code size but typically reduces execution time by avoiding a function
 *       call overhead. Use this function judiciously where performance is critical.
 *
 * @warning Ensure that the pointers passed to this function are valid and point to appropriate
 *          memory locations. Passing invalid pointers may lead to undefined behavior, including
 *          crashes. Also, consider the potential for race conditions if the clock configuration
 *          can be changed by other parts of the program while this function is being executed.
 */
inline static void HAL_CRM_GetGpt_sClkConfig(uint32_t* div_m){
    
    
    
    *div_m = IP_SYSCTRL->REG_PERI_CLK_CFG3.bit.DIV_GPT_S_CLK_M;
    
}

/**
 * @brief Retrieves the current operating frequency of GPT_S.
 *
 * This function returns the frequency (in Hz) at which the GPT_S
 * is currently operating. The frequency is calculated based on the current configuration of
 * the system's clock sources and the GPT_S clock divider settings.
 *
 * @return uint32_t The operating frequency of GPT_S in Hertz. If the frequency cannot be
 *                  determined, or if GPT_S is not properly configured, the function may return
 *                  0.
 *
 * @note The returned frequency value is dependent on the current state of the system's clock
 *       configuration and the GPT_S divider settings. Changes in these parameters can affect
 *       the GPT_S frequency.
 *
 * @warning Ensure that GPT_S and its clock sources are properly configured before calling this
 *          function. Calling this function without proper initialization may lead to undefined
 *          behavior or incorrect frequency values.
 */
uint32_t CRM_GetGpt_sFreq();
/**
  * @}
  */

/** @defgroup _CRM_GPT GPT_CLK_FUNC
  * @brief GPT clock control function which can enable, disable or get status from corresponding
  * device, set ip clock source, set ip clock divider, or get ip clock configuration, help you calculate the ip divider
  * parameter when having ip reference clock and desire clock
  * @{
  */

/**
 * @brief Macro to enable the clock for GPT.
 *
 * This macro enables the clock for the GPT module.
 * It typically modifies a specific bit in a hardware register to provide the clock
 * to GPT, allowing the module to operate. This macro should be called before
 * initializing or using GPT to ensure that the hardware is properly powered and
 * ready for operation.
 *
 * Usage:
 *      __HAL_CRM_GPT_CLK_ENABLE();
 *
 * @note This macro directly interacts with hardware registers, and its effects are
 *       immediate. Ensure that the system is in a state where enabling the GPT clock
 *       is safe and appropriate.
 *
 * @warning Incorrect use of this macro, such as enabling the clock without proper
 *          configuration of the GPT module, may lead to unexpected behavior or
 *          system instability. Always ensure that the peripheral is configured
 *          correctly before enabling its clock.
 */
#define __HAL_CRM_GPT_CLK_ENABLE()    \
do { \
	IP_SYSCTRL->REG_CLK_EN_CTRL0.bit.ENA_GPT_CLK = 0x1; \
} while(0)
/**
 * @brief Macro to disable the clock for GPT.
 *
 * This macro disables the clock for the GPT module.
 * Disabling the clock can be useful in power-saving modes or when the GPT module is
 * not in use. This macro typically modifies a specific bit in a hardware register to
 * stop the clock supply to GPT.
 *
 * Usage:
 *      __HAL_CRM_SPI0_CLK_DISABLE();
 *
 * @note Disabling the clock to a module while it is in use can lead to incomplete or
 *       corrupted data transfers and should be done with caution. Ensure that GPT
 *       is not actively transmitting or receiving data before calling this macro.
 *
 * @warning Improper use of this macro, such as disabling the clock during an active
 *          GPT operation, may result in system instability or data corruption. Always
 *          make sure that the peripheral is idle or powered off before disabling its clock.
 */
#define __HAL_CRM_GPT_CLK_DISABLE()    \
do { \
	IP_SYSCTRL->REG_CLK_EN_CTRL0.bit.ENA_GPT_CLK = 0x0; \
} while(0)
/**
 * @brief Checks if the GPT clock is enabled.
 *
 * This inline static function determines whether the clock for the GPT module is currently
 * enabled. It typically checks a specific bit in a control register and returns the status.
 * This function can be used to verify the clock state of GPT before performing operations
 * that require the clock to be active.
 *
 * @return uint32_t Returns 1 if the GPT clock is enabled, and 0 if it is disabled.
 *
 * @note Since this is an inline function, it is expanded at the point of each call, which can
 *       lead to increased code size if used frequently. However, inlining often results in
 *       faster execution, as the overhead of a function call is eliminated.
 *
 * @warning This function should be used with the understanding that the state of the clock could
 *          change immediately after the function call, especially in multi-threaded or
 *          interrupt-driven environments. Additional synchronization mechanisms may be needed
 *          in such cases.
 */
inline static uint32_t HAL_CRM_GptClkIsEnabled(){
    return IP_SYSCTRL->REG_CLK_EN_CTRL0.bit.ENA_GPT_CLK;
}

/**
 * @brief Retrieves the current operating frequency of GPT.
 *
 * This function returns the frequency (in Hz) at which the GPT
 * is currently operating. The frequency is calculated based on the current configuration of
 * the system's clock sources and the GPT clock divider settings.
 *
 * @return uint32_t The operating frequency of GPT in Hertz. If the frequency cannot be
 *                  determined, or if GPT is not properly configured, the function may return
 *                  0.
 *
 * @note The returned frequency value is dependent on the current state of the system's clock
 *       configuration and the GPT divider settings. Changes in these parameters can affect
 *       the GPT frequency.
 *
 * @warning Ensure that GPT and its clock sources are properly configured before calling this
 *          function. Calling this function without proper initialization may lead to undefined
 *          behavior or incorrect frequency values.
 */
uint32_t CRM_GetGptFreq();
/**
  * @}
  */

/** @defgroup _CRM_GPT_T0 GPT_T0_CLK_FUNC
  * @brief GPT_T0 clock control function which can enable, disable or get status from corresponding
  * device, set ip clock source, set ip clock divider, or get ip clock configuration, help you calculate the ip divider
  * parameter when having ip reference clock and desire clock
  * @{
  */

/**
 * @brief Macro to enable the clock for GPT_T0.
 *
 * This macro enables the clock for the GPT_T0 module.
 * It typically modifies a specific bit in a hardware register to provide the clock
 * to GPT_T0, allowing the module to operate. This macro should be called before
 * initializing or using GPT_T0 to ensure that the hardware is properly powered and
 * ready for operation.
 *
 * Usage:
 *      __HAL_CRM_GPT_T0_CLK_ENABLE();
 *
 * @note This macro directly interacts with hardware registers, and its effects are
 *       immediate. Ensure that the system is in a state where enabling the GPT_T0 clock
 *       is safe and appropriate.
 *
 * @warning Incorrect use of this macro, such as enabling the clock without proper
 *          configuration of the GPT_T0 module, may lead to unexpected behavior or
 *          system instability. Always ensure that the peripheral is configured
 *          correctly before enabling its clock.
 */
#define __HAL_CRM_GPT_T0_CLK_ENABLE()    \
do { \
	IP_SYSCTRL->REG_CLK_EN_CTRL0.bit.ENA_GPT_T0_CLK = 0x1; \
} while(0)
/**
 * @brief Macro to disable the clock for GPT_T0.
 *
 * This macro disables the clock for the GPT_T0 module.
 * Disabling the clock can be useful in power-saving modes or when the GPT_T0 module is
 * not in use. This macro typically modifies a specific bit in a hardware register to
 * stop the clock supply to GPT_T0.
 *
 * Usage:
 *      __HAL_CRM_SPI0_CLK_DISABLE();
 *
 * @note Disabling the clock to a module while it is in use can lead to incomplete or
 *       corrupted data transfers and should be done with caution. Ensure that GPT_T0
 *       is not actively transmitting or receiving data before calling this macro.
 *
 * @warning Improper use of this macro, such as disabling the clock during an active
 *          GPT_T0 operation, may result in system instability or data corruption. Always
 *          make sure that the peripheral is idle or powered off before disabling its clock.
 */
#define __HAL_CRM_GPT_T0_CLK_DISABLE()    \
do { \
	IP_SYSCTRL->REG_CLK_EN_CTRL0.bit.ENA_GPT_T0_CLK = 0x0; \
} while(0)
/**
 * @brief Checks if the GPT_T0 clock is enabled.
 *
 * This inline static function determines whether the clock for the GPT_T0 module is currently
 * enabled. It typically checks a specific bit in a control register and returns the status.
 * This function can be used to verify the clock state of GPT_T0 before performing operations
 * that require the clock to be active.
 *
 * @return uint32_t Returns 1 if the GPT_T0 clock is enabled, and 0 if it is disabled.
 *
 * @note Since this is an inline function, it is expanded at the point of each call, which can
 *       lead to increased code size if used frequently. However, inlining often results in
 *       faster execution, as the overhead of a function call is eliminated.
 *
 * @warning This function should be used with the understanding that the state of the clock could
 *          change immediately after the function call, especially in multi-threaded or
 *          interrupt-driven environments. Additional synchronization mechanisms may be needed
 *          in such cases.
 */
inline static uint32_t HAL_CRM_Gpt_t0ClkIsEnabled(){
    return IP_SYSCTRL->REG_CLK_EN_CTRL0.bit.ENA_GPT_T0_CLK;
}

/**
 * @brief Sets the clock divider for Gpt_t0.
 *
 * This function configures the clock division for Gpt_t0 by setting the divider ratios
 * to the specified values. The division is defined by  and 'div_m'.
 * These parameters determine how the input clock frequency is divided to derive the
 * desired Gpt_t0 clock frequency.
 *
 *
 * @param div_m The denominator part of the clock division ratio. Specifies the lower part
 *              of the division ratio, The div_m select range from [1 - 15].
 *
 *
 * @return int32_t Returns 0 on success, or a non-zero error code on failure. The error code
 *                 typically indicates what went wrong during the configuration process.
 *
 * @note The exact behavior and limitations of the division ratio depend on the specific
 *       hardware capabilities and clock configuration. Ensure that the values of  and 'div_m'
 *       are within the valid range for your hardware.
 *
 * @warning Improper configuration of the clock divider might disrupt UART0 communication.
 *          It's important to ensure that  and 'div_m' are set to values that are
 *          compatible with the Gpt_t0 specifications and the overall system clock settings.
 */
int32_t HAL_CRM_SetGpt_t0ClkDiv(uint32_t div_m);

/**
 * @brief Retrieves the clock configuration for GPT_T0.
 *
 * This inline static function obtains the current clock source and division factors for the
 * GPT_T0 module. The clock source and divider values are returned
 * through the pointer parameters  'div_m'.
 *
 *
 *
 * @param[out] div_m Pointer to a 'uint32_t' variable where the denominator of the clock division
 *                   ratio will be stored.
 *
 * @note Being an inline function, 'HAL_CRM_GetGpt_t0ClkConfig()' is expanded at each point of call,
 *       which can increase code size but typically reduces execution time by avoiding a function
 *       call overhead. Use this function judiciously where performance is critical.
 *
 * @warning Ensure that the pointers passed to this function are valid and point to appropriate
 *          memory locations. Passing invalid pointers may lead to undefined behavior, including
 *          crashes. Also, consider the potential for race conditions if the clock configuration
 *          can be changed by other parts of the program while this function is being executed.
 */
inline static void HAL_CRM_GetGpt_t0ClkConfig(uint32_t* div_m){
    
    
    
    *div_m = IP_SYSCTRL->REG_PERI_CLK_CFG3.bit.DIV_GPT_T0_CLK_M;
    
}

/**
 * @brief Retrieves the current operating frequency of GPT_T0.
 *
 * This function returns the frequency (in Hz) at which the GPT_T0
 * is currently operating. The frequency is calculated based on the current configuration of
 * the system's clock sources and the GPT_T0 clock divider settings.
 *
 * @return uint32_t The operating frequency of GPT_T0 in Hertz. If the frequency cannot be
 *                  determined, or if GPT_T0 is not properly configured, the function may return
 *                  0.
 *
 * @note The returned frequency value is dependent on the current state of the system's clock
 *       configuration and the GPT_T0 divider settings. Changes in these parameters can affect
 *       the GPT_T0 frequency.
 *
 * @warning Ensure that GPT_T0 and its clock sources are properly configured before calling this
 *          function. Calling this function without proper initialization may lead to undefined
 *          behavior or incorrect frequency values.
 */
uint32_t CRM_GetGpt_t0Freq();
/**
  * @}
  */

/** @defgroup _CRM_I2S I2S_CLK_FUNC
  * @brief I2S clock control function which can enable, disable or get status from corresponding
  * device, set ip clock source, set ip clock divider, or get ip clock configuration, help you calculate the ip divider
  * parameter when having ip reference clock and desire clock
  * @{
  */

/**
 * @brief Macro to enable the clock for I2S.
 *
 * This macro enables the clock for the I2S module.
 * It typically modifies a specific bit in a hardware register to provide the clock
 * to I2S, allowing the module to operate. This macro should be called before
 * initializing or using I2S to ensure that the hardware is properly powered and
 * ready for operation.
 *
 * Usage:
 *      __HAL_CRM_I2S_CLK_ENABLE();
 *
 * @note This macro directly interacts with hardware registers, and its effects are
 *       immediate. Ensure that the system is in a state where enabling the I2S clock
 *       is safe and appropriate.
 *
 * @warning Incorrect use of this macro, such as enabling the clock without proper
 *          configuration of the I2S module, may lead to unexpected behavior or
 *          system instability. Always ensure that the peripheral is configured
 *          correctly before enabling its clock.
 */
#define __HAL_CRM_I2S_CLK_ENABLE()    \
do { \
	IP_SYSCTRL->REG_CLK_EN_CTRL0.bit.ENA_I2S_CLK = 0x1; \
} while(0)
/**
 * @brief Macro to disable the clock for I2S.
 *
 * This macro disables the clock for the I2S module.
 * Disabling the clock can be useful in power-saving modes or when the I2S module is
 * not in use. This macro typically modifies a specific bit in a hardware register to
 * stop the clock supply to I2S.
 *
 * Usage:
 *      __HAL_CRM_SPI0_CLK_DISABLE();
 *
 * @note Disabling the clock to a module while it is in use can lead to incomplete or
 *       corrupted data transfers and should be done with caution. Ensure that I2S
 *       is not actively transmitting or receiving data before calling this macro.
 *
 * @warning Improper use of this macro, such as disabling the clock during an active
 *          I2S operation, may result in system instability or data corruption. Always
 *          make sure that the peripheral is idle or powered off before disabling its clock.
 */
#define __HAL_CRM_I2S_CLK_DISABLE()    \
do { \
	IP_SYSCTRL->REG_CLK_EN_CTRL0.bit.ENA_I2S_CLK = 0x0; \
} while(0)
/**
 * @brief Checks if the I2S clock is enabled.
 *
 * This inline static function determines whether the clock for the I2S module is currently
 * enabled. It typically checks a specific bit in a control register and returns the status.
 * This function can be used to verify the clock state of I2S before performing operations
 * that require the clock to be active.
 *
 * @return uint32_t Returns 1 if the I2S clock is enabled, and 0 if it is disabled.
 *
 * @note Since this is an inline function, it is expanded at the point of each call, which can
 *       lead to increased code size if used frequently. However, inlining often results in
 *       faster execution, as the overhead of a function call is eliminated.
 *
 * @warning This function should be used with the understanding that the state of the clock could
 *          change immediately after the function call, especially in multi-threaded or
 *          interrupt-driven environments. Additional synchronization mechanisms may be needed
 *          in such cases.
 */
inline static uint32_t HAL_CRM_I2sClkIsEnabled(){
    return IP_SYSCTRL->REG_CLK_EN_CTRL0.bit.ENA_I2S_CLK;
}

/**
 * @brief Retrieves the current operating frequency of I2S.
 *
 * This function returns the frequency (in Hz) at which the I2S
 * is currently operating. The frequency is calculated based on the current configuration of
 * the system's clock sources and the I2S clock divider settings.
 *
 * @return uint32_t The operating frequency of I2S in Hertz. If the frequency cannot be
 *                  determined, or if I2S is not properly configured, the function may return
 *                  0.
 *
 * @note The returned frequency value is dependent on the current state of the system's clock
 *       configuration and the I2S divider settings. Changes in these parameters can affect
 *       the I2S frequency.
 *
 * @warning Ensure that I2S and its clock sources are properly configured before calling this
 *          function. Calling this function without proper initialization may lead to undefined
 *          behavior or incorrect frequency values.
 */
uint32_t CRM_GetI2sFreq();
/**
  * @}
  */

/** @defgroup _CRM_CODEC CODEC_CLK_FUNC
  * @brief CODEC clock control function which can enable, disable or get status from corresponding
  * device, set ip clock source, set ip clock divider, or get ip clock configuration, help you calculate the ip divider
  * parameter when having ip reference clock and desire clock
  * @{
  */

/**
 * @brief Macro to enable the clock for CODEC.
 *
 * This macro enables the clock for the CODEC module.
 * It typically modifies a specific bit in a hardware register to provide the clock
 * to CODEC, allowing the module to operate. This macro should be called before
 * initializing or using CODEC to ensure that the hardware is properly powered and
 * ready for operation.
 *
 * Usage:
 *      __HAL_CRM_CODEC_CLK_ENABLE();
 *
 * @note This macro directly interacts with hardware registers, and its effects are
 *       immediate. Ensure that the system is in a state where enabling the CODEC clock
 *       is safe and appropriate.
 *
 * @warning Incorrect use of this macro, such as enabling the clock without proper
 *          configuration of the CODEC module, may lead to unexpected behavior or
 *          system instability. Always ensure that the peripheral is configured
 *          correctly before enabling its clock.
 */
#define __HAL_CRM_CODEC_CLK_ENABLE()    \
do { \
	IP_SYSCTRL->REG_CLK_EN_CTRL0.bit.ENA_CODEC_CLK = 0x1; \
} while(0)
/**
 * @brief Macro to disable the clock for CODEC.
 *
 * This macro disables the clock for the CODEC module.
 * Disabling the clock can be useful in power-saving modes or when the CODEC module is
 * not in use. This macro typically modifies a specific bit in a hardware register to
 * stop the clock supply to CODEC.
 *
 * Usage:
 *      __HAL_CRM_SPI0_CLK_DISABLE();
 *
 * @note Disabling the clock to a module while it is in use can lead to incomplete or
 *       corrupted data transfers and should be done with caution. Ensure that CODEC
 *       is not actively transmitting or receiving data before calling this macro.
 *
 * @warning Improper use of this macro, such as disabling the clock during an active
 *          CODEC operation, may result in system instability or data corruption. Always
 *          make sure that the peripheral is idle or powered off before disabling its clock.
 */
#define __HAL_CRM_CODEC_CLK_DISABLE()    \
do { \
	IP_SYSCTRL->REG_CLK_EN_CTRL0.bit.ENA_CODEC_CLK = 0x0; \
} while(0)
/**
 * @brief Checks if the CODEC clock is enabled.
 *
 * This inline static function determines whether the clock for the CODEC module is currently
 * enabled. It typically checks a specific bit in a control register and returns the status.
 * This function can be used to verify the clock state of CODEC before performing operations
 * that require the clock to be active.
 *
 * @return uint32_t Returns 1 if the CODEC clock is enabled, and 0 if it is disabled.
 *
 * @note Since this is an inline function, it is expanded at the point of each call, which can
 *       lead to increased code size if used frequently. However, inlining often results in
 *       faster execution, as the overhead of a function call is eliminated.
 *
 * @warning This function should be used with the understanding that the state of the clock could
 *          change immediately after the function call, especially in multi-threaded or
 *          interrupt-driven environments. Additional synchronization mechanisms may be needed
 *          in such cases.
 */
inline static uint32_t HAL_CRM_CodecClkIsEnabled(){
    return IP_SYSCTRL->REG_CLK_EN_CTRL0.bit.ENA_CODEC_CLK;
}

/**
 * @brief Retrieves the current operating frequency of CODEC.
 *
 * This function returns the frequency (in Hz) at which the CODEC
 * is currently operating. The frequency is calculated based on the current configuration of
 * the system's clock sources and the CODEC clock divider settings.
 *
 * @return uint32_t The operating frequency of CODEC in Hertz. If the frequency cannot be
 *                  determined, or if CODEC is not properly configured, the function may return
 *                  0.
 *
 * @note The returned frequency value is dependent on the current state of the system's clock
 *       configuration and the CODEC divider settings. Changes in these parameters can affect
 *       the CODEC frequency.
 *
 * @warning Ensure that CODEC and its clock sources are properly configured before calling this
 *          function. Calling this function without proper initialization may lead to undefined
 *          behavior or incorrect frequency values.
 */
uint32_t CRM_GetCodecFreq();
/**
  * @}
  */

/** @defgroup _CRM_APC APC_CLK_FUNC
  * @brief APC clock control function which can enable, disable or get status from corresponding
  * device, set ip clock source, set ip clock divider, or get ip clock configuration, help you calculate the ip divider
  * parameter when having ip reference clock and desire clock
  * @{
  */

/**
 * @brief Macro to enable the clock for APC.
 *
 * This macro enables the clock for the APC module.
 * It typically modifies a specific bit in a hardware register to provide the clock
 * to APC, allowing the module to operate. This macro should be called before
 * initializing or using APC to ensure that the hardware is properly powered and
 * ready for operation.
 *
 * Usage:
 *      __HAL_CRM_APC_CLK_ENABLE();
 *
 * @note This macro directly interacts with hardware registers, and its effects are
 *       immediate. Ensure that the system is in a state where enabling the APC clock
 *       is safe and appropriate.
 *
 * @warning Incorrect use of this macro, such as enabling the clock without proper
 *          configuration of the APC module, may lead to unexpected behavior or
 *          system instability. Always ensure that the peripheral is configured
 *          correctly before enabling its clock.
 */
#define __HAL_CRM_APC_CLK_ENABLE()    \
do { \
	IP_SYSCTRL->REG_CLK_EN_CTRL0.bit.ENA_APC_CLK = 0x1; \
} while(0)
/**
 * @brief Macro to disable the clock for APC.
 *
 * This macro disables the clock for the APC module.
 * Disabling the clock can be useful in power-saving modes or when the APC module is
 * not in use. This macro typically modifies a specific bit in a hardware register to
 * stop the clock supply to APC.
 *
 * Usage:
 *      __HAL_CRM_SPI0_CLK_DISABLE();
 *
 * @note Disabling the clock to a module while it is in use can lead to incomplete or
 *       corrupted data transfers and should be done with caution. Ensure that APC
 *       is not actively transmitting or receiving data before calling this macro.
 *
 * @warning Improper use of this macro, such as disabling the clock during an active
 *          APC operation, may result in system instability or data corruption. Always
 *          make sure that the peripheral is idle or powered off before disabling its clock.
 */
#define __HAL_CRM_APC_CLK_DISABLE()    \
do { \
	IP_SYSCTRL->REG_CLK_EN_CTRL0.bit.ENA_APC_CLK = 0x0; \
} while(0)
/**
 * @brief Checks if the APC clock is enabled.
 *
 * This inline static function determines whether the clock for the APC module is currently
 * enabled. It typically checks a specific bit in a control register and returns the status.
 * This function can be used to verify the clock state of APC before performing operations
 * that require the clock to be active.
 *
 * @return uint32_t Returns 1 if the APC clock is enabled, and 0 if it is disabled.
 *
 * @note Since this is an inline function, it is expanded at the point of each call, which can
 *       lead to increased code size if used frequently. However, inlining often results in
 *       faster execution, as the overhead of a function call is eliminated.
 *
 * @warning This function should be used with the understanding that the state of the clock could
 *          change immediately after the function call, especially in multi-threaded or
 *          interrupt-driven environments. Additional synchronization mechanisms may be needed
 *          in such cases.
 */
inline static uint32_t HAL_CRM_ApcClkIsEnabled(){
    return IP_SYSCTRL->REG_CLK_EN_CTRL0.bit.ENA_APC_CLK;
}

/**
 * @brief Retrieves the current operating frequency of APC.
 *
 * This function returns the frequency (in Hz) at which the APC
 * is currently operating. The frequency is calculated based on the current configuration of
 * the system's clock sources and the APC clock divider settings.
 *
 * @return uint32_t The operating frequency of APC in Hertz. If the frequency cannot be
 *                  determined, or if APC is not properly configured, the function may return
 *                  0.
 *
 * @note The returned frequency value is dependent on the current state of the system's clock
 *       configuration and the APC divider settings. Changes in these parameters can affect
 *       the APC frequency.
 *
 * @warning Ensure that APC and its clock sources are properly configured before calling this
 *          function. Calling this function without proper initialization may lead to undefined
 *          behavior or incorrect frequency values.
 */
uint32_t CRM_GetApcFreq();
/**
  * @}
  */

/** @defgroup _CRM_CLASSD CLASSD_CLK_FUNC
  * @brief CLASSD clock control function which can enable, disable or get status from corresponding
  * device, set ip clock source, set ip clock divider, or get ip clock configuration, help you calculate the ip divider
  * parameter when having ip reference clock and desire clock
  * @{
  */

/**
 * @brief Retrieves the current operating frequency of CLASSD.
 *
 * This function returns the frequency (in Hz) at which the CLASSD
 * is currently operating. The frequency is calculated based on the current configuration of
 * the system's clock sources and the CLASSD clock divider settings.
 *
 * @return uint32_t The operating frequency of CLASSD in Hertz. If the frequency cannot be
 *                  determined, or if CLASSD is not properly configured, the function may return
 *                  0.
 *
 * @note The returned frequency value is dependent on the current state of the system's clock
 *       configuration and the CLASSD divider settings. Changes in these parameters can affect
 *       the CLASSD frequency.
 *
 * @warning Ensure that CLASSD and its clock sources are properly configured before calling this
 *          function. Calling this function without proper initialization may lead to undefined
 *          behavior or incorrect frequency values.
 */
uint32_t CRM_GetClassdFreq();
/**
  * @}
  */

/** @defgroup _CRM_ADC ADC_CLK_FUNC
  * @brief ADC clock control function which can enable, disable or get status from corresponding
  * device, set ip clock source, set ip clock divider, or get ip clock configuration, help you calculate the ip divider
  * parameter when having ip reference clock and desire clock
  * @{
  */

/**
 * @brief Retrieves the current operating frequency of ADC.
 *
 * This function returns the frequency (in Hz) at which the ADC
 * is currently operating. The frequency is calculated based on the current configuration of
 * the system's clock sources and the ADC clock divider settings.
 *
 * @return uint32_t The operating frequency of ADC in Hertz. If the frequency cannot be
 *                  determined, or if ADC is not properly configured, the function may return
 *                  0.
 *
 * @note The returned frequency value is dependent on the current state of the system's clock
 *       configuration and the ADC divider settings. Changes in these parameters can affect
 *       the ADC frequency.
 *
 * @warning Ensure that ADC and its clock sources are properly configured before calling this
 *          function. Calling this function without proper initialization may lead to undefined
 *          behavior or incorrect frequency values.
 */
uint32_t CRM_GetAdcFreq();
/**
  * @}
  */

/** @defgroup _CRM_CORE_32K CORE_32K_CLK_FUNC
  * @brief CORE_32K clock control function which can enable, disable or get status from corresponding
  * device, set ip clock source, set ip clock divider, or get ip clock configuration, help you calculate the ip divider
  * parameter when having ip reference clock and desire clock
  * @{
  */

/**
 * @brief Macro to enable the clock for CORE_32K.
 *
 * This macro enables the clock for the CORE_32K module.
 * It typically modifies a specific bit in a hardware register to provide the clock
 * to CORE_32K, allowing the module to operate. This macro should be called before
 * initializing or using CORE_32K to ensure that the hardware is properly powered and
 * ready for operation.
 *
 * Usage:
 *      __HAL_CRM_CORE_32K_CLK_ENABLE();
 *
 * @note This macro directly interacts with hardware registers, and its effects are
 *       immediate. Ensure that the system is in a state where enabling the CORE_32K clock
 *       is safe and appropriate.
 *
 * @warning Incorrect use of this macro, such as enabling the clock without proper
 *          configuration of the CORE_32K module, may lead to unexpected behavior or
 *          system instability. Always ensure that the peripheral is configured
 *          correctly before enabling its clock.
 */
#define __HAL_CRM_CORE_32K_CLK_ENABLE()    \
do { \
	IP_SYSCTRL->REG_CLK_EN_CTRL0.bit.ENA_CORE_32K_CLK = 0x1; \
} while(0)
/**
 * @brief Macro to disable the clock for CORE_32K.
 *
 * This macro disables the clock for the CORE_32K module.
 * Disabling the clock can be useful in power-saving modes or when the CORE_32K module is
 * not in use. This macro typically modifies a specific bit in a hardware register to
 * stop the clock supply to CORE_32K.
 *
 * Usage:
 *      __HAL_CRM_SPI0_CLK_DISABLE();
 *
 * @note Disabling the clock to a module while it is in use can lead to incomplete or
 *       corrupted data transfers and should be done with caution. Ensure that CORE_32K
 *       is not actively transmitting or receiving data before calling this macro.
 *
 * @warning Improper use of this macro, such as disabling the clock during an active
 *          CORE_32K operation, may result in system instability or data corruption. Always
 *          make sure that the peripheral is idle or powered off before disabling its clock.
 */
#define __HAL_CRM_CORE_32K_CLK_DISABLE()    \
do { \
	IP_SYSCTRL->REG_CLK_EN_CTRL0.bit.ENA_CORE_32K_CLK = 0x0; \
} while(0)
/**
 * @brief Checks if the CORE_32K clock is enabled.
 *
 * This inline static function determines whether the clock for the CORE_32K module is currently
 * enabled. It typically checks a specific bit in a control register and returns the status.
 * This function can be used to verify the clock state of CORE_32K before performing operations
 * that require the clock to be active.
 *
 * @return uint32_t Returns 1 if the CORE_32K clock is enabled, and 0 if it is disabled.
 *
 * @note Since this is an inline function, it is expanded at the point of each call, which can
 *       lead to increased code size if used frequently. However, inlining often results in
 *       faster execution, as the overhead of a function call is eliminated.
 *
 * @warning This function should be used with the understanding that the state of the clock could
 *          change immediately after the function call, especially in multi-threaded or
 *          interrupt-driven environments. Additional synchronization mechanisms may be needed
 *          in such cases.
 */
inline static uint32_t HAL_CRM_Core_32kClkIsEnabled(){
    return IP_SYSCTRL->REG_CLK_EN_CTRL0.bit.ENA_CORE_32K_CLK;
}

/**
  * @}
  */

/** @defgroup _CRM_MTIME MTIME_CLK_FUNC
  * @brief MTIME clock control function which can enable, disable or get status from corresponding
  * device, set ip clock source, set ip clock divider, or get ip clock configuration, help you calculate the ip divider
  * parameter when having ip reference clock and desire clock
  * @{
  */

/**
 * @brief Macro to enable the clock for MTIME.
 *
 * This macro enables the clock for the MTIME module.
 * It typically modifies a specific bit in a hardware register to provide the clock
 * to MTIME, allowing the module to operate. This macro should be called before
 * initializing or using MTIME to ensure that the hardware is properly powered and
 * ready for operation.
 *
 * Usage:
 *      __HAL_CRM_MTIME_CLK_ENABLE();
 *
 * @note This macro directly interacts with hardware registers, and its effects are
 *       immediate. Ensure that the system is in a state where enabling the MTIME clock
 *       is safe and appropriate.
 *
 * @warning Incorrect use of this macro, such as enabling the clock without proper
 *          configuration of the MTIME module, may lead to unexpected behavior or
 *          system instability. Always ensure that the peripheral is configured
 *          correctly before enabling its clock.
 */
#define __HAL_CRM_MTIME_CLK_ENABLE()    \
do { \
	IP_SYSCTRL->REG_PERI_CLK_CFG4.bit.ENA_MTIME_TOGGLE_A = 0x1; \
} while(0)
/**
 * @brief Macro to disable the clock for MTIME.
 *
 * This macro disables the clock for the MTIME module.
 * Disabling the clock can be useful in power-saving modes or when the MTIME module is
 * not in use. This macro typically modifies a specific bit in a hardware register to
 * stop the clock supply to MTIME.
 *
 * Usage:
 *      __HAL_CRM_SPI0_CLK_DISABLE();
 *
 * @note Disabling the clock to a module while it is in use can lead to incomplete or
 *       corrupted data transfers and should be done with caution. Ensure that MTIME
 *       is not actively transmitting or receiving data before calling this macro.
 *
 * @warning Improper use of this macro, such as disabling the clock during an active
 *          MTIME operation, may result in system instability or data corruption. Always
 *          make sure that the peripheral is idle or powered off before disabling its clock.
 */
#define __HAL_CRM_MTIME_CLK_DISABLE()    \
do { \
	IP_SYSCTRL->REG_PERI_CLK_CFG4.bit.ENA_MTIME_TOGGLE_A = 0x0; \
} while(0)
/**
 * @brief Checks if the MTIME clock is enabled.
 *
 * This inline static function determines whether the clock for the MTIME module is currently
 * enabled. It typically checks a specific bit in a control register and returns the status.
 * This function can be used to verify the clock state of MTIME before performing operations
 * that require the clock to be active.
 *
 * @return uint32_t Returns 1 if the MTIME clock is enabled, and 0 if it is disabled.
 *
 * @note Since this is an inline function, it is expanded at the point of each call, which can
 *       lead to increased code size if used frequently. However, inlining often results in
 *       faster execution, as the overhead of a function call is eliminated.
 *
 * @warning This function should be used with the understanding that the state of the clock could
 *          change immediately after the function call, especially in multi-threaded or
 *          interrupt-driven environments. Additional synchronization mechanisms may be needed
 *          in such cases.
 */
inline static uint32_t HAL_CRM_MtimeClkIsEnabled(){
    return IP_SYSCTRL->REG_PERI_CLK_CFG4.bit.ENA_MTIME_TOGGLE_A;
}

/**
 * @brief Sets the clock divider for Mtime.
 *
 * This function configures the clock division for Mtime by setting the divider ratios
 * to the specified values. The division is defined by  and 'div_m'.
 * These parameters determine how the input clock frequency is divided to derive the
 * desired Mtime clock frequency.
 *
 *
 * @param div_m The denominator part of the clock division ratio. Specifies the lower part
 *              of the division ratio, The div_m select range from [1 - 63].
 *
 *
 * @return int32_t Returns 0 on success, or a non-zero error code on failure. The error code
 *                 typically indicates what went wrong during the configuration process.
 *
 * @note The exact behavior and limitations of the division ratio depend on the specific
 *       hardware capabilities and clock configuration. Ensure that the values of  and 'div_m'
 *       are within the valid range for your hardware.
 *
 * @warning Improper configuration of the clock divider might disrupt UART0 communication.
 *          It's important to ensure that  and 'div_m' are set to values that are
 *          compatible with the Mtime specifications and the overall system clock settings.
 */
int32_t HAL_CRM_SetMtimeClkDiv(uint32_t div_m);

/**
 * @brief Retrieves the clock configuration for MTIME.
 *
 * This inline static function obtains the current clock source and division factors for the
 * MTIME module. The clock source and divider values are returned
 * through the pointer parameters  'div_m'.
 *
 *
 *
 * @param[out] div_m Pointer to a 'uint32_t' variable where the denominator of the clock division
 *                   ratio will be stored.
 *
 * @note Being an inline function, 'HAL_CRM_GetMtimeClkConfig()' is expanded at each point of call,
 *       which can increase code size but typically reduces execution time by avoiding a function
 *       call overhead. Use this function judiciously where performance is critical.
 *
 * @warning Ensure that the pointers passed to this function are valid and point to appropriate
 *          memory locations. Passing invalid pointers may lead to undefined behavior, including
 *          crashes. Also, consider the potential for race conditions if the clock configuration
 *          can be changed by other parts of the program while this function is being executed.
 */
inline static void HAL_CRM_GetMtimeClkConfig(uint32_t* div_m){
    
    
    
    *div_m = IP_SYSCTRL->REG_PERI_CLK_CFG4.bit.DIV_MTIME_TOGGLE_A_M;
    
}

/**
 * @brief Retrieves the current operating frequency of MTIME.
 *
 * This function returns the frequency (in Hz) at which the MTIME
 * is currently operating. The frequency is calculated based on the current configuration of
 * the system's clock sources and the MTIME clock divider settings.
 *
 * @return uint32_t The operating frequency of MTIME in Hertz. If the frequency cannot be
 *                  determined, or if MTIME is not properly configured, the function may return
 *                  0.
 *
 * @note The returned frequency value is dependent on the current state of the system's clock
 *       configuration and the MTIME divider settings. Changes in these parameters can affect
 *       the MTIME frequency.
 *
 * @warning Ensure that MTIME and its clock sources are properly configured before calling this
 *          function. Calling this function without proper initialization may lead to undefined
 *          behavior or incorrect frequency values.
 */
uint32_t CRM_GetMtimeFreq();
/**
  * @}
  */

/** @defgroup _CRM_LUNA LUNA_CLK_FUNC
  * @brief LUNA clock control function which can enable, disable or get status from corresponding
  * device, set ip clock source, set ip clock divider, or get ip clock configuration, help you calculate the ip divider
  * parameter when having ip reference clock and desire clock
  * @{
  */

/**
 * @brief Macro to enable the clock for LUNA.
 *
 * This macro enables the clock for the LUNA module.
 * It typically modifies a specific bit in a hardware register to provide the clock
 * to LUNA, allowing the module to operate. This macro should be called before
 * initializing or using LUNA to ensure that the hardware is properly powered and
 * ready for operation.
 *
 * Usage:
 *      __HAL_CRM_LUNA_CLK_ENABLE();
 *
 * @note This macro directly interacts with hardware registers, and its effects are
 *       immediate. Ensure that the system is in a state where enabling the LUNA clock
 *       is safe and appropriate.
 *
 * @warning Incorrect use of this macro, such as enabling the clock without proper
 *          configuration of the LUNA module, may lead to unexpected behavior or
 *          system instability. Always ensure that the peripheral is configured
 *          correctly before enabling its clock.
 */
#define __HAL_CRM_LUNA_CLK_ENABLE()    \
do { \
	IP_SYSCTRL->REG_CLK_EN_CTRL0.bit.ENA_LUNA_CLK = 0x1; \
} while(0)
/**
 * @brief Macro to disable the clock for LUNA.
 *
 * This macro disables the clock for the LUNA module.
 * Disabling the clock can be useful in power-saving modes or when the LUNA module is
 * not in use. This macro typically modifies a specific bit in a hardware register to
 * stop the clock supply to LUNA.
 *
 * Usage:
 *      __HAL_CRM_SPI0_CLK_DISABLE();
 *
 * @note Disabling the clock to a module while it is in use can lead to incomplete or
 *       corrupted data transfers and should be done with caution. Ensure that LUNA
 *       is not actively transmitting or receiving data before calling this macro.
 *
 * @warning Improper use of this macro, such as disabling the clock during an active
 *          LUNA operation, may result in system instability or data corruption. Always
 *          make sure that the peripheral is idle or powered off before disabling its clock.
 */
#define __HAL_CRM_LUNA_CLK_DISABLE()    \
do { \
	IP_SYSCTRL->REG_CLK_EN_CTRL0.bit.ENA_LUNA_CLK = 0x0; \
} while(0)
/**
 * @brief Checks if the LUNA clock is enabled.
 *
 * This inline static function determines whether the clock for the LUNA module is currently
 * enabled. It typically checks a specific bit in a control register and returns the status.
 * This function can be used to verify the clock state of LUNA before performing operations
 * that require the clock to be active.
 *
 * @return uint32_t Returns 1 if the LUNA clock is enabled, and 0 if it is disabled.
 *
 * @note Since this is an inline function, it is expanded at the point of each call, which can
 *       lead to increased code size if used frequently. However, inlining often results in
 *       faster execution, as the overhead of a function call is eliminated.
 *
 * @warning This function should be used with the understanding that the state of the clock could
 *          change immediately after the function call, especially in multi-threaded or
 *          interrupt-driven environments. Additional synchronization mechanisms may be needed
 *          in such cases.
 */
inline static uint32_t HAL_CRM_LunaClkIsEnabled(){
    return IP_SYSCTRL->REG_CLK_EN_CTRL0.bit.ENA_LUNA_CLK;
}

/**
 * @brief Retrieves the current operating frequency of LUNA.
 *
 * This function returns the frequency (in Hz) at which the LUNA
 * is currently operating. The frequency is calculated based on the current configuration of
 * the system's clock sources and the LUNA clock divider settings.
 *
 * @return uint32_t The operating frequency of LUNA in Hertz. If the frequency cannot be
 *                  determined, or if LUNA is not properly configured, the function may return
 *                  0.
 *
 * @note The returned frequency value is dependent on the current state of the system's clock
 *       configuration and the LUNA divider settings. Changes in these parameters can affect
 *       the LUNA frequency.
 *
 * @warning Ensure that LUNA and its clock sources are properly configured before calling this
 *          function. Calling this function without proper initialization may lead to undefined
 *          behavior or incorrect frequency values.
 */
uint32_t CRM_GetLunaFreq();
/**
  * @}
  */

/**********************************CORE************************************/

/** @defgroup _CRM_AP_PERI_PCLK AP_PERI_PCLK_CLK_FUNC
  * @brief AP_PERI_PCLK clock control function which can enable, disable or get status from corresponding
  * source clock, set core clock source, set core clock divider, or get core clock configuration, help you calculate the core divider
  * parameter when having core reference clock and desire clock
  * @{
  */

/**
 * @brief Sets the clock divider for the Ap_peri_pclk.
 *
 * This function configures the clock division for the Ap_peri_pclk
 * using the specified divider ratios. The division is determined by 'div_n'
 * and 'div_m'. These parameters define how the input clock frequency is divided to obtain
 * the desired Ap_peri_pclk clock frequency.
 *
 * @param div_n The numerator part of the clock division ratio. It specifies the upper part
 *              of the division ratio, controlling how much the input clock frequency is
 *              divided, the div_n select range from [1 - 7].
 *
 *
 * @param div_m The denominator part of the clock division ratio. It specifies the lower part
 *              of the division ratio, contributing to the final clock frequency calculation,
 *              The div_m select range from [1 - 7]
 *
 * @return void This function does not return a value. Any errors or status conditions may be
 *              handled internally or reported through other mechanisms, as per the hardware
 *              design and system requirements.
 *
 * @note The behavior and constraints of the division ratio are dependent on the hardware
 *       capabilities and current clock configuration. It is important to ensure that the values
 *       for 'div_n' and 'div_m' are within the acceptable range for the system's hardware.
 *
 * @warning Incorrect configuration of the clock divider can affect the operation of the Ap_peri_pclk
 *          and connected bus. Ensure that 'div_n' and 'div_m' are set
 *          to values that are appropriate for the desired clock frequency and compatible with
 *          the peripheral's operational requirements.
 */
int32_t HAL_CRM_SetAp_peri_pclkClkDiv(uint32_t div_n, uint32_t div_m);

/**
 * @brief Retrieves the clock configuration for AP_PERI_PCLK.
 *
 * This inline static function obtains the current clock source and division factors for the
 * AP_PERI_PCLK module. The clock source and divider values are returned
 * through the pointer parameters  'div_n', and'div_m'.
 *
 *
 * @param[out] div_n Pointer to a 'uint32_t' variable where the numerator of the clock division
 *                   ratio will be stored.
 *
 *
 * @param[out] div_m Pointer to a 'uint32_t' variable where the denominator of the clock division
 *                   ratio will be stored.
 *
 * @note Being an inline function, 'HAL_CRM_GetAP_PERI_PCLKClkConfig()' is expanded at each point of call,
 *       which can increase code size but typically reduces execution time by avoiding a function
 *       call overhead. Use this function judiciously where performance is critical.
 *
 * @warning Ensure that the pointers passed to this function are valid and point to appropriate
 *          memory locations. Passing invalid pointers may lead to undefined behavior, including
 *          crashes. Also, consider the potential for race conditions if the clock configuration
 *          can be changed by other parts of the program while this function is being executed.
 */
inline static void HAL_CRM_GetAp_peri_pclkClkConfig(uint32_t* div_n, uint32_t* div_m){
    
    
    *div_n = IP_SYSNODEF->REG_BUS_CLK_CFG0.bit.DIV_AP_PERI_PCLK_N;
    
    
    *div_m = IP_SYSNODEF->REG_BUS_CLK_CFG0.bit.DIV_AP_PERI_PCLK_M;
    
}

/**
 * @brief Retrieves the current operating frequency of the Ap_peri_pclk.
 *
 * This function returns the frequency (in Hz) at which the Ap_peri_pclk is currently operating.
 * its frequency is crucial for determining the performance and timing characteristics of
 * various system components. The frequency is determined by the current configuration of
 * the system's clock sources and any relevant clock dividers.
 *
 * @return uint32_t The operating frequency of Ap_peri_pclk in Hertz. If the frequency cannot be
 *                  determined, or if Ap_peri_pclk is not properly configured, the function may
 *                  return 0.
 *
 * @note The returned frequency value is dependent on the current state of the system's
 *       clock configuration. Changes in clock source or divider settings can affect the
 *       Ap_peri_pclk frequency.
 *
 * @warning This function can call anywhere which get the Ap_peri_pclk clock frequency.
 */
uint32_t CRM_GetAp_peri_pclkFreq();
/**
  * @}
  */

/** @defgroup _CRM_AON_CFG_PCLK AON_CFG_PCLK_CLK_FUNC
  * @brief AON_CFG_PCLK clock control function which can enable, disable or get status from corresponding
  * source clock, set core clock source, set core clock divider, or get core clock configuration, help you calculate the core divider
  * parameter when having core reference clock and desire clock
  * @{
  */

/**
 * @brief Sets the clock divider for the Aon_cfg_pclk.
 *
 * This function configures the clock division for the Aon_cfg_pclk
 * using the specified divider ratios. The division is determined by 'div_n'
 * and 'div_m'. These parameters define how the input clock frequency is divided to obtain
 * the desired Aon_cfg_pclk clock frequency.
 *
 * @param div_n The numerator part of the clock division ratio. It specifies the upper part
 *              of the division ratio, controlling how much the input clock frequency is
 *              divided, the div_n select range from [1 - 31].
 *
 *
 * @param div_m The denominator part of the clock division ratio. It specifies the lower part
 *              of the division ratio, contributing to the final clock frequency calculation,
 *              The div_m select range from [1 - 31]
 *
 * @return void This function does not return a value. Any errors or status conditions may be
 *              handled internally or reported through other mechanisms, as per the hardware
 *              design and system requirements.
 *
 * @note The behavior and constraints of the division ratio are dependent on the hardware
 *       capabilities and current clock configuration. It is important to ensure that the values
 *       for 'div_n' and 'div_m' are within the acceptable range for the system's hardware.
 *
 * @warning Incorrect configuration of the clock divider can affect the operation of the Aon_cfg_pclk
 *          and connected bus. Ensure that 'div_n' and 'div_m' are set
 *          to values that are appropriate for the desired clock frequency and compatible with
 *          the peripheral's operational requirements.
 */
int32_t HAL_CRM_SetAon_cfg_pclkClkDiv(uint32_t div_n, uint32_t div_m);

/**
 * @brief Retrieves the clock configuration for AON_CFG_PCLK.
 *
 * This inline static function obtains the current clock source and division factors for the
 * AON_CFG_PCLK module. The clock source and divider values are returned
 * through the pointer parameters  'div_n', and'div_m'.
 *
 *
 * @param[out] div_n Pointer to a 'uint32_t' variable where the numerator of the clock division
 *                   ratio will be stored.
 *
 *
 * @param[out] div_m Pointer to a 'uint32_t' variable where the denominator of the clock division
 *                   ratio will be stored.
 *
 * @note Being an inline function, 'HAL_CRM_GetAON_CFG_PCLKClkConfig()' is expanded at each point of call,
 *       which can increase code size but typically reduces execution time by avoiding a function
 *       call overhead. Use this function judiciously where performance is critical.
 *
 * @warning Ensure that the pointers passed to this function are valid and point to appropriate
 *          memory locations. Passing invalid pointers may lead to undefined behavior, including
 *          crashes. Also, consider the potential for race conditions if the clock configuration
 *          can be changed by other parts of the program while this function is being executed.
 */
inline static void HAL_CRM_GetAon_cfg_pclkClkConfig(uint32_t* div_n, uint32_t* div_m){
    
    
    *div_n = IP_SYSNODEF->REG_BUS_CLK_CFG0.bit.DIV_AON_CFG_PCLK_N;
    
    
    *div_m = IP_SYSNODEF->REG_BUS_CLK_CFG0.bit.DIV_AON_CFG_PCLK_M;
    
}

/**
 * @brief Retrieves the current operating frequency of the Aon_cfg_pclk.
 *
 * This function returns the frequency (in Hz) at which the Aon_cfg_pclk is currently operating.
 * its frequency is crucial for determining the performance and timing characteristics of
 * various system components. The frequency is determined by the current configuration of
 * the system's clock sources and any relevant clock dividers.
 *
 * @return uint32_t The operating frequency of Aon_cfg_pclk in Hertz. If the frequency cannot be
 *                  determined, or if Aon_cfg_pclk is not properly configured, the function may
 *                  return 0.
 *
 * @note The returned frequency value is dependent on the current state of the system's
 *       clock configuration. Changes in clock source or divider settings can affect the
 *       Aon_cfg_pclk frequency.
 *
 * @warning This function can call anywhere which get the Aon_cfg_pclk clock frequency.
 */
uint32_t CRM_GetAon_cfg_pclkFreq();
/**
  * @}
  */

/** @defgroup _CRM_HCLK HCLK_CLK_FUNC
  * @brief HCLK clock control function which can enable, disable or get status from corresponding
  * source clock, set core clock source, set core clock divider, or get core clock configuration, help you calculate the core divider
  * parameter when having core reference clock and desire clock
  * @{
  */

/**
 * @brief Sets the clock source for the Hclk.
 *
 * This function configures the system's Hclk to use a specific
 * clock source as defined by the 'src' parameter. The 'src' parameter should be one of
 * the values defined in the 'clock_src_name_t' enumeration, representing the various
 * clock sources available in the system. Changing the Hclk source can be crucial for
 * system performance tuning, power management, or adapting to different operational
 * modes.
 *
 * @param src The desired clock source for Hclk. It should be a value from the
 *            'clock_src_name_t' enumeration that specifies which clock source to use, can choose this source -> [CRM_IpSrcOscClk, CRM_IpSrcXtalClk, CRM_IpSrcCoreClk, ]
 *
 * @return uint32_t Returns 0 on success, or a non-zero error code on failure. The error
 *                  code can indicate issues such as an invalid clock source or failure
 *                  in applying the new clock source setting.
 *
 * @note The function's ability to change the clock source may depend on the current state
 *       of Hclk and the system's overall clock configuration. It is advisable to ensure
 *       that the system is in a suitable state to change the clock source without
 *       disrupting operational stability.
 *
 * @warning Using an incorrect or unsupported clock source for Hclk can lead to system
 *          instability or malfunction. Ensure that the selected clock source is compatible
 *          with the system's requirements and that any dependent subsystems are
 *          appropriately configured to handle the change in clock source.
 */
uint32_t HAL_CRM_SetHclkClkSrc(clock_src_name_t src);

/**
 * @brief Sets the clock divider for the Hclk.
 *
 * This function configures the clock division for the Hclk
 * using the specified divider ratios. The division is determined by 'div_n'
 * and 'div_m'. These parameters define how the input clock frequency is divided to obtain
 * the desired Hclk clock frequency.
 *
 * @param div_n The numerator part of the clock division ratio. It specifies the upper part
 *              of the division ratio, controlling how much the input clock frequency is
 *              divided, the div_n select range from [1 - 15].
 *
 *
 * @param div_m The denominator part of the clock division ratio. It specifies the lower part
 *              of the division ratio, contributing to the final clock frequency calculation,
 *              The div_m select range from [1 - 15]
 *
 * @return void This function does not return a value. Any errors or status conditions may be
 *              handled internally or reported through other mechanisms, as per the hardware
 *              design and system requirements.
 *
 * @note The behavior and constraints of the division ratio are dependent on the hardware
 *       capabilities and current clock configuration. It is important to ensure that the values
 *       for 'div_n' and 'div_m' are within the acceptable range for the system's hardware.
 *
 * @warning Incorrect configuration of the clock divider can affect the operation of the Hclk
 *          and connected bus. Ensure that 'div_n' and 'div_m' are set
 *          to values that are appropriate for the desired clock frequency and compatible with
 *          the peripheral's operational requirements.
 */
int32_t HAL_CRM_SetHclkClkDiv(uint32_t div_n, uint32_t div_m);

/**
 * @brief Retrieves the clock configuration for HCLK.
 *
 * This inline static function obtains the current clock source and division factors for the
 * HCLK module. The clock source and divider values are returned
 * through the pointer parameters 'src', 'div_n', and'div_m'.
 *
 * @param[out] src Pointer to a 'clock_src_name_t' variable where the clock source will be stored.
 *                 The clock source is indicated as an enumeration value of type 'clock_src_name_t'.
 *
 *
 * @param[out] div_n Pointer to a 'uint32_t' variable where the numerator of the clock division
 *                   ratio will be stored.
 *
 *
 * @param[out] div_m Pointer to a 'uint32_t' variable where the denominator of the clock division
 *                   ratio will be stored.
 *
 * @note Being an inline function, 'HAL_CRM_GetHCLKClkConfig()' is expanded at each point of call,
 *       which can increase code size but typically reduces execution time by avoiding a function
 *       call overhead. Use this function judiciously where performance is critical.
 *
 * @warning Ensure that the pointers passed to this function are valid and point to appropriate
 *          memory locations. Passing invalid pointers may lead to undefined behavior, including
 *          crashes. Also, consider the potential for race conditions if the clock configuration
 *          can be changed by other parts of the program while this function is being executed.
 */
inline static void HAL_CRM_GetHclkClkConfig(clock_src_name_t* src, uint32_t* div_n, uint32_t* div_m){
    
    uint32_t src_t;
    src_t = IP_SYSNODEF->REG_BUS_CLK_CFG1.bit.SEL_SYS_CLK;
    
    if (src_t == 0){
        *src = CRM_IpSrcOscClk;
    }
    
    if (src_t == 1){
        *src = CRM_IpSrcXtalClk;
    }
    
    if (src_t == 2){
        *src = CRM_IpSrcCoreClk;
    }
    
    
    
    *div_n = IP_SYSNODEF->REG_BUS_CLK_CFG1.bit.DIV_AP_HCLK_N;
    
    
    *div_m = IP_SYSNODEF->REG_BUS_CLK_CFG1.bit.DIV_AP_HCLK_M;
    
}

/**
 * @brief Retrieves the current operating frequency of the Hclk.
 *
 * This function returns the frequency (in Hz) at which the Hclk is currently operating.
 * its frequency is crucial for determining the performance and timing characteristics of
 * various system components. The frequency is determined by the current configuration of
 * the system's clock sources and any relevant clock dividers.
 *
 * @return uint32_t The operating frequency of Hclk in Hertz. If the frequency cannot be
 *                  determined, or if Hclk is not properly configured, the function may
 *                  return 0.
 *
 * @note The returned frequency value is dependent on the current state of the system's
 *       clock configuration. Changes in clock source or divider settings can affect the
 *       Hclk frequency.
 *
 * @warning This function can call anywhere which get the Hclk clock frequency.
 */
uint32_t CRM_GetHclkFreq();
/**
  * @}
  */

/** @defgroup _CRM_CPU CPU_CLK_FUNC
  * @brief CPU clock control function which can enable, disable or get status from corresponding
  * source clock, set core clock source, set core clock divider, or get core clock configuration, help you calculate the core divider
  * parameter when having core reference clock and desire clock
  * @{
  */

/**
 * @brief Retrieves the current operating frequency of the Cpu.
 *
 * This function returns the frequency (in Hz) at which the Cpu is currently operating.
 * its frequency is crucial for determining the performance and timing characteristics of
 * various system components. The frequency is determined by the current configuration of
 * the system's clock sources and any relevant clock dividers.
 *
 * @return uint32_t The operating frequency of Cpu in Hertz. If the frequency cannot be
 *                  determined, or if Cpu is not properly configured, the function may
 *                  return 0.
 *
 * @note The returned frequency value is dependent on the current state of the system's
 *       clock configuration. Changes in clock source or divider settings can affect the
 *       Cpu frequency.
 *
 * @warning This function can call anywhere which get the Cpu clock frequency.
 */
uint32_t CRM_GetCpuFreq();
/**
  * @}
  */

int32_t SYSPLL_Init();

int32_t CRM_InitCoreSrc(clock_src_core_div_t div);

int32_t CRM_InitFlashSrc(clock_src_flash_div_t div);

/**
 * @brief Retrieves the frequency of a specified clock source.
 *
 * This function returns the current frequency (in Hz) of a specified clock source in the system.
 * The clock source is determined by the 'src' parameter, which should be one of the values
 * defined in the 'clock_src_name_t' enumeration.
 *
 * @param src The clock source for which the frequency is requested. This parameter should be
 *            a value from the 'clock_src_name_t' enumeration, indicating the specific clock source.
 *
 * @return uint32_t The frequency of the specified clock source in Hertz. If the specified source
 *                  is invalid or the frequency cannot be determined, the function may return 0
 *                  or an error code (if defined).
 *
 * @note The accuracy and availability of the returned frequency may depend on the system's
 *       current state and the specific clock source queried.
 *
 * @warning Ensure that the clock source specified in 'src' is initialized and active before
 *          calling this function. Querying an inactive or uninitialized clock source might
 *          lead to undefined behavior or incorrect frequency values.
 */
uint32_t CRM_GetSrcFreq(clock_src_name_t src);

#endif /* INCLUDE_DRIVER_CLOCKMANAGER_H_ */