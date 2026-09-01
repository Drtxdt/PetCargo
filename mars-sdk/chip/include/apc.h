/*
 * apc.h
 *
 *  Created on: Nov. 1, 2022 for VEGA
 *  Ported on: Jan. 23, 2024 for MARS
 *
 */

#ifndef __APC_MARS_H
#define __APC_MARS_H

#include "mars.h"
#include "dma.h"
#include "cmn_syscfg_reg.h" // for sysctrl regigers (clock etc.)
#include "Driver_Common_Audio.h" // for PIPO_IN_BLOCK & PIPO_OUT_BLOCK

// APC data interface: ADC, DAC, I2S(IN & OUT), and Echo
typedef enum {
    APC_INTF_UNSET = 0, // default state
    APC_INTF_ADC_PDM,   // [IN] ADC_PDM (PDM for DMIC)
    APC_INTF_CLASSD,    // [OUT] CLASSD
    APC_INTF_I2S_IN,    // [IN] I2S
    APC_INTF_I2S_OUT,   // [OUT] I2S
    //APC_INTF_ECHO,      // [IN] Echo (hidden, from output audio)
    APC_INTF_ECHO0,      // [IN] Echo (hidden, from TX CH0 output audio)
    APC_INTF_ECHO_CLASSD = APC_INTF_ECHO0,
    APC_INTF_ECHO1,      // [IN] Echo (hidden, from TX CH1 output audio)
    APC_INTF_ECHO_I2SOUT = APC_INTF_ECHO1,
    APC_INTF_TYPE_COUNT
} APC_INTF_TYPE;

/* //[REMOVED]:
// APC interface index of each interface type
typedef enum {
    APC_INTF_IDX0 = 0,
    APC_INTF_IDX_ADC_PDM01 = APC_INTF_IDX0, // ADC0 ONLY!!
    APC_INTF_IDX_CLASSD = APC_INTF_IDX0,
    APC_INTF_IDX_I2S0 = APC_INTF_IDX0,
    APC_INTF_IDX_ECHO = APC_INTF_IDX0,
} APC_INTF_INDEX;
*/

// Channel bitmap of some interface type, i.e. ADC01, DMIC01, I2S, ECHO etc.
typedef enum {
    APC_INTF_CH_LEFT = (0x1 << 0x0),     // ADC0, DMIC0, I2S LEFT, ECHO LEFT
    APC_INTF_CH_RIGHT = (0x1 << 0x1),    // ADC1 (NOT on MARS), DMIC1, I2S RIGHT, ECHO RIGHT
    APC_INTF_CH_STEREO = (APC_INTF_CH_LEFT | APC_INTF_CH_RIGHT),   // ADC01, DMIC01, I2S, ECHO
} APC_INTF_CHBMP;
#define APC_INTF_CH_MASK     APC_INTF_CH_STEREO


// APC dual_channel: 1 IN Dual_FIFOs (IN0 ~ IN1), 1 OUT Dual_FIFOs (OUT0 ~ OUT1)
typedef enum {
    APC_DCH_IN0 = 0, // [IN] FIFO 0 & 1
    APC_DCH_OUT0,    // [OUT] FIFO 2 & 3
    APC_DCH_COUNT
} APC_DCH; // DCH = Dual CHannel (Left + Right)

// APC channel: 2 IN FIFOs (IN0_L, IN0_R), 2 OUT FIFOs (OUT0_L, OUT0_R)
typedef enum {
    APC_CH_IN_FIFO0 = 0, // [IN] FIFO 0
    APC_CH_ADC_PDM01 = APC_CH_IN_FIFO0,
    APC_CH_I2S0_IN = APC_CH_IN_FIFO0,
    APC_CH_IN_MIN = APC_CH_IN_FIFO0,

    APC_CH_IN_FIFO1,     // [IN] FIFO 1
    APC_CH_I2S0_IN_ALT = APC_CH_IN_FIFO1,
    APC_CH_ECHO0 = APC_CH_IN_FIFO1,
    APC_CH_ECHO_CLASSD01 = APC_CH_IN_FIFO1,
    APC_CH_ECHO1 = APC_CH_IN_FIFO1,
    APC_CH_ECHO_I2S0_OUT = APC_CH_IN_FIFO1,
    APC_CH_IN_MAX = APC_CH_IN_FIFO1,

    APC_CH_OUT_FIFO0,     // [OUT] FIFO 2
    APC_CH_CLASSD01 = APC_CH_OUT_FIFO0,
    APC_CH_OUT_MIN = APC_CH_OUT_FIFO0,

    APC_CH_OUT_FIFO1,     // [OUT] FIFO 3
    APC_CH_I2S0_OUT = APC_CH_OUT_FIFO1,
    APC_CH_OUT_MAX = APC_CH_OUT_FIFO1,

    APC_CH_COUNT
} APC_CH;

// APC Dual_channel count & channel count
//#define APC_DCH_IN_COUNT    (1)
//#define APC_DCH_OUT_COUNT   (1)
#define APC_CH_IN_COUNT     (2)
#define APC_CH_OUT_COUNT    (2)

//JUST HERE!! BSD2024.1.23.

// APC channel bitmap
#define APC_DCH_BMP_LEFT                (0x1 << 0)
#define APC_DCH_BMP_RIGHT               (0x1 << 1)
#define APC_DCH_BMP_STEREO              (APC_DCH_BMP_LEFT | APC_DCH_BMP_RIGHT)
#define APC_DCH_BMP_MASK                APC_DCH_BMP_STEREO // 0x3

// APC channel <=> APC dual_channel
#define APC_CH_TO_DCH(ch)           (ch >> 1)
#define APC_CH_LR_IDX(ch)           (ch & 0x1)
#define APC_CH_LR_BMP(ch)           ((ch & 0x1) ? APC_DCH_BMP_RIGHT : APC_DCH_BMP_LEFT)
#define APC_DCH_TO_CH(dch, lr_idx)  ((dch << 1) + (lr_idx & 0x1))
#define APC_IS_LEFT_CH(ch)          ((ch & 0x1) ? 0 : 1)
#define APC_IS_RIGHT_CH(ch)         ((ch & 0x1) ? 1 : 0)

// channel mode
typedef enum {
    APC_CHMODE_16BITS = 0,      // 16-bit mono mode, {L1, L0} or {R1, R0} or {R1, L0}
    APC_CHMODE_24BITS_LOW = 1,  // 24-bit mono mode, {8'd0, L0} or {8'd0, R0}
    APC_CHMODE_32BITS = 2,      // 32-bit mono mode, L0 or R0
    APC_CHMODE_24BITS_HIGH = 3, // 24-bit mono mode, {L0, 8'd0} or {R0, 8'd0}
    APC_CHMODE_COUNT
} APC_CHMODE;

// Audio buffer used in data transfer (read/write) based on DMA LLP
// NOTE: AUDIO_BUFFER_LLI (array) SHOULD NOT in the stack, and SHOULD NOT
//      be released until data transfer is completed or aborted!
// See Also: AUDIO_BUFFER_USER (defined in Driver_Common.h, for driver's caller)
typedef struct {
    uint32_t* sample_data;
    uint32_t  sample_cnt; // SHOULD be EVEN when 16-bit sample!!
    DMA_LLI  dma_lli; // DON'T TOUCH IT! reserved for DMA driver only
} AUDIO_BUFFER_LLI; //TODO: TO BE CHANGED LATER!!

// APC Event Type, used by APC callback CSK_APC_SignalEvent_t (see below)
// an APC channel read/write is completed
#define APC_EVENT_TRANSFER_COMPLETE     (0x1UL << 0)
// RX FIFO overflow is found during an APC channel read/write
#define APC_EVENT_RX_FIFO_OVERRUN       (0x1UL << 1)
// TX FIFO underflow is found during an APC channel read/write
#define APC_EVENT_TX_FIFO_UNDERRUN      (0x1UL << 2)
// RX FIFO full is found during an APC channel read/write
#define APC_EVENT_RX_FIFO_FULL          (0x1UL << 3)
// TX FIFO empty is found during an APC channel read/write
#define APC_EVENT_TX_FIFO_EMPTY         (0x1UL << 4)

// an APC channel block read/write is completed (based on DMA block interrupt)
#define APC_EVENT_BLOCK_COMPLETE        (0x1UL << 5)

// I2S signal error, e.g. glitch on BCK/LRCK signal
#define APC_EVENT_I2S_ERROR             (0x1UL << 6)

// a DMA error is found during an APC channel read/write
#define APC_EVENT_DMA_ERROR             (0x1UL << 7)


/**
 \fn          void CSK_APC_SignalEvent_t (uint32_t event, uint32_t usr_param)
 \brief       Signal APC Events.
 \param[in]   event_info APC event and channel information
              bit[7:0] is event type, bit[15:8] is APC channel no.
 \param[in]   usr_param    user parameter specified in read / write operation.
 \return      none
*/
typedef void
(*CSK_APC_SignalEvent_t)(uint32_t event_info, uint32_t usr_param);


/**
  \fn          int32_t apc_initialize (void)
  \brief       Initialize Audio Processing Center
  \returns
   - \b  0: function succeeded
   - \b -1: function failed
*/
int32_t apc_initialize (void);


/**
  \fn          int32_t apc_uninitialize (void)
  \brief       De-initialize Audio Processing Center
  \returns
   - \b  0: function succeeded
   - \b -1: function failed
*/
int32_t apc_uninitialize (void);


/**
  \fn          int32_t apc_channel_setup ()
  \brief       Configure APC channel to transfer data for ADC_PDM(AMIC, DMIC), CLASSD, I2S and ECHO etc.
               NOTE: the last setup is vaild if the function is called many times
                    for the same APC channel.
  \param[in]   ch           The selected APC Channel, see definitions of APC Channel.
  \param[in]   itf_type     one of interface type, ADC_PDM, CLASSD, I2S, or ECHO.
  \param[in]   cb_event  Pointer to \ref CSK_APC_SignalEvent_t
  \param[in]   usr_param  User-defined value, acts as last parameter of cb_event
  \returns
   - \b  0: function succeeded
   - \b -1: function failed
*/

// generally called in XXX_Initialize()
/*
int32_t apc_channel_acquire (APC_CH    ch,
                           APC_INTF_TYPE    itf_type,
                           APC_INTF_INDEX   itf_idx,
                           CSK_APC_SignalEvent_t    cb_event,
                           uint32_t         usr_param);
*/
int32_t apc_channel_acquire (APC_CH ch,
                           APC_INTF_TYPE itf_type, // interface type
                           //APC_INTF_INDEX itf_idx,
                           APC_INTF_CHBMP itf_ch, // interface channel
                           uint8_t dma_ch, // preferred DMA channel
                           CSK_APC_SignalEvent_t cb_event,
                           uint32_t usr_param);

// generally called in XXX_Uninitialize()
//int32_t apc_dual_channel_release (APC_DCH    dch);
int32_t apc_channel_release (APC_CH ch);

// retrieve interface type, device instance of the type, and channel direction for APC channel
/*
int32_t apc_dual_channel_owner (APC_DCH dch,
                            APC_INTF_TYPE *itf_type_p,
                            APC_INTF_INDEX *itf_idx_p,
                            uint8_t *ch_dir_p); // 0=OUT, 1=IN
*/
int32_t apc_channel_owner (APC_CH ch,
                           APC_INTF_TYPE *itf_type_p,
                           //APC_INTF_INDEX *itf_idx_p,
                           APC_INTF_CHBMP *itf_ch_p,
                            uint8_t *ch_dir_p);

uint8_t apc_channel_dir (APC_CH ch); // for fast call

// generally called in XXX_Control()
/*
int32_t apc_dual_channel_setup (APC_DCH     dch,
                           APC_CHMODE       ch_mode, // 16, 24 MSB, 32 or 24 LSB?
                           uint8_t          ch_sel, // select APC_LEFT_CHANNEL, APC_RIGHT_CHANNEL, or APC_STEREO_CHANNEL?
                           uint8_t          ch_mix, // read L/R channel as a whole, or read L/R channel respectively
                           uint8_t          trim_16bits); // only valid for 24 MSB and 32 channel mode, when set to 1:
                                                         // record: trim low 16bits, get 16bits audio data
                                                         // playback: 16bits audio data is placed at high 16bits of WORD
*/
/*
int32_t apc_channel_setup (APC_CH     ch,
                           APC_CHMODE ch_mode, // 16, 24 MSB, 32 or 24 LSB?
                           uint8_t    ch_mix, // read L/R channel as a whole, or read L/R channel respectively
                           uint8_t    trim_16bits); // only valid for 24 MSB and 32 channel mode, when set to 1:
                                                         // record: trim low 16bits, get 16bits audio data
                                                         // playback: 16bits audio data is placed at high 16bits of WORD
*/
// APC channel Flag
//
//bit[0]: only valid for 24bit MSB and 32bit channel mode, when set to 1:
//  record (RX): trim low 16bits, get 16bits audio data
//  playback (TX): 16bits audio data is placed at high 16bits of WORD
//
//bit[1]: only valid for 16bit channel mode,
//  merge two 16bits samples as one WORD in APC FIFO when set to 1
//
//bit[2]: use PIO instead of DMA operation if set to 1
//
#define APCH_FLAG_TRIM_16BITS       0x1
#define APCH_FLAG_LONE_16BITS       0x2
#define APCH_FLAG_USE_PIO           0x4 //TODO: will be added later...

int32_t apc_channel_setup (APC_CH     ch,
                           APC_CHMODE ch_mode, // 16, 24 LSB, 32 or 24 MSB?
                           uint8_t    ch_mix, // R/W L/R channel as a whole, or R/W L/R channel respectively
                           uint8_t    ch_flag);

// echo setting for TX (TX FIFO0 and FIFO1 both support echo currently)
/*
int32_t apc_echo_setup (APC_DCH  dch,
                        uint32_t tx_samp_rate,
                        uint32_t echo_samp_rate,
                        uint8_t auto_feed);
*/
int32_t apc_echo_setup (APC_CH  dch,
                        uint32_t tx_samp_rate,
                        uint32_t echo_samp_rate,
                        uint8_t auto_feed);

// NOTE: For following apc_channel_xxxxx API functions,
//      all sample count SHOULD be EVEN for 16-bit sample.
int32_t apc_channel_read (APC_CH ch, uint32_t *sample_data, uint32_t  sample_cnt);
int32_t apc_channel_write (APC_CH ch, const uint32_t* sample_data, uint32_t  sample_cnt);

int32_t apc_channel_read_pipo (APC_CH ch, PIPO_IN_BLOCK *aud_blks, uint8_t *blk_cnt_p);
int32_t apc_channel_write_pipo (APC_CH ch, PIPO_OUT_BLOCK *aud_blks, uint8_t *blk_cnt_p);

// cancel the circular Ping/Ping IN or OUT operation,
// and just break the circular chain, NOT stop IN/OUT operation right now!
int32_t apc_channel_cancel_pipo (APC_CH ch);

// return count of transferred block if >= 0, else return the error value.
int32_t apc_channel_get_pipo_blks(APC_CH ch, PIPO_IO_BLOCK *aud_blks, uint8_t blk_cnt);

// ON MARS DMA DON'T support Scatter/Gather, so remove the last 2 parameters!!
/*
int32_t apc_channel_read (APC_CH    ch,
                          uint32_t  *sample_data,
                          uint32_t  sample_cnt);
//                          uint32_t  buf_offset, // byte offset to each buffer (default 0, NO SG support)
//                          uint32_t  dst_scat); // destination scatter setting (default 0, NO SG support)

int32_t apc_channel_write (APC_CH    ch,
                           const uint32_t* sample_data,
                           uint32_t  sample_cnt);
//                           uint32_t buf_offset, // byte offset to each buffer (default 0, NO SG support)
//                           uint32_t src_gath); // source gather setting (default 0, NO SG support)
*/


// read channel data based on DMA Linked List Pointer (multiple buffers) //TODO:
int32_t apc_channel_read_LLP (APC_CH ch,
                            AUDIO_BUFFER_LLI * bufs,
                            uint32_t buf_cnt);
//                            uint32_t buf_offset, // byte offset to each buffer (default 0, NO SG support)
//                            uint32_t dst_scat); // destination scatter setting (default 0, NO SG support)

// write channel data based on DMA Linked List Pointer (multiple buffers) //TODO:
int32_t apc_channel_write_LLP (APC_CH ch,
                              AUDIO_BUFFER_LLI * bufs,
                              uint32_t buf_cnt);
//                              uint32_t buf_offset, // byte offset to each buffer (default 0, NO SG support)
//                              uint32_t src_gath); // source gather setting (default 0, NO SG support)

/*
int32_t apc_dual_channel_read (APC_DCH  dch,
                              uint32_t  *sample_data,
                              uint32_t  sample_cnt);
//                              uint32_t  buf_offset, // byte offset to each buffer (default 0, NO SG support)
//                              uint32_t  dst_scat); // destination scatter setting (default 0, NO SG support)

int32_t apc_dual_channel_write (APC_DCH dch,
                               const uint32_t *sample_data,
                               uint32_t sample_cnt);
//                               uint32_t buf_offset, // byte offset to each buffer (default 0, NO SG support)
//                               uint32_t src_gath); // source gather setting (default 0, NO SG support)
*/

/*
// read channel data based on DMA Linked List Pointer (multiple buffers)
int32_t apc_dual_channel_read_LLP (APC_DCH dch,
                                AUDIO_BUFFER_LLI * bufs,
                                uint32_t buf_cnt);
//                                uint32_t buf_offset, // byte offset to each buffer (default 0, NO SG support)
//                                uint32_t dst_scat); // destination scatter setting (default 0, NO SG support)

// write channel data based on DMA Linked List Pointer (multiple buffers)
int32_t apc_dual_channel_write_LLP (APC_DCH dch,
                                  AUDIO_BUFFER_LLI * bufs,
                                  uint32_t buf_cnt);
//                                  uint32_t buf_offset, // byte offset to each buffer (default 0, NO SG support)
//                                  uint32_t src_gath); // source gather setting (default 0, NO SG support)
*/

//// read mixed data of APC channels 0 ~ 3 (4 IN channels)
//// sample_cnt is the count of uint16 or uint32, and SHOULD be EVEN when 16-bit sample!
//typedef enum {
//     QCH_ADC01_ADC23 = 0,
//     QCH_ADC01_ECHO,
//     QCH_ADC23_ECHO,
//     QCH_TYPE_COUNT
//} QCH_TYPE;

//int32_t apc_read_quad_channels (QCH_TYPE qch_type, uint32_t* sample_data, uint32_t  sample_cnt);
//
//// read mixed data of APC channels 0 ~ 5 (4 IN + 2 ECHO)
//// sample_cnt is the count of uint16 or uint32, and SHOULD be EVEN when 16-bit sample!
//int32_t apc_read_six_channels (uint32_t* sample_data, uint32_t  sample_cnt);

/**
  \fn          uint32_t apc_channel_get_count (APC_CH ch)
  \brief       Get number of transferred data items
  \param[in]   ch Channel number
  \returns     Number of transferred data items
*/
uint32_t apc_channel_get_count (APC_CH ch);
//uint32_t apc_dual_channel_get_count (APC_DCH dch);

int32_t apc_channel_enable (APC_CH ch); // enable APC channel only
int32_t apc_channel_disable (APC_CH ch); // disable APC channel only
int32_t apc_channel_abort (APC_CH ch); // abort APC channel data transfer only
// en = 1, set NOT_SYNC_CACHE; en = 0, clear NOT_SYNC_CACHE
void apc_channel_set_nsynca (APC_CH ch, uint8_t en);

// set TX channel source select to play back 1/2 channels' audio data via 1/2 CLASSD/DAC devices.
// (set .TX_CHL_SRC_SEL or .TX_CHR_SRC_SEL of TX_CH_CFG register value)
typedef enum {
    APC_TX_SRC_BOTH = 0x0, // left data and right data are both valid
    APC_TX_SRC_LEFT = 0x1, // only left data is valid, right data is 0
    APC_TX_SRC_RIGHT = 0x2, // only right data is valid, left data is 0
    APC_TX_SRC_COUNT
}  APC_TX_SRC_SEL;

// set TX channel source select
int32_t apc_channel_tx_src(APC_CH ch, APC_TX_SRC_SEL tx_src);

/*
int32_t apc_dual_channel_enable (APC_DCH dch); // enable APC dual_channel only
int32_t apc_dual_channel_disable (APC_DCH dch); // disable APC dual_channel only
int32_t apc_dual_channel_abort (APC_DCH dch); // abort APC dual_channel data transfer only
// en = 1, set NOT_SYNC_CACHE; en = 0, clear NOT_SYNC_CACHE
void apc_dual_channel_set_nsynca (APC_DCH dch, uint8_t en);
*/

//int32_t apc_eq_set_coef_array(APC_DCH dch, uint32_t *eqcoefs, uint32_t num);
//int32_t apc_eq_set_coef(APC_DCH dch, uint32_t index, uint32_t eqcoef);
//int32_t apc_eq_enable(APC_DCH dch, uint32_t stages);
//int32_t apc_eq_disble(APC_DCH dch);
//int32_t apc_eq_clear(APC_DCH dch, uint8_t wait_done);

//------------------------------------------------------------
// adc/dac clock related API
//------------------------------------------------------------

//#define SYSCTRL_CFG     ((SYSCFG_RegDef*) CMN_SYSCTRL_BASE)
//#define AUDPLL_CFG      ((AUDPLL_CTRL_RegDef*) CMN_AUDPLL_CTRL_BASE)

//typedef enum {
//    AUDIO_CLK_SRC_XTAL = 0, // from XTAL 24MHz [default]
//    AUDIO_CLK_SRC_AUDPLL = 1 // from AUDPLL output
//} AUDIO_CLK_SRC;

typedef enum {
    AUDIO_CLK_SRC_RC24M = 0, // from RC 24MHz [default]
    AUDIO_CLK_SRC_PLL = 1, // from PLL (24MHz) output
    AUDIO_CLK_SRC_XO24M = 2, // from XTAL 24MHz
} AUDIO_CLK_SRC;

//static inline void adc_clk_enable() {
//    IP_SYSCTRL->REG_MISC_CLK_CFG2.bit.ENA_CODEC_CLK_ADC = 1;
//}
//
//static inline void adc_clk_disable() {
//    IP_SYSCTRL->REG_MISC_CLK_CFG2.bit.ENA_CODEC_CLK_ADC = 0;
//}

//static inline void dac_clk_enable() {
//    IP_SYSCTRL->REG_MISC_CLK_CFG2.bit.ENA_CODEC_CLK_DAC = 1;
//}
//
//static inline void dac_clk_disable() {
//    IP_SYSCTRL->REG_MISC_CLK_CFG2.bit.ENA_CODEC_CLK_DAC = 0;
//}
//
//static inline void adc_clk_select(AUDIO_CLK_SRC src) {
//    IP_SYSCTRL->REG_MISC_CLK_CFG2.bit.SEL_CODEC_CLK_ADC = src & 0x1;
//}
//
//static inline void dac_clk_select(AUDIO_CLK_SRC src) {
//    IP_SYSCTRL->REG_MISC_CLK_CFG2.bit.SEL_CODEC_CLK_DAC = src & 0x1;
//}
//
//static inline AUDIO_CLK_SRC adc_clk_src() {
//    return (AUDIO_CLK_SRC)IP_SYSCTRL->REG_MISC_CLK_CFG2.bit.SEL_CODEC_CLK_ADC;
//}
//
//static inline AUDIO_CLK_SRC dac_clk_src() {
//    return (AUDIO_CLK_SRC)IP_SYSCTRL->REG_MISC_CLK_CFG2.bit.SEL_CODEC_CLK_DAC;
//}

#endif /* __APC_MARS_H */
