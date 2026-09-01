/**
  ******************************************************************************
  * @file    Driver_KEYSENSE.h
  * @author  ListenAI Application Team
  * @brief   Header file of KEYSENSE HAL module.
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
#ifndef __CSK_DRIVER_KEYSENSE_H
#define __CSK_DRIVER_KEYSENSE_H


#ifdef __cplusplus
 extern "C" {
#endif


/* Includes ------------------------------------------------------------------*/
#include "Driver_Common.h"

/** @addtogroup CSK_HAL_Driver
  * @{
  */

/** @addtogroup KEYSENSE
  * @{
  */


/* Exported types ------------------------------------------------------------*/
/** @defgroup KEYSENSE_Exported_Types KEYSENSE Exported Types
  * @{
  */

/**
  * @brief  add description
  */
typedef void (*CSK_KEYSENSE_SignalEvent_t) (void* workspace);

 /**
   * @}
   */


/* Exported constants --------------------------------------------------------*/

/** @defgroup KEYSENSE_Exported_Constants KEYSENSE Exported Constants
  * @{
  */

#define CSK_KEYSENSE_API_VERSION CSK_DRIVER_VERSION_MAJOR_MINOR(1, 0)
#define CSK_KEYSENSE_DRV_VERSION CSK_DRIVER_VERSION_MAJOR_MINOR(1,1)

#ifndef NULL
#define NULL                        (void *)0
#endif


/*---------------------Control mode for application---------------------------------*/
/*----- KEYSENSE ADC Trigger counter threshold : 16bit, default 0x40-----*/
#define CSK_KEYSENSE_ADC_TRIGGER_THD_Pos             0
#define CSK_KEYSENSE_ADC_TRIGGER_THD_Msk             (0xFFUL << CSK_KEYSENSE_ADC_TRIGGER_THD_Pos)

/*----- KEYSENSE wakeup counter threshold : 16bit, default 0x20 -----*/
#define CSK_KEYSENSE_WAKEUP_CNT_THD_Pos              16
#define CSK_KEYSENSE_WAKEUP_CNT_THD_Msk            	 (0xFFUL << CSK_KEYSENSE_WAKEUP_CNT_THD_Pos)


/*----- KEYSENSE interrupt mode : press/release/adc_trig/wakeup -----*/
/*----- KEYSENSE interrupt mode define -----*/
typedef enum {
	CSK_KEYSENSE_INTERRUPT_MODE_WAKEUP                 	= 0x01,
	CSK_KEYSENSE_INTERRUPT_MODE_ADCTRIGGER              = 0x02,
	CSK_KEYSENSE_INTERRUPT_MODE_RELEASE                 = 0x04,
	CSK_KEYSENSE_INTERRUPT_MODE_PRESS                  	= 0x08,
} KEYSENSE_INTERRUPT_MODE_TYPE;


/*----- KEYSENSE mode define -----*/
typedef enum {
	CSK_KEYSENSE_WAKEUP                 	= 0,
	CSK_KEYSENSE_ADCTRIG                  	= 1,
	CSK_KEYSENSE_RELEASE                  	= 2,
	CSK_KEYSENSE_PRESS                  	= 3,
} KEYSENSE_MODE_TYPE;

#define CSK_KEYSENSE_MODE_NUM					  0x04

#define CSK_KEYSENSE_THD                          0x00200040

/**
  * @}
  */

/* Exported macro ------------------------------------------------------------*/
/** @defgroup KEYSENSE_Exported_Macro KEYSENSE Exported Macro
  * @{
  */

/**
  * @}
  */

/* Exported functions --------------------------------------------------------*/
/** @addtogroup KEYSENSE_Exported_Functions KEYSENSE Exported Functions
  * @{
  */

/** @addtogroup KEYSENSE_Exported_Functions_Group1 Initialization and de-initialization functions
  * @{
  */
void* KEYSENSE0(void);

/**
  * @brief Initialize the keysense hardware.
  *
  * This function initializes the keysense hardware with the provided resources.
  * It sets up the internal registers, enables the keysense IRQ and configures
  * the necessary hardware parameters.
  *
  * @param res Pointer to keysense resources.
  *
  * @note Ensure that the resources provided are valid and compatible with the
  *       keysense hardware.
  *
  * @retval int32_t Returns CSK_DRIVER_OK if successful, error code otherwise.
  */
int32_t HAL_KEYSENSE_Initialize(void *res);

/**
  * @brief Uninitialize the keysense hardware.
  *
  * This function uninitializes the keysense hardware, disables the IRQ and
  * clears any hardware configurations.
  *
  * @param res Pointer to keysense resources.
  *
  * @note This function should be called to safely turn off or reset the
  *       keysense hardware.
  *
  * @retval int32_t Returns CSK_DRIVER_OK if successful, error code otherwise.
  */
int32_t HAL_KEYSENSE_Uninitialize(void *res);

/**
  * @brief Control keysense hardware settings.
  *
  * This function configures the keysense hardware based on the specified control parameters.
  * It sets the ADC trigger threshold and the wakeup count threshold.
  *
  * @param res Pointer to keysense resources.
  * @param control Control flags combined using bitwise OR. These flags determine the
  *        settings to be applied to the keysense hardware.
  *        - CSK_KEYSENSE_ADC_TRIGGER_THD_Msk: Mask for ADC trigger threshold.
  *        - CSK_KEYSENSE_ADC_TRIGGER_THD_Pos: Position for ADC trigger threshold.
  *        - CSK_KEYSENSE_WAKEUP_CNT_THD_Msk: Mask for wakeup count threshold.
  *        - CSK_KEYSENSE_WAKEUP_CNT_THD_Pos: Position for wakeup count threshold.
  *
  * @note The keysense hardware must be powered on (KEYSENSE_FLAG_POWERED flag set) before
  *       calling this function.
  *
  * @retval int32_t Returns CSK_DRIVER_OK if successful, error code otherwise.
  */
int32_t HAL_KEYSENSE_Control(void* res, uint32_t control);
/**
  * @}
  */

/** @addtogroup KEYSENSE_Exported_Functions_Group2 Peripheral Control functions
  * @{
  */

/**
  * @brief Enable specific interrupts for keysense.
  *
  * This function enables the interrupts for keysense based on the specified interrupt mode.
  *
  * @param res Pointer to keysense resources.
  * @param intmode Interrupt mode type defined in KEYSENSE_INTERRUPT_MODE_TYPE.
  *
  * @note Ensure that the keysense resources are valid before calling this function.
  *
  * @retval int32_t Returns CSK_DRIVER_OK if successful.
  */
int32_t HAL_KEYSENSE_InterruptEnable(void* res, KEYSENSE_INTERRUPT_MODE_TYPE intmode);

/**
  * @brief Disable specific interrupts for keysense.
  *
  * This function disables the interrupts for keysense based on the specified interrupt mode.
  *
  * @param res Pointer to keysense resources.
  * @param intmode Interrupt mode type defined in KEYSENSE_INTERRUPT_MODE_TYPE.
  *
  * @note Ensure that the keysense resources are valid before calling this function.
  *
  * @retval int32_t Returns CSK_DRIVER_OK if successful.
  */
int32_t HAL_KEYSENSE_InterruptDisable(void* res, KEYSENSE_INTERRUPT_MODE_TYPE intmode);

/**
  * @brief Enable the keysense hardware.
  *
  * This function enables the keysense hardware by setting the corresponding configuration bit.
  *
  * @param res Pointer to keysense resources.
  *
  * @note Ensure that the keysense resources are valid before calling this function.
  *
  * @retval int32_t Returns CSK_DRIVER_OK if successful.
  */
int32_t HAL_KEYSENSE_Enable(void* res);

/**
  * @brief Disable the keysense hardware.
  *
  * This function disables the keysense hardware by resetting the corresponding configuration bit.
  *
  * @param res Pointer to keysense resources.
  *
  * @note Ensure that the keysense resources are valid before calling this function.
  *
  * @retval int32_t Returns CSK_DRIVER_OK if successful.
  */
int32_t HAL_KEYSENSE_Disable(void* res);

/**
  * @brief Register a callback function for keysense events.
  *
  * This function registers a user-defined callback function that will be called
  * when keysense events occur. The callback is associated with a specific keysense mode.
  *
  * @param res Pointer to keysense resources.
  * @param mode The keysense mode for which the callback is registered, defined in KEYSENSE_MODE_TYPE.
  * @param cb_event The callback function of type CSK_KEYSENSE_SignalEvent_t to be called on keysense events.
  *
  * @note Ensure that the keysense resources are valid and that the mode is supported by the hardware.
  *
  * @retval int32_t Returns CSK_DRIVER_OK if successful, error code otherwise.
  */
int32_t HAL_KEYSENSE_RegisterCallback(void *res, KEYSENSE_MODE_TYPE mode, CSK_KEYSENSE_SignalEvent_t cb_event);
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


#endif /* __CSK_DRIVER_KEYSENSE_H */
