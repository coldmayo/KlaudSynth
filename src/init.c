#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "types.h"

// Initialize a filter with default values
FILTER * init_filter(FILTER * filter, int index, FILTER_TYPES type) {
	filter->type = type;

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

// Helper to clear buffers to zero
void clear_buffer(double *buffer, int size) {
    for (int i = 0; i < size; i++) {
        buffer[i] = 0.0;
    }
}

void init_chorus_state(CHORUS_STATE *state) {
    clear_buffer(state->buffer, MAX_CHORUS_SAMPLES);
    state->write_head = 0;
    state->lfo_phase = 0.0;
}

void init_delay_state(DELAY_STATE *state, int sample_rate) {
    clear_buffer(state->buffer, MAX_DELAY_SAMPLES);
    state->write_head = 0;
    // Default to 500ms delay, capped by MAX_DELAY_SAMPLES
    state->delay_samples = 0.5 * sample_rate; 
    if (state->delay_samples >= MAX_DELAY_SAMPLES) {
        state->delay_samples = MAX_DELAY_SAMPLES - 1;
    }
}

void init_flanger_state(FLANGER_STATE *state, int sample_rate) {
    clear_buffer(state->buffer, MAX_DELAY_SAMPLES);
    state->write_head = 0;
    state->buffer_size = MAX_DELAY_SAMPLES;
    state->lfo_value = 0.0;
    // Flanging happens at very short time scales (usually 1ms to 10ms)
    state->base_delay_samples = (2.0 * sample_rate) / 1000.0; 
    state->depth_samples = (5.0 * sample_rate) / 1000.0;
}

void init_all_pass_state(ALL_PASS_STATE *state) {
    state->input_1 = 0.0;
    state->input_2 = 0.0;
    state->output_1 = 0.0;
    state->output_2 = 0.0;
    clear_buffer(state->buffer, MAX_REVERB_SAMPLES);
    state->write_head = 0;
}

void init_reverb_state(REVERB_STATE *state) {
    clear_buffer(state->buffer, MAX_REVERB_SAMPLES);
    state->write_head = 0;
}

void init_phaser_state(PHASER_STATE *state) {
    state->min_freq = 440.0;
    state->max_freq = 1600.0;
    state->lfo_phase = 0.0;
    state->phase = 0.0;
    state->Q = 0.707;
    for (int i = 0; i < NUM_PHASER_STAGES; i++) {
        init_all_pass_state(&state->filter[i]);
    }
}

void init_master_reverb(MASTER_REVERB *state, int sample_rate) {
    state->sample_rate = sample_rate;
    state->mix = 0.3;
    
    // Suggest unique prime-ish delay times for comb filters to avoid resonance
    double comb_times[NUM_COMBS] = {29.7, 37.1, 41.1, 43.7};
    double comb_gains[NUM_COMBS] = {0.75, 0.75, 0.75, 0.75};

    for (int i = 0; i < NUM_COMBS; i++) {
        init_reverb_state(&state->comb_states[i]);
        state->comb_delay_ms[i] = comb_times[i];
        state->comb_gains[i] = comb_gains[i];
    }

    for (int i = 0; i < NUM_ALLPASS; i++) {
        init_all_pass_state(&state->ap_states[i]);
        state->ap_center_freq[i] = 500.0 + (i * 200.0);
        state->ap_Q[i] = 0.5;
    }
}

FX * init_FX(FX * eff, int i, FX_TYPES type, int sample_rate) {
    eff->type = type;

    eff->time = 50;   // in ms
    eff->mix = 0.50;   // 50%
    eff->index = i;
    eff->feedback = 0.50;   // 50%

    switch (type) {
        case CHORUS:
            eff->lfo = (LFO *)malloc(sizeof(LFO));
            eff->chorus_state = (CHORUS_STATE *)malloc(sizeof(CHORUS_STATE *));
            init_chorus_state(eff->chorus_state); 
            break;
        case FLANGER:
            eff->flanger_state = (FLANGER_STATE *)malloc(sizeof(FLANGER_STATE *));
            init_flanger_state(eff->flanger_state, sample_rate);
            break;
        case DELAY:
            eff->delay_state = (DELAY_STATE *)malloc(sizeof(DELAY_STATE *));
            init_delay_state(eff->delay_state, sample_rate);
            break;
        case REVERB:
            eff->reverb_state = (REVERB_STATE *)malloc(sizeof(REVERB_STATE *));
            init_reverb_state(eff->reverb_state);
            break;
        case PHASER:
            eff->lfo = (LFO *)malloc(sizeof(LFO));
            eff->phaser_state = (PHASER_STATE *)malloc(sizeof(PHASER_STATE *));
            init_phaser_state(eff->phaser_state);
            break;
        default:
            
    }
    return eff;
}

// Initialize a single oscillator/wave
WAVE * init_wave(WAVE * dawave, int i) {
	// Allocate arrays of pointers for the sub-objects
	dawave->filters = (FILTER **)malloc(sizeof(FILTER *) * 1); // Start with 1 slot
	dawave->lfos = (LFO **)malloc(sizeof(LFO *) * 1);

	dawave->type = WAVE_SINE;
	dawave->octave = 0;
	dawave->pitch = C;
	dawave->pulse_width = 0.5;
	dawave->volume = 0.8;
	dawave->velocity = 1.0;

	dawave->index = i;
	dawave->num_filters = 0;
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
	init->num_fx = 0;

	// Allocate space for the pointers to WAVE structs
	init->waves = (WAVE **)malloc(sizeof(WAVE *) * init->num_waves);
	init->fxs = NULL;

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

	// Initialize global filters
	init->num_filts = 0;
	init->glob_filters = (FILTER **)malloc(sizeof(FILTER *));

	init->focus = GLOBAL;
	init->master_volume = 0.5;
	init->pan = 0.0;
	init->glide_time = 0.005;
	init->current_tempo = 120.0;
	init->fx_send = 0.0;
	init->channels = STEREO;

	return init;
}
