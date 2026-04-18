#include <complex.h>

#ifndef convolution_h_INCLUDED
#define convolution_h_INCLUDED

void IFFT(double complex * x, int N);
void FFT(double complex * x, int N);
void fft_conv(double complex *x, int N, double complex *h, int M);

#endif // convolution_h_INCLUDED
