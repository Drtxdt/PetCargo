/*
 * AON_IOMuxManager.h
 *
 *  Created on: 2024.1.1
 *      Author: USER
 */

#ifndef INCLUDE_DRIVER_AON_IOMUXMANAGER_H_
#define INCLUDE_DRIVER_AON_IOMUXMANAGER_H_

#include <stdint.h>

#include "Driver_Common.h"

#define CSK_IOMUX_PAD_A                        0
// PIN NUM: 0-12
#define CSK_IOMUX_PAD_B                        1
// PIN NUM: 0-12

#define CSK_IOMUX_PAD_A_MAX_PIN                12
#define CSK_IOMUX_PAD_B_MAX_PIN                12

// IO mux function selector
//**************************** NORMAL IOMUX
#define CSK_IOMUX_FUNC_DEFAULT                 (0U)
#define CSK_IOMUX_FUNC_ALTER1                  (1U)
#define CSK_IOMUX_FUNC_ALTER2                  (2U)
#define CSK_IOMUX_FUNC_ALTER3                  (3U)
#define CSK_IOMUX_FUNC_ALTER4                  (4U)
#define CSK_IOMUX_FUNC_ALTER5                  (5U)
#define CSK_IOMUX_FUNC_ALTER6                  (6U)
#define CSK_IOMUX_FUNC_ALTER7                  (7U)
#define CSK_IOMUX_FUNC_ALTER8                  (8U)
#define CSK_IOMUX_FUNC_ALTER9                  (9U)
#define CSK_IOMUX_FUNC_ALTER10                 (10U)
#define CSK_IOMUX_FUNC_ALTER11                 (11U)
#define CSK_IOMUX_FUNC_ALTER12                 (12U)
#define CSK_IOMUX_FUNC_ALTER13                 (13U)
#define CSK_IOMUX_FUNC_ALTER14                 (14U)
#define CSK_IOMUX_FUNC_ALTER15                 (15U)

//**************************** AON IOMUX
#define CSK_AON_IOMUX_FUNC_DEFAULT             (0U)
#define CSK_AON_IOMUX_FUNC_ALTER1              (1U)
#define CSK_AON_IOMUX_FUNC_ALTER2              (2U)
#define CSK_AON_IOMUX_FUNC_ALTER3              (3U)
#define CSK_AON_IOMUX_FUNC_ALTER4              (4U)
#define CSK_AON_IOMUX_FUNC_ALTER5              (5U)

//**************************** ANA IOMUX
#define CSK_ANA_IOMUX_FUNC_DEFAULT             	(0U)
#define CSK_ANA_IOMUX_FUNC_ALTER1              	(1U)
#define CSK_ANA_IOMUX_FUNC_ALTER2              	(2U)
#define CSK_ANA_IOMUX_FUNC_ALTER3              	(3U)
#define CSK_ANA_IOMUX_FUNC_ALTER4              	(4U)
#define CSK_ANA_IOMUX_FUNC_ALTER5              	(5U)
#define CSK_ANA_IOMUX_FUNC_ALTER6              	(6U)
#define CSK_ANA_IOMUX_FUNC_ALTER7              	(7U)
#define CSK_ANA_IOMUX_FUNC_ALTER8             	(8U)
#define CSK_ANA_IOMUX_FUNC_ALTER9              	(9U)
#define CSK_ANA_IOMUX_FUNC_ALTER10              (10U)
#define CSK_ANA_IOMUX_FUNC_ALTER11              (11U)
#define CSK_ANA_IOMUX_FUNC_ALTER12              (12U)
#define CSK_ANA_IOMUX_FUNC_ALTER13              (13U)
#define CSK_ANA_IOMUX_FUNC_ALTER14              (14U)
#define CSK_ANA_IOMUX_FUNC_ALTER15              (15U)


//**************************** IOMUX PULL MODE
#define HAL_IOMUX_NONE_MODE                    (0U)
#define HAL_IOMUX_PULLUP_MODE                  (1U)
#define HAL_IOMUX_PULLDOWN_MODE                (2U)

//**************************** IOMUX FORCE DATA
#define HAL_IOMUX_FORCE_OUT_LOW                (0U)
#define HAL_IOMUX_FORCE_OUT_HIGH               (1U)
#define HAL_IOMUX_FORCE_OFF                    (2U)

/**
 * @brief Configure a specific pin on an I/O multiplexer.
 *
 * This function configures a pin of an I/O multiplexer (IOMux) based on the provided
 * configuration parameters. The IOMux allows different peripheral functions to be
 * assigned to physical pins. It supports configuration for pads CSK_IOMUX_PAD_A and
 * CSK_IOMUX_PAD_B.
 *
 * @param pad The pad number on the IOMux. It should be either CSK_IOMUX_PAD_A or
 *        CSK_IOMUX_PAD_B, representing the specific group or controller within the
 *        IOMux that the pin belongs to.
 * @param pin_num The pin number (ID) within the specified pad to be configured.
 *        The valid range for pin_num is 0 to 12.
 * @param pin_cfg The configuration settings for the pin, based on predefined macros
 *        in the header file. For example, CSK_IOMUX_FUNC_DEFAULT. This setting
 *        determines the pin's function select.
 *
 * @return int32_t Returns CSK_DRIVER_OK (0) to indicate success or CSK_DRIVER_ERROR (non-zero)
 *         to indicate failure. Refer to Driver_Common.h for more details on these return values.
 */
int32_t IOMuxManager_PinConfigure(uint8_t pad, uint8_t pin_num, uint32_t pin_cfg);
/**
 * @brief Configure a specific pin on an Analog I/O multiplexer.
 *
 * This function configures an analog pin of an I/O multiplexer (IOMux) in either the
 * CSK_IOMUX_PAD_A or CSK_IOMUX_PAD_B group, specifically for analog functionalities.
 * The pin configuration is dependent on the pad selected. For CSK_IOMUX_PAD_A, only
 * pins 11 and 12 can be configured. For CSK_IOMUX_PAD_B, pins 0 to 12 are available
 * for configuration. This function is ideal for setting up analog peripherals that
 * require specific pin assignments.
 *
 * @param pad The pad number on the Analog IOMux. Depending on the pad, different pins
 *        can be configured: CSK_IOMUX_PAD_A (only pins 11 and 12) or CSK_IOMUX_PAD_B
 *        (pins 0 to 12).
 * @param pin_num The pin number (ID) within the specified pad to be configured for
 *        analog functions. The valid range for pin_num is 11 to 12 for CSK_IOMUX_PAD_A
 *        and 0 to 12 for CSK_IOMUX_PAD_B.
 * @param pin_cfg The configuration settings for the pin, based on predefined macros in
 *        the header file, tailored for analog functions.
 *
 * @return int32_t Returns CSK_DRIVER_OK (0) to indicate success or CSK_DRIVER_ERROR
 *         (non-zero) to indicate failure. Refer to Driver_Common.h for more details
 *         on these return values.
 */
int32_t ANA_IOMuxManager_PinConfigure(uint8_t pad, uint8_t pin_num, uint32_t pin_cfg);

/**
 * @brief Configure a specific pin on an Always-On (AON) I/O multiplexer.
 *
 * This function configures a pin on the Always-On (AON) domain of an I/O multiplexer (IOMux),
 * which remains powered in low power or standby modes. It's suitable for functions requiring
 * continuous operation or quick wake-up from a low power state. This function is specifically
 * designed for the CSK_IOMUX_PAD_B group in the AON IOMux.
 *
 * @param pad The pad number on the AON IOMux. For the AON domain, this should always be
 *        CSK_IOMUX_PAD_B, as it's the only group available for AON IOMux configuration.
 * @param pin_num The pin number (ID) within the CSK_IOMUX_PAD_B pad to be configured.
 *        The valid range for pin_num is 0 to 12.
 * @param pin_cfg The configuration setting for the pin, which should be CSK_AON_IOMUX_FUNC_DEFAULT
 *        as per the available macros for AON IOMux configuration. For more details on possible
 *        values, refer to the Driver_Common.h file.
 *
 * @return int32_t Returns CSK_DRIVER_OK (0) to indicate success or CSK_DRIVER_ERROR (non-zero)
 *         to indicate failure. Refer to Driver_Common.h for more details on these return values.
 */
int32_t AON_IOMuxManager_PinConfigure (uint8_t pad, uint8_t pin_num, uint32_t pin_cfg);
/**
 * @brief Configure the mode of a specific pin on an I/O multiplexer.
 *
 * This function is used to set the mode of a pin on an I/O multiplexer (IOMux), which includes
 * the configuration of pull-up, pull-down resistors, or no pull resistors. It supports both
 * CSK_IOMUX_PAD_A and CSK_IOMUX_PAD_B pads with pin numbers ranging from 0 to 12. This function
 * allows for detailed control over the electrical characteristics of IOMux pins, which is
 * crucial for proper hardware interfacing and signal integrity.
 *
 * @param pad The pad number on the IOMux. It can be either CSK_IOMUX_PAD_A or CSK_IOMUX_PAD_B,
 *        representing the specific group or controller within the IOMux that the pin belongs to.
 * @param pin_num The pin number (ID) within the specified pad to be configured. The valid range
 *        for pin_num is 0 to 12.
 * @param pin_mode The mode to be set for the pin. The available modes are HAL_IOMUX_NONE_MODE
 *        (no pull resistor), HAL_IOMUX_PULLUP_MODE (pull-up resistor), and HAL_IOMUX_PULLDOWN_MODE
 *        (pull-down resistor).
 *
 * @return int32_t Returns CSK_DRIVER_OK (0) to indicate success or CSK_DRIVER_ERROR (non-zero)
 *         to indicate failure. Refer to Driver_Common.h for more details on these return values.
 */
int32_t IOMuxManager_ModeConfigure(uint8_t pad, uint8_t pin_num, uint8_t pin_mode);
/**
 * @brief Force the specified I/O pin to a high or low state in the Always-On (AON) domain.
 *
 * This function forcibly sets the state of a specified pin on an AON I/O multiplexer (IOMux)
 * within the CSK_IOMUX_PAD_B group. It is capable of maintaining the pin state in low power
 * or standby modes, making it suitable for applications requiring the pin to remain active or
 * in a specific state during such modes. Note that CSK_IOMUX_PAD_A is not suitable for low
 * power operations and thus not supported by this function.
 *
 * @param pad The pad number on the AON IOMux. This should be CSK_IOMUX_PAD_B, as it's the only
 *        group capable of operating in low power modes within the AON IOMux.
 * @param pin_num The pin number (ID) within the CSK_IOMUX_PAD_B pad. The valid range for pin_num
 *        is 0 to 12.
 * @param data The desired state of the pin, where HAL_IOMUX_FORCE_OUT_LOW represents a forced low
 *        state and HAL_IOMUX_FORCE_OUT_HIGH represents a forced high state.
 *
 * @return int32_t Returns CSK_DRIVER_OK (0) to indicate success or CSK_DRIVER_ERROR (non-zero)
 *         to indicate failure. Refer to Driver_Common.h for more details on these return values.
 */
int32_t AON_IOMuxManager_PinForce(uint8_t pad, uint8_t pin_num, uint8_t data);


#endif /* INCLUDE_DRIVER_AON_IOMUXMANAGER_H_ */
