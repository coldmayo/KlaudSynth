#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <pthread.h>
#include <signal.h>
#include <unistd.h>

#include "types.h"
#include "gen.h"
#include "filter.h"
#include "init.h"

#define MAXCOMMSIZE 256
#define MAX_WAVES 20

pthread_t play_thread;
pthread_t veiw_thread;
volatile bool playing = false;
volatile bool stop_req = false;

// Helper to safely slice strings for command parsing
char * slice_str(const char * str, char * buffer, int start, int end) {
	int len = strlen(str);
	if (start >= len) {
		buffer[0] = '\0';
		return buffer;
	}
	if (end >= len) end = len - 1;

	int j = 0;
	for (int i = start; i <= end; ++i) {
		buffer[j++] = str[i];
	}
	buffer[j] = '\0';
	return buffer;
}

void handle_sigint(int sig) {
	if (playing) {
		stop_req = true;
		pthread_join(play_thread, NULL);
	}
	printf("\nExiting Synth...\n");
	exit(0);
}

void * show_veiwer(void * args) {
	system("./SynthVeiw");
}

const char * get_wave_name(WaveType type) {
	switch(type) {
		case WAVE_SINE:   return "sine";
		case WAVE_SAW:    return "saw";
		case WAVE_SQUARE: return "square";
		case WAVE_TRI:    return "triangle";
		default:          return "unknown";
	}
}

const char * get_filter_name(FILTER_TYPES type) {
	switch(type) {
		case HPF_F:   return "high pass";
		case LPF_F:    return "low pass";
		case BPF_F: return "band pass";
		case NOTCH_F:    return "notch";
		case LOW_SHELF_F: return "low shelf";
		case HIGH_SHELF_F: return "high shelf";
		case ALL_PASS_F: return "all pass filter";
		case PEAKING_EQ_F: return "peaking eq";
		default:          return "unknown";
	}
}

int main(int argc, char ** argv) {
	signal(SIGINT, handle_sigint);

	char buff[MAXCOMMSIZE];
	char inp[MAXCOMMSIZE];
	SOUND * sound = init_sound();

	printf("Welcome to C-Synth. Type 'new_wave' to start.\n");

	while(1) {
		printf("Wave[%d]> ", sound->curr_wave);
		if (!fgets(inp, MAXCOMMSIZE, stdin)) break;
		inp[strcspn(inp, "\n")] = '\0';

		if (strcmp(slice_str(inp, buff, 0, 4), "msine") == 0) {
			sound->waves[sound->curr_wave]->type = WAVE_SINE;
		}
		else if (strcmp(slice_str(inp, buff, 0, 2), "msq") == 0) {
			sound->waves[sound->curr_wave]->type = WAVE_SQUARE;
		}
		else if (strcmp(inp, "new_wave") == 0) {
			if (sound->num_waves < MAX_WAVES) {
				sound->waves[sound->num_waves] = (WAVE *)malloc(sizeof(WAVE));
				init_wave(sound->waves[sound->num_waves], sound->num_waves);
				sound->curr_wave = sound->num_waves;
				sound->num_waves++;
				printf("Created Wave %d\n", sound->curr_wave);
			}
		}
		// Filter Commands
		else if (strcmp(slice_str(inp, buff, 0, 2), "lpf") == 0) {
			WAVE *cw = sound->waves[sound->curr_wave];
			if (cw->num_filters < 5) { // Assuming max 5 filters
				cw->filters[cw->num_filters] = (FILTER*)malloc(sizeof(FILTER));
				init_filter(cw->filters[cw->num_filters], cw->num_filters, LPF_F);
				cw->num_filters++;
				printf("LPF added to Wave %d\n", sound->curr_wave);
			}
		} else if (strcmp(slice_str(inp, buff, 0, 2), "hpf") == 0) {
			WAVE *cw = sound->waves[sound->curr_wave];
			if (cw->num_filters < 5) {
    			cw->filters[cw->num_filters] = (FILTER*)malloc(sizeof(FILTER));
				init_filter(cw->filters[cw->num_filters], cw->num_filters, HPF_F);
				cw->num_filters++;
				printf("HPF added to Wave %d\n", sound->curr_wave);
			}
		} else if (strcmp(slice_str(inp, buff, 0, 2), "bpf") == 0) {
			WAVE *cw = sound->waves[sound->curr_wave];
			if (cw->num_filters < 5) {
    			cw->filters[cw->num_filters] = (FILTER*)malloc(sizeof(FILTER));
				init_filter(cw->filters[cw->num_filters], cw->num_filters, BPF_F);
				cw->num_filters++;
				printf("BPF added to Wave %d\n", sound->curr_wave);
			}
		} else if (strcmp(slice_str(inp, buff, 0, 4), "notch") == 0) {
			WAVE *cw = sound->waves[sound->curr_wave];
			if (cw->num_filters < 5) {
    			cw->filters[cw->num_filters] = (FILTER*)malloc(sizeof(FILTER));
				init_filter(cw->filters[cw->num_filters], cw->num_filters, NOTCH_F);
				cw->num_filters++;
				printf("Notch filter added to Wave %d\n", sound->curr_wave);
			}
		} else if (strcmp(slice_str(inp, buff, 0, 10), "cutoff freq") == 0) {
    		float cut = atof(inp + 12);
    		WAVE * wave_rn = sound->waves[sound->curr_wave];
    		FILTER * filter_i = wave_rn->curr_filter;
    		if (cut < 20000 && cut > 0) {
        		sound->waves[sound->curr_wave]->filters[filter_i]->cutt_freq = cut;
    		}
    		printf("Cut off Frequency for %s (index %d) filter is %d", get_filter_name(wave_rn->filters[filter_i]->type), filter_i, sound->waves[sound->curr_wave]->filters[filter_i]->cutt_freq);
		} else if (strcmp(slice_str(inp, buffer, 0, 8), "resonance\n") == 0) {
    		WAVE * wave_rn = sound->waves[sound->curr_wave];
    		FILTER * filter_i = wave_rn->curr_filter;
    		float res = atof(inp + 10);
    		if (res > 0 && res < 20000) {
        		sound->waves[sound->curr_wave]->filters[filter_i]->resonance = res;
    		}
    		printf("Resonance Frequency for %s (index %d) filter is %d\n", get_filter_name(wave_rn->filters[filter_i]->type), filter_i, sound->waves[sound->curr_wave]->filters[filter_i]->resonance);
		} else if (strcmp(slice_str(inp, buff, 0, 4), "drive")) {
    		WAVE * wave_rn = sound->waves[sound->curr_wave];
    		FILTER * filter_i = wave_rn->curr_filter;
    		float drive = atof(inp + 6);
    		if (drive > 0 && drive < 20000) {
        		sound->waves[sound->curr_wave]->filters[filter_i]->drive = drive;
    		}
    		printf("Drive for %s (index %d) filter is %d\n", get_filter_name(wave_rn->filters[filter_i]->type), filter_i, sound->waves[sound->curr_wave]->filters[filter_i]->drive);
		} else if (strcmp(slice_str(inp, buff, 0, 10), "center freq")) {
    		WAVE * wave_rn = sound->waves[sound->curr_wave];
    		FILTER * filter_i = wave_rn->curr_filter;
    		float cent = atof(inp + 12);
    		if (cent > 0 && cent < 20000) {
        		sound->waves[sound->curr_wave]->filters[filter_i]->center_freq = cent;
    		}
    		printf("Center Frequency for %s (index %d) filter is %d\n", get_filter_name(wave_rn->filters[filter_i]->type), filter_i, sound->waves[sound->curr_wave]->filters[filter_i]->center_freq);
		} else if (strcmp(slice_str(inp, buff, 0, 2), "pan") == 0) {
			float pa = atof(inp + 4); // Start after "pan "
			if (pa <= 1.0f && pa >= -1.0f) {
				sound->pan = pa;
				printf("Pan set to %.2f\n", pa);
			}
		} else if (strcmp(inp, "p") == 0) {
			if (!playing) {
				sound->env->state = ATTACK;
				for (int i = 0; i < sound->num_waves; i++) {
					sound->waves[i]->is_active = true;
				}
				playing = true;
				stop_req = false;
				pthread_create(&play_thread, NULL, generate, sound);
				printf("Playback started...\n");
			}
		} else if (strcmp(inp, "stop") == 0) {
			if (playing) {
				stop_req = true;
				pthread_join(play_thread, NULL);
				playing = false;
				printf("Playback stopped.\n");
			}
		} else if (strcmp(inp, "ls") == 0) {
    		for (int i = 0; sound->num_waves > i; i++) {
        		printf("Index: %d Type: %s\n", i, get_wave_name(sound->waves[i]->type));
    		}
    	// change wave
		} else if (strcmp(inp, "cw") == 0) {
    		int wn;
    		for (int i = 0; i < sound->num_waves; i++) {
        		printf("Index: %d, Type: %s\n", i, get_wave_name(sound->waves[i]->type));
    		}
    		printf("Choose which wave to switch to> ");
    		scanf("%d", &wn);
    		if (wn <= sound->num_waves) {
        		sound->curr_wave = wn;
        		printf("Current Wave:\nIndex: %d, Type: %s\n", sound->curr_wave, get_wave_name(sound->waves[sound->curr_wave]->type));
    		}
    	// next wave
		} else if (strcmp(inp, "nw") == 0) {
    		if (sound->curr_wave + 1 > sound->num_waves-1) {
        		sound->curr_wave = 0;
    		} else {
        		sound->curr_wave += 1;
    		}
    		printf("Current Wave:\nIndex: %d, Type: %s\n", sound->curr_wave, get_wave_name(sound->waves[sound->curr_wave]->type));
    	// previous wave
		} else if (strcmp(inp, "pw") == 0) {
    		if (sound->curr_wave - 1 < 0) {
        		sound->curr_wave = 0;
    		} else {
        		sound->curr_wave -= 1;
    		}
		} else if (strcmp(inp, "veiw") == 0) {
    		pthread_create(&veiw_thread, NULL, show_veiwer, NULL);
		} else if (strcmp(inp, "q") == 0) {
			break;
		}
	}

	// Cleanup before exit
	if (playing) {
		stop_req = true;
		pthread_join(play_thread, NULL);
	}
	// Note: You should call a full cleanup_sound() here to free all waves/filters
	return 0;
}
