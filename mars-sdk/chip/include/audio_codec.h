/*
 * audio_codec.h
 *
 *  Created on: Mar. 9, 2021 for VENUS (Common code shared by AON_ADC, ADC and DAC.)
 *  Ported on: Feb. 16, 2024 for MARS (Common code shared by ADC and CLASSD.)
 *
 */

#ifndef __AUDIO_CODEC_H
#define __AUDIO_CODEC_H

#include "mars.h"
#include "audio_codec_reg.h" // for CODEC registers (AUDIO_CODEC_RegDef)
//#include "ana_aon_regfile_reg_venus.h" // for 2.5v ldo control (needed by CODEC)

//=============================== AUDIO_CODEC Register Map ===============================

// R1_GLOBAL0
#define AUD_R1_GLOBAL0_BITS     AUDIO_CODEC_REG_AUD_R1_GLOBAL0_BITS
#define AUD_R1_GLOBAL0          AUDIO_CODEC_REG_AUD_R1_GLOBAL0

/*
struct AUD_R1_GLOBAL0_BITS
{
    volatile uint32_t DEBUG_MODE                    : 3; // bit 0~2
    volatile uint32_t TEST_IN_SEL                   : 3; // bit 3~5
    volatile uint32_t TEST_OUT_SELB                 : 4; // bit 6~9
    volatile uint32_t TEST_OUT_SELA                 : 5; // bit 10~14
    volatile uint32_t RESV_15_31                    : 17; // bit 15~31
};

union AUD_R1_GLOBAL0 {
    volatile uint32_t               all;
    struct AUD_R1_GLOBAL0_BITS      bit;
};
*/


// R2_GLOBAL1
#define AUD_R2_GLOBAL1_BITS     AUDIO_CODEC_REG_AUD_R2_GLOBAL1_BITS
#define AUD_R2_GLOBAL1          AUDIO_CODEC_REG_AUD_R2_GLOBAL1

#define R2_AUD_EN_IREF          (0x1 << 0)
#define R2_AUD_EN_VMID          (0x1 << 1)

/*
struct R2_GLOBAL1_BITS
{
    volatile uint32_t AUD_EN_IREF                   : 1; // bit 0~0
    volatile uint32_t AUD_EN_VMID                   : 1; // bit 1~1
    volatile uint32_t RESV_2_31                     : 30; // bit 2~31
};

union AUD_R2_GLOBAL1 {
    volatile uint32_t           all;
    struct R2_GLOBAL1_BITS      bit;
};
*/


// R5 ADC_CTRL0
#define AUD_R5_ADC_CTRL0_BITS   AUDIO_CODEC_REG_AUD_R5_ADC_CTRL0_BITS
#define AUD_R5_ADC_CTRL0        AUDIO_CODEC_REG_AUD_R5_ADC_CTRL0

#define BIT_INDEX_ADCCLK_EN     7
#define R5_ADCCLK_EN     (1 << 7)

/*
struct AUD_R5_ADC_CTRL0_BITS
{
    volatile uint32_t ADCSR                         : 4; // bit 0~3
    volatile uint32_t ADCOSR                        : 3; // bit 4~6
    volatile uint32_t ADCCLK_EN                     : 1; // bit 7~7
    volatile uint32_t REG_ADC_RSTN                  : 1; // bit 8~8
    volatile uint32_t PGA_TOEN                      : 1; // bit 9~9
    volatile uint32_t ADC_CAP_CALI_GO               : 1; // bit 10~10
    volatile uint32_t ADC_GAIN_COMP_SEL             : 1; // bit 11~11
    volatile uint32_t RESV_12_31                    : 20; // bit 12~31
};

union AUD_R5_ADC_CTRL0 {
    volatile uint32_t               all;
    struct AUD_R5_ADC_CTRL0_BITS    bit;
};
*/


// R6 ADC_CTRL1
#define AUD_R6_ADC_CTRL1_BITS   AUDIO_CODEC_REG_AUD_R6_ADC_CTRL1_BITS
#define AUD_R6_ADC_CTRL1        AUDIO_CODEC_REG_AUD_R6_ADC_CTRL1

#define R6_ADCR_VOL_MASK        (0x7F << 0)
#define R6_ADCL_VOL_MASK        (0x7F << 7)
#define R6_ADCR_VOL(n)          (((n) & 0x7F) << 0) // bit[6:0]
#define R6_ADCL_VOL(n)          (((n) & 0x7F) << 7) // bit[13:7]
#define R6_ADC_VOL_MIN          0x0     // -83dB
#define R6_ADC_VOL_MAX          0x7F    // +42dB

#define R6_HPF2_EN              (1 << 14)   // bit[14]
#define R6_HPF1_EN              (1 << 15)   // bit[15]
#define R6_ADC_PGA_MASK         (0x1F << 16)
#define R6_ADC_PGA_VOL(n)      (((n) & 0x1F) << 16)    // bit[20:16]
#define R6_ADC_PGA_VOL_MIN      0x0     // -12dB
#define R6_ADC_PGA_VOL_MAX      0x18    // +36dB
#define R6_HPF_CUT(n)           (((n) & 0x7) << 21)     // bit[23:21]
#define R6_ADC_SINGLE_CH_MODE   (1 << 24)   // bit[24]
#define R6_HPF_OUT_SEL          (1 << 25)   // bit[25]

/*
struct AUD_R6_ADC_CTRL1_BITS
{
    volatile uint32_t ADCVOL_R                      : 7; // bit 0~6
    volatile uint32_t ADCVOL_L                      : 7; // bit 7~13
    volatile uint32_t HPF2EN                        : 1; // bit 14~14
    volatile uint32_t HPF1EN                        : 1; // bit 15~15
    volatile uint32_t ADC_PGA_LEVEL                 : 5; // bit 16~20
    volatile uint32_t HPFCUT                        : 3; // bit 21~23
    volatile uint32_t ADC_SINGLE_CH_MODE            : 1; // bit 24~24
    volatile uint32_t ADC_HPFOUT_SEL                : 1; // bit 25~25
    volatile uint32_t RESV_26_31                    : 6; // bit 26~31
};

union AUD_R6_ADC_CTRL1 {
    volatile uint32_t               all;
    struct AUD_R6_ADC_CTRL1_BITS    bit;
};
*/


// R7 ADC_CTRL2
#define AUD_R7_ADC_CTRL2_BITS   AUDIO_CODEC_REG_AUD_R7_ADC_CTRL2_BITS
#define AUD_R7_ADC_CTRL2        AUDIO_CODEC_REG_AUD_R7_ADC_CTRL2

/*
struct AUD_R7_ADC_CTRL2_BITS
{
    volatile uint32_t NFA0                          : 14; // bit 0~13
    volatile uint32_t NFEN                          : 1; // bit 14~14
    volatile uint32_t RESV_15_15                    : 1; // bit 15~15
    volatile uint32_t NFA1                          : 14; // bit 16~29
    volatile uint32_t RESV_30_31                    : 2; // bit 30~31
};

union AUD_R7_ADC_CTRL2 {
    volatile uint32_t                               all;
    struct AUD_R7_ADC_CTRL2_BITS    bit;
};
*/


// R8 ADC_CTRL3
#define AUD_R8_ADC_CTRL3_BITS   AUDIO_CODEC_REG_AUD_R8_ADC_CTRL3_BITS
#define AUD_R8_ADC_CTRL3        AUDIO_CODEC_REG_AUD_R8_ADC_CTRL3

#define R8_ERR_TOLERANCE_MIN                0
#define R8_ERR_TOLERANCE_MAX                4 //FIXME: 7?
#define R8_TARGET_LEVEL_MIN                 0
#define R8_TARGET_LEVEL_MAX                 23
#define R8_NGATE_FLOOR_MIN                  0
#define R8_NGATE_FLOOR_MAX                  22
#define R8_ALCMIN_MIN                       0
#define R8_ALCMIN_MAX                       24
#define R8_ALCMAX_MIN                       0
#define R8_ALCMAX_MAX                       24

/*
struct AUD_R8_ADC_CTRL3_BITS
{
    volatile uint32_t ALCMIN                        : 5; // bit 0~4
    volatile uint32_t ALCMAX                        : 5; // bit 5~9
    volatile uint32_t NG                            : 5; // bit 10~14
    volatile uint32_t NG_EN                         : 1; // bit 15~15
    volatile uint32_t ALCSEL                        : 1; // bit 16~16
    volatile uint32_t ALCMODE                       : 1; // bit 17~17
    volatile uint32_t TARGET                        : 5; // bit 18~22
    volatile uint32_t TOLERANCE                     : 3; // bit 23~25
    volatile uint32_t RESV_26_31                    : 6; // bit 26~31
};

union AUD_R8_ADC_CTRL3 {
    volatile uint32_t                   all;
    struct AUD_R8_ADC_CTRL3_BITS        bit;
};
*/


// R9 ADC_CTRL4
#define AUD_R9_ADC_CTRL4_BITS   AUDIO_CODEC_REG_AUD_R9_ADC_CTRL4_BITS
#define AUD_R9_ADC_CTRL4        AUDIO_CODEC_REG_AUD_R9_ADC_CTRL4

#define R9_ALC_DECAY_MIN        0x0
#define R9_ALC_DECAY_MAX        0xF
#define R9_ALC_ATTACK_MIN       0x0
#define R9_ALC_ATTACK_MAX       0xF
#define R9_ALC_HOLD_MIN         0x0
#define R9_ALC_HOLD_MAX         0xF

#define R9_ALC_DECAY(n)         (((n) & 0xF) << 0)
#define R9_ALC_ATTACK(n)        (((n) & 0xF) << 4)
#define R9_ALC_HOLD(n)          (((n) & 0xF) << 8)
#define R9_PEAK_FASTALC_EN      (1 << 12)

//TODO: check if only "double edge" is implemented in CODEC IP?
// 1: double edge on DMIC0 or DMIC1, 0: single edge on both DMIC0 & DMIC1
#define R9_DMIC_MODE_DBL_EDGE   (1 << 16)
// 1: from DMIC1, 0: from DMIC0 only when DMIC_MODE=1
#define R9_DMIC_SRC_DMIC1       (1 << 21)
#define R9_DMIC_SRC_DMIC0       (0 << 21)
#define R9_DMIC_EN              (1 << 22)

#define R9_AUTORST_TYPE(n)      (((n) & 0x7) << 23)     //0x0~0x5, default 0x1 (256us)
#define R9_AUTORST_EN           (1 << 26)

/*
struct AUD_R9_ADC_CTRL4_BITS
{
    volatile uint32_t ALCDCY                        : 4; // bit 0~3
    volatile uint32_t ALCATK                        : 4; // bit 4~7
    volatile uint32_t ALCHLD                        : 4; // bit 8~11
    volatile uint32_t PEAK_FASTALC_EN               : 1; // bit 12~12
    volatile uint32_t RESV_13_15                    : 3; // bit 13~15
    volatile uint32_t DMIC_MODE                     : 1; // bit 16~16
    volatile uint32_t UNCONNECT                     : 2; // bit 17~18
    volatile uint32_t DMIC_LATCH_ADJ                : 2; // bit 19~20
    volatile uint32_t DMIC_SRC                      : 1; // bit 21~21
    volatile uint32_t DMIC_ENABLE                   : 1; // bit 22~22
    volatile uint32_t AUTORST_TYPE                  : 3; // bit 23~25
    volatile uint32_t AUTORST_EN                    : 1; // bit 26~26
    volatile uint32_t RESV_27_31                    : 5; // bit 27~31
};

union AUD_R9_ADC_CTRL4 {
    volatile uint32_t               all;
    struct AUD_R9_ADC_CTRL4_BITS    bit;
};
*/


// R10 ADC_CTR5
#define AUD_R10_ADC_CTRL5_BITS  AUDIO_CODEC_REG_AUD_R10_ADC_CTRL5_BITS
#define AUD_R10_ADC_CTRL5       AUDIO_CODEC_REG_AUD_R10_ADC_CTRL5

#define R10_FILGAIN_REG_MASK    (0xFFFFF << 0)  // bit[19:0]
#define R10_FILGAIN_REG(n)      (((n) & 0xFFFFF) << 0)
#define R10_FILGAIN_REGEN       (0x1 << 20)     // bit[20]

/*
struct AUD_R10_ADC_CTRL5_BITS
{
    volatile uint32_t FILGAIN_REG                   : 20; // bit 0~19
    volatile uint32_t FILGAIN_REGEN                 : 1; // bit 20~20
    volatile uint32_t OFFSET_REG                    : 10; // bit 21~30
    volatile uint32_t OFFSET_REGEN                  : 1; // bit 31~31
};

union AUD_R10_ADC_CTRL5 {
    volatile uint32_t                     all;
    struct AUD_R10_ADC_CTRL5_BITS         bit;
};
*/


// R11 ADC_CTRL6
#define AUD_R11_ADC_CTRL6_BITS  AUDIO_CODEC_REG_AUD_R11_ADC_CTRL6_BITS
#define AUD_R11_ADC_CTRL6       AUDIO_CODEC_REG_AUD_R11_ADC_CTRL6

#define R11_ADC_CLK_EN          (1 << 0) // bit[0] // Analog CLK
#define R11_ADC_EN              (1 << 1) // bit[1]

#define R11_ADC_CLK_SRC_MASK    (0x3 << 2) // bit[3:2]
#define R11_ADC_CLK_SRC(n)      (((n) & 0x3) << 2)

#define R11_ADC_CLK_LVL_SEL     (0x1 << 4) // bit[4] //FIXME: what's it for?
#define R11_ADC_CLK_INV         (0x1 << 5) // bit[5]
#define R11_ADC_ANACLK_INV      (0x1 << 6) // bit[6]
#define R11_ADC_ANACLK_BUF      (0x0 << 6) // bit[6]
#define R11_ADC_ANA_RST         (0x1 << 7) // bit[7], [RW]ADC analog path reset
#define R11_ADC_LP_AUTORST_SPLIT    (0x1 << 8) // bit[8] //FIXME: what's it for?

#define R11_ADC_IB_CTRL_MASK    (0x3 << 9) // bit[10:9], IB=IBIAS
#define R11_ADC_IB_CTRL(n)      (((n) & 0x3) << 9)

#define IB_CTRL_1P5UA           0x0
#define IB_CTRL_2UA             0x1
#define IB_CTRL_2P5UA           0x2
#define IB_CTRL_3UA             0x3
#define IB_CTRL_VAL_DEF         IB_CTRL_2UA
#define IB_CTRL_VAL_12MHZ       IB_CTRL_2P5UA //FIXME: make sure to work under limiting conditions
#define IB_CTRL_VAL_LP          IB_CTRL_1P5UA //FIXME:

#define R11_ADC_IDAC_CTRL_MASK  (0x3 << 11) // bit[12:11]
#define R11_ADC_IDAC_CTRL(n)    (((n) & 0x3) << 11)

#define IDAC_7P5UA_M5P          (0x0)
#define IDAC_7P5UA              (0x1)
#define IDAC_7P5UA_P6P          (0x2)
#define IDAC_7P5UA_P13P         (0x3)
//#define R11_ADC_IDAC_7P5UA_M5PER    (IDAC_7P5UA_M5P << 7)
//#define R11_ADC_IDAC_7P5UA          (IDAC_7P5UA << 7)
//#define R11_ADC_IDAC_7P5UA_P6PER    (IDAC_7P5UA_P6P << 7)
//#define R11_ADC_IDAC_7P5UA_P13PER   (IDAC_7P5UA_P13P << 7)

#define R11_ADC_CAP_CALI_EN_SRC_MASK    (0x1 << 13) // bit[13]
#define R11_ADC_CAP_CALI_EN_SRC_REG     (0x1 << 13) // enable from register
#define R11_ADC_CAP_CALI_EN_SRC_ISM     (0x0 << 13) // enable from internal state machine?
#define R11_ADC_CAP_CALI_EN_REG_MASK    (0x1 << 14) // bit[14]
#define R11_ADC_CAP_CALI_EN_REG_ENA     (0x1 << 14) // enable
#define R11_ADC_CAP_CALI_EN_REG_DIS     (0x0 << 14) // disable

#define R11_ADC_CAP_CALI_REGVAL_MASK    (0x1F << 15) // bit[19:15]
#define R11_ADC_CAP_CALI_REGVAl(n)      (((n) & 0x1F) << 15)
#define R11_ADC_CAP_CALI_SRC_MASK       (0x1 << 20)
#define R11_ADC_CAP_CALI_SRC_REG        (0x1 << 20) // CALI setting from register
#define R11_ADC_CAP_CALI_SRC_ISM        (0x0 << 20) // CALI setting from internal state machine

/*
struct AUD_R11_ADC_CTRL6_BITS
{
    volatile uint32_t AUD_EN_ADC_CLK                : 1; // bit 0~0
    volatile uint32_t REG_AUD_EN_ADC                : 1; // bit 1~1
    volatile uint32_t AUD_ADC_CLK_SRC               : 2; // bit 2~3
    volatile uint32_t AUD_ADC_CLK_LVL_SEL           : 1; // bit 4~4
    volatile uint32_t AUD_ADC_CLK_INV               : 1; // bit 5~5
    volatile uint32_t REG_ADC_ANACLK_INV            : 1; // bit 6~6
    volatile uint32_t ANA_ADC_RST_REG               : 1; // bit 7~7
    volatile uint32_t ADC_LP_AUTORST_SPLIT          : 1; // bit 8~8
    volatile uint32_t AUD_ADC_IB_CTRL               : 2; // bit 9~10
    volatile uint32_t AUD_ADC_IDAC_CTRL             : 2; // bit 11~12
    volatile uint32_t ADC_CAP_CALI_EN_SRC           : 1; // bit 13~13
    volatile uint32_t ADC_CAP_CALI_EN_REG           : 1; // bit 14~14
    volatile uint32_t ADC_CAP_CALI_REG              : 5; // bit 15~19
    volatile uint32_t ADC_CAP_CALI_SRC              : 1; // bit 20~20
    volatile uint32_t RESV_21_31                    : 11; // bit 21~31
};

union AUD_R11_ADC_CTRL6 {
    volatile uint32_t               all;
    struct AUD_R11_ADC_CTRL6_BITS   bit;
};
*/


// R12 ADC_CTRL7
#define AUD_R12_ADC_CTRL7_BITS  AUDIO_CODEC_REG_AUD_R12_ADC_CTRL7_BITS
#define AUD_R12_ADC_CTRL7       AUDIO_CODEC_REG_AUD_R12_ADC_CTRL7

#define R12_PGA_VCMBUF_EN               (1 << 0) // bit[0]: Enable ADC PGA0 VCM Buffer
#define R12_PGA_VCOM_SEL_VMID_75PER     (1 << 1) // bit[1]: ADC PGA0 VCOM Select: VMID*0.75
#define R12_PGA_VCOM_SEL_VMID           (0 << 1) // bit[1]: ADC PGA0 VCOM Select: VMID (Default)

#define R12_PGA_EN                      (1 << 2) // bit[2]: Enable ADC PGA0
#define R12_PGA_MUTE                    (1 << 3) // bit[3]: Mute ADC PGA0
#define R12_PGA_SINGLE                  (1 << 4) // bit[4]: ADC PGA0 input mode: Single
#define R12_PGA_DIFF                    (0 << 4) // bit[4]: ADC PGA0 input mode: Differential
#define R12_PGA_LP                      (1 << 5) // bit[5]: ADC PGA0 power mode: Low Power
#define R12_PGA_NORM                    (0 << 5) // bit[5]: ADC PGA0 power mode: Normal
#define R12_PGA_ZCEN                    (1 << 6) // bit[6]: Enable ADC PGA0 Zero Crossing
#define R12_ADC_ATB_CTRL(n)             (((n) & 0x3) << 7) // bit[8:7], 0~2, default 0

#define R12_ADC_SAR_TEST_EN             (0x1 << 9) // bit[9]: Enable ADC SAR TEST
#define R12_ADC_SAR_DELAY_MASK          (0x7 << 10) // bit[12:10]: ADC SAR Delay Control
#define R12_ADC_SAR_DELAY_CTRL(n)        (((n) & 0x7) << 10) // 0x0~0x7: 6/7/8/9.5(default)/12/14/16/19ns
#define R12_ADC_SAR_COMP_LP             (0x1 << 13) // bit[13]: ADC SAR Comparator Low Power, 1 = Low Current
#define R12_ADC_INT2_LP                 (0x1 << 14) // bit[14]: ADC INTegrator 2 Low Power, 1 = Low Current (default)
#define R12_ADC_INT1_LP                 (0x1 << 15) // bit[15]: ADC INTegrator 1 Low Power, 1 = Low Current (default)

#define R12_ADC_IDAC_OS_MASK            (0x3 << 16) // bit[17:16]
#define R12_ADC_IDAC_OS_CTRL(n)         (((n) & 0x3) << 16)

#define IDAC_OS_0MV                 (0x0)
#define IDAC_OS_20MV                (0x1)
#define IDAC_OS_40MV                (0x2)
#define IDAC_OS_80MV                (0x3)
//#define R12_ADC_IDAC_OFFSET_0MV         (IDAC_OS_0MV << 16)
//#define R12_ADC_IDAC_OFFSET_20MV        (IDAC_OS_20MV << 16)
//#define R12_ADC_IDAC_OFFSET_40MV        (IDAC_OS_40MV << 16)
//#define R12_ADC_IDAC_OFFSET_80MV        (IDAC_OS_80MV << 16)

#define R12_IDAC_BIAS_SRC               (0x1 << 18) // bit[18], 1=from register value?
#define R12_IDAC_BIAS_REG               (0x1 << 19) // bit[19], register value?

#define R12_ADC_VREF_EN                 (0x1 << 20) // bit[20], Enable ADC0 VREF
#define R12_ADC_MODE_12MHZ              (0x1 << 21) // bit[21], 12MHz mode
#define R12_ADC_MODE_4MHZ               (0x0 << 21) // bit[21], 4MHz mode

/*
struct AUD_R12_ADC_CTRL7_BITS
{
    volatile uint32_t AUD_EN_PGA_VCMBUF             : 1; // bit 0~0
    volatile uint32_t AUD_PGA_VCOM_SEL              : 1; // bit 1~1
    volatile uint32_t REG_AUD_EN_PGA                : 1; // bit 2~2
    volatile uint32_t AUD_PGA_MUTE                  : 1; // bit 3~3
    volatile uint32_t AUD_EN_PGA_SINGLE             : 1; // bit 4~4
    volatile uint32_t AUD_PGA_LPR                   : 1; // bit 5~5
    volatile uint32_t PGA_ZCEN_REG                  : 1; // bit 6~6
    volatile uint32_t AUD_ADC_ATB_CTRL              : 2; // bit 7~8
    volatile uint32_t AUD_ADC_SAR_TEST_EN           : 1; // bit 9~9
    volatile uint32_t AUD_ADC_SAR_DELAY_CTRL        : 3; // bit 10~12
    volatile uint32_t AUD_ADC_SAR_COMP_LPR          : 1; // bit 13~13
    volatile uint32_t AUD_ADC_INT2_LPR              : 1; // bit 14~14
    volatile uint32_t AUD_ADC_INT1_LPR              : 1; // bit 15~15
    volatile uint32_t AUD_ADC_IDAC_OS_CTRL          : 2; // bit 16~17
    volatile uint32_t AUD_IDAC_BIAS_SRC             : 1; // bit 18~18
    volatile uint32_t AUD_IDAC_BIAS_REG             : 1; // bit 19~19
    volatile uint32_t AUD_EN_ADC_VREF               : 1; // bit 20~20
    volatile uint32_t AUD_ADC_MODE                  : 1; // bit 21~21
    volatile uint32_t RESV_22_31                    : 10; // bit 22~31
};

union AUD_R12_ADC_CTRL7 {
    volatile uint32_t               all;
    struct AUD_R12_ADC_CTRL7_BITS   bit;
};
*/


// R25 STATUS0
#define AUD_R25_STATUS0_BITS    AUDIO_CODEC_REG_AUD_R25_STATUS0_BITS
#define AUD_R25_STATUS0         AUDIO_CODEC_REG_AUD_R25_STATUS0

/*
struct AUD_R25_STATUS0_BITS
{
    volatile uint32_t AUD_ADC_CAP_CALI_FLAG_INST    : 1; // bit 0~0
    volatile uint32_t ADC_CAP_CALI_DONE             : 1; // bit 1~1
    volatile uint32_t ADC_CAP_CALI_FAIL             : 1; // bit 2~2
    volatile uint32_t AUD_ADC_CAP_CALI              : 5; // bit 3~7
    volatile uint32_t RESV_8_31                     : 24; // bit 8~31
};

union AUD_R25_STATUS0 {
    volatile uint32_t               all;
    struct AUD_R25_STATUS0_BITS     bit;
};
*/


//----------------------------------------------------------------------
typedef struct
{
    uint32_t                        REG_RSVD0; // 0x000
    union AUD_R1_GLOBAL0            REG_AUD_R1_GLOBAL0; // 0x004
    union AUD_R2_GLOBAL1            REG_AUD_R2_GLOBAL1; // 0x008

    uint32_t                        REG_RSVD1[2]; // 0x00C ~ 0x010
    union AUD_R5_ADC_CTRL0          REG_AUD_R5_ADC_CTRL0; // 0x014
    union AUD_R6_ADC_CTRL1          REG_AUD_R6_ADC_CTRL1; // 0x018
    union AUD_R7_ADC_CTRL2          REG_AUD_R7_ADC_CTRL2; // 0x01C
    union AUD_R8_ADC_CTRL3          REG_AUD_R8_ADC_CTRL3; // 0x020
    union AUD_R9_ADC_CTRL4          REG_AUD_R9_ADC_CTRL4; // 0x024
    union AUD_R10_ADC_CTRL5         REG_AUD_R10_ADC_CTRL5; // 0x028
    union AUD_R11_ADC_CTRL6         REG_AUD_R11_ADC_CTRL6; // 0x02C
    union AUD_R12_ADC_CTRL7         REG_AUD_R12_ADC_CTRL7; // 0x030

    uint32_t                        REG_RSVD2[4]; // 0x034 ~ 0x040
    union AUD_R25_STATUS0           REG_AUD_R25_STATUS0; // 0x044
} CSK_ADC_PDM_RegDef;

#define CSK_ADC01               ((CSK_ADC_PDM_RegDef *)AUDIO_CODEC_BASE)

//----------------------------------------------------------------------

// R15 CLD_CTRL0
#define AUD_R15_CLD_CTRL0_BITS  AUDIO_CODEC_REG_AUD_R15_CLD_CTRL0_BITS
#define AUD_R15_CLD_CTRL0       AUDIO_CODEC_REG_AUD_R15_CLD_CTRL0

#define R15_CLD_DIG_CLK_EN          (1 << 4) // bit[4]: Enable ClassD Digital CLK

#define R15_CLD_RSTN                (0 << 5) // bit[5]: ClassD Digital reset
#define R15_CLD_RELEASE             (1 << 5) // bit[5]: ClassD Digital release for reset

#define R15_REV_P                   (1 << 7) // bit[7]: invert P-channel signal internally
#define R15_REV_N                   (1 << 8) // bit[8]: invert N-channel signal internally

#define R15_OFF_VAL0                (0 << 9) // bit[9]: set Off value to 0 for P/N channel when REMAP is used
#define R15_OFF_VAL1                (1 << 9) // bit[9]: set Off value to 1 for P/N channel when REMAP is used
#define R15_USE_REMAP               (1 << 10) // bit[10]: use REMAP PWM version
#define R15_PWM_SEL_CENT_ALGN       (0 << 11) // bit[11]: central aligned
#define R15_PWM_SEL_EDGE_ALGN       (1 << 11) // bit[11]: edge aligned

/*
struct AUD_R15_CLD_CTRL0_BITS
{
    volatile uint32_t CLASSD_SR                     : 2; // bit 0~1
    volatile uint32_t CLASSD_OSR                    : 2; // bit 2~3
    volatile uint32_t CLASSD_CLK_EN                 : 1; // bit 4~4
    volatile uint32_t REG_CLD_RSTN                  : 1; // bit 5~5
    volatile uint32_t INI_LR_OUT                    : 1; // bit 6~6
    volatile uint32_t REV_P                         : 1; // bit 7~7
    volatile uint32_t REV_N                         : 1; // bit 8~8
    volatile uint32_t OFF_VAL                       : 1; // bit 9~9
    volatile uint32_t USE_REMAP_VER                 : 1; // bit 10~10
    volatile uint32_t PWM_SEL                       : 1; // bit 11~11
    volatile uint32_t RESV_12_31                    : 20; // bit 12~31
};

union AUD_R15_CLD_CTRL0 {
    volatile uint32_t               all;
    struct AUD_R15_CLD_CTRL0_BITS   bit;
};
*/


// R16 CLD_CTRL1
#define AUD_R16_CLD_CTRL1_BITS  AUDIO_CODEC_REG_AUD_R16_CLD_CTRL1_BITS
#define AUD_R16_CLD_CTRL1       AUDIO_CODEC_REG_AUD_R16_CLD_CTRL1

#define R16_CLD_CLK_EN                  (1 << 0) // bit[0]: Enable ClassD Analog CLK (AnaClk->DigClk)
#define R16_CLD_ANA_EN                  (1 << 13) // bit[13]: Enable ClassD Analog
#define R16_CLD_DIG_EN                  (1 << 14) // bit[14]: Enable ClassD Digital
#define R16_CLD_FULL_EN                 (R16_CLD_CLK_EN | R16_CLD_ANA_EN | R16_CLD_DIG_EN)

#define R16_CLD_MCLK_INVT               (1 << 3) // bit[3]: Invert ClassD MCLK
#define R16_CLD_HVLDO_EN                (1 << 4) // bit[4]: Enable ClassD HVLDO (5V?)
#define R16_CLD_SLEWCTL_MASK            (0x7 << 7) // bit[9:7]: ClassD Slew Rate Control Mask

#define R16_CLD_N_DATA1                 (1 << 15) // bit[15]: Force ClassD N-channel to 1
#define R16_CLD_P_DATA1                 (1 << 16) // bit[16]: Force ClassD P-channel to 1
#define R16_CLD_DATA_FRC                (1 << 17) // bit[17]: Use register value as P/N-channel output data

/*
struct AUD_R16_CLD_CTRL1_BITS
{
    volatile uint32_t AUD_EN_CLD_CLK                : 1; // bit 0~0
    volatile uint32_t AUD_CLD_MCLK_SRC              : 2; // bit 1~2
    volatile uint32_t AUD_CLD_CLK_INV               : 1; // bit 3~3
    volatile uint32_t CLASSD_HVLDO_EN               : 1; // bit 4~4
    volatile uint32_t CLASSD_MODE                   : 2; // bit 5~6
    volatile uint32_t CLASSD_SLEWCTL                : 3; // bit 7~9
    volatile uint32_t CLASSD_TEST_CTRL              : 3; // bit 10~12
    volatile uint32_t CLASSD_EN                     : 1; // bit 13~13
    volatile uint32_t CLASSD_ENABLE_DIG             : 1; // bit 14~14
    volatile uint32_t REG_CLASSD_DATA_N             : 1; // bit 15~15
    volatile uint32_t REG_CLASSD_DATA_P             : 1; // bit 16~16
    volatile uint32_t CLASSD_DATA_FORCE             : 1; // bit 17~17
    volatile uint32_t RESV_18_31                    : 14; // bit 18~31
};

union AUD_R16_CLD_CTRL1 {
    volatile uint32_t               all;
    struct AUD_R16_CLD_CTRL1_BITS   bit;
};
*/

//----------------------------------------------------------------------

typedef struct
{
    uint32_t                        REG_RSVD0; // 0x000
    union AUD_R1_GLOBAL0            REG_AUD_R1_GLOBAL0; // 0x004
    union AUD_R2_GLOBAL1            REG_AUD_R2_GLOBAL1; // 0x008

    uint32_t                        REG_RSVD1[12]; // 0x00C ~ 0x038
    union AUD_R15_CLD_CTRL0         REG_AUD_R15_CLD_CTRL0; // 0x03C
    union AUD_R16_CLD_CTRL1         REG_AUD_R16_CLD_CTRL1; // 0x040

    uint32_t                        REG_RSVD2; // 0x044
} CSK_CLASSD_RegDef;

#define CSK_CLASSD01                ((CSK_CLASSD_RegDef *)AUDIO_CODEC_BASE)

//====================== Register-related Macros & Functions =================

typedef struct {
    uint32_t reg_val;
    uint32_t real_val;
} REG_VAL_MAP;

typedef struct {
    uint32_t major;
    uint32_t minor;
} VAL_PAIR;

typedef struct {
    uint32_t major; // 1st value
    uint16_t minor; // 2nd value
    uint16_t least; // 3rd value
} VAL_TRIPLE;

#define ARRAY_COUNT(a)  (sizeof(a)/sizeof(a[0]))


static inline void codec_clk_enable() {
    //Enable Audio Codec Clock
    IP_SYSCTRL->REG_CLK_EN_CTRL0.bit.ENA_CODEC_CLK = 1;
}

static inline void codec_clk_disable() {
    //Disable Audio Codec Clock
    IP_SYSCTRL->REG_CLK_EN_CTRL0.bit.ENA_CODEC_CLK = 0;
}

static inline void ENABLE_CODEC_BASIC() {
    //FIXME: consult ANALOG guys about CODEC's external power & clock settings!!

    //IP_AON_CTRL->REG_XO24M_CTRL.bit.ENA_XO24M_SW = 1;
    IP_AON_CTRL->REG_AON_TUNE2.bit.EN_LDO_VA = 1;
    IP_AON_CTRL->REG_AON_TUNE2.bit.EN_CAPLESS_LDO_VA = 1;
    //IP_SYSCTRL->REG_CLK_EN_CTRL0.bit.ENA_CODEC_CLK = 1;
}

static inline void CONFIG_CODEC_ADC_ANALOG() {
    //FIXME: consult ANALOG guys about CODEC's analog settings!!
    // i.e. VMID, VREF, MICBIAS etc.

#if (IC_BOARD == 0) // FPGA, FOR TEST ONLY...
    // SDM 3bits input from DBG_IN, ADC_MCLK output from DBG_CLK

    // sdm 3bit input
    IP_AUDIO_CODEC->REG_AUD_R1_GLOBAL0.bit.TEST_IN_SEL = 2; //3bit sdm
    IP_AUDIO_CODEC->REG_AUD_R1_GLOBAL0.bit.DEBUG_MODE = 2; //adc_dig_debug_mode

//    // adc mclk output
//    IP_AUDIO_CODEC->REG_AUD_R1_GLOBAL0.bit.TEST_OUT_SELA = 1; // 1: adc_mclk_test
//    IP_SYSCTRL->REG_TEST_CTRL.bit.DBG_CLK_SEL = 17; // 17: aud_debug_clk
//    IP_SYSCTRL->REG_TEST_CTRL.bit.DBG_CLK_EN = 1;

    // 24MHz RC/XTAL/PLL clock output
    IP_SYSCTRL->REG_TEST_CTRL.bit.DBG_CLK_SEL = 0; // 0: A2D_CORE_OSC24M_CLK_24M_DIG_CORE (RC)
    //IP_SYSCTRL->REG_TEST_CTRL.bit.DBG_CLK_SEL = 1; // 1: A2D_CORE_XO24M_CLK_DIG_CORE (XTAL)
    //IP_SYSCTRL->REG_TEST_CTRL.bit.DBG_CLK_SEL = 2; // 2: A2D_CORE_SYSPLL_CLK_OUT_AUD (SYSPLL)
    IP_SYSCTRL->REG_TEST_CTRL.bit.DBG_CLK_EN = 1;

    //sdm 3bit pins
    IP_CMN_IOMUX->REG_PAD_GPIOA_00.bit.PAD_GPIOA_00_FSEL = 13; //sdm0 @ A00
    IP_CMN_IOMUX->REG_PAD_GPIOA_01.bit.PAD_GPIOA_01_FSEL = 13;//sdm1 @ A01
    IP_CMN_IOMUX->REG_PAD_GPIOA_02.bit.PAD_GPIOA_02_FSEL = 13; //sdm2 @ A02

//    IP_AON_IOMUX->REG_PAD_AON_GPIOB_05.bit.PAD_AON_GPIOB_05_FSEL = 0; // normal B5
//    IP_AON_IOMUX->REG_PAD_AON_GPIOB_06.bit.PAD_AON_GPIOB_06_FSEL = 0; // normal B6
//    IP_AON_IOMUX->REG_PAD_AON_GPIOB_07.bit.PAD_AON_GPIOB_07_FSEL = 0; // normal B7
//    IP_CMN_IOMUX->REG_PAD_GPIOB_05.bit.PAD_GPIOB_05_FSEL = 13; //sdm0 @ B05
//    IP_CMN_IOMUX->REG_PAD_GPIOB_06.bit.PAD_GPIOB_06_FSEL = 13;//sdm1 @ B06
//    IP_CMN_IOMUX->REG_PAD_GPIOB_07.bit.PAD_GPIOB_07_FSEL = 13; //sdm2 @ B07

    // clock output pin
//    IP_CMN_IOMUX->REG_PAD_GPIOA_08.bit.PAD_GPIOA_08_FSEL = 11; //dbg_clk @ A08

    IP_AON_IOMUX->REG_PAD_AON_GPIOB_08.bit.PAD_AON_GPIOB_08_FSEL = 0; // normal B8
    IP_CMN_IOMUX->REG_PAD_GPIOB_08.bit.PAD_GPIOB_08_FSEL = 11; //dbg_clk @ B08

#else // ASIC
    //NOTE: The PinMux is removed from within driver!!
    //IP_AON_IOMUX->REG_PAD_AON_GPIOB_02.bit.PAD_AON_GPIOB_02_ANA_SEL = 12; //analog pin sel
    //IP_AON_IOMUX->REG_PAD_AON_GPIOB_02.bit.PAD_AON_GPIOB_02_FSEL = 5; // MIC_P @ B02
    //IP_AON_IOMUX->REG_PAD_AON_GPIOB_03.bit.PAD_AON_GPIOB_03_FSEL = 5; // MIC_N @ B03
#endif

    //uint32_t val = IP_AUDIO_CODEC->REG_AUD_R2_GLOBAL1.all;
    IP_AUDIO_CODEC->REG_AUD_R2_GLOBAL1.all |= R2_AUD_EN_IREF | R2_AUD_EN_VMID;
}

static inline void CONFIG_CODEC_CLASSD_ANALOG() {
    //FIXME: consult ANALOG guys about CODEC's analog settings!!

#if (IC_BOARD == 0) // FPGA, FOR TEST ONLY...
    //TODO: PWM_P/N from DBG_OUT?
    IP_SYSCTRL->REG_TEST_CTRL.bit.DBG_OUT_SEL = 2; // 2: Audio debug data out
    IP_SYSCTRL->REG_TEST_CTRL.bit.DBG_OUT_EN = 1; // enable debug data out

    IP_AUDIO_CODEC->REG_AUD_R1_GLOBAL0.bit.TEST_OUT_SELB = 0x5; // 5: {14'd0, ssi_pwm_p, ssi_pwm_n} ??

    IP_AON_IOMUX->REG_PAD_AON_GPIOB_05.bit.PAD_AON_GPIOB_05_FSEL = 0; // normal B5
    IP_AON_IOMUX->REG_PAD_AON_GPIOB_06.bit.PAD_AON_GPIOB_06_FSEL = 0; // normal B6
    IP_CMN_IOMUX->REG_PAD_GPIOB_05.bit.PAD_GPIOB_05_FSEL = 12; //pwm-p @ dbg_out_0
    IP_CMN_IOMUX->REG_PAD_GPIOB_06.bit.PAD_GPIOB_06_FSEL = 12; //pwm-n @ dbg_out_1

#else // ASIC
    //NOTE: The PinMux is removed from within driver!!
    //IP_AON_IOMUX->REG_PAD_AON_GPIOB_00.bit.PAD_AON_GPIOB_00_FSEL = 5; // CLASSD_N @ B00
    //IP_AON_IOMUX->REG_PAD_AON_GPIOB_01.bit.PAD_AON_GPIOB_01_FSEL = 5; // CLASSD_P @ B01
#endif
}

#endif /* __AUDIO_CODEC_H */
