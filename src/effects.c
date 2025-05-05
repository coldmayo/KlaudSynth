#include <math.h>
#include <stdlib.h>
#include "types.h"

DELAY_STATE* init_delay(float max_delay_seconds, int sample_rate) {
    DELAY_STATE* state = malloc(sizeof(DELAY_STATE));
    state->delay_size = max_delay_seconds * sample_rate;
    state->delay_buffer = calloc(state->delay_size, sizeof(float));
    state->delay_pos = 0;
    return state;
}

void free_delay(DELAY_STATE* state) {
    free(state->delay_buffer);
    free(state);
}

float process_delay(float input, DELAY_STATE* state, float delay_time, float feedback) {
    int delay_samples = delay_time * state->delay_size;
    int read_pos = (state->delay_pos - delay_samples + state->delay_size) % state->delay_size;

    float delayed = state->delay_buffer[read_pos];
    state->delay_buffer[state->delay_pos] = input + delayed * feedback;
    state->delay_pos = (state->delay_pos + 1) % state->delay_size;

    return delayed;
}

CHORUS_STATE* init_chorus() {
    CHORUS_STATE* state = malloc(sizeof(CHORUS_STATE));
    state->phase = 0.0f;
    state->lfo_phase = 0.0f;
    return state;
}

float process_chorus(float input, CHORUS_STATE* state, float rate, float depth, int sample_rate) {
    float lfo = sinf(2.0f * M_PI * state->lfo_phase);
    state->lfo_phase += rate / sample_rate;
    if (state->lfo_phase >= 1.0f) state->lfo_phase -= 1.0f;

    float delayed_phase = state->phase - (0.0005f + 0.0005f * lfo * depth) * sample_rate;
    if (delayed_phase < 0.0f) delayed_phase += 1.0f;

    // implement delay line later
    return sinf(2.0f * M_PI * delayed_phase) * input;
}
