#include <math.h>
#include <string.h>
#include <stdlib.h>

#include "types.h"
#include "filter.h"

// Chorus Effect: Uses a modulated delay line to create pitch thickness
double chorus(double input, CHORUS_STATE *state, double mix, LFO * lfo, int sample_rate) {
    double rate = lfo->rate;
    double depth = lfo->depth; // depth in ms

    // Write to buffer
    state->buffer[state->write_head] = input;
    state->write_head = (state->write_head + 1) % MAX_CHORUS_SAMPLES;

    // Update LFO
    state->lfo_phase += rate / (double)sample_rate;
    if (state->lfo_phase >= 1.0) state->lfo_phase -= 1.0;

    double lfo_val = sin(2.0 * M_PI * state->lfo_phase);

    // Chorus typically has a base delay of ~20ms
    double base_delay_samples = 20.0 * sample_rate / 1000.0;
    double depth_samples = depth * sample_rate / 1000.0;
    double delay_samples = base_delay_samples + (depth_samples * lfo_val);

    // Linear Interpolation for smooth delay modulation
    double read_f = (double)state->write_head - delay_samples;
    while (read_f < 0) read_f += MAX_CHORUS_SAMPLES;
    read_f = fmod(read_f, MAX_CHORUS_SAMPLES);

    int idx0 = (int)read_f;
    int idx1 = (idx0 + 1) % MAX_CHORUS_SAMPLES;
    double frac = read_f - idx0;

    double delayed = state->buffer[idx0] + frac * (state->buffer[idx1] - state->buffer[idx0]);

    return (input * (1.0 - mix)) + (delayed * mix);
}

// Simple Digital Delay
double delay(double input, DELAY_STATE *state, double mix, double feedback) {
    double read_f_index = (double)state->write_head - state->delay_samples;

    while (read_f_index < 0.0) {
        read_f_index += (double)MAX_DELAY_SAMPLES;
    }

    int idx1 = (int)read_f_index % MAX_DELAY_SAMPLES;
    int idx2 = (idx1 + 1) % MAX_DELAY_SAMPLES;
    double frac = read_f_index - (double)idx1;

    double delayed_sample = state->buffer[idx1] + frac * (state->buffer[idx2] - state->buffer[idx1]);

    // Apply feedback and write to buffer
    state->buffer[state->write_head] = input + (delayed_sample * feedback);
    state->write_head = (state->write_head + 1) % MAX_DELAY_SAMPLES;

    return (input * (1.0 - mix)) + (delayed_sample * mix);
}

// Flanger: Very short modulated delay with feedback for "jet" sound
double flanger(double input, FLANGER_STATE * state, double mix, double feedback, int sample_rate) {
    double delay_samples = state->base_delay_samples + (state->depth_samples * state->lfo_value);

    double read_f_index = (double)state->write_head - delay_samples;
    while (read_f_index < 0.0) read_f_index += (double)MAX_DELAY_SAMPLES;

    int idx1 = (int)read_f_index % MAX_DELAY_SAMPLES;
    int idx2 = (idx1 + 1) % MAX_DELAY_SAMPLES;
    double frac = read_f_index - idx1;

    double delayed_sample = state->buffer[idx1] + frac * (state->buffer[idx2] - state->buffer[idx1]);

    state->buffer[state->write_head] = input + (delayed_sample * feedback);
    state->write_head = (state->write_head + 1) % MAX_DELAY_SAMPLES;

    return (input * (1.0 - mix)) + (delayed_sample * mix);
}

// Internal Reverb Helper: Feedback Comb Filter
double feedback_comb_filter(double input, REVERB_STATE *state, double mix, double delay_ms, double gain, int sample_rate) {
    double delay_samples = (delay_ms * sample_rate) / 1000.0;

    double read_f = (double)state->write_head - delay_samples;
    while (read_f < 0) read_f += MAX_REVERB_SAMPLES;

    int idx0 = (int)read_f % MAX_REVERB_SAMPLES;
    int idx1 = (idx0 + 1) % MAX_REVERB_SAMPLES;
    double frac = read_f - idx0;

    double delayed_output = state->buffer[idx0] + frac * (state->buffer[idx1] - state->buffer[idx0]);
    double current_output = input + (gain * delayed_output);

    state->buffer[state->write_head] = current_output;
    state->write_head = (state->write_head + 1) % MAX_REVERB_SAMPLES;

    return (input * (1.0 - mix)) + (delayed_output * mix);
}

// Schroeder Reverb: Parallel Combs followed by Series All-Passes
double reverb(double input, MASTER_REVERB *master_state, double mix) {
    double wet_signal = 0.0;

    // Parallel Comb Filters
    for (int i = 0; i < NUM_COMBS; i++) {
        wet_signal += feedback_comb_filter(input, &master_state->comb_states[i], 1.0,
                                           master_state->comb_delay_ms[i],
                                           master_state->comb_gains[i],
                                           master_state->sample_rate);
    }
    wet_signal /= (double)NUM_COMBS;

    // Series All-Pass Filters (using the function from your filter.c)
    for (int i = 0; i < NUM_ALLPASS; i++) {
        ALL_PASS_STATE *ap = &master_state->ap_states[i];
        wet_signal = all_pass(wet_signal, master_state->ap_center_freq[i],
                              &ap->input_1, &ap->input_2,
                              &ap->output_1, &ap->output_2,
                              master_state->ap_Q[i], master_state->sample_rate);
    }

    return (input * (1.0 - mix)) + (wet_signal * mix);
}

// Phaser: Uses cascading all-pass filters to create moving notches
double phaser(double input, LFO * lfo, PHASER_STATE * state, double feedback, double mix, int sample_rate) {
    // Increment LFO phase
    lfo->curr_phase += lfo->rate / (double)sample_rate;
    if (lfo->curr_phase >= 1.0) lfo->curr_phase -= 1.0;

    double lfo_val = sin(2.0 * M_PI * lfo->curr_phase);
    double modulation = 0.5 * (lfo_val + 1.0); // 0 to 1 range

    double freq_range = state->max_freq - state->min_freq;
    double modulated_freq = state->min_freq + (freq_range * modulation * lfo->depth);

    // Simple feedback loop
    double input_with_feedback = input + (feedback * state->filter[NUM_PHASER_STAGES - 1].output_1);

    double processed_signal = input_with_feedback;
    for (int i = 0; i < NUM_PHASER_STAGES; i++) {
        ALL_PASS_STATE *ap = &state->filter[i];
        processed_signal = all_pass(processed_signal, modulated_freq,
                                    &ap->input_1, &ap->input_2,
                                    &ap->output_1, &ap->output_2,
                                    state->Q, sample_rate);
    }

    return (input * (1.0 - mix)) + (processed_signal * mix);
}
