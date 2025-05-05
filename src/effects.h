#ifndef effects_h_INCLUDED
#define effects_h_INCLUDED

#include "types.h"

DELAY_STATE* init_delay(float max_delay_seconds, int sample_rate);
void free_delay(DELAY_STATE* state);
float process_delay(float input, DELAY_STATE* state, float delay_time, float feedback);
CHORUS_STATE* init_chorus();
float process_chorus(float input, CHORUS_STATE* state, float rate, float depth, int sample_rate);

#endif
