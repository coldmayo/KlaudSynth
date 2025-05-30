#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <pthread.h>
#include <signal.h>
#include <unistd.h>

#include "types.h"
#include "gen.h"
//#include "plots.h"

#define MAXCOMMSIZE 256
#define MAX_WAVES 20

pthread_t play;
volatile bool playing = false;
volatile bool stop_req = false;

void handle_sigint(int sig) {
    if (playing) {
        stop_req = true;
        pthread_join(play, NULL);
    }
    exit(0);
}

char * slice_str(const char * str, char * buffer, int start, int end) {
    int j = 0;
    for (int i = start; i <= end; ++i) {
        buffer[j++] = str[i];
    }
    buffer[j] = 0;
    return buffer;
}

void * show_veiwer(void * args) {
	system("./SynthVeiw");
}

// type:
    // 0: sine
    // 1: square
    // 2: sawtooth
void basic_wav(GEN_INP * wave, int type) {
	if (!wave) return;
	
	strcpy(wave->name, "Not Saved");
	wave->freq = 440;
	wave->amp = 0.5;
	wave->phase = 0;
	wave->sexs = 2;
	wave->sample_rate = 44100;
	wave->pan = 0.0;
	wave->phase_offset = 0.25f; // Default 90° phase offset
	wave->delay = 0.0f;
	wave->feedback = 0.0f;
	wave->chorus_rate = 0.0f;
	wave->chorus_depth = 0.0f;
	
	switch (type) {
		case 0:
    		strcpy(wave->wave_form, "sine");
    		strcpy(wave->wave_form_r, "sine");
    		break;
    	case 1:
        	strcpy(wave->wave_form, "square");
        	strcpy(wave->wave_form_r, "square");
        	break;
        case 2:
            strcpy(wave->wave_form, "saw");
            strcpy(wave->wave_form_r, "saw");
            break;
        case 3:
            strcpy(wave->wave_form, "tri");
            strcpy(wave->wave_form_r, "tri");
            break;
        default:
            strcpy(wave->wave_form, "sine");
            strcpy(wave->wave_form_r, "sine");
	}
    
	strcpy(wave->channels, "stereo");
    strcpy(wave->pcm_device, "default");
    wave->saved = false;
    printf("%s wave configured\n", wave->wave_form);
}

void show_info(GEN_INP * wave) {
	printf("\n---General---\n\n");
	printf("Name: %s\n", wave->name);
	printf("Wave type (left): %s\n", wave->wave_form);
	printf("Wave type (right): %s\n", wave->wave_form_r);
	printf("Channel Setting: %s\n", wave->channels);
	printf("Seconds played: %f\n", wave->sexs);
	printf("\n---Wave Stats---\n\n");
	printf("Frequency: %f Hz\n", wave->freq);
	printf("Amplitude: %f\n", wave->amp);
	printf("Phase: %f\n", wave->phase);
	printf("Sample Rate: %d Hz\n", wave->sample_rate);
	printf("\n---Filter and Effects Stuff---\n\n");
	if (wave->filter_num > 0) {
		printf("Filters applied:");
		for (int i = 0; wave->filter_num > i; i++) {
			printf(" %s", wave->filters[i].name);
		}
		printf("\n");
	} else {
		printf("No filters applied currently");
	}
	
	printf("Panning: %f\n", wave->pan);
	printf("Phase offset: %f\n", wave->phase_offset);
	printf("Delay: %f\n", wave->delay);
	printf("Feedback: %f\n", wave->feedback);
	printf("Chorus Rate: %f\n", wave->chorus_rate);
	printf("Chorus Depth: %f\n", wave->chorus_depth);
}

int main(int argc, char ** argv) {

	signal(SIGINT, handle_sigint);
	char buff[64];
	GEN_INP waves[20] = {0};
	GEN_INP wave = {0};
	bool wave_q = false;
	int wave_count = 0;
	char id[69] = "KlaudSynth";
	int global_filter_num = 0;
	while(1) {
		char * inp = malloc(MAXCOMMSIZE);
		printf("%s> ", id);
		fgets(inp, MAXCOMMSIZE, stdin);
		inp[strcspn(inp, "\n")] = '\0';
		if (strcmp(slice_str(inp, buff, 0, 4), "msine") == 0) {
    		basic_wav(&wave, 0);
    		wave_q = true;
    		//printf("wave.freq after :msine = %f\n", wave.freq);
		} else if (strcmp(slice_str(inp, buff, 0, 2), "msq") == 0) {
    		basic_wav(&wave, 1);
    		wave_q = true;
		} else if (strcmp(inp, "msaw") == 0) {
			basic_wav(&wave, 2);
			wave_q = true;
		} else if (strcmp(slice_str(inp, buff, 0, 3), "mtri") == 0) {
			basic_wav(&wave, 3);
			wave_q = true;
		} else if (strcmp(slice_str(inp, buff, 0, 3), "info") == 0) {
			if (strlen(inp) > 4) {
				char * search = slice_str(inp, buff, 5, strlen(inp)-1);
				for (int i = 0; i < wave_count; i++) {
					if (strcmp(waves[i].name, search) == 0) {
						show_info(&waves[i]);
						break;
					}
				}
			} else {
				show_info(&wave);
			}
		} else if (strcmp(slice_str(inp, buff, 0, 2), "lpf") == 0) {
			strcpy(wave.filters[wave.filter_num].name, "lpf");
			char * cutt_freq = slice_str(inp, buff, 4, strlen(inp)-1);
			wave.filters[wave.filter_num].cutt_freq = atoi(cutt_freq);
			wave.filters[wave.filter_num].Q = -1;
			wave.filter_num += 1;
		} else if (strcmp(slice_str(inp, buff, 0, 2), "hpf") == 0) {
			strcpy(wave.filters[wave.filter_num].name, "hpf");
			char * cutt_freq = slice_str(inp, buff, 4, strlen(inp)-1);
			wave.filters[wave.filter_num].cutt_freq = atoi(cutt_freq);
			wave.filters[wave.filter_num].Q = -1;
			wave.filter_num += 1;
		} else if (strcmp(slice_str(inp, buff, 0, 2), "pan") == 0) {
			char * p = slice_str(inp, buff, 4, strlen(inp) - 1);
			float pa = atof(p);
			if (pa <= 1 && pa >= -1) {
    			printf("Set pan to: %f\n", pa);
				wave.pan = pa;
			}
		} else if (strcmp(slice_str(inp, buff, 0, 2), "amp") == 0) {
			char * a = slice_str(inp, buff, 4, strlen(inp)-1);
			float am = atof(a);
			wave.amp = am;
			printf("Set amplitude to: %f\n", am);
		} else if (strcmp(slice_str(inp, buff, 0, 0), "p") == 0) {
    		if (!playing) {
				playing = true;
				stop_req = false;
				SYNTH_WAVES* synth_input = malloc(sizeof(SYNTH_WAVES));
    			synth_input->waves = malloc(wave_count * sizeof(GEN_INP));
    			if (strlen(inp) > 1) {
					strcpy(synth_input->type, slice_str(inp, buff, 2, strlen(inp)-1));
    			} else {
					strcpy(synth_input->type, "sub");
    			}
    			synth_input->count = wave_count;
    			printf("waves playing:\n");
    			for (int i = 0; i < wave_count; i++) {
        			synth_input->waves[i] = waves[i];
        			printf("%s\n", synth_input->waves[i].name);
   				}
   				printf("synthesis type: %s\n", synth_input->type);
    			int res = pthread_create(&play, NULL, generate, synth_input);
    			if (res != 0) {
            		fprintf(stderr, "Error creating thread: %d\n", res);
            		playing = false;
            		free(synth_input->waves);
            		free(synth_input);
        		}
    		} else {
				printf("Already playing\n");
    		}
		} else if (strcmp(slice_str(inp, buff, 0, 3), "stop") == 0) {
			if (playing) {
                stop_req = true;
                pthread_join(play, NULL);
                printf("Playback stopped\n");
            } else {
                printf("No playback in progress\n");
            }
		} else if (inp[0] == 'w') {
			char * n = slice_str(inp, buff, 2, strlen(inp)-1);
			if (wave.saved == false && MAX_WAVES > wave_count) {
    			wave.saved = true;
				strcpy(wave.name, n);
				waves[wave_count] = wave;
				wave_count++;
			} else {
				for (int i = 0; wave_count > i; i++) {
					if (strcmp(waves[i].name, wave.name) == 0) {
						waves[i] = wave;   // overwriteing the old one I guess
					}
				}
			}
		} else if (strcmp(inp, "q") == 0) {
    		free(inp);
			return 0;
		} else if (strcmp(inp, "ls") == 0) {
			for (int i = 0; i < wave_count; i++) {
				printf("%s\n", waves[i].name);
			}
		} else if (strcmp(slice_str(inp, buff, 0, 3), "edit") == 0) {
			char * search = slice_str(inp, buff, 5, strlen(inp)-1);
			bool didthing = false;
			for (int i = 0; i < wave_count; i++) {
				if (strcmp(waves[i].name, search) == 0) {
					wave = waves[i];
					didthing = true;
					break;
				}
			}
			if (!didthing) {
				printf("Could not find that wave, use the ls command to see all the created waves\n");
			}
		} else if (strcmp(slice_str(inp, buff, 0, 3), "veiw") == 0) {
			pthread_create(&play, NULL, show_veiwer, NULL);
		} else {
			printf("Huh?\n");
		}
		if (wave_q) {
			strcpy(id, wave.name);
		}
	}
}
