#define _USE_MATH_DEFINES
#include <math.h>
#include <stdio.h>
#include "types.h"

// Simple One-Pole Low Pass Filter
double LPF(double alpha, double curr_sample, double * prev_output) {
    double output = alpha * curr_sample + (1.0 - alpha) * (*prev_output);
    *prev_output = output;
    return output;
}

// Simple One-Pole High Pass Filter
double HPF(double alpha, double curr_sample, double * prev_out, double * prev_inp) {
    double output = alpha * (*prev_out + curr_sample - (*prev_inp));
    *prev_out = output;
    *prev_inp = curr_sample;
    return output;
}

// Band Pass Filter (Biquad)
double BPF(double input, double * prev_inp_1, double * prev_inp_2, double * prev_out_1, double * prev_out_2, double Q, double center_freq, int sample_rate) {
    double w0 = 2.0 * M_PI * center_freq / (double)sample_rate;
    double alpha = sin(w0) / (2.0 * Q);

    double b0 = Q * alpha;
    double b1 = 0;
    double b2 = -Q * alpha;
    double a0 = 1.0 + alpha;
    double a1 = -2.0 * cos(w0);
    double a2 = 1.0 - alpha;

    double output = (b0/a0)*input + (b1/a0)*(*prev_inp_1) + (b2/a0)*(*prev_inp_2) - (a1/a0)*(*prev_out_1) - (a2/a0)*(*prev_out_2);

    *prev_inp_2 = *prev_inp_1;
    *prev_inp_1 = input;
    *prev_out_2 = *prev_out_1;
    *prev_out_1 = output;

    return output;
}

// Notch Filter (Biquad)
double Notch(double input, double center_freq, double * prev_inp_1, double * prev_inp_2, double * prev_out_1, double * prev_out_2, double Q, double sample_rate) {
    double w0 = 2.0 * M_PI * center_freq / sample_rate;
    double alpha = sin(w0) / (2.0 * Q);

    double b0 = 1.0;
    double b1 = -2.0 * cos(w0);
    double b2 = 1.0;
    double a0 = 1.0 + alpha;
    double a1 = -2.0 * cos(w0);
    double a2 = 1.0 - alpha;

    double output = (b0/a0)*input + (b1/a0)*(*prev_inp_1) + (b2/a0)*(*prev_inp_2) - (a1/a0)*(*prev_out_1) - (a2/a0)*(*prev_out_2);

    *prev_inp_2 = *prev_inp_1;
    *prev_inp_1 = input;
    *prev_out_2 = *prev_out_1;
    *prev_out_1 = output;

    return output;
}

// Peaking EQ (Biquad)
double peakingEQ(double input, double G, double* prev_inp_1, double* prev_inp_2, double* prev_out_1, double* prev_out_2, double Q, double center_freq, int sample_rate) {
    double w0 = 2.0 * M_PI * center_freq / (double)sample_rate;
    double alpha = sin(w0) / (2.0 * Q);
    double A = pow(10.0, G/40.0); // Standard biquad gain adjustment

    double b0 = 1.0 + alpha * A;
    double b1 = -2.0 * cos(w0);
    double b2 = 1.0 - alpha * A;
    double a0 = 1.0 + alpha / A;
    double a1 = -2.0 * cos(w0);
    double a2 = 1.0 - alpha / A;

    double output = (b0/a0) * input + (b1/a0) * (*prev_inp_1) + (b2/a0) * (*prev_inp_2) - (a1/a0)*(*prev_out_1) - (a2/a0)*(*prev_out_2);

    *prev_inp_2 = *prev_inp_1;
    *prev_inp_1 = input;
    *prev_out_2 = *prev_out_1;
    *prev_out_1 = output;

    return output;
}

// All Pass Filter (Standard Biquad)
double all_pass(double input, double center_freq, double* prev_inp_1, double* prev_inp_2, double* prev_out_1, double* prev_out_2, double Q, int sample_rate) {
    double w0 = 2.0 * M_PI * center_freq / (double)sample_rate;
    double alpha = sin(w0) / (2.0 * Q);

    double a0 = 1.0 + alpha;
    double a1 = -2.0 * cos(w0);
    double a2 = 1.0 - alpha;
    double b0 = a2;
    double b1 = a1;
    double b2 = a0;

	double output = (b0/a0)*input + (b1/a0)*(*prev_inp_1) + (b2/a0)*(*prev_inp_2) - (a1/a0)*(*prev_out_1) - (a2/a0)*(*prev_out_2);
    
    *prev_inp_2 = *prev_inp_1;
    *prev_inp_1 = input;
    *prev_out_2 = *prev_out_1;
    *prev_out_1 = output;

    return output;
}

// Low Shelf Filter
double lowShelf(double input, double G_db, double* prev_inp_1, double* prev_inp_2, double* prev_out_1, double* prev_out_2, double Q, double center_freq, int sample_rate) {
    double w0 = 2.0 * M_PI * center_freq / (double)sample_rate;
    double alpha = sin(w0) / (2.0 * Q);
    double A = pow(10.0, G_db/40.0);

    double b0 = A * ((A + 1.0) - (A - 1.0) * cos(w0) + 2.0 * sqrt(A) * alpha);
    double b1 = 2.0 * A * ((A - 1.0) - (A + 1.0) * cos(w0));
    double b2 = A * ((A + 1.0) - (A - 1.0) * cos(w0) - 2.0 * sqrt(A) * alpha);
    double a0 = (A + 1.0) + (A - 1.0) * cos(w0) + 2.0 * sqrt(A) * alpha;
    double a1 = -2.0 * ((A - 1.0) + (A + 1.0) * cos(w0));
    double a2 = (A + 1.0) + (A - 1.0) * cos(w0) - 2.0 * sqrt(A) * alpha;

    double output = (b0/a0) * input + (b1/a0) * (*prev_inp_1) + (b2/a0) * (*prev_inp_2) - (a1/a0) * (*prev_out_1) - (a2/a0) * (*prev_out_2);

    *prev_inp_2 = *prev_inp_1;
    *prev_inp_1 = input;
    *prev_out_2 = *prev_out_1;
    *prev_out_1 = output;

    return output;
}

// High Shelf Filter
double highShelf(double input, double G_db, double* prev_inp_1, double* prev_inp_2, double* prev_out_1, double* prev_out_2, double Q, double center_freq, int sample_rate) {
    double w0 = 2.0 * M_PI * center_freq / (double)sample_rate;
    double alpha = sin(w0) / (2.0 * Q);
    double A = pow(10.0, G_db/40.0);

    // Coefficients based on Audio EQ Cookbook
    double b0 =    A*( (A+1) + (A-1)*cos(w0) + 2*sqrt(A)*alpha );
    double b1 = -2*A*( (A-1) + (A+1)*cos(w0) );
    double b2 =    A*( (A+1) + (A-1)*cos(w0) - 2*sqrt(A)*alpha );
    double a0 =        (A+1) - (A-1)*cos(w0) + 2*sqrt(A)*alpha;
    double a1 =    2*( (A-1) - (A+1)*cos(w0) );
    double a2 =        (A+1) - (A-1)*cos(w0) - 2*sqrt(A)*alpha;

	double output = (b0/a0) * input + (b1/a0) * (*prev_inp_1) + (b2/a0) * (*prev_inp_2) - (a1/a0) * (*prev_out_1) - (a2/a0) * (*prev_out_2);

    *prev_inp_2 = *prev_inp_1;
    *prev_inp_1 = input;
    *prev_out_2 = *prev_out_1;
    *prev_out_1 = output;

    return output;
}
