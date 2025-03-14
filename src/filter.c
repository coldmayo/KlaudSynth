// Basically a file with a bunch of filters

#include <math.h>

// First order recurssive filter
float low_pass(float input, float prev_out, float alpha) {
	float output = alpha * input + (1.0f - alpha) * (prev_output);
    prev_out = output;
    return output;
}



// High Pass Filter
float high_pass(float input, float prev_out, float ) {
	return input;
}

//Band Pass filter

// Notch Filter
