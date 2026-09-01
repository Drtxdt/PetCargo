/*
 * Driver_Common_Audio.h
 *
 *  Created on: Feb. 17, 2024 for MARS
 *
 */

#ifndef _DRIVER_COMMON_AUDIO_H_
#define _DRIVER_COMMON_AUDIO_H_

#include "Driver_Common.h"

//-------------------------------------------------------------
// For PingPong block transfer
typedef struct {
    uint32_t reserved; // SHOULD set to 0
    uint32_t *sample_data;
    uint32_t sample_cnt; // SHOULD be EVEN when 16-bit sample!!
    uint32_t flags; //1: Stop PingPing after this block transfer
} PIPO_IN_BLOCK;

typedef struct {
    uint32_t *sample_data;
    uint32_t reserved; // SHOULD set to 0
    uint32_t sample_cnt; // SHOULD be EVEN when 16-bit sample!!
    uint32_t flags; // 1: indicates stop PingPing after the block transfer
} PIPO_OUT_BLOCK;

typedef union {
    PIPO_IN_BLOCK in;
    PIPO_OUT_BLOCK out;
} PIPO_IO_BLOCK;


//-------------------------------------------------------------
// Audio buffer used for DMA transfer of multiple non-continuous buffers
// NOTE: AUDIO_BUFFER_USER (array) SHOULD NOT be in the stack, and SHOULD NOT
//      be released until data transfer is completed or aborted!
typedef struct {
    //uint32_t dma_lli_words[12];
    uint32_t dma_lli_words[6];
} DMA_DESC;

typedef struct {
    uint32_t* sample_data;
    uint32_t  sample_cnt; // SHOULD be EVEN when 16-bit sample!!
    DMA_DESC  dma_desc; // DON'T TOUCH IT! reserved for DMA driver only
} AUDIO_BUFFER_USER;

// Channel definition in audio interface, i.e. I2S, ADC_PDM couple, DAC couple etc.
#define CH_BMP_LEFT    (0x1 << 0)
#define CH_BMP_RIGHT   (0x1 << 1)
#define CH_BMP_STEREO  (CH_BMP_LEFT | CH_BMP_RIGHT)

//typedef struct {
//    uint32_t    samp_rate : 24; // echo sample rate, generally equals to recording sample rate
//    uint32_t    trim_16bits : 8;// only valid for 24 MSB and 32 channel mode, when set to 1:
//                            // record: trim low 16bits, get 16bits audio data
//                            // playback: 16bits audio data is placed at high 16bits of WORD
//} ECHO_PARAMS;

typedef struct {
    uint32_t    samp_rate : 24; // echo sample rate, generally equals to recording (IN) sample rate
    uint32_t    trim_16bits : 1;// only valid for 24 MSB and 32 channel mode, when set to 1:
                            // record: trim low 16bits, get 16bits audio data
                            // playback: 16bits audio data is placed at high 16bits of WORD
    uint32_t    mixed_in_echo : 1; // whether IN data and echo data are mixed
                            // 1: mixed, 0: NOT mixed. NATURALLY mixed (2ch) or mono (1ch) on MARS!
    uint32_t    reserved : 6;
} ECHO_PARAMS;

#endif /* _DRIVER_COMMON_AUDIO_H_ */
