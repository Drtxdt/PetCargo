/**
  ******************************************************************************
  * @file    Driver_GPADC.h
  * @author  ListenAI Application Team
  * @brief   Header file of GPADC HAL module.
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
#ifndef __CSK_DRIVER_GPADC_H
#define __CSK_DRIVER_GPADC_H


#ifdef __cplusplus
 extern "C" {
#endif

 /* Includes ------------------------------------------------------------------*/
#include "Driver_Common.h"


/** @addtogroup CSK_HAL_Driver
  * @{
  */

/** @addtogroup GPADC
  * @{
  */


/* Exported types ------------------------------------------------------------*/
/** @defgroup GPADC_Exported_Types TRNG Exported Types
  * @{
  */

/**
  * @brief  add description
  */
typedef void (*CSK_GPADC_SignalEvent_t) (uint32_t event, void* workspace);

/*----- GPADC channel define -----*/
typedef enum {
    CSK_GPADC_VBAT                      = 0,                /**< GPADC channel bat */
    CSK_GPADC_KEYSENSE0                 = 1,                /**< GPADC channel keysense0 */
    CSK_GPADC_KEYSENSE1                 = 2,                /**< GPADC channel keysense1 */
    CSK_GPADC_TEMPSENSOR                = 3,                /**< GPADC channel tempsensor */
    CSK_GPADC_CHANNEL0                  = 4,                /**< GPADC channel 0 */
    CSK_GPADC_CHANNEL1                  = 5,                /**< GPADC channel 1 */
    CSK_GPADC_CHANNEL2                  = 6,                /**< GPADC channel 2 */
    CSK_GPADC_CHANNEL3                  = 7,                /**< GPADC channel 3 */
} GPADC_CHANNEL_TYPE;


typedef enum {
	CSK_GPADC_FIFO_EMPTY                  = 0,
	CSK_GPADC_FIFO_FULL                  = 1,
	CSK_GPADC_FIFO_THD                  = 2,
}GPADC_FIFOINT_TYPE;

/**
  * @}
  */


/* Exported constants --------------------------------------------------------*/
/** @defgroup GPADC_Exported_Constants GPADC Exported Constants
  * @{
  */
#define CSK_GPADC_API_VERSION CSK_DRIVER_VERSION_MAJOR_MINOR(1, 0)
#define CSK_GPADC_DRV_VERSION CSK_DRIVER_VERSION_MAJOR_MINOR(1,1)

#define CSK_GPADC_CHANNEL_NUM					  0x08

/*---------------------Control mode for application---------------------------------*/
/*----- GPADC Trigger mode : hardware or software or both -----*/
#define CSK_GPADC_TRIGGER_CONTROL_Pos             0
#define CSK_GPADC_TRIGGER_CONTROL_Msk             (0x07UL << CSK_GPADC_TRIGGER_CONTROL_Pos)
#define CSK_GPADC_TRIGGER_KEYSENSE0_Enable        (0x01UL << CSK_GPADC_TRIGGER_CONTROL_Pos)
#define CSK_GPADC_TRIGGER_KEYSENSE0_Enable_Pos	  0x1
#define CSK_GPADC_TRIGGER_GPT_Enable       		  (0x02UL << CSK_GPADC_TRIGGER_CONTROL_Pos)
#define CSK_GPADC_TRIGGER_GPT_Enable_Pos	  	  0x2

/*----- GPADC Sample time : 4/8/16/32 adc clocks -----*/
#define CSK_GPADC_SAMPLETIME_Pos                  3
#define CSK_GPADC_SAMPLETIME_Msk                  (0x7UL << CSK_GPADC_SAMPLETIME_Pos)
#define CSK_GPADC_SAMPLETIME_4                    (0x02UL << CSK_GPADC_SAMPLETIME_Pos)
#define CSK_GPADC_SAMPLETIME_8                    (0x03UL << CSK_GPADC_SAMPLETIME_Pos)
#define CSK_GPADC_SAMPLETIME_16                   (0x04UL << CSK_GPADC_SAMPLETIME_Pos)
#define CSK_GPADC_SAMPLETIME_32                   (0x05UL << CSK_GPADC_SAMPLETIME_Pos)
#define CSK_GPADC_SAMPLETIME_64                   (0x06UL << CSK_GPADC_SAMPLETIME_Pos)
#define CSK_GPADC_SAMPLETIME_128                  (0x07UL << CSK_GPADC_SAMPLETIME_Pos)

/*----- GPADC Clock frequency : 24M/12M/6M/3M -----*/
#define CSK_GPADC_CLKFREQ_Pos             	  	  6
#define CSK_GPADC_CLKFREQ_Msk             	  	  (0x3UL << CSK_GPADC_CLKFREQ_Pos)
#define CSK_GPADC_CLKFREQ_24M					  (0x00UL << CSK_GPADC_CLKFREQ_Pos)
#define CSK_GPADC_CLKFREQ_12M					  (0x01UL << CSK_GPADC_CLKFREQ_Pos)
#define CSK_GPADC_CLKFREQ_6M					  (0x02UL << CSK_GPADC_CLKFREQ_Pos)
#define CSK_GPADC_CLKFREQ_3M					  (0x03UL << CSK_GPADC_CLKFREQ_Pos)

/*----- GPADC Channel select : 0 / 1 / 2 / 3 / (1/5VCC) / (1/3VDDIO) / Keysense / Temp-sensor -----*/
#define CSK_GPADC_CHANNEL_SEL_Pos             	  8
#define CSK_GPADC_CHANNEL_SEL_Msk             	  (0x3FFFUL << CSK_GPADC_CHANNEL_SEL_Pos)
#define CSK_GPADC_CHANNEL_SEL_1_5_VCC             (0x01UL << CSK_GPADC_CHANNEL_SEL_Pos)
#define CSK_GPADC_CHANNEL_SEL_1_3_VDDIO           (0x02UL << CSK_GPADC_CHANNEL_SEL_Pos)
#define CSK_GPADC_CHANNEL_SEL_1_3_KEYSENSE        (0x04UL << CSK_GPADC_CHANNEL_SEL_Pos)
#define CSK_GPADC_CHANNEL_SEL_TEMP                (0x08UL << CSK_GPADC_CHANNEL_SEL_Pos)
#define CSK_GPADC_CHANNEL_SEL_0                   (0x10UL << CSK_GPADC_CHANNEL_SEL_Pos)
#define CSK_GPADC_CHANNEL_SEL_1                   (0x20UL << CSK_GPADC_CHANNEL_SEL_Pos)
#define CSK_GPADC_CHANNEL_SEL_2                   (0x40UL << CSK_GPADC_CHANNEL_SEL_Pos)
#define CSK_GPADC_CHANNEL_SEL_3                   (0x80UL << CSK_GPADC_CHANNEL_SEL_Pos)
#define CSK_GPADC_CHANNEL_SEL_ALL                 (0xFFUL << CSK_GPADC_CHANNEL_SEL_Pos)


/*----- GPADC input mode : single end / differential -----*/
#define CSK_GPADC_INPUT_MODE_Pos             	  22
#define CSK_GPADC_INPUT_MODE_Msk             	  (0x01UL << CSK_GPADC_INPUT_MODE_Pos)
#define CSK_GPADC_INPUT_MODE_Diff				  (0x00UL << CSK_GPADC_INPUT_MODE_Pos)
#define CSK_GPADC_INPUT_MODE_Single				  (0x01UL << CSK_GPADC_INPUT_MODE_Pos)

/*----- GPADC vref sel : 1.25 internal vref / 0.5VDDIO /external -----*/
#define CSK_GPADC_VREF_SEL_Pos             	  	  23
#define CSK_GPADC_VREF_SEL_Msk             	  	  (0x07UL << CSK_GPADC_VREF_SEL_Pos)
//for vega
#define CSK_GPADC_VREF_SEL_VBG1_2                 (0x04UL << CSK_GPADC_VREF_SEL_Pos)
#define CSK_GPADC_VREF_SEL_VDD_VA                 (0x05UL << CSK_GPADC_VREF_SEL_Pos)
#define CSK_GPADC_VREF_SEL_VDDIO                  (0x06UL << CSK_GPADC_VREF_SEL_Pos)
#define CSK_GPADC_VREF_SEL_EXT                    (0x07UL << CSK_GPADC_VREF_SEL_Pos)

/*----- GPADC data transfer mode : dma mode enable/disable -----*/
#define CSK_GPADC_DMA_ENABLE_Pos             	  26
#define CSK_GPADC_DMA_ENABLE_Msk             	  (0x01UL << CSK_GPADC_DMA_ENABLE_Pos)
#define CSK_GPADC_DMA_ENABLE             	  	  (0x01UL << CSK_GPADC_DMA_ENABLE_Pos)
#define CSK_GPADC_DMA_DISABLE             	  	  (0x00UL << CSK_GPADC_DMA_ENABLE_Pos)

////CVD control

#define CSK_GPADC_GURADRING_ENABLE					1
#define CSK_GPADC_GURADRING_DISABLE					0

#define CSK_GPADC_DOUBLE_SAPMLE_ENABLE				1
#define CSK_GPADC_DOUBLE_SAPMLE_DISABLE				0

#define CSK_GPADC_CHARGE_INV_ENABLE					1
#define CSK_GPADC_CHARGE_INV_DISABLE				0


/*----- CVD prechargeA timer -----*/
#define CSK_GPADC_PRECHARGEA_CONTROL_Pos             0
#define CSK_GPADC_PRECHARGEA_CONTROL_Msk             (0xFFUL << CSK_GPADC_PRECHARGEA_CONTROL_Pos)

/*----- CVD prechargeB timer -----*/
#define CSK_GPADC_PRECHARGEB_CONTROL_Pos             8
#define CSK_GPADC_PRECHARGEB_CONTROL_Msk             (0xFFUL << CSK_GPADC_PRECHARGEB_CONTROL_Pos)

/*----- CVD Acquicition timer -----*/
#define CSK_GPADC_ACQUICITION_CONTROL_Pos            16
#define CSK_GPADC_ACQUICITION_CONTROL_Msk            (0xFFUL << CSK_GPADC_ACQUICITION_CONTROL_Pos)

/*----- CVD Additional Capacitor -----*/
#define CSK_GPADC_ADDITIONAL_CAP_CONTROL_Pos         24
#define CSK_GPADC_ADDITIONAL_CAP_CONTROL_Msk         (0x07UL << CSK_GPADC_ADDITIONAL_CAP_CONTROL_Pos)




/****** GPADC Event *****/
#define CSK_GPADC_FIFO_EMPTY    					(0x01)
#define CSK_GPADC_FIFO_FULL  						(0x02)
#define CSK_GPADC_FIFO_THD  						(0x03)
#define CSK_GPADC_COMPLETE  						(0x04)
#define CSK_GPADC_EOC_ERROR  						(0x05)


/**
  * @}
  */




/* Exported macro ------------------------------------------------------------*/
/** @defgroup GPADC_Exported_Macro GPADC Exported Macro
  * @{
  */

/**
  * @}
  */


/* Exported functions --------------------------------------------------------*/
/** @addtogroup GPADC_Exported_Functions GPADC Exported Functions
  * @{
  */

/** @addtogroup GPADC_Exported_Functions_Group1 Initialization and de-initialization functions
  * @{
  */

/**
  * @brief Retrieve GPADC resource structure.
  *
  * This function returns a pointer to the GPADC resource structure, providing access
  * to the configuration and state of the GPADC module.
  *
  * @note This function can be used to obtain the GPADC module's resources necessary
  *       for further operations.
  *
  * @retval void* Pointer to the GPADC resource structure.
  */
void* GPADC(void);

/**
  * @brief Initialize the GPADC module.
  *
  * This function initializes the GPADC module, resets the hardware, sets up
  * initial configuration, and enables the GPADC interrupt.
  *
  * @param res Pointer to GPADC resources, representing the GPADC module to be initialized.
  *
  * @note This function must be called before performing any operations on the GPADC module.
  *       The resources should be properly configured and valid.
  *
  * @retval int32_t Returns CSK_DRIVER_OK if the initialization is successful.
  */
int32_t HAL_GPADC_Initialize(void *res);

/**
  * @brief Uninitialize the GPADC module.
  *
  * This function uninitializes the GPADC module, disables the GPADC, masks all interrupts,
  * and clears interrupt status registers. It also disables and unregisters the IRQ handler.
  *
  * @param res Pointer to GPADC resources, representing the GPADC module to be uninitialized.
  *
  * @note This function should be called to safely turn off the GPADC module.
  *
  * @retval int32_t Returns CSK_DRIVER_OK if the uninitialization is successful.
  */
int32_t HAL_GPADC_Uninitialize(void *res);

/**
  * @brief Control and configure the GPADC module.
  *
  * This function configures the GPADC module based on the provided control flags. It sets
  * the trigger mode, sample time, channel selection, DMA configuration, and voltage reference.
  *
  * @param res Pointer to GPADC resources.
  * @param control The configuration flags for controlling and configuring the GPADC module.
  *                - CSK_GPADC_TRIGGER_CONTROL_Msk: Mask for trigger control.
  *                - CSK_GPADC_SAMPLETIME_Msk: Mask for sample time.
  *                - CSK_GPADC_CHANNEL_SEL_Msk: Mask for channel selection.
  *                - CSK_GPADC_DMA_ENABLE_Msk: Mask for DMA enable.
  *                - CSK_GPADC_VREF_SEL_Msk: Mask for voltage reference selection.
  *
  * @note Ensure that the GPADC module is powered before calling this function.
  *
  * @retval int32_t Returns CSK_DRIVER_OK if successful, error code otherwise.
  */
int32_t HAL_GPADC_Control(void* res, uint32_t control);
/**
  * @}
  */

/** @addtogroup GPADC_Exported_Functions_Group2 control functions
  * @{
  */

/**
  * @brief Start the GPADC conversion.
  *
  * This function triggers the GPADC module to start the analog-to-digital conversion.
  *
  * @param res Pointer to GPADC resources.
  *
  * @note Ensure that the GPADC module is properly configured before starting the conversion.
  *
  * @retval uint32_t Returns CSK_DRIVER_OK if the conversion is started successfully.
  */
uint32_t HAL_GPADC_Start(void* res);

/**
  * @brief Stop the GPADC conversion.
  *
  * This function stops the ongoing GPADC conversion and clears any pending statuses.
  *
  * @param res Pointer to GPADC resources.
  *
  * @note This function should be used to safely terminate a GPADC conversion.
  *
  * @retval uint32_t Returns CSK_DRIVER_OK if the conversion is stopped successfully.
  */
uint32_t HAL_GPADC_Stop(void* res);

/**
  * @brief Poll for GPADC conversion completion.
  *
  * This function polls the GPADC module to wait for the completion of an analog-to-digital conversion.
  * It checks the conversion status and can optionally wait until the conversion is complete or timeout occurs.
  *
  * @param res Pointer to GPADC resources.
  * @param Timeout Timeout duration in milliseconds for waiting for the conversion to complete.
  *
  * @note This function should be used when GPADC conversions are done in polling mode.
  *
  * @retval uint32_t Returns CSK_DRIVER_OK if the conversion is completed successfully.
  */
uint32_t HAL_GPADC_PollForConversion(void* res, uint32_t Timeout);

/**
  * @brief Start GPADC conversion with interrupt.
  *
  * This function starts an analog-to-digital conversion in GPADC and configures the module to use
  * interrupts for notifying the completion of the conversion.
  *
  * @param res Pointer to GPADC resources.
  *
  * @note This function configures the GPADC to generate an interrupt upon completion of the conversion.
  *
  * @retval uint32_t Returns CSK_DRIVER_OK if the conversion is started successfully.
  */
uint32_t HAL_GPADC_Start_IT(void* res);

/**
  * @brief Stop GPADC conversion with interrupt.
  *
  * This function stops an ongoing GPADC conversion that was started with interrupt mode.
  * It masks the completion interrupt and clears any pending statuses.
  *
  * @param res Pointer to GPADC resources.
  *
  * @note This function should be called to safely terminate an ongoing GPADC conversion
  *       that was started with HAL_GPADC_Start_IT.
  *
  * @retval uint32_t Returns CSK_DRIVER_OK if the conversion is stopped successfully.
  */
uint32_t HAL_GPADC_Stop_IT(void* res);


/**
  * @brief Set the number of triggers for GPADC conversion.
  *
  * This function sets the number of triggers required for starting the GPADC conversion.
  * The trigger number determines how many times the trigger condition must be met before
  * a conversion is started.
  *
  * @param res Pointer to GPADC resources.
  * @param trignum The number of triggers to set.
  *
  * @note This function is used to configure the ADC trigger mechanism.
  *
  * @retval uint32_t Returns CSK_DRIVER_OK if the trigger number is set successfully.
  */
uint32_t HAL_GPADC_SetTriggerNum(void* res, uint32_t trignum);

/**
  * @brief Set GPADC sample wait time.
  *
  * This function sets the wait time between each sampling period in the GPADC module.
  *
  * @param res Pointer to GPADC resources.
  * @param waitTime The wait time to set between ADC samples.
  *
  * @note The wait time determines the interval between consecutive ADC samples.
  *
  * @retval uint32_t Returns CSK_DRIVER_OK if the sample wait time is set successfully.
  */
uint32_t HAL_GPADC_SetSampleWaitTime(void* res, uint32_t waitTime);

/**
  * @brief Set the setup wait time for GPADC.
  *
  * This function configures the setup wait time for the GPADC module.
  *
  * @param res Pointer to GPADC resources.
  * @param waitTime The wait time to set.
  *
  * @retval uint32_t Returns CSK_DRIVER_OK if the setup wait time is set successfully.
  */
uint32_t HAL_GPADC_SetSetupWaitTime(void* res, uint8_t waitTime);

/**
  * @brief Set the FIFO threshold for a specific GPADC channel.
  *
  * This function sets the FIFO threshold level for a specified channel in the GPADC module.
  * When the number of samples in the FIFO reaches this threshold, an interrupt is generated if enabled.
  *
  * @param res Pointer to GPADC resources.
  * @param channelnum The channel number to set the FIFO threshold for.
  * @param threshold The threshold level to set for the FIFO.
  *
  * @note This function is used to control when the GPADC module generates an interrupt based
  *       on the number of samples in the FIFO.
  *
  * @retval uint32_t Returns CSK_DRIVER_OK if successful, CSK_DRIVER_ERROR_PARAMETER if the channel number is invalid.
  */
uint32_t HAL_GPADC_SetFifoThd(void* res, uint8_t channelnum, uint8_t threshold);

/**
  * @brief Enable specific FIFO interrupt for the GPADC channel.
  *
  * This function enables the specified FIFO interrupt type for the given GPADC channel.
  *
  * @param res Pointer to GPADC resources.
  * @param channelnum The GPADC channel number.
  * @param type The type of FIFO interrupt to enable (empty, full, or threshold).
  *
  * @retval uint32_t Returns CSK_DRIVER_OK if successful, CSK_DRIVER_ERROR_PARAMETER if invalid parameters.
  */
uint32_t HAL_GPADC_EnableFifoInterrupt(void* res, uint8_t channelnum, GPADC_FIFOINT_TYPE type);

/**
  * @brief Disable specific FIFO interrupt for the GPADC channel.
  *
  * This function disables the specified FIFO interrupt type for the given GPADC channel.
  *
  * @param res Pointer to GPADC resources.
  * @param channelnum The GPADC channel number.
  * @param type The type of FIFO interrupt to disable (empty, full, or threshold).
  *
  * @retval uint32_t Returns CSK_DRIVER_OK if successful, CSK_DRIVER_ERROR_PARAMETER if invalid parameters.
  */
uint32_t HAL_GPADC_DisableFifoInterrupt(void* res, uint8_t channelnum, GPADC_FIFOINT_TYPE type);

/**
  * @brief Enable comparison interrupt for GPADC.
  *
  * This function enables the comparison interrupt feature of the GPADC module.
  *
  * @param res Pointer to GPADC resources.
  *
  * @retval uint32_t Returns CSK_DRIVER_OK if the comparison interrupt is enabled successfully.
  */
uint32_t HAL_GPADC_EnableCmpInterrupt(void* res);

/**
  * @brief Disable comparison interrupt for GPADC.
  *
  * This function disables the comparison interrupt feature of the GPADC module.
  *
  * @param res Pointer to GPADC resources.
  *
  * @retval uint32_t Returns CSK_DRIVER_OK if the comparison interrupt is disabled successfully.
  */
uint32_t HAL_GPADC_DisableCmpInterrupt(void* res);

/**
  * @brief Retrieve the current value from a GPADC channel.
  *
  * This function reads and returns the current value from the specified GPADC channel.
  *
  * @param res Pointer to GPADC resources.
  * @param channelnum The GPADC channel number to read from.
  *
  * @retval uint16_t The current value from the specified GPADC channel.
  */
uint16_t HAL_GPADC_GetValue(void* res, uint32_t channelnum);

/**
  * @brief Register a callback function for a specific GPADC channel.
  *
  * This function registers a user-defined callback function for a specified GPADC channel.
  *
  * @param res Pointer to GPADC resources.
  * @param channel The GPADC channel to register the callback for.
  * @param cb_event The callback function to be registered.
  *
  * @retval int32_t Returns CSK_DRIVER_OK if the callback is registered successfully.
  */
int32_t HAL_GPADC_RegisterChannelCallback(void *res, GPADC_CHANNEL_TYPE channel, CSK_GPADC_SignalEvent_t cb_event);

/**
  * @brief Register a callback function for GPADC conversion completion.
  *
  * This function registers a user-defined callback function that will be called upon
  * GPADC conversion completion.
  *
  * @param res Pointer to GPADC resources.
  * @param cb_event The callback function to be registered for conversion completion.
  *
  * @retval int32_t Returns CSK_DRIVER_OK if the callback is registered successfully.
  */
int32_t HAL_GPADC_RegisterCompleteCallback(void *res, CSK_GPADC_SignalEvent_t cb_event);

/**
  * @brief Configure the GPADC for CVD (Capacitive Voltage Divider) operation.
  *
  * This function configures various parameters required for CVD operation in the GPADC module.
  * It includes settings for guard ring, double sampling, and second charge inversion.
  *
  * @param res Pointer to GPADC resources.
  * @param gr_state State of the guard ring (enable/disable).
  * @param db_sample_state State of double sampling (enable/disable).
  * @param second_charge_invert State of second charge inversion (enable/disable).
  *
  * @retval uint32_t Returns CSK_DRIVER_OK if successful.
  */
uint32_t HAL_GPADC_CVD_Config(void* res, uint32_t gr_state, uint32_t db_sample_state, uint32_t second_charge_invert);

/**
  * @brief Control the GPADC for specific CVD settings.
  *
  * This function sets various timing parameters for the CVD operation in the GPADC module.
  * It includes settings for precharge A/B timers and acquisition time.
  *
  * @param res Pointer to GPADC resources.
  * @param control Bitmask of control parameters for CVD configuration.
  *
  * @note Ensure that the GPADC module is powered on before calling this function.
  *
  * @retval uint32_t Returns CSK_DRIVER_OK if successful, CSK_DRIVER_ERROR if not powered.
  */
uint32_t HAL_GPADC_CVD_Control(void* res, uint32_t control);

/**
  * @brief Retrieve the current value from a specified GPADC channel in CVD mode.
  *
  * This function reads and returns the current value from the specified channel
  * of the GPADC module when operating in CVD mode.
  *
  * @param res Pointer to GPADC resources.
  * @param channelnum The GPADC channel number to read from in CVD mode.
  *
  * @retval uint32_t The current value from the specified GPADC channel in CVD mode.
  */
uint32_t HAL_GPADC_CVD_GetValue(void* res, uint32_t channelnum);


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

#endif /* __DRIVER_GPADC_H */
