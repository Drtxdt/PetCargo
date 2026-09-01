#ifndef _AUDIO_PLAYER_UTILS_AMPLIFIER_H_
#define _AUDIO_PLAYER_UTILS_AMPLIFIER_H_

#include <stdint.h>

float calc_multiplier(int8_t db);
void amplifier_16bits(
		const int16_t *const pcm_in, uint16_t sample_cnt, int16_t *const pcm_out, float multiplier);
void amplifier_16bits_2(
		const int16_t *const pcm_in, uint16_t sample_cnt, int16_t *const pcm_out, int8_t db);

#endif