/*
 * Driver_I2S.h
 *
 *  Created on: Nov. 10, 2022 for VEGA
 *  Ported on: Jan. 30, 2024 for MARS
 *
 */

#ifndef __DRIVER_I2S_H
#define __DRIVER_I2S_H

#include "Driver_Common_Audio.h"

#define CSK_I2S_API_VERSION CSK_DRIVER_VERSION_MAJOR_MINOR(1,0)  /* API version */

/****** I2S Control Codes *****/

/*----- I2S Control Codes: Configuration Parameters: MODE -----*/
#define CSK_I2S_MODE_Pos                                 0
#define CSK_I2S_MODE_Msk                                 (0x3UL << CSK_I2S_MODE_Pos) // bit[1:0], 2 bits
#define CSK_I2S_MODE_UNSET                               (0x0UL << CSK_I2S_MODE_Pos) // I2S mode is kept unchanged or default
//#define CSK_I2S_MODE_MASTER                              (0x1UL << CSK_I2S_MODE_Pos) // I2S Master mode, arg(Low 24bits) = sample rate (if NOT 0)
//                                                                                     // arg(High 8bits) specify input clock of I2S module:
//                                                                                     // 0: 24Mhz (default); 1: 24.576Mhz; 2: 12.288Mhz
//                                                                                     // NOTE: 1 & 2 are for tradition CODECs, bck/lrck = 256 (2^N) etc.
#define CSK_I2S_MODE_MASTER                              (0x1UL << CSK_I2S_MODE_Pos) // I2S Master mode, arg = sample rate (if NOT 0)
#define CSK_I2S_MODE_SLAVE                               (0x2UL << CSK_I2S_MODE_Pos) // I2S Slave mode, arg = BCK / LRCK (if NOT 0) only
                                                                                     // when Right Justified mode, invalid in other cases

/*----- I2S Control Codes: Configuration Parameters: PROTOCOL -----*/
#define CSK_I2S_PROTO_Pos                                2
#define CSK_I2S_PROTO_Msk                                (0x7UL << CSK_I2S_PROTO_Pos) // bit[4:2], 3 bits
#define CSK_I2S_PROTO_UNSET                              (0x0UL << CSK_I2S_PROTO_Pos) // I2S protocol is kept unchanged or default
#define CSK_I2S_PROTO_PHILIPS                            (0x1UL << CSK_I2S_PROTO_Pos) // I2S Justified Mode(Philips)
#define CSK_I2S_PROTO_LEFT                               (0x2UL << CSK_I2S_PROTO_Pos) // Left Justified Mode
#define CSK_I2S_PROTO_RIGHT                              (0x3UL << CSK_I2S_PROTO_Pos) // Right Justified Mode
#define CSK_I2S_PROTO_PCMMODE_0                          (0x4UL << CSK_I2S_PROTO_Pos) // DSP/PCM mode 0
#define CSK_I2S_PROTO_PCMMODE_A                          CSK_I2S_PROTO_PCMMODE_0
#define CSK_I2S_PROTO_PCMMODE_1                          (0x5UL << CSK_I2S_PROTO_Pos) // DSP/PCM mode 1
#define CSK_I2S_PROTO_PCMMODE_B                          CSK_I2S_PROTO_PCMMODE_1
#define CSK_I2S_PROTO_LAST_INDEX                         0x05

/*----- I2S Control Codes: Configuration Parameters: DATA_FORMAT -----*/
#define CSK_I2S_DATA_FORMAT_Pos                          5
#define CSK_I2S_DATA_FORMAT_Msk                          (0x7UL << CSK_I2S_DATA_FORMAT_Pos) // bit[7:5], 3 bits
#define CSK_I2S_DATA_FORMAT_UNSET                        (0x0UL << CSK_I2S_DATA_FORMAT_Pos) // DATA_FORMAT is kept unchanged or default
#define CSK_I2S_DATA_FORMAT_DUAL_16BIT                   (0x1UL << CSK_I2S_DATA_FORMAT_Pos) // 16-bit {DATA1, DATA0}
#define CSK_I2S_DATA_FORMAT_24BIT_HIGH                   (0x2UL << CSK_I2S_DATA_FORMAT_Pos) // 24-bit {DATA, 8'd0}
#define CSK_I2S_DATA_FORMAT_32BIT                        (0x3UL << CSK_I2S_DATA_FORMAT_Pos) // 32-bit {DATA}
#define CSK_I2S_DATA_FORMAT_24BIT_LOW                    (0x4UL << CSK_I2S_DATA_FORMAT_Pos) // 24-bit {8'd0, DATA}
#define CSK_I2S_DATA_FORMAT_20BIT_HIGH                   (0x5UL << CSK_I2S_DATA_FORMAT_Pos) // 20-bit {DATA, 8'd0}
//#define CSK_I2S_DATA_FORMAT_20BIT_LOW                    (0x6UL << CSK_I2S_DATA_FORMAT_Pos) // 20-bit {8'd0, DATA} (NOT SUPPORTED!!)

/*----- I2S Control Codes: Configuration Parameters: TDM_CHANNELS -----*/
#define CSK_I2S_TDM_CHS_Pos                              8
#define CSK_I2S_TDM_CHS_Msk                              (0xFUL << CSK_I2S_TDM_CHS_Pos) // bit[11:8], 4 bits
#define CSK_I2S_TDM_CHS(n)                               (((n) & 0xF) << CSK_I2S_TDM_CHS_Pos) // TDM is not used if n == 0

/* NOTE: On Mars, I2S (IN/OUT) Left channel and Right channel share the same APC FIFO, so L/R channel data are always mixed!! */
/*----- I2S Control Codes: Configuration Parameters: RX CHANNEL CONFIG -----*/
/* VALID ONLY for I2S Philips/Left/Right justified.
 * For PCM mode (support TDM), all channels' data are always mixed */
/*
#define CSK_I2S_RXCH_Pos                                12
#define CSK_I2S_RXCH_Msk                                (0x3UL << CSK_I2S_RXCH_Pos) // bit[13:12], 2 bits
#define CSK_I2S_RXCH_UNSET                              (0x0UL << CSK_I2S_RXCH_Pos) // RXCH CONFIG is kept unchanged or default
#define CSK_I2S_RXCH_SEPA                               (0x1UL << CSK_I2S_RXCH_Pos) // RX channels'data are separate, read out respectively
#define CSK_I2S_RXCH_MIXED                              (0x2UL << CSK_I2S_RXCH_Pos) // RX channels'data are mixed, read out together
*/

/*----- I2S Control Codes: Configuration Parameters: TX CHANNEL (and SRC) CONFIG -----*/
/* VALID ONLY for I2S Philips/Left/Right justified.
 * For PCM mode (support TDM), all channels' data are always mixed */
#define CSK_I2S_TXCH_Pos                                14
#define CSK_I2S_TXCH_Msk                                (0x7UL << CSK_I2S_TXCH_Pos) // bit[16:14], 3 bits
#define CSK_I2S_TXCH_UNSET                              (0x0UL << CSK_I2S_TXCH_Pos) // TXCH CONFIG is kept unchanged or default
#define CSK_I2S_TXCH_MONO_SRC_MONO                      (0x1UL << CSK_I2S_TXCH_Pos) // mono channel data, sent to mono channel or one of L/R channels
#define CSK_I2S_TXCH_STEREO_SRC_MONO                    (0x2UL << CSK_I2S_TXCH_Pos) // mono channel data, duplicated and sent to both L&R channels
#define CSK_I2S_TXCH_STEREO_SRC_STEREO                  (0x3UL << CSK_I2S_TXCH_Pos) // stereo channel data, sent to L&R channels respectively

/*-----[NEW]: I2S Control Codes: Configuration Parameters: BIT_ORDER -----*/
#define CSK_I2S_BIT_ORDER_Pos                            17
#define CSK_I2S_BIT_ORDER_Msk                            (0x3UL << CSK_I2S_BIT_ORDER_Pos) // bit[18:17], 2 bit
#define CSK_I2S_BIT_ORDER_UNSET                          (0x0UL << CSK_I2S_BIT_ORDER_Pos) // BIT_ORDER is kept unchanged or default (MSB)
#define CSK_I2S_BIT_ORDER_MSB                            (0x1UL << CSK_I2S_BIT_ORDER_Pos) // MSB first
#define CSK_I2S_BIT_ORDER_LSB                            (0x2UL << CSK_I2S_BIT_ORDER_Pos) // LSB first


/*----- I2S Control Codes: Exclusive Controls -----*/
/*----- exclusive operations, CANNOT coexist with other Control Codes -----*/
#define CSK_I2S_EXCL_OP_Pos                 29
#define CSK_I2S_EXCL_OP_Msk                 (0x7UL << CSK_I2S_EXCL_OP_Pos)      // bit[31:29], 3 bits
#define CSK_I2S_EXCL_OP_UNSET               (0UL << CSK_I2S_EXCL_OP_Pos)        // NO exclusive operations
#define CSK_I2S_ABORT_TRANSFER              (1UL << CSK_I2S_EXCL_OP_Pos)        // Abort current data transfer, arg indicates IN,OUT,ECHO channels
                                                                                // arg: I2S_CHS_XXX for the parameter i2s_ch_set of I2S_Abort_Channels.
//#define CSK_I2S_ABORT_TRANSFER              (1UL << CSK_I2S_EXCL_OP_Pos)        // Abort current data transfer, arg indicates IN,OUT channels
//                                                                                // arg = (ch_bmp_out << 8)|(ch_bmp_in)
#define CSK_I2S_GET_SAMP_RATE               (2UL << CSK_I2S_EXCL_OP_Pos)        // Get sample rate, return sample rate
#define CSK_I2S_SET_SAMP_RATE               (3UL << CSK_I2S_EXCL_OP_Pos)        // Set sample rate, arg = sample rate
#define CSK_I2S_SET_ECHO_PARAMS             (4UL << CSK_I2S_EXCL_OP_Pos)        // Set ECHO parameters, arg = pointer to ECHO_PARAMS [NEW]:
#define CSK_I2S_RESET                       (5UL << CSK_I2S_EXCL_OP_Pos)        // Reset I2S module, RX & TX path (NOT reset registers!!) [NEW]:

/* The following I2S Control Codes are NOT IMPLEMENTED by I2S driver unless otherwise stated , for future use -----*/

///*----- I2S Control Codes: Configuration Parameters: BCK_OUTPUT_GATE_AFTER_SENT -----*/
//#define CSK_I2S_BCKOUT_GATE_Pos                          24
//#define CSK_I2S_BCKOUT_GATE_Msk                          (0x1UL << CSK_I2S_BCKOUT_GATE_Pos) // bit[24], 1 bit
//#define CSK_I2S_BCKOUT_GATE_NOGATE                       (0x0UL << CSK_I2S_BCKOUT_GATE_Pos) // No gate
//#define CSK_I2S_BCKOUT_GATE_GATED                        (0x1UL << CSK_I2S_BCKOUT_GATE_Pos) // Gated (no output after sent)
//
///*----- I2S Control Codes: Configuration Parameters: DEBUG_MODE -----*/
//#define CSK_I2S_DBGMODE_Pos                              26
//#define CSK_I2S_DBGMODE_Msk                              (0x1UL << CSK_I2S_DBGMODE_Pos) // bit[26], 1 bit
//#define CSK_I2S_DBGMODE_NORMAL                           (0x0UL << CSK_I2S_DBGMODE_Pos) // Normal
//#define CSK_I2S_DBGMODE_LOOPBACK                         (0x1UL << CSK_I2S_DBGMODE_Pos) // Loop back (for debug only, not work for DAI)
//
///*----- I2S Control Codes: Configuration Parameters: SWAP_IN_CHLR -----*/
//#define CSK_I2S_IN_CHLR_Pos                              27
//#define CSK_I2S_IN_CHLR_Msk                              (0x1UL << CSK_I2S_IN_CHLR_Pos) // bit[27], 1 bit
//#define CSK_I2S_IN_CHLR_NORM                             (0x0UL << CSK_I2S_IN_CHLR_Pos) // NOT swap IN L/R channel
//#define CSK_I2S_IN_CHLR_SWAP                             (0x1UL << CSK_I2S_IN_CHLR_Pos) // swap IN L/R channel
//
///*----- I2S Control Codes: Configuration Parameters: SWAP_OUT_CHLR -----*/
//#define CSK_I2S_OUT_CHLR_Pos                             28
//#define CSK_I2S_OUT_CHLR_Msk                             (0x1UL << CSK_I2S_SWAP_LROUT_Pos) // bit[28], 1 bit
//#define CSK_I2S_OUT_CHLR_NORM                            (0x0UL << CSK_I2S_SWAP_LROUT_Pos) // NOT swap OUT L/R channel
//#define CSK_I2S_OUT_CHLR_SWAP                            (0x1UL << CSK_I2S_SWAP_LROUT_Pos) // swap OUT L/R channel
//
///*----- I2S Control Codes: Configuration Parameters: LRCK_POL -----*/
//#define CSK_I2S_LRCK_POL_Pos                             29
//#define CSK_I2S_LRCK_POL_Msk                             (0x1UL << CSK_I2S_LRCK_POL_Pos) // bit[29], 1 bit
//#define CSK_I2S_LRCK_POL_NORM                            (0x0UL << CSK_I2S_LRCK_POL_Pos) // Normal (Low Idle?)
//#define CSK_I2S_LRCK_POL_INVT                            (0x1UL << CSK_I2S_LRCK_POL_Pos) // Invert (High Idle?)
//
///*----- I2S Control Codes: Configuration Parameters: BCK_POL -----*/
//#define CSK_I2S_BCK_POL_Pos                              30
//#define CSK_I2S_BCK_POL_Msk                              (0x1UL << CSK_I2S_BCK_POL_Pos) // bit[30], 1 bit
//#define CSK_I2S_BCK_POL_NORM                             (0x0UL << CSK_I2S_BCK_POL_Pos) // Left High Right Low
//#define CSK_I2S_BCK_POL_INVT                             (0x1UL << CSK_I2S_BCK_POL_Pos) // Left Low Right High


/****** I2S specific error codes *****/
#define CSK_I2S_ERROR_MODE              (CSK_DRIVER_ERROR_SPECIFIC - 1)     // Specified Mode not supported
#define CSK_I2S_ERROR_PROTOCOL          (CSK_DRIVER_ERROR_SPECIFIC - 2)     // Specified Protocol not supported
#define CSK_I2S_ERROR_DATA_FORMAT       (CSK_DRIVER_ERROR_SPECIFIC - 3)     // Specified Data Format not supported
//#define CSK_I2S_ERROR_RXCH_CONFIG       (CSK_DRIVER_ERROR_SPECIFIC - 4)     // Specified RX Channel Config not supported
#define CSK_I2S_ERROR_TXCH_CONFIG       (CSK_DRIVER_ERROR_SPECIFIC - 5)     // Specified TX Channel Config not supported
#define CSK_I2S_ERROR_BIT_ORDER         (CSK_DRIVER_ERROR_SPECIFIC - 6)     // Specified Bit Order Config not supported [NEW]:
#define CSK_I2S_ERROR_INITED_ALREADY    (CSK_DRIVER_ERROR_SPECIFIC - 8)     // I2S has already been initialized


//------------------------------------------------------------------------------------------

//#define I2S_CH_BMP_LEFT    CH_BMP_LEFT
//#define I2S_CH_BMP_RIGHT   CH_BMP_RIGHT
//#define I2S_CH_BMP_STEREO  CH_BMP_STEREO

// supported protocol of CSK I2S
typedef enum {
    I2S_PROTO_UNKNOWN       =   0,
    I2S_PROTO_PHILIPS       =   (CSK_I2S_PROTO_PHILIPS >> CSK_I2S_PROTO_Pos),
    I2S_PROTO_LEFT          =   (CSK_I2S_PROTO_LEFT >> CSK_I2S_PROTO_Pos),
    I2S_PROTO_RIGHT         =   (CSK_I2S_PROTO_RIGHT >> CSK_I2S_PROTO_Pos),
    I2S_PROTO_PCMMODE_0     =   (CSK_I2S_PROTO_PCMMODE_0 >> CSK_I2S_PROTO_Pos),
    I2S_PROTO_PCMMODE_A     =   I2S_PROTO_PCMMODE_0,
    I2S_PROTO_PCMMODE_1     =   (CSK_I2S_PROTO_PCMMODE_1 >> CSK_I2S_PROTO_Pos),
    I2S_PROTO_PCMMODE_B     =   I2S_PROTO_PCMMODE_1,
    I2S_PROTO_COUNT
} I2S_PROTOCOL;

// supported sample rate array of CSK I2S
static const uint32_t csk_i2s_samp_rates[] = {
        96000,
        48000,
        24000,
        32000,
        16000,
        8000,
//        44100,
//        22050,
//        11025
};

// I2S status bit definitions [NEW]:
#define CSK_I2S_STATUS_RX_BUSY      (0x1 << 0)
#define CSK_I2S_STATUS_TX_BUSY      (0x1 << 1)
#define CSK_I2S_STATUS_ECHO_BUSY    (0x1 << 2)
#define CSK_I2S_STATUS_BUSY_MASK    (CSK_I2S_STATUS_RX_BUSY | CSK_I2S_STATUS_TX_BUSY | CSK_I2S_STATUS_ECHO_BUSY)
typedef struct _CSK_I2S_STATUS_BIT
{
    uint32_t rx_busy :1;       // Receiver busy flag
    uint32_t tx_busy :1;       // Transmitter busy flag
    uint32_t ech_busy :1;      // ECHO Receiver busy flag
    uint32_t rx_ovf :1;     // Receiver overflow (cleared on start of Receive operation)
    uint32_t tx_unf :1;     // Transmit underflow (cleared on start of Send operation)
    uint32_t ech_ovf :1;     // ECHO Receiver overflow (cleared on start of ECHO Receive operation)
    uint32_t reserved :25;

    //uint32_t clkin_idx :2; // input clock, 0: 24Mhz (default)
    uint32_t master :1;    // 0 = slave, 1 = master
} CSK_I2S_STATUS_BIT;

// I2S status
typedef union {
    uint32_t all;
    CSK_I2S_STATUS_BIT bit;
} CSK_I2S_STATUS;


///****** I2S Event *****/
#define CSK_I2S_EVENT_RECEIVE_COMPLETE      (0x1UL << 0) ///< Data Receive completed
#define CSK_I2S_EVENT_TRANSMIT_COMPLETE     (0x1UL << 1) ///< Data Transmit completed

#define CSK_I2S_EVENT_RX_FIFO_OVERRUN       (0x1UL << 2) ///< Data Receive overflow
#define CSK_I2S_EVENT_TX_FIFO_UNDERRUN      (0x1UL << 3) ///< Data Transmit underflow
#define CSK_I2S_EVENT_RX_FIFO_FULL          (0x1UL << 4) ///< Data Receive full
#define CSK_I2S_EVENT_TX_FIFO_EMPTY         (0x1UL << 5) ///< Data Transmit empty

#define CSK_I2S_EVENT_ECHO_RX_COMPLETE      (0x1UL << 6) ///< ECHO Data Receive completed [NEW]:
#define CSK_I2S_EVENT_CLOCK_ERROR           (0x1UL << 7) ///< Glitch on BCK/LRCK clock etc. [NEW]:
#define CSK_I2S_EVENT_OTHER_ERROR           (0x1UL << 8) ///< Other Error

#define CSK_I2S_EVENT_RX_BLOCK_COMPLETE     (0x1UL << 9) ///< Block Receive completed
#define CSK_I2S_EVENT_TX_BLOCK_COMPLETE     (0x1UL << 10) ///< Block Send completed

/**
 \fn          void CSK_I2S_SignalEvent_t (uint32_t event, uint32_t usr_param)
 \brief       Signal I2S Events.
 \param[in]   event_info I2S event and channel information
              bit[14:0] is event type,
              bit[15] indicate the direction, 1 = IN, 0 = OUT
              bit[23:16] indicate APC channel number [NEW]:
 \param[in]   usr_param    user parameter
 \return      none
*/
typedef void
(*CSK_I2S_SignalEvent_t)(uint32_t event_info, uint32_t usr_param);

#define CSK_I2S_EVENT_HIGHEST_POS       14 // 15
#define CSK_I2S_EVENT_MASK              ((0x1UL << (CSK_I2S_EVENT_HIGHEST_POS + 1)) - 1)

#define CSK_I2S_EVENT_XFER_DIR_POS      (CSK_I2S_EVENT_HIGHEST_POS + 1)
#define CSK_I2S_EVENT_XFER_DIR_MASK     (0x1 << CSK_I2S_EVENT_XFER_DIR_POS)
#define CSK_I2S_EVENT_APC_CH_POS        (CSK_I2S_EVENT_HIGHEST_POS + 2)
#define CSK_I2S_EVENT_APC_CH_MASK       (0xFF << CSK_I2S_EVENT_APC_CH_POS)

//------------------------------------------------------------------------------------------
/**
 \fn          CSK_DRIVER_VERSION I2S_GetVersion (void)
 \brief       Get driver version.
 \return      \ref CSK_DRIVER_VERSION
*/
CSK_DRIVER_VERSION
I2S_GetVersion();

/**
 \fn          int32_t I2S_Initialize (void *i2s_dev, CSK_I2S_SignalEvent_t cb_event, uint32_t usr_param)
 \brief       Initialize I2S Interface.
 \param[in]   i2s_dev  Pointer to I2S device instance
 \param[in]   cb_event  Pointer to \ref CSK_I2S_SignalEvent_t
 \param[in]   usr_param  User-defined value, acts as last parameter of cb_event
 \param[in]   dev_bmp_flag  indicates I2S IN/OUT/ECHO channels used currently and other flags
 \param[in]   dma_chs_p     Pointer to the structure specifying DMA channels.
 \return      \ref execution_status
*/

// bit flag of the parameter 'dev_bmp_flag'
#define I2S_BMP_FLAG_IN_POS         0 // bit[1:0] for I2S IN
#define I2S_BMP_FLAG_OUT_POS        2 // bit[3:2] for I2S OUT
#define I2S_BMP_FLAG_ECHO_POS       4 // bit[5:4] for I2S ECHO

#define I2S_BMP_FLAG_IN_LEFT        (0x1 << 0) // bit[0] for I2S IN Left Channel
#define I2S_BMP_FLAG_IN_RIGHT       (0x1 << 1) // bit[1] for I2S IN Right Channel
#define I2S_BMP_FLAG_IN_STEREO      (0x3 << 0) // bit[1:0] for I2S IN Left & Right Channels

#define I2S_BMP_FLAG_OUT_LEFT       (0x1 << 2) // bit[2] for I2S OUT Left Channel
#define I2S_BMP_FLAG_OUT_RIGHT      (0x1 << 3) // bit[3] for I2S OUT Right Channel
#define I2S_BMP_FLAG_OUT_STEREO     (0x3 << 2) // bit[3:2] for I2S OUT Left & Right Channels

#define I2S_BMP_FLAG_ECHO_LEFT      (0x1 << 4) // bit[4] for ECHO Left Channel
#define I2S_BMP_FLAG_ECHO_RIGHT     (0x1 << 5) // bit[5] for ECHO Right Channel
#define I2S_BMP_FLAG_ECHO_STEREO    (0x3 << 4) // bit[5:4] for ECHO Left & Right Channels

#define I2S_BMP_FLAG_USE_ALT_IN     (0x1 << 6) // bit[6]=1 indicates to use alternate APC channel (IN FIFO1),
                                               // else use normal one (IN FIFO0)
#define I2S_BMP_FLAG_MIX_IN_ECHO    (0x1 << 7) // bit[7]=1 indicates that I2S IN data are mixed with ECHO data when received! [NEW]:
// NOTE: ONLY 1 (LEFT or RIGHT) channel's IN data and 1 channel's ECHO data CAN be mixed and received simultaneously,
// that means, >=2 channels' IN data or >=1 channel's ECHO data SHOULD be received separately!!

// DMA channels specified by user, set to 0xFF if ignored or NOT used.
typedef struct {
    uint32_t dma_ch_in_lr : 8; // DMA channel no. for I2S IN Left and/or Right channel
    uint32_t dma_ch_out_lr : 8; // DMA channel no. for I2S OUT Left and/or Right channel
    uint32_t dma_ch_echo_lr : 8; // DMA channel no. for I2S ECHO Left and/or Right channel
    uint32_t reserved : 8;
} I2S_DMA_CHS;

//int32_t
//I2S_Initialize(void *i2s_dev, CSK_I2S_SignalEvent_t cb_event, uint32_t usr_param,
//                uint8_t ch_bmp_in, uint8_t ch_bmp_out);
int32_t
I2S_Initialize(void *i2s_dev, CSK_I2S_SignalEvent_t cb_event, uint32_t usr_param,
               uint32_t dev_bmp_flag, I2S_DMA_CHS *dma_chs_p); // [NEW]:


/**
 \fn          int32_t I2S_Uninitialize (void)
 \brief       De-initialize I2S Interface.
 \param[in]   i2s_dev  Pointer to I2S device instance
 \return      \ref execution_status
*/
int32_t
I2S_Uninitialize(void *i2s_dev);

/**
 \fn          int32_t I2S_PowerControl (void *i2s_dev, CSK_POWER_STATE state)
 \brief       Control I2S Interface Power.
 \param[in]   i2s_dev  Pointer to I2S device instance
 \param[in]   state  Power state
 \return      \ref execution_status
*/
int32_t
I2S_PowerControl(void *i2s_dev, CSK_POWER_STATE state);

/**
 \fn          int32_t I2S_Send (void *i2s_dev, const void *data, uint32_t num)
 \brief       Start sending data to I2S out channel.
 \param[in]   i2s_dev  Pointer to I2S device instance
 \param[in]   ch_no  I2S channel number (data is sent on the channel)
 \param[in]   data  Pointer to buffer with data to be sent
 \param[in]   num   Number of data items to send
 \return      \ref execution_status
*/

// bit[0]=1 indicates Start sending immediately, or else
//  it will NOT transfer data until I2S_Enable_Channels is called
#define I2S_TX_FLAG_START_NOW   (0x1 << 0)

// bit[1]=1 indicates don't sync cache internally for SEND
//#define I2S_TX_FLAG_NSYNCA      (0x1 << 1)

int32_t
I2S_Send(void *i2s_dev, const uint32_t *data, uint32_t num, uint32_t tx_flag); // uint8_t ch_bmp

// Send data via I2S OUT interface in the Ping/Pong mode
//NOTE: Cancel Ping/Pong playback if (blks == NULL || *blk_cnt_p == 0)
// Just break the Ping/Pong circular chain, NOT stop playback right now!
int32_t
I2S_Send_PiPo(void *i2s_dev, PIPO_OUT_BLOCK *blks, uint8_t *blk_cnt_p, uint8_t tx_flag);

//[OUT]  blks  Pointer to array of PIPO_OUT_BLOCK to hold transferred block descriptors
//[IN]   blk_cnt  Number of PIPO_OUT_BLOCK in the array
// return count of transferred block if >= 0, else return the error value.
int32_t
I2S_PiPo_Txed_Blocks(void *i2s_dev, PIPO_OUT_BLOCK *blks, uint8_t blk_cnt);

int32_t
I2S_Send_LLP(void *i2s_dev, AUDIO_BUFFER_USER *bufs, uint32_t buf_cnt, uint32_t tx_flag); // uint8_t ch_bmp

/**
 \fn          int32_t I2S_Receive (void *i2s_dev, void *data, uint32_t num)
 \brief       Start receiving data from I2S in channel.
 \param[in]   i2s_dev  Pointer to I2S device instance
 \param[in]   ch_no  I2S channel number (data is received on the channel)
 \param[out]  data  Pointer to buffer for data to receive from I2S in channel
 \param[in]   num   Number of data items to receive
 \return      \ref execution_status
*/

// bit[0]=1 indicates Start receiving immediately, or else
// it will NOT transfer data until I2S_Enable_Channels is called
#define I2S_RX_FLAG_START_NOW   (0x1 << 0)

//NOTE: data will include 1 (LEFT or RIGHT) channel's IN data and 1 channel's ECHO data
// if I2S_BMP_FLAG_MIX_IN_ECHO is set in I2S_Initialize() and CSK_I2S_SET_ECHO_PARAMS is called for setup ECHO parameters.
int32_t
I2S_Receive(void *i2s_dev, uint32_t *data, uint32_t num, uint32_t rx_flag); // uint8_t ch_bmp, uint8_t rx_flag

// Receive data from I2S IN interface in the Ping/Pong mode
//NOTE: Cancel Ping/Pong receive if (blks == NULL || *blk_cnt_p == 0)
// Just break the Ping/Pong circular chain, NOT stop receiving right now!
int32_t
I2S_Receive_PiPo(void *i2s_dev, PIPO_IN_BLOCK *blks, uint8_t *blk_cnt_p, uint8_t rx_flag);

//[OUT]  blks  Pointer to array of PIPO_IN_BLOCK to hold received block descriptors
//[IN]   blk_cnt  Number of PIPO_IN_BLOCK in the array
// return count of transferred block if >= 0, else return the error value.
int32_t
I2S_PiPo_Rxed_Blocks(void *i2s_dev, PIPO_IN_BLOCK *blks, uint8_t blk_cnt);

int32_t
I2S_Receive_LLP(void *i2s_dev, AUDIO_BUFFER_USER *bufs, uint32_t buf_cnt, uint32_t rx_flag);  // uint8_t ch_bmp, uint8_t rx_flag

int32_t
I2S_Echo_Receive(void *i2s_dev, uint32_t *data, uint32_t num); //, uint8_t ch_bmp //[NEW]:

int32_t
I2S_Echo_Receive_LLP(void *i2s_dev, AUDIO_BUFFER_USER *bufs, uint32_t buf_cnt); //, uint8_t ch_bmp //[NEW]:


#define I2S_CHS_IN       (0x1 << 0) // I2S IN channel(s)
#define I2S_CHS_OUT      (0x1 << 1) // I2S OUT channel(s)
#define I2S_CHS_ECHO     (0x1 << 2) // I2S ECHO channel(s)
#define I2S_CHS_ALL      (I2S_CHS_IN | I2S_CHS_OUT | I2S_CHS_ECHO)
#define I2S_CHS_INOUT    (I2S_CHS_IN | I2S_CHS_OUT)
/**
 \fn          int32_t I2S_Abort_Channels (void *i2s_dev, uint8_t ch_bmp_in, uint8_t ch_bmp_out)
 \brief       Abort data transfer of I2S in and/or out channels if any.
 \param[in]   i2s_dev  Pointer to I2S device instance
 \param[in]   i2_chs  see I2S_CH_XXX definitions, can be any bit-OR value of I2S_CH_IN / I2S_CH_OUT / I2S_CH_ECHO.
 \return      \ref execution_status
*/
int32_t
I2S_Abort_Channels(void *i2s_dev, uint32_t i2s_ch_set); // uint8_t ch_bmp_in, uint8_t ch_bmp_out, uint8_t ch_bmp_echo //[NEW]:
//int32_t
//I2S_Abort_Channels(void *i2s_dev, uint8_t ch_bmp_in, uint8_t ch_bmp_out);

/**
 \fn          int32_t I2S_Enable_Channels (void *i2s_dev, uint8_t ch_bmp_in, uint8_t ch_bmp_out)
 \brief       Enable I2S in and/or out channels, and data are started to transfer on the lines
              from now on if I2s_Receive_XXX and/or I2s_Send_XXX are called before.
 \param[in]   i2s_dev  Pointer to I2S device instance
 \param[in]   i2_chs  see I2S_CH_XXX definitions, can be any bit-OR value of I2S_CH_IN / I2S_CH_OUT / I2S_CH_ECHO.
 \return      \ref execution_status
*/
int32_t
I2S_Enable_Channels(void *i2s_dev, uint32_t i2s_ch_set); // uint8_t ch_bmp_in, uint8_t ch_bmp_out

/**
 \fn          int32_t I2S_Disable_Channels (void *i2s_dev, uint8_t ch_bmp_in, uint8_t ch_bmp_out)
 \brief       Disable I2S in and/or out channels (and data transfer on the lines are suspended if any).
 \param[in]   i2s_dev  Pointer to I2S device instance
 \param[in]   i2_chs  see I2S_CH_XXX definitions, can be any bit-OR value of I2S_CH_IN / I2S_CH_OUT / I2S_CH_ECHO.
 \return      \ref execution_status
*/
int32_t
I2S_Disable_Channels(void *i2s_dev, uint32_t i2s_ch_set); // uint8_t ch_bmp_in, uint8_t ch_bmp_out

/**
 \fn          uint32_t I2S_GetTxCount (void *i2s_dev)
 \brief       Get transferred data count.
 \param[in]   i2s_dev  Pointer to I2S device instance
 \return      number of data items transferred if positive,
              error value if negative.
*/
int32_t
I2S_GetTxCount(void *i2s_dev); //, uint8_t ch_bmp

/**
 \fn          uint32_t I2S_GetRxCount (void *i2s_dev)
 \brief       Get transferred data count.
 \param[in]   i2s_dev  Pointer to I2S device instance
 \return      number of data items transferred if positive,
              error value if negative.
*/
//NOTE: if I2S_Receive/I2S_Receive_LLP with I2S_RX_FLAG_WITH_ECHO flag,
// I2S_GetRxCount return transferred data count on IN (RX) channel & ECHO channel,
// while I2S_GetEchoCount may return 0!!
int32_t
I2S_GetRxCount(void *i2s_dev); //, uint8_t ch_bmp

/**
 \fn          uint32_t I2S_GetEchoCount
 \brief       Get transferred echo data count.
 \param[in]   i2s_dev  Pointer to I2S device instance
 \return      number of data items transferred if positive,
              error value if negative.
*/
int32_t
I2S_GetEchoCount(void *i2s_dev); //, uint8_t ch_bmp

/**
 \fn          int32_t I2S_Control (void *i2s_dev, uint32_t control, uint32_t arg)
 \brief       Control I2S Interface.
 \param[in]   i2s_dev  Pointer to I2S device instance
 \param[in]   control  Operation
 \param[in]   arg  Argument of operation (optional), i.e. the speed of i2s when as master
 \return      common \ref execution_status and driver specific \ref i2s execution_status
*/
int32_t
I2S_Control(void *i2s_dev, uint32_t control, uint32_t arg);

/**
 \fn          int32_t I2S_GetStatus (void *spi_dev, CSK_I2S_STATUS *status)
 \brief       Get I2S status.
 \param[in]   i2s_dev  Pointer to I2S device instance
 \param[out]  status  Pointer to CSK_I2S_STATUS buffer
 \return      \ref execution_status
 */
int32_t
I2S_GetStatus(void *i2s_dev, CSK_I2S_STATUS *status);


//// EQ API
//int32_t I2S_EQ_Set_Coef_Array(void *i2s_dev, uint32_t *eqcoefs, uint32_t num);
//int32_t I2S_EQ_Set_Coef(void *i2s_dev, uint32_t index, uint32_t eqcoef);
//int32_t I2S_EQ_Enable(void *i2s_dev, uint32_t stages);
//int32_t I2S_EQ_Disble(void *i2s_dev);
//int32_t I2S_EQ_Clear(void *i2s_dev, uint8_t wait_done);

//------------------------------------------------------------------------------------------
/**
 \fn          void* I2S0()
 \brief       Get I2S0 device instance
 \return      I2S0 device instance
 */
void* I2S0();

#endif /* __DRIVER_I2S_H */
