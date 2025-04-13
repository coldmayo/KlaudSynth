#ifndef types_h_INCLUDED
#define types_h_INCLUDED

typedef struct {
	double freq;
	double cutt_freq;
	double amp;
	double phase;
	double sexs;
	int sample_rate;
	char * wave_form;
	char * channels;
	char * pcm_device;
} GEN_INP;

#endif // types_h_INCLUDED
