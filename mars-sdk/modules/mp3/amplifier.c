#include "amplifier.h"
#include <math.h>

#ifndef INT16_MAX
#define INT16_MAX (32766)
#endif
#ifndef INT16_MIN
#define INT16_MIN (-32767)
#endif

#define PI (3.14159265)

float calc_multiplier(int8_t db)
{
	// return pow(10, db / 20.0f);
	// const float d = (db + 100) * 0.45 * PI / 180;
	// return tan(d);
	const float d = (db + 100) * 2 / 100.0;
	return d;
}

void amplifier_16bits(
		const int16_t *const pcm_in, uint16_t sample_cnt, int16_t *const pcm_out, float multiplier)
{
	int32_t pcm_val;
	uint16_t i;
	for (i = 0; i < sample_cnt; i++) {
		pcm_val = pcm_in[i] * multiplier;
		if (pcm_val <= INT16_MAX && pcm_val >= INT16_MIN) {
			pcm_out[i] = pcm_val;
		} else if (pcm_val > INT16_MAX) {
			pcm_out[i] = INT16_MAX;
		} else /* if (pcm_val < INT16_MIN) */ {
			pcm_out[i] = INT16_MIN;
		}
	}
}

void amplifier_16bits_2(
		const int16_t *const pcm_in, uint16_t sample_cnt, int16_t *const pcm_out, int8_t db)
{
	const float multiplier = calc_multiplier(db);
	amplifier_16bits(pcm_in, sample_cnt, pcm_out, multiplier);
}
