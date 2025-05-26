// Basically a file with a bunch of filters

#include <math.h>

// cutt off frequency from alpha

// Low Pass Filter
float low_pass(float input, float*prev_out, float alpha, float*prev_inp) {
	float output = alpha * input + (1.0f - alpha) * (*prev_out);
    *prev_out = output;
    *prev_inp = input;
    return output;
}

// High Pass Filter
float high_pass(float input, float*prev_out, float alpha, float*prev_inp) {
	float output = alpha * (*prev_out + input - *prev_inp);
	*prev_out = output;
	*prev_inp = input;
	return output;
}

// Bandpass Filter
float band_pass(float input, float *prev_in1, float *prev_in2, float *prev_out1, float *prev_out2, float center_freq, float Q, float sample_rate) {
    float w0 = 2 * M_PI * center_freq / sample_rate;
    float alpha = sinf(w0) / (2 * Q);

    float b0 = alpha;
    float b1 = 0;
    float b2 = -alpha;
    float a0 = 1 + alpha;
    float a1 = -2 * cosf(w0);
    float a2 = 1 - alpha;

    float output = (b0/a0)*input + (b1/a0)*(*prev_in1) + (b2/a0)*(*prev_in2) - (a1/a0)*(*prev_out1) - (a2/a0)*(*prev_out2);

    // Update state variables
    *prev_in2 = *prev_in1;
    *prev_in1 = input;
    *prev_out2 = *prev_out1;
    *prev_out1 = output;

    return output;
}

// Notch Filter
float notch_filter(float input, float *prev_in1, float *prev_in2, float *prev_out1, float *prev_out2, float center_freq, float Q, float sample_rate) {
    float w0 = 2 * M_PI * center_freq / sample_rate;
    float alpha = sinf(w0) / (2 * Q);

    float b0 = 1;
    float b1 = -2 * cosf(w0);
    float b2 = 1;
    float a0 = 1 + alpha;
    float a1 = -2 * cosf(w0);
    float a2 = 1 - alpha;

    float output = (b0/a0)*input + (b1/a0)*(*prev_in1) + (b2/a0)*(*prev_in2) - (a1/a0)*(*prev_out1) - (a2/a0)*(*prev_out2);

    // Update state variables
    *prev_in2 = *prev_in1;
    *prev_in1 = input;
    *prev_out2 = *prev_out1;
    *prev_out1 = output;

    return output;
}
