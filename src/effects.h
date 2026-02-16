#ifndef effects_h_INCLUDED
#define effects_h_INCLUDED

#include "types.h"

double chorus(double input, CHORUS_STATE *state, double mix, LFO * lfo, int sample_rate);
double delay(double input, DELAY_STATE *state, double mix, double feedback);
double flanger(double input, FLANGER_STATE * state, double mix, double feedback, int sample_rate);
double feedback_comb_filter(double input, REVERB_STATE *state, double mix, double delay_ms, double gain, int sample_rate);
double reverb(double input, REVERB_MASTER_STATE *master_state);
double phaser(double input, LFO * lfo, PHASER_STATE * state, double feedback, double mix, int sample_rate);

#endif
