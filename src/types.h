#ifndef types_h_INCLUDED
#define types_h_INCLUDED

#define LEN 32

typedef struct {
    char name[LEN];
	double freq;
	double cutt_freq;
	double amp;
	double phase;
	double sexs;
	int sample_rate;
	char filter[LEN];
	char wave_form[LEN];   // sine, square
	char channels[LEN];
	char pcm_device[LEN];
} GEN_INP;

typedef struct {
	GEN_INP * waves;
	char type[LEN];   // add, sub
	int count;
} SYNTH_WAVES;

#endif // types_h_INCLUDED
