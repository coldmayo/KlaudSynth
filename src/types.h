#include <stdbool.h>

#ifndef types_h_INCLUDED
#define types_h_INCLUDED

#define LEN 32

typedef struct {
	char name[LEN];
	double cutt_freq;
	float Q;
} FILTER;

typedef struct {
	float alpha;
	float prev_in;
	float prev_out;
} FILTER_STATE;

typedef struct {
    char name[LEN];
	double freq;
	double amp;
	double phase;
	double sexs;
	int sample_rate;
	float pan;   // -1.0 (left) to 1.0 (right), 0.0 = center
	float phase_offset;  // Right channel phase offset (0.0-1.0)
    float delay;        // Stereo delay in seconds
    float feedback;     // Delay feedback amount
    float chorus_rate;  // Chorus modulation rate
    float chorus_depth; // Chorus modulation depth
	char wave_form[LEN];   // sine, square
	char wave_form_r[LEN];
	char channels[LEN];
	char pcm_device[LEN];
	bool saved;
	FILTER filters[LEN];
	int filter_num;
} GEN_INP;

typedef struct {
    float* delay_buffer;
    int delay_pos;
    int delay_size;
} DELAY_STATE;

typedef struct {
    float phase;
    float lfo_phase;
} CHORUS_STATE;

typedef struct {
	GEN_INP * waves;
	char type[LEN];   // add, sub
	int count;
	int filter_num;
	FILTER filters[LEN];
} SYNTH_WAVES;

#endif // types_h_INCLUDED
