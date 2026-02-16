#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "types.h"

// Initialize a filter with default values
FILTER * init_filter(FILTER * filter, int index, FILTER_TYPES type) {
	snprintf(filter->type, MAX_LEN, "FILTER_%d", type);

	filter->cutt_freq = 20000;
	filter->center_freq = 20000;
	filter->resonance = 0;
	filter->drive = 0;
	filter->index = index;
	filter->alpha = 0.0;
	filter->type = type;

	for (int i = 0; i < FILTER_STAGES; i++) {
		filter->prev_out[i] = 0.0;
		filter->prev_inp[i] = 0.0;
	}

	return filter;
}

// Initialize a single oscillator/wave
WAVE * init_wave(WAVE * dawave, int i) {
	// Allocate arrays of pointers for the sub-objects
	dawave->filters = (FILTER **)malloc(sizeof(FILTER *) * 1); // Start with 1 slot
	dawave->fxs = (FX **)malloc(sizeof(FX *) * 1);
	dawave->lfos = (LFO **)malloc(sizeof(LFO *) * 1);

	dawave->type = WAVE_SINE;
	dawave->octave = 0;
	dawave->pitch = C;
	dawave->pulse_width = 0.5;
	dawave->volume = 0.8;
	dawave->velocity = 1.0;

	dawave->index = i;
	dawave->num_filters = 0;
	dawave->num_fx = 0;
	dawave->num_lfos = 0;
	dawave->is_active = 1;

	dawave->current_phase = 0.0;

	return dawave;
}

// Initialize the master sound engine
SOUND * init_sound() {
	SOUND *init = (SOUND *)malloc(sizeof(SOUND));
	if (!init) return NULL;

	init->sample_rate = 44100;
	init->num_waves = 1;
	init->curr_wave = 0;

	// Allocate space for the pointers to WAVE structs
	init->waves = (WAVE **)malloc(sizeof(WAVE *) * init->num_waves);

	// Allocate the actual WAVE struct for the first slot
	init->waves[0] = (WAVE *)malloc(sizeof(WAVE));
	init_wave(init->waves[0], 0);

	// Initialize Master Envelope
	init->env = (ENVELOPE *)malloc(sizeof(ENVELOPE));
	init->env->attack = 0.01;
	init->env->decay = 0.1;
	init->env->sustain = 0.8;
	init->env->release = 0.2;
	init->env->state = IDLE;
	init->env->curr_gain = 0.0;

	init->master_volume = 0.5;
	init->pan = 0.0;
	init->glide_time = 0.005;
	init->current_tempo = 120.0;
	init->fx_send = 0.0;
	init->channels = STEREO;

	return init;
}
