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

void sound_effect_param(SOUND * s) {
    if (s == NULL || s->fxs == NULL) {
        printf("No effects initialized.\n");
        return;
    }

    printf("--- Effects Chain (%d effects) ---", s->num_fx);

    for (int i = 0; i < s->num_fx; i++) {
        FX *eff = s->fxs[i];
        if (eff == NULL) continue;

        printf("\n[%d] ", eff->index);

        switch (eff->type) {
            case CHORUS:
                printf("Type: CHORUS\n");
                printf("    Mix: %.2f%% | Feedback: %.2f%%\n", eff->mix, eff->feedback);
                if (eff->lfo) {
                    printf("    LFO Rate: %.2f Hz | LFO Depth: %.2f\n", eff->lfo->rate, eff->lfo->depth);
                }
                break;

            case FLANGER:
                printf("Type: FLANGER\n");
                printf("    Mix: %.2f%% | Feedback: %.2f%%\n", eff->mix, eff->feedback);
                if (eff->flanger_state) {
                    printf("    Base Delay: %.2f samples\n", eff->flanger_state->base_delay_samples);
                }
                break;

            case DELAY:
                printf("Type: DELAY\n");
                printf("    Mix: %.2f%% | Feedback: %.2f%% | Time: %.2f ms\n", 
                        eff->mix, eff->feedback, eff->time);
                if (eff->delay_state) {
                    printf("    Delay Samples: %.2f\n", eff->delay_state->delay_samples);
                }
                break;

            case REVERB:
                printf("Type: REVERB\n");
                printf("    Mix: %.2f%% | Feedback: %.2f%%\n", eff->mix, eff->feedback);
                // Note: REVERB uses REVERB_STATE, but MASTER_REVERB contains the arrays
                break;

            case PHASER:
                printf("Type: PHASER\n");
                printf("    Mix: %.2f%% | Feedback: %.2f%%\n", eff->mix, eff->feedback);
                if (eff->phaser_state) {
                    printf("    Range: %.2f Hz - %.2f Hz | Q: %.2f\n", 
                            eff->phaser_state->min_freq, 
                            eff->phaser_state->max_freq, 
                            eff->phaser_state->Q);
                }
                break;

            default:
                printf("Type: UNKNOWN\n");
                break;
        }
    }
    printf("---------------------------------------\n");
}

const char * get_filter_name(FILTER_TYPES type) {
	switch(type) {
		case HPF_F:   return "high pass";
		case LPF_F:    return "low pass";
		case HPF_F_2:   return "2nd order high pass";
		case LPF_F_2: return "2nd order low pass";
		case BPF_F: return "band pass";
		case NOTCH_F:    return "notch";
		case LOW_SHELF_F: return "low shelf";
		case HIGH_SHELF_F: return "high shelf";
		case ALL_PASS_F: return "all pass filter";
		case PEAKING_EQ_F: return "peaking eq";
		default:          return "unknown";
	}
}

const char * get_filt_adv(FILTER_TYPES type) {
    switch (type) {
        case HPF_F:
            return "A high pass filter passes signal below the cuttoff. You may choose a frequency between 20 and 20,000Hz. My suggestion would be to keep it under 10,000Hz";
        default:
            return "idk dude";
    }
}

double get_base_frequency(const char * note_str) {
    if (strcasecmp(note_str, "C") == 0)  return C;
    if (strcasecmp(note_str, "C#") == 0) return C_sharp;
    if (strcasecmp(note_str, "D") == 0)  return D;
    if (strcasecmp(note_str, "D#") == 0) return D_sharp;
    if (strcasecmp(note_str, "E") == 0)  return E;
    if (strcasecmp(note_str, "F") == 0)  return F;
    if (strcasecmp(note_str, "F#") == 0) return F_sharp;
    if (strcasecmp(note_str, "G") == 0)  return G;
    if (strcasecmp(note_str, "G#") == 0) return G_sharp;
    if (strcasecmp(note_str, "A") == 0)  return A;
    if (strcasecmp(note_str, "A#") == 0) return A_sharp;
    if (strcasecmp(note_str, "B") == 0)  return B;
    return -1.0;
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
		} else if (strcmp(slice_str(inp, buff, 0, 2), "msq") == 0) {
			sound->waves[sound->curr_wave]->type = WAVE_SQUARE;
		} else if (strcmp(slice_str(inp, buff, 0, 3), "msaw") == 0) {
    		sound->waves[sound->curr_wave]->type = WAVE_SAW;
		} else if (strcmp(slice_str(inp, buff, 0, 3), "mtri") == 0) {
    		sound->waves[sound->curr_wave]->type = WAVE_TRI;
		} else if (strcmp(inp, "new_wave") == 0) {
			if (sound->num_waves < MAX_WAVES) {
				sound->waves[sound->num_waves] = (WAVE *)malloc(sizeof(WAVE));
				init_wave(sound->waves[sound->num_waves], sound->num_waves);
				sound->curr_wave = sound->num_waves;
				sound->num_waves++;
				printf("Created Wave %d\n", sound->curr_wave);
			}
		}
		// Filter Commands (for individial waves)
		else if (strcmp(slice_str(inp, buff, 0, 2), "lpf") == 0 ||
			strcmp(slice_str(inp, buff, 0, 2), "hpf") == 0 ||
			strcmp(slice_str(inp, buff, 0, 2), "bpf") == 0 ||
			strcmp(slice_str(inp, buff, 0, 4), "notch") == 0) {
    			WAVE * cw = NULL;
    			bool space_available = false;
    			
				if (sound->focus == WAVE_SPEC) {
    				if (sound->num_waves > 0) {
            			cw = sound->waves[sound->curr_wave];
            			if (cw->num_filters < 5) space_available = true;
        			} else {
            			printf("Error: No waves exist to add filters to.\n");
        			}
    			} else { // GLOBAL focus
        			if (sound->num_filts < 5) space_available = true;
    			}	
			
		if (space_available) {

    		// Determine Type
			FILTER_TYPES f_type = LPF_F;
			if (inp[0] == 'h') {
    			f_type = (strstr(inp, "2") != NULL) ? HPF_F_2 : HPF_F;
			}
			else if (inp[0] == 'b') f_type = BPF_F;
			else if (inp[0] == 'l') {
    			f_type = (strstr(inp, "2") != NULL) ? LPF_F_2 : LPF_F;
			}
			else if (inp[0] == 'n') {
				f_type = NOTCH_F;
			} 

    		if (sound->focus != GLOBAL) {
        		cw->filters = realloc(cw->filters, sizeof(FILTER*) * (cw->num_filters + 1));
				cw->filters[cw->num_filters] = (FILTER*)malloc(sizeof(FILTER));

				// Initialize with USABLE defaults (Not 20kHz which is often inaudible)
				init_filter(cw->filters[cw->num_filters], cw->num_filters, f_type);
				cw->filters[cw->num_filters]->cutt_freq = 1000.0;
				cw->filters[cw->num_filters]->resonance = 0.707; // Standard Q
				cw->filters[cw->num_filters]->gain = 1.0;

				cw->curr_filter = cw->num_filters; // Auto-select the new filter
				cw->num_filters++;

				printf("Added %s to Wave %d at Filter Index %d\n", get_filter_name(f_type), sound->curr_wave, cw->curr_filter);
    		} else {
        		sound->glob_filters = realloc(sound->glob_filters, sizeof(FILTER *) * (sound->num_filts+1));
        		sound->glob_filters[sound->num_filts] = (FILTER *)malloc(sizeof(FILTER));

        		init_filter(sound->glob_filters[sound->num_filts], sound->num_filts, f_type);
				sound->glob_filters[sound->num_filts]->cutt_freq = 1000.0;
				sound->glob_filters[sound->num_filts]->resonance = 0.707; // Standard Q
				sound->glob_filters[sound->num_filts]->gain = 1.0;

				sound->curr_filter = sound->num_filts;
				sound->num_filts++;

				printf("Added %s to Global at Filter Index %d\n", get_filter_name(f_type), sound->curr_filter);
    		}
			
		} else {
			printf("Error: Max filters reached (5).\n");
		}
			}

			// --- Filter Parameter Editing ---
			else if (strncmp(inp, "cutoff ", 7) == 0) {
				float cut = atof(inp + 7);
				FILTER * target_f = NULL;
				if (sound->focus == WAVE_SPEC) {
    				WAVE *cw = sound->waves[sound->curr_wave];
    				if (cw->num_filters > 0) target_f = cw->filters[cw->curr_filter];
				} else {
    				if (sound->num_filts > 0) {
        				target_f = sound->glob_filters[sound->curr_filter];
    				}
				}
				if (target_f && cut >= 20.0 && cut <= 20000.0) {
        			target_f->cutt_freq = cut;
        			printf("%s Filter Cutoff -> %.2f Hz\n", (sound->focus == GLOBAL ? "Master/Global" : "Wave"), cut);
				}

			} else if (strncmp(inp, "res ", 4) == 0) {
				float res = atof(inp + 4);
				FILTER * target_f = NULL;
				int filt_ind;
				const char * foc = "MASTER";
				if (sound->focus == WAVE_SPEC) {
    				WAVE *cw = sound->waves[sound->curr_wave];
    				if (cw->num_filters > 0) target_f = cw->filters[cw->curr_filter];
    				foc = "WAVE";
    				filt_ind = cw->curr_filter;
				} else {
    				if (sound->num_filts > 0) target_f = sound->glob_filters[sound->curr_filter];
    				filt_ind = sound->curr_filter;
				}
				
				target_f->resonance = res; // Usually 0.0 to 10.0+
				printf("Filter %d Resonance -> %.2f (focus: %s)\n", filt_ind, target_f->resonance, foc);
			}

			// --- Filter Navigation ---
			else if (strcmp(inp, "lf") == 0) {
				if (sound->focus == WAVE_SPEC) {
					if (sound->num_waves == 0) {
						printf("No waves created yet.\n");
					} else {
						WAVE *cw = sound->waves[sound->curr_wave];
						printf("--- Filters for Wave %d ---\n", sound->curr_wave);
						for (int i = 0; i < cw->num_filters; i++) {
							printf("%s [%d] %s (Freq: %.1f)\n",
								   (i == cw->curr_filter ? "->" : "   "),
								   i, get_filter_name(cw->filters[i]->type), cw->filters[i]->cutt_freq);
						}
					}
				} else { // FOCUS_MASTER
					printf("--- Master Effects Chain ---\n");
					for (int i = 0; i < sound->num_filts; i++) {
						printf("%s [%d] %s (Freq: %.1f)\n",
							   (i == sound->curr_filter ? "->" : "   "),
							   i, get_filter_name(sound->glob_filters[i]->type), sound->glob_filters[i]->cutt_freq);
					}
					if (sound->num_filts == 0) printf(" (No master filters active)\n");
				}
			} else if (strcmp(inp, "nf") == 0) {
				if (sound->focus == WAVE_SPEC) {
    				WAVE * cw = sound->waves[sound->curr_wave];
					if (cw->curr_filter + 1 < cw->num_filters) {
    					cw->curr_filter+=1;
					} else {
    					cw->curr_filter = 0;
					}
					printf("Current Filter: Index %d, Type %s\n", cw->curr_filter, get_filter_name(cw->filters[cw->curr_filter]->type));
				} else if (sound->focus == GLOBAL) {
    				if (sound->curr_filter + 1 < sound->num_filts) {
        				sound->curr_filter+=1;
    				} else {
        				sound->curr_filter = 0;
    				}
    				printf("Current Filter: Index %d, Type %s\n", sound->curr_filter, get_filter_name(sound->glob_filters[sound->curr_filter]->type));
				}
			} else if (strcmp(inp, "pf") == 0) {
    			if (sound->focus == WAVE_SPEC) {
    				WAVE * cw = sound->waves[sound->curr_wave];
					if (cw->curr_filter - 1 > 0) {
    					cw->curr_filter-=1;
					} else {
    					cw->curr_filter = cw->num_filters-1;
					}
					printf("Current Filter: Index %d, Type %s\n", cw->curr_filter, get_filter_name(cw->filters[cw->curr_filter]->type));
				} else if (sound->focus == GLOBAL) {
    				if (sound->curr_filter - 1 >= 0) {
        				sound->curr_filter-=1;
    				} else {
        				sound->curr_filter = sound->num_filts-1;
    				}
    				printf("Current Filter: Index %d, Type %s\n", sound->curr_filter, get_filter_name(sound->glob_filters[sound->curr_filter]->type));
				}
			} else if (strncmp(inp, "cf ", 3) == 0) {
				int f_idx = atoi(inp + 3);

				if (sound->focus == WAVE_SPEC) {
					WAVE *cw = sound->waves[sound->curr_wave];
					if (f_idx >= 0 && f_idx < cw->num_filters) {
						cw->curr_filter = f_idx;
						printf("Wave %d: Now editing Filter %d (%s)\n",
							   sound->curr_wave, f_idx, get_filter_name(cw->filters[f_idx]->type));
					} else {
						printf("Error: Invalid filter index for Wave %d\n", sound->curr_wave);
					}
				} else { // FOCUS_MASTER
					if (f_idx >= 0 && f_idx < sound->num_filts) {
						sound->curr_filter = f_idx;
						printf("Master: Now editing Filter %d (%s)\n",
							   f_idx, get_filter_name(sound->glob_filters[f_idx]->type));
					} else {
						printf("Error: Invalid master filter index.\n");
					}
				}
			}
		else if (strcmp("focus", slice_str(inp, buff, 0, 4)) == 0) {
    		char filt_focus[10];
    		if (sscanf(inp + 6, "%s", filt_focus) == 1) {
        		if (strcmp(filt_focus, "global") == 0) {
            		sound->focus = GLOBAL;
            		printf("Filter focus switched to Global\n");
        		} else if (strcmp(filt_focus, "wave") == 0) {
            		sound->focus = WAVE_SPEC;
            		printf("Filter focus switched to Wave Specific\n");
        		} else {
            		printf("Set to either global or wave\n");
        		}
    		}
    	// effects stuff
		} else if (strcmp("fx", slice_str(inp, buff, 0, 1)) == 0) {
    		char * typ = inp + 3;
    		FX_TYPES type_fx;
    		bool type_ass = false;
    		
        		if (strcmp(typ, "chorus") == 0) {
            		type_fx = CHORUS;
            		type_ass = true;
        		} else if (strcmp(typ, "reverb") == 0) {
            		type_fx = REVERB;
            		type_ass = true;
        		} else if (strcmp(typ, "flanger") == 0) {
            		type_fx = FLANGER;
            		type_ass = true;
        		} else if (strcmp(typ, "delay") == 0) {
            		type_fx = DELAY;
            		type_ass = true;
        		} else if (strcmp(typ, "phaser") == 0) {
            		type_fx = PHASER;
            		type_ass = true;
        		} else if (strcmp(typ, "params") == 0) {
            		sound_effect_param(sound);
            		type_ass = true;
        		} else {
            		type_ass = false;
        		}
    		if (type_ass) {
        		sound->fxs = (FX **)realloc(sound->fxs, sizeof(FX *) * (sound->num_fx + 1));	
        		sound->fxs[sound->num_fx] = (FX *)malloc(sizeof(FX));	
        		
        		init_FX(sound->fxs[sound->num_fx], sound->num_fx, type_fx, sound->sample_rate);
        		
        		sound->curr_fx = sound->num_fx;
				sound->num_fx++;
        		printf("Added a %s filter\n", typ);
    		} else {
        		printf("Effects allowed: chorus, reverb, flanger, delay, phaser\n");
    		}
		} else if (strcmp(slice_str(inp, buff, 0, 2), "pan") == 0) {
			float pa = atof(inp + 4); // Start after "pan "
			if (pa <= 1.0f && pa >= -1.0f) {
				sound->pan = pa;
				printf("Pan set to %.2f\n", pa);
			}
		} else if (strcmp(slice_str(inp, buff, 0, 3), "note") == 0) {
    		char note_name[4];
    		int octave = 0;
    
    		// Parses "note C# 1" or "note A -1" where 1 and -1 set the octave
    		if (sscanf(inp + 5, "%s %d", note_name, &octave) >= 1) {
        		
        		double base_freq = get_base_frequency(note_name);
        
        		if (base_freq > 0) {
            		WAVE *cw = sound->waves[sound->curr_wave];
            		// Shift frequency based on octave: freq * 2^octave
            		cw->pitch = base_freq;
            		cw->octave = octave;
            		printf("Wave %d pitch set to %s (Octave %d): %.2f Hz\n", sound->curr_wave, note_name, octave, cw->pitch);
        		} else {
            		printf("Invalid note name: %s\n", note_name);
        		}
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
				playing = false;
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
		} else if (strncmp(inp, "rm ", 3) == 0) {
			char target[MAXCOMMSIZE];
			sscanf(inp + 3, "%s", target);

			if (strcmp(target, "wave") == 0) {
				if (sound->num_waves > 0) {
					int idx = sound->curr_wave;
					printf("Removing Wave %d (%s)...\n", idx, get_wave_name(sound->waves[idx]->type));

					// Logic to free and shift the wave array
					free(sound->waves[idx]); // Ensure you have a deep free if filters are attached
					for (int i = idx; i < sound->num_waves - 1; i++) {
						sound->waves[i] = sound->waves[i + 1];
					}
					sound->num_waves--;
					if (sound->curr_wave >= sound->num_waves && sound->num_waves > 0) {
						sound->curr_wave = sound->num_waves - 1;
					}
				} else {
					printf("No waves to remove.\n");
				}
			} else if (strcmp(target, "filter") == 0) {
				FILTER **target_list = NULL;
				int *count = NULL;
				int *current_idx = NULL;

				// Determine if we are removing from Global or a Specific Wave
				if (sound->focus == WAVE_SPEC && sound->num_waves > 0) {
					WAVE *cw = sound->waves[sound->curr_wave];
					target_list = cw->filters;
					count = &cw->num_filters;
					current_idx = &cw->curr_filter;
				} else if (sound->focus == GLOBAL) {
					target_list = sound->glob_filters;
					count = &sound->num_filts;
					current_idx = &sound->curr_filter;
				}

				if (count && *count > 0) {
					printf("Current Filters:\n");
					for (int i = 0; i < *count; i++) {
						printf("[%d] %s (%.1f Hz)\n", i, get_filter_name(target_list[i]->type), target_list[i]->cutt_freq);
					}
					printf("Enter filter index to remove> ");
					int rm_idx;
					scanf("%d", &rm_idx);
					getchar(); // Clear newline from buffer

					if (rm_idx >= 0 && rm_idx < *count) {
						free(target_list[rm_idx]);
						for (int i = rm_idx; i < *count - 1; i++) {
							target_list[i] = target_list[i + 1];
						}
						(*count)--;
						if (*current_idx >= *count && *count > 0) *current_idx = *count - 1;
						printf("Filter removed.\n");
					} else {
						printf("Invalid index.\n");
					}
				} else {
					printf("No filters to remove. Maybe in wrong focus?\n");
				}
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
