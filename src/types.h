#ifndef types_h_INCLUDED
#define types_h_INCLUDED

typedef struct {
	int freq = 440;
	int amp = 0.5;
	int phase = 0;
	int sexs = 2;
	int sample_rate = 44100;
	char * wave_form = "sine";
	char * channels = "stereo";
	char * pcm_device = "default";
} GEN_INP;

#endif // types_h_INCLUDED
