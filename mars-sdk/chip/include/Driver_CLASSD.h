/*
 * Driver_CLASSD.h
 *
 *  Created on: Mar. 10, 2024 for MARS (Modified from ARCS_C0 DAC driver)
 *
 */

#ifndef __DRIVER_CLASSD_H
#define __DRIVER_CLASSD_H


#include "Driver_Common_Audio.h"

#define CSK_CLASSD_API_VERSION CSK_DRIVER_VERSION_MAJOR_MINOR(1,0)  /* API version */

/****** CLASSD Control Codes *****/

/*----- CLASSD Control Codes: Configuration Parameters: Sample Rate -----*/
#define CSK_CLASSD_SR_Pos                           0
#define CSK_CLASSD_SR_Msk                           (0x7UL << CSK_CLASSD_SR_Pos) // bit[2:0], 3 bits
#define CSK_CLASSD_SR_UNSET                         (0x0UL << CSK_CLASSD_SR_Pos) // Sample Rate is kept unchanged or default
#define CSK_CLASSD_SR_8KHZ                          (0x1UL << CSK_CLASSD_SR_Pos) // Sample Rate = 8KHz
#define CSK_CLASSD_SR_16KHZ                         (0x2UL << CSK_CLASSD_SR_Pos) // Sample Rate = 16KHz
#define CSK_CLASSD_SR_32KHZ                         (0x3UL << CSK_CLASSD_SR_Pos) // Sample Rate = 32KHz
#define CSK_CLASSD_SR_48KHZ                         (0x4UL << CSK_CLASSD_SR_Pos) // Sample Rate = 48KHz

/*----- CLASSD Control Codes: Configuration Parameters: Over Sample Ratio -----*/
#define CSK_CLASSD_OSR_Pos                          3
#define CSK_CLASSD_OSR_Msk                          (0x7UL << CSK_CLASSD_OSR_Pos) // bit[5:3], 3 bits
#define CSK_CLASSD_OSR_UNSET                        (0x0UL << CSK_CLASSD_OSR_Pos) // Over Sample Ratio is kept unchanged or default
#define CSK_CLASSD_OSR_60                           (0x1UL << CSK_CLASSD_OSR_Pos) // Over Sample Ratio = 60
#define CSK_CLASSD_OSR_30                           (0x2UL << CSK_CLASSD_OSR_Pos) // Over Sample Ratio = 30
#define CSK_CLASSD_OSR_15                           (0x3UL << CSK_CLASSD_OSR_Pos) // Over Sample Ratio = 15
#define CSK_CLASSD_OSR_10                           (0x4UL << CSK_CLASSD_OSR_Pos) // Over Sample Ratio = 10

/*----- CLASSD Control Codes: Configuration Parameters: Master Clock Source -----*/
#define CSK_CLASSD_MCLK_SRC_Pos                     6
#define CSK_CLASSD_MCLK_SRC_Msk                     (0x3UL << CSK_CLASSD_MCLK_SRC_Pos) // bit[7:6], 2 bits
#define CSK_CLASSD_MCLK_SRC_UNSET                   (0x0UL << CSK_CLASSD_MCLK_SRC_Pos) // MCLK source is kept unchanged or default
#define CSK_CLASSD_MCLK_SRC_RC24M                   (0x1UL << CSK_CLASSD_MCLK_SRC_Pos) // from RC24MHz
#define CSK_CLASSD_MCLK_SRC_PLL24M                  (0x2UL << CSK_CLASSD_MCLK_SRC_Pos) // from PLL24MHz
#define CSK_CLASSD_MCLK_SRC_XO24M                   (0x3UL << CSK_CLASSD_MCLK_SRC_Pos) // from XO24MHz

/*----- CLASSD Control Codes: Configuration Parameters: Merge dual 16bits -----*/
#define CSK_CLASSD_MERGE_16BITS_Pos                 8
#define CSK_CLASSD_MERGE_16BITS_Msk                 (0x3UL << CSK_CLASSD_MERGE_16BITS_Pos) // bit[9:8], 2 bits
#define CSK_CLASSD_MERGE_16BITS_UNSET               (0x0UL << CSK_CLASSD_MERGE_16BITS_Pos) // Merge_16bit is kept unchanged or default
#define CSK_CLASSD_MERGE_16BITS_ENA                 (0x1UL << CSK_CLASSD_MERGE_16BITS_Pos) // Merge dual 16bits as one WORD in TX FIFO (default)
#define CSK_CLASSD_MERGE_16BITS_DIS                 (0x2UL << CSK_CLASSD_MERGE_16BITS_Pos) // NOT Merge dual 16bits

/*----- CLASSD Control Codes: Configuration Parameters: TX config. (mono/stereo input, mixed output or not?) -----*/
/*
#define CSK_CLASSD_TXCFG_Pos                        8
#define CSK_CLASSD_TXCFG_Msk                        (0x3UL << CSK_CLASSD_TXCFG_Pos) // bit[9:8], 2 bits
 // CLASSD config (mono/stereo, mixed or not) is kept unchanged or default
#define CSK_CLASSD_TXCFG_UNSET                      (0x0UL << CSK_CLASSD_TXCFG_Pos)
// [default] each mono channel data is sent to CLASSD device group (CLASSD0 & CLASSDC1) respectively
#define CSK_CLASSD_TXCFG_MONO_SRC_MONO              (0x1UL << CSK_CLASSD_TXCFG_Pos)
// mono channel data, duplicated and sent to both CLASSDs (CLASSD0 & CLASSD1)
#define CSK_CLASSD_TXCFG_STEREO_SRC_MONO            (0x2UL << CSK_CLASSD_TXCFG_Pos)
// stereo channel data, sent to CLASSD device group (CLASSD0 & CLASSD1) respectively
#define CSK_CLASSD_TXCFG_STEREO_SRC_STEREO          (0x3UL << CSK_CLASSD_TXCFG_Pos)
*/

///*-----INTERNAL USE ONLY, CLASSD Control Codes: Configuration Parameters: PWM selection -----*/
//#define CSK_CLASSD_PWM_SEL_Pos                      10
//#define CSK_CLASSD_PWM_SEL_Msk                      (0x3UL << CSK_CLASSD_PWM_SEL_Pos) // bit[11:10], 2 bits
//#define CSK_CLASSD_PWM_SEL_UNSET                    (0x0UL << CSK_CLASSD_PWM_SEL_Pos) // PWM selection is kept unchanged or default
//#define CSK_CLASSD_PWM_SEL_CENTRAL                  (0x1UL << CSK_CLASSD_PWM_SEL_Pos) // central aligned
//#define CSK_CLASSD_PWM_SEL_EDGE                     (0x2UL << CSK_CLASSD_PWM_SEL_Pos) // edge aligned
//
///*-----INTERNAL USE ONLY, CLASSD Control Codes: Configuration Parameters: REMAP & Off value -----*/
//#define CSK_CLASSD_REMAP_Pos                        12
//#define CSK_CLASSD_REMAP_Msk                        (0x3UL << CSK_CLASSD_REMAP_Pos) // bit[13:12], 2 bits
//#define CSK_CLASSD_REMAP_UNSET                      (0x0UL << CSK_CLASSD_REMAP_Pos) // Remap & Off value is kept unchanged or default
//#define CSK_CLASSD_REMAP_DISABLE                    (0x1UL << CSK_CLASSD_REMAP_Pos) // Don't use Remap PWM version
//#define CSK_CLASSD_REMAP_OFFVAL0                    (0x2UL << CSK_CLASSD_REMAP_Pos) // Use Remap PWM version, Off value 0
//#define CSK_CLASSD_REMAP_OFFVAL1                    (0x3UL << CSK_CLASSD_REMAP_Pos) // Use Remap PWM version, Off value 1
//
///*-----INTERNAL USE ONLY, CLASSD Control Codes: Configuration Parameters: Invert PWM signal of Positive / Negative channel -----*/
//#define CSK_CLASSD_INVSIG_PN_Pos                    14
//#define CSK_CLASSD_INVSIG_PN_Msk                    (0x7UL << CSK_CLASSD_INVSIG_P_Pos) // bit[16:14], 3 bits
//#define CSK_CLASSD_INVSIG_PN_UNSET                  (0x0UL << CSK_CLASSD_INVSIG_P_Pos) // Invert P/N signal setting is kept unchanged or default
//#define CSK_CLASSD_INVSIG_N_ONLY                    (0x1UL << CSK_CLASSD_INVSIG_P_Pos) // Invert N-signal only
//#define CSK_CLASSD_INVSIG_P_ONLY                    (0x2UL << CSK_CLASSD_INVSIG_P_Pos) // Invert P-signal only
//#define CSK_CLASSD_INVSIG_BOTH_PN                   (0x3UL << CSK_CLASSD_INVSIG_P_Pos) // Invert both P & N signals
//#define CSK_CLASSD_INVSIG_NEITHER_PN                (0x4UL << CSK_CLASSD_INVSIG_P_Pos) // Invert neither P nor N signals (default)
//
///*-----INTERNAL USE ONLY, CLASSD Control Codes: Configuration Parameters: Slew Rate Control -----*/
//#define CSK_CLASSD_SLEW_RATE_Pos                    17
//#define CSK_CLASSD_SLEW_RATE_Msk                    (0x1UL << CSK_CLASSD_SLEW_RATE_Pos) // bit[17]
//#define CSK_CLASSD_SLEW_RATE_UNSET                  (0x0UL << CSK_CLASSD_SLEW_RATE_Pos) // Slew Rate value is kept unchanged or default
//#define CSK_CLASSD_SLEW_RATE_SET                    (0x1UL << CSK_CLASSD_SLEW_RATE_Pos) // Set Slew Rate value, 3 bits slew rate value @ arg bit[2:0]
//#define SLEW_RATE_GET_VAL(arg)                      (arg & 0x7)
//#define SLEW_RATE_SET_VAL(arg, n)                   (arg &= ~0x7UL, arg |= n)
//#define SLEW_RATE_SLOW                              0 // Slew Rate slowest value
//#define SLEW_RATE_FAST                              7 // Slew Rate fastest value
//
///*-----INTERNAL USE ONLY, CLASSD Control Codes: Configuration Parameters: ClassD Mode Selection -----*/
//#define CSK_CLASSD_MODE_SEL_Pos                     18
//#define CSK_CLASSD_MODE_SEL_Msk                     (0x7UL << CSK_CLASSD_MODE_SEL_Pos) // bit[20:18], 3 bits
//#define CSK_CLASSD_MODE_SEL_UNSET                   (0x0UL << CSK_CLASSD_MODE_SEL_Pos) // ClassD Mode is kept unchanged or default
//#define CSK_CLASSD_MODE_SEL_NOLATCH_NOINV           (0x1UL << CSK_CLASSD_MODE_SEL_Pos) // Not Latch, Not Inverted
//#define CSK_CLASSD_MODE_SEL_NOLATCH_INVERT          (0x2UL << CSK_CLASSD_MODE_SEL_Pos) // Not Latch, Inverted
//#define CSK_CLASSD_MODE_SEL_LATCH_NOINV             (0x3UL << CSK_CLASSD_MODE_SEL_Pos) // Clock Latch, Not Inverted
//#define CSK_CLASSD_MODE_SEL_LATCH_INVERT            (0x4UL << CSK_CLASSD_MODE_SEL_Pos) // Clock Latch, Inverted
//
///*-----INTERNAL USE ONLY, CLASSD Control Codes: Configuration Parameters: HVLDO Enable -----*/
//#define CSK_CLASSD_HVLDO_Pos                        21
//#define CSK_CLASSD_HVLDO_Msk                        (0x3UL << CSK_CLASSD_HVLDO_Pos) // bit[22:21], 2 bits
//#define CSK_CLASSD_HVLDO_UNSET                      (0x0UL << CSK_CLASSD_HVLDO_Pos) // HVLDO enable setting is kept unchanged or default
//#define CSK_CLASSD_HVLDO_DIS                        (0x1UL << CSK_CLASSD_HVLDO_Pos) // disable ClassD HVLDO
//#define CSK_CLASSD_HVLDO_ENA                        (0x2UL << CSK_CLASSD_HVLDO_Pos) // Enable ClassD HVLDO
//
///*-----INTERNAL USE ONLY, CLASSD Control Codes: Configuration Parameters: Invert MCLK or not -----*/
//#define CSK_CLASSD_MCLKINV_Pos                      23
//#define CSK_CLASSD_MCLKINV_Msk                      (0x3UL << CSK_CLASSD_MCLKINV_Pos) // bit[24:23], 2 bits
//#define CSK_CLASSD_MCLKINV_UNSET                    (0x0UL << CSK_CLASSD_MCLKINV_Pos) // Invert MCLK setting is kept unchanged or default
//#define CSK_CLASSD_MCLKINV_NOT                      (0x1UL << CSK_CLASSD_MCLKINV_Pos) // NOT Invert MCLK
//#define CSK_CLASSD_MCLKINV_INV                      (0x2UL << CSK_CLASSD_MCLKINV_Pos) // Invert MCLK


//TODO/FIXME: other control codes...

/*----- CLASSD Control Codes: Exclusive Controls -----*/
/*----- exclusive operations, CANNOT coexist with other Control Codes -----*/
#define CSK_CLASSD_EXCL_OP_Pos                 29
#define CSK_CLASSD_EXCL_OP_Msk                 (0x7UL << CSK_CLASSD_EXCL_OP_Pos) // bit[31:29], 3 bits
#define CSK_CLASSD_EXCL_OP_UNSET               (0x0UL << CSK_CLASSD_EXCL_OP_Pos) // NO exclusive operations
#define CSK_CLASSD_ABORT_TRANSFER              (0x1UL << CSK_CLASSD_EXCL_OP_Pos) // Abort current data transfer,
                                                                           // arg bit[1:0] = dev_bmp, arg bit[9:8] = echo_bmp
#define CSK_CLASSD_GET_SAMP_RATE               (0x2UL << CSK_CLASSD_EXCL_OP_Pos) // Get sample rate, return sample rate
#define CSK_CLASSD_SET_ECHO_PARAMS             (0x3UL << CSK_CLASSD_EXCL_OP_Pos) // Set ECHO parameters, arg = pointer to ECHO_PARAMS

//TODO: other exclusive operation code...

/****** CLASSD specific error codes *****/
#define CSK_CLASSD_ERROR_SAMP_RATE             (CSK_DRIVER_ERROR_SPECIFIC - 1) // Specified Sample Rate not supported
#define CSK_CLASSD_ERROR_OVER_SAMP_RATIO       (CSK_DRIVER_ERROR_SPECIFIC - 2) // Specified Over Sample Ratio not supported
#define CSK_CLASSD_ERROR_SR_OSR_PAIR           (CSK_DRIVER_ERROR_SPECIFIC - 3) // Specified combination of Sample Rate & Over Sample Ratio not supported
//#define CSK_CLASSD_ERROR_TXCFG                 (CSK_DRIVER_ERROR_SPECIFIC - 4) // Specified TX Config (mono/stereo, mixed or not) not supported
#define CSK_CLASSD_ERROR_MCLK_SRC              (CSK_DRIVER_ERROR_SPECIFIC - 5) // Specified Main clock source not supported
#define CSK_CLASSD_ERROR_MERG_16BITS           (CSK_DRIVER_ERROR_SPECIFIC - 6) // Error setting of Merge dual 16bits
#define CSK_CLASSD_ERROR_INITED_ALREADY        (CSK_DRIVER_ERROR_SPECIFIC - 8) // CLASSD has already been initialized


// CLASSD status bit definitions
typedef struct _CSK_CLASSD_STATUS_BIT
{
    uint32_t busy :2;       // Send busy flag, bit[0] for CLASSD0 (left channel), bit[1] for CLASSD1 (right channel, NOT USED)
    uint32_t tx_emp :1;     // Send TX FIFO empty (cleared on start of transfer operation)
    uint32_t tx_undf :1;    // Send TX FIFO underflow (cleared on start of transfer operation)
    uint32_t mute :1;     // CLASSD0 is mute or not, 1 means mute
    //uint32_t l_mute :1;     // CLASSD0 (left channel) is mute or not, 1 means mute
    //uint32_t r_mute :1;     // CLASSD1 (right channel) is mute or not, 1 means mute

    uint32_t reserved :27;
} CSK_CLASSD_STATUS_BIT;

// CLASSD status
typedef union {
    uint32_t all;
    CSK_CLASSD_STATUS_BIT bit;
} CSK_CLASSD_STATUS;

///****** CLASSD Event *****/
#define CSK_CLASSD_EVENT_SEND_COMPLETE         (0x1UL << 0) ///< Data Send completed
#define CSK_CLASSD_EVENT_TX_FIFO_UNDERRUN      (0x1UL << 1) ///< Data TX FIFO underflow
#define CSK_CLASSD_EVENT_TX_FIFO_EMPTY         (0x1UL << 2) ///< Data TX FIFO empty
#define CSK_CLASSD_EVENT_BLOCK_COMPLETE        (0x1UL << 3) ///< Block Send completed
#define CSK_CLASSD_EVENT_ECHO_RX_COMPLETE      (0x1UL << 6) ///< ECHO Data Receive completed
#define CSK_CLASSD_EVENT_OTHER_ERROR           (0x1UL << 7) ///< Other Error

/**
 \fn          void CSK_CLASSD_SignalEvent_t (uint32_t event, uint32_t usr_param)
 \brief       Signal CLASSD Events.
  \param[in]  event_info CLASSD event and channel information
              bit[7:0] is event type, bit[15:8] is CLASSD/channel number (0=CLASSD0/Left, 1=CLASSD1/Right, always 0 on MARS)
              bit[23:16] indicate APC channel number
 \param[in]   usr_param     user parameter
 \return      none
*/
typedef void
(*CSK_CLASSD_SignalEvent_t)(uint32_t event_info, uint32_t usr_param);

/* NOTE: ONLY 16 bits of sample is supported for CLASSD on MARS !! */
#define CSK_CLASSD_SAMPLE_BITS         16


//------------------------------------------------------------------------------------------
/**
 \fn          CSK_DRIVER_VERSION CLASSD_GetVersion (void)
 \brief       Get driver version.
 \return      \ref CSK_DRIVER_VERSION
*/
CSK_DRIVER_VERSION
CLASSD_GetVersion();

#define CLSD_BMP_LEFT        CH_BMP_LEFT     // (0x1 << 0)
#define CLSD_BMP_RIGHT       CH_BMP_RIGHT    // (0x1 << 1)
#define CLSD_BMP_STEREO      CH_BMP_STEREO   // (0x3 << 0)

/**
 \fn          int32_t CLASSD_Initialize(void *clsd_grp, ...)
 \brief       Initialize CLASSD device group. A CLASSD device group includes at most two CLASSD devices,
              i.e. CLASSD01 includes CLASSD0 (Left Channel) and CLASSD1 (Right Channel) two devices.
              A CLASSD device must belong to some CLASSD device group.
 \param[in]   clsd_grp  Pointer to CLASSD device group instance
 \param[in]   cb_event  Pointer to \ref CSK_CLASSD_SignalEvent_t
 \param[in]   usr_param  User-defined value, acts as last parameter of cb_event
 \param[in]   dev_bmp  which CLASSD devices (similar to "I2S channels") are used,
              CLASSD0 (Left Channel, @bit[0]) and CLASSD1 (Right Channel, @bit[1]),
              and dev_bmp = 0x3 indicates both CLASSD0 & CLASSD1
 \param[in]   echo_bmp  which CLASSD devices' echo channels are used,
              CLASSD0 echo (Left Channel, @bit[0]) and CLASSD1 echo (Right Channel, @bit[1]),
              and echo_bmp = 0x3 indicates both echo channels of CLASSD0 & CLASSD1
 \param[in]   init_flags  see below
 \return      \ref execution_status
*/

// bit flag of the parameter 'dev_bmp_flag'
#define CLSD_BMP_FLAG_OUT_LEFT        (0x1 << 0) // bit[0] for CLASSD Left Channel (ONLY CLASSD0 on MARS)
#define CLSD_BMP_FLAG_OUT_RIGHT       (0x1 << 1) // bit[1] for CLASSD Channel (NO CLASSD1 on MARS, NOT USED)
#define CLSD_BMP_FLAG_OUT_STEREO      (0x3 << 0) // bit[1:0] for CLASSD0 (@Left Channel) & CLASSD1 (@Right Channel)

// ECHO channels are handled in the ADC_PDM driver or CLASSD driver?
#define CLSD_BMP_FLAG_ECHO_LEFT      (0x1 << 2) // bit[2] for ECHO Left Channel (ONLY Echo Left for CLASSD0 on MARS)
#define CLSD_BMP_FLAG_ECHO_RIGHT     (0x1 << 3) // bit[3] for ECHO Right Channel (NO Echo Right)
#define CLSD_BMP_FLAG_ECHO_STEREO    (0x3 << 2) // bit[3:2] for ECHO Left & Right Channels

#define CLSD_BMP_FLAG_DIS_MERGE_16BITS   (0x1 << 4) // bit[4] = 1 indicates NOT merge dual 16bits in TX FIFO

#define CLSD_BMP_FLAG_OUT_POS        0 // bit[1:0] for CLASSD
#define CLSD_BMP_FLAG_ECHO_POS       2 // bit[3:2] for ECHO

// DMA channels specified by user, set to 0xFF if ignored or NOT used.
typedef struct {
    uint32_t dma_ch_out_lr : 8; // DMA channel no. for CLASSD0 (Left channel) & CLASSD1 (Right channel, NOT existing on MARS)
    uint32_t rsvd0 : 8;
    uint32_t dma_ch_echo_lr : 8; // DMA channel no. for ECHO Left channel & Right channel (NOT existing on MARS)
    uint32_t rsvd1 : 8;
} CLASSD_DMA_CHS;

int32_t
CLASSD_Initialize(void *clsd_grp, CSK_CLASSD_SignalEvent_t cb_event, uint32_t usr_param,
                uint8_t dev_bmp_flag, CLASSD_DMA_CHS *dma_chs_p);

/**
 \fn          int32_t CLASSD_Uninitialize(void *clsd_grp)
 \brief       De-initialize CLASSD device group.
 \param[in]   clsd_grp  Pointer to CLASSD device group instance
 \return      \ref execution_status
*/
int32_t
CLASSD_Uninitialize(void *clsd_grp);

/**
 \fn          int32_t CLASSD_PowerControl(void *clsd_grp, ...)
 \brief       Control CLASSD device group's Power.
 \param[in]   clsd_grp  Pointer to CLASSD device group instance
 \param[in]   state  Power state
 \return      \ref execution_status
*/
int32_t
CLASSD_PowerControl(void *clsd_grp, CSK_POWER_STATE state);

/**
 \fn          int32_t CLASSD_Send(void *clsd_grp, ...)
 \brief       Send data within the buffer
 \param[in]   clsd_grp  Pointer to CLASSD device group instance
 \param[in]   data  Pointer to buffer of data to send
 \param[in]   num   Number of data items to send
 \param[in]   dev_bmp  send data from which CLASSD devices in the device group,
              CLASSD0 (Left Channel, @bit[0]) and CLASSD1 (Right Channel, @bit[1]),
              and dev_bmp = 0x3 indicates both CLASSD0 & CLASSD1
              NOTE: it can be CLSD_BMP_LEFT ONLY on MARS! SAME BELOW...
 \param[in]   tx_flag   bit flags of send operation
 \return      \ref execution_status
*/

// bit[0]=1 indicates Start sending immediately, or else
//  it will NOT transfer data until CLASSD_Enable is called
#define CLSD_TX_FLAG_START_NOW   (0x1 << 0)

// bit[1]=1 indicates don't sync cache internally for SEND
#define CLSD_TX_FLAG_NSYNCA      (0x1 << 1)

int32_t
CLASSD_Send(void *clsd_grp, const uint32_t *data, uint32_t num, uint8_t tx_flag); //, uint8_t dev_bmp

// Send data via CLASSD interface in the Ping/Pong mode
//NOTE: Cancel Ping/Pong playback if (blks == NULL || *blk_cnt_p == 0)
// Just break the Ping/Pong circular chain, NOT stop playback right now!
int32_t
CLASSD_Send_PiPo(void *clsd_grp, PIPO_OUT_BLOCK *blks, uint8_t *blk_cnt_p, uint8_t tx_flag);

//[OUT]  blks  Pointer to array of PIPO_OUT_BLOCK to hold transferred block descriptors
//[IN]   blk_cnt  Number of PIPO_OUT_BLOCK in the array
// return count of transferred block if >= 0, else return the error value.
int32_t
CLASSD_PiPo_Xferred_Blocks(void *clsd_grp, PIPO_OUT_BLOCK *blks, uint8_t blk_cnt);

/**
 \fn          int32_t CLASSD_Send_LLP(void *clsd_grp, ...)
 \brief       Send data within several non-continuous buffers
 \param[in]   clsd_grp  Pointer to CLASSD device group instance
 \param[in]   bufs  Pointer to list of buffers of data to send
 \param[in]   buf_cnt  Number of data buffers in the list
 \param[in]   dev_bmp  send data from which CLASSD devices in the device group,
              CLASSD0 (Left Channel, @bit[0]) and CLASSD1 (Right Channel, @bit[1]),
              and dev_bmp = 0x3 indicates both CLASSD0 & CLASSD1
 \param[in]   tx_flag   bit flags of send operation
 \return      \ref execution_status
*/
int32_t
CLASSD_Send_LLP(void *clsd_grp, AUDIO_BUFFER_USER *bufs, uint32_t buf_cnt, uint8_t tx_flag); //, uint8_t dev_bmp

/**
 \fn          int32_t CLASSD_Echo_Receive(void *clsd_grp, ...)
 \brief       Receive echo data into the buffer
 \param[in]   clsd_grp  Pointer to CLASSD device group instance
 \param[in]   data  Pointer to buffer to receive echo data
 \param[in]   num   Number of data items to receive
 \param[in]   echo_bmp  receive data from which echo channels of CLASSD device group,
              Left Echo Channel @bit[0] and Right Echo Channel @bit[1],
              and echo_bmp = 0x3 indicates both echo channels of CLASSD device group
              NOTE: it can be CH_BMP_LEFT ONLY on MARS! SAME BELOW...
 \return      \ref execution_status
*/
int32_t
CLASSD_Echo_Receive(void *clsd_grp, uint32_t *data, uint32_t num); //, uint8_t echo_bmp

/**
 \fn          int32_t CLASSD_Echo_Receive_LLP(void *clsd_grp, ...)
 \brief       Receive echo data into several non-continuous buffers
 \param[in]   clsd_grp  Pointer to CLASSD device group instance
 \param[in]   bufs  Pointer to list of buffer to receive echo data
 \param[in]   buf_cnt  Number of data buffers in the list
 \param[in]   echo_bmp  receive data from which echo channels of CLASSD device group,
              Left Echo Channel @bit[0] and Right Echo Channel @bit[1],
              and echo_bmp = 0x3 indicates both echo channels of CLASSD device group
 \return      \ref execution_status
*/
int32_t
CLASSD_Echo_Receive_LLP(void *clsd_grp, AUDIO_BUFFER_USER *bufs, uint32_t buf_cnt); //, uint8_t echo_bmp

/**
 \fn          int32_t CLASSD_Enable(void *clsd_grp, ...)
 \brief       Enable CLASSD device(s) and data send is started
              if CLASSD_Send_XXX is called before and START_NOW NOT specified.
 \param[in]   clsd_grp  Pointer to CLASSD device group instance
 \param[in]   dev_bmp  specify which CLASSD devices in the device group,
              CLASSD0 (Left Channel, @bit[0]) and CLASSD1 (Right Channel, @bit[1],
              NOT SUPPORTED on MARS, SAME BELOW),
              and dev_bmp = 0x3 indicates both CLASSD0 & CLASSD1
 \param[in]   echo_bmp  receive data from which echo channels of CLASSD device group,
              Left Echo Channel @bit[0] and Right Echo Channel @bit[1]
              (Right Channel is NOT SUPPORTED on MARS, SAME BELOW),
              and echo_bmp = 0x3 indicates both echo channels of CLASSD device group
 \return      \ref execution_status
*/
int32_t
CLASSD_Enable(void *clsd_grp, uint8_t dev_bmp, uint8_t echo_bmp);

/**
 \fn          int32_t CLASSD_Disable(void *clsd_grp, ...)
 \brief       Disable CLASSD device(s) (and data send is suspended if any).
 \param[in]   clsd_grp  Pointer to CLASSD device group instance
 \param[in]   dev_bmp  specify which CLASSD devices in the device group,
              CLASSD0 (Left Channel, @bit[0]) and CLASSD1 (Right Channel, @bit[1]),
              and dev_bmp = 0x3 indicates both CLASSD0 & CLASSD1
 \param[in]   echo_bmp  receive data from which echo channels of CLASSD device group,
              Left Echo Channel @bit[0] and Right Echo Channel @bit[1],
              and echo_bmp = 0x3 indicates both echo channels of CLASSD device group
 \return      \ref execution_status
*/
int32_t
CLASSD_Disable(void *clsd_grp, uint8_t dev_bmp, uint8_t echo_bmp);

/**
 \fn          int32_t CLASSD_Abort(void *clsd_grp, ...)
 \brief       Abort CLASSD data transfer if any.
 \param[in]   clsd_grp  Pointer to CLASSD device group instance
 \param[in]   dev_bmp  specify which CLASSD devices in the device group
              CLASSD0 (Left Channel, @bit[0]) and CLASSD1 (Right Channel, @bit[1]),
              and dev_bmp = 0x3 indicates both CLASSD0 & CLASSD1
 \param[in]   echo_bmp  which CLASSD devices' echo data channels are used,
              CLASSD0 echo (Left Channel, @bit[0]) and CLASSD1 echo (Right Channel, @bit[1]),
              and echo_bmp = 0x3 indicates both echo data channels of CLASSD0 & CLASSD1
 \return      \ref execution_status
*/
int32_t
CLASSD_Abort(void *clsd_grp, uint8_t dev_bmp, uint8_t echo_bmp);

/**
 \fn          uint32_t CLASSD_GetTxCount(void *clsd_grp, ...)
 \brief       Get count of data sent from CLASSD device(s).
 \param[in]   clsd_grp  Pointer to CLASSD device group instance
 \param[in]   dev_bmp  specify which CLASSD devices in the device group
              CLASSD0 (Left Channel, @bit[0]) and CLASSD1 (Right Channel, @bit[1]),
              and dev_bmp = 0x3 indicates both CLASSD0 & CLASSD1
 \return      number of data items (24-bit samples)transferred if positive, error value if negative.
*/
int32_t
CLASSD_GetTxCount(void *clsd_grp); //, uint8_t dev_bmp

/**
 \fn          uint32_t CLASSD_GetEchoCount(void *clsd_grp, ...)
 \brief       Get transferred echo data count.
 \param[in]   clsd_grp  Pointer to CLASSD device group instance
 \param[in]   echo_bmp  which CLASSD devices' echo data channels are used,
              CLASSD0 echo (Left Channel, @bit[0]) and CLASSD1 echo (Right Channel, @bit[1]),
              and echo_bmp = 0x3 indicates both echo data channels of CLASSD0 & CLASSD1
 \return      number of data items transferred if positive,
              error value if negative.
*/
int32_t
CLASSD_GetEchoCount(void *clsd_grp); //, uint8_t echo_bmp

/**
 \fn          int32_t CLASSD_Control(void *clsd_grp, ...)
 \brief       Control CLASSD device group.
 \param[in]   clsd_grp  Pointer to CLASSD device group instance
 \param[in]   control  Operation
 \param[in]   arg  Argument of operation (optional), i.e. sample rate
 \return      common \ref execution_status and driver specific \ref CLASSD execution_status
*/
int32_t
CLASSD_Control(void *clsd_grp, uint32_t control, uint32_t arg);

/**
 \fn          int32_t CLASSD_SetMute(void *clsd_grp, ...)
 \brief       Set mute/unmute value of CLASSD device group.
 \param[in]   clsd_grp  Pointer to CLASSD device group instance
 \param[in]   mute_val  Mute/UnMute bit of Left Channel (CLASSD0, @ mute_val[0])
                        and Right Channel (CLASSD1, @ mute_val[1]),
 \param[in]   dev_bmp  specify which CLASSD devices in the device group
                      CLASSD0 (Left Channel, @bit[0]) and CLASSD1 (Right Channel, @bit[1],
                      NOT SUPPORTED on MARS, SAME BELOW),
                      and dev_bmp = 0x3 indicates both CLASSD0 & CLASSD1
 \return      common \ref execution_status and driver specific \ref CLASSD execution_status
*/
//NOTE: ONLY CLASSD0 (Left channel) exists on MARS!
int32_t
CLASSD_SetMute(void *clsd_grp, uint8_t mute_val, uint8_t dev_bmp);

/**
 \fn          int32_t CLASSD_GetStatus(void *clsd_grp, ...)
 \brief       Get CLASSD device group's status.
 \param[in]   clsd_grp  Pointer to CLASSD device group instance
 \param[out]  status  Pointer to CSK_CLASSD_STATUS buffer
 \return      \ref execution_status
 */
int32_t
CLASSD_GetStatus(void *clsd_grp, CSK_CLASSD_STATUS *status);

//------------------------------------------------------------------------------------------
/**
 \fn          void* CLASSD01()
 \brief       Get CLASSD01 device group instance
 \return      CLASSD01 device instance (NOTE: ONLY CLASDD0 exists in the group on MARS!!)
 */
void* CLASSD01();

#endif /* __DRIVER_CLASSD_H */
