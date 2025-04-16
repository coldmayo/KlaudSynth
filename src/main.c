#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <pthread.h>

#include "types.h"
#include "gen.h"

#define MAXCOMMSIZE 256
#define MAX_WAVES 20

char * slice_str(const char * str, char * buffer, int start, int end) {
    int j = 0;
    for (int i = start; i <= end; ++i) {
        buffer[j++] = str[i];
    }
    buffer[j] = 0;
    return buffer;
}

// notation: ./synth <frequency> <amplitude> <phase> <seconds> <sample rate> <wave form> <channels> <device>

int main(int argc, char ** argv) {
	//GEN_INP input = {atoi(argv[1]), atoi(argv[2]), atoi(argv[3]), argv[4], argv[5], argv[6], argv[7]};

	//generate(input);
	char buff[64];
	pthread_t play;
	bool playing = false;
	GEN_INP waves[20] = {0};
	GEN_INP wave = {0};
	int wave_count = 0;
	while(1) {
		char * inp = malloc(MAXCOMMSIZE);
		printf("KldSynth> ");
		fgets(inp, MAXCOMMSIZE, stdin);
		inp[strcspn(inp, "\n")] = '\0';
		if (strcmp(slice_str(inp, buff, 0, 5), ":msine") == 0) {
    		strcpy(wave.name, "main");
			wave.freq = 440;
			wave.cutt_freq = -1;
			wave.amp = 0.5;
			wave.phase = 0;
			wave.sexs = 2;
			wave.sample_rate = 44100;
			strcpy(wave.filter, "None");
    		strcpy(wave.wave_form, "sine");
    		strcpy(wave.channels, "mono");
    		strcpy(wave.pcm_device, "default");
    		printf("Sine wave configured\n");
    		//printf("wave.freq after :msine = %f\n", wave.freq);

		} else if (strcmp(slice_str(inp, buff, 0, 3), ":msq") == 0) {
    		strcpy(wave.name, "main");
			wave.freq = 440;
			wave.cutt_freq = -1;
			wave.amp = 0.5;
			wave.phase = 0;
			wave.sexs = 2;
			wave.sample_rate = 44100;
			strcpy(wave.filter, "None");
    		strcpy(wave.wave_form, "square");
    		strcpy(wave.channels, "mono");
    		strcpy(wave.pcm_device, "default");
    		printf("Square wave configured\n");
    		
		} else if (strcmp(slice_str(inp, buff, 0, 3), ":lpf") == 0) {
			strcpy(wave.filter, "lpf");
			char * cutt_freq = slice_str(inp, buff, 5, strlen(inp)-1);
			wave.cutt_freq = atoi(cutt_freq);
		} else if (strcmp(slice_str(inp, buff, 0, 3), ":hpf") == 0) {
			strcpy(wave.filter, "hpf");
			char * cutt_freq = slice_str(inp, buff, 5, strlen(inp)-1);
			wave.cutt_freq = atoi(cutt_freq);
		} else if (strcmp(inp, ":p") == 0) {
			playing = true;
			SYNTH_WAVES* synth_input = malloc(sizeof(SYNTH_WAVES));
    		synth_input->waves = malloc(wave_count * sizeof(GEN_INP));
    		strcpy(synth_input->type, "add");
    		synth_input->count = wave_count;
    		for (int i = 0; i < wave_count; i++) {
        		synth_input->waves[i] = waves[i];
   			}
    		pthread_create(&play, NULL, generate, synth_input);
			playing = false;
		} else if (strcmp(slice_str(inp, buff, 0, 4), ":stop") == 0) {
			if (playing) {
				printf("Stopping..."); // Actually do this
			}
		} else if (strcmp(slice_str(inp, buff, 0, 1), ":w") == 0) {
			char * n = slice_str(inp, buff, 3, strlen(inp)-1);
			if (wave_count < MAX_WAVES) {
    			strcpy(wave.name, n);
				waves[wave_count] = wave;
				wave_count++;
			}
		} else if (strcmp(inp, ":q") == 0) {
    		free(inp);
			return 0;
		} else {
			printf("Huh?\n");
		}
	}
}
