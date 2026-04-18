#include <math.h>
#include <complex.h>
#include <stdint.h>
#include <stdlib.h>
#include <assert.h>

// return the smallest power of 2 > (or equal to) n
uint32_t next_power_of_two(uint32_t n) {
    if (n == 0) return 1;

    n--;
    n |= n >> 1;
    n |= n >> 2;
    n |= n >> 4;
    n |= n >> 8;
    n |= n >> 16;
    return n + 1;
}

// bit reversal
uint32_t reverse_bits_parallel(uint32_t n, int log2N) {
    n = ((n >> 1) & 0x55555555) | ((n & 0x55555555) << 1); // Swap adjacent bits
    n = ((n >> 2) & 0x33333333) | ((n & 0x33333333) << 2); // Swap 2-bit pairs
    n = ((n >> 4) & 0x0F0F0F0F) | ((n & 0x0F0F0F0F) << 4); // Swap nibbles
    n = ((n >> 8) & 0x00FF00FF) | ((n & 0x00FF00FF) << 8); // Swap bytes
    n = (n >> 16) | (n << 16);                             // Swap 16-bit halves
    return n >> (32 - log2N);
}

// IFFT 

void IFFT(double complex * x, int N) {

    assert((N & (N - 1)) == 0);

    int log2N = (int)log2(N);

	// bit reversal stuff
    for (int i = 0; i < N; i++) {
        int j = reverse_bits_parallel(i, log2N);
        if (j > i) {
            double complex tmp = x[i];
            x[i] = x[j];
            x[j] = tmp;
        }
    }

	// FFT steps

    double complex w_m;
    double complex w;
    double complex u;
    double complex t;
    
    for (int size = 2; size <= N; size *= 2) {

        int half = size / 2;
        w_m = cexp(+2.0 * I * M_PI / size);

        for (int i = 0; i < N; i += size) {

            w = 1.0;

            for (int j = 0; j < half; j++) {

                u = x[i + j];
                t = w * x[i + j + half];

                x[i + j] = u + t;
                x[i + j + half] = u - t;

                w *= w_m;
            }
        }
    }

    for (int i = 0; i < N; i++) {
        x[i] /= N;
    }
}
// Radix-2 Decimation-in-Time (DIT) FFT
void FFT(double complex * x, int N) {

    assert((N & (N - 1)) == 0);

    int log2N = (int)log2(N);

	// bit reversal stuff
    for (int i = 0; i < N; i++) {
        int j = reverse_bits_parallel(i, log2N);
        if (j > i) {
            double complex tmp = x[i];
            x[i] = x[j];
            x[j] = tmp;
        }
    }

	// FFT steps

    double complex w_m;
    double complex w;
    double complex u;
    double complex t;
    
    for (int size = 2; size <= N; size *= 2) {

        int half = size / 2;
        w_m = cexp(-2.0 * I * M_PI / size);

        for (int i = 0; i < N; i += size) {

            w = 1.0;

            for (int j = 0; j < half; j++) {

                u = x[i + j];
                t = w * x[i + j + half];

                x[i + j] = u + t;
                x[i + j + half] = u - t;

                w *= w_m;
            }
        }
    }
}

void fft_conv(double complex *x, int N, double complex *h, int M) {

    int size = next_power_of_two(N + M - 1);

    double complex *X = calloc(size, sizeof(double complex));
    double complex *H = calloc(size, sizeof(double complex));

    for (int i = 0; i < N; i++) X[i] = x[i];
    for (int i = 0; i < M; i++) H[i] = h[i];

    FFT(X, size);
    FFT(H, size);

    for (int i = 0; i < size; i++) {
        X[i] *= H[i];
    }

    IFFT(X, size);

    free(H);
}
