#include <alsa/asoundlib.h>
#include <alsa/pcm.h>
#include <stdio.h>
#include <math.h>
#include <stdbool.h>
//#include "main.h"
#include "types.h"
#include "filter.h"
#include "amp.h"
#include "effects.h"

extern volatile bool playing;
extern volatile bool stop_req;

// sine wave

float gen_sin(float phase) {
	return sinf(2.0f* M_PI* phase);
}

// sqaure wave

float square(float phase) {
	float val = sinf(2.0f*M_PI*phase);
	if (val > 0) {
		return 1;
	} else {
		return -1;
	}
}

// sawtooth wave functions:

float saw_mod(float phase) {
    float ret = (2.0*fmod(phase, 1.0))-1.0;
	return ret;
}

float saw_series(float phase, int iters) {
	float val = 0.0;
	for (int i = 0; i < iters; i++) {
		val += (1.0/i)*sinf(2*M_PI*i*phase);
	}
	return -2*val/M_PI;
}

// reverse sawtooth


void * generate(void * arg) {
    SYNTH_WAVES * input = (SYNTH_WAVES*)arg;
	snd_pcm_t *pcm_handle;
    snd_pcm_hw_params_t *params;
	int pcm, tmp, buff_size;
	int chans;
	unsigned int sample_rate = input->waves[0].sample_rate;

	// opened pcm device
    if ((pcm = snd_pcm_open(&pcm_handle, input->waves[0].pcm_device, SND_PCM_STREAM_PLAYBACK, 0)) < 0) {
		printf("Cant open PCM Device");
		return NULL;
 	}

    chans = 1;
    if (strcmp(input->waves[0].channels, "stereo") == 0) {
		chans = 2;
    }

    // Initallize + Allocate hardware parameters
    snd_pcm_hw_params_alloca(&params);
	snd_pcm_hw_params_any(pcm_handle, params);

	snd_pcm_hw_params_set_access(pcm_handle, params, SND_PCM_ACCESS_RW_INTERLEAVED);
	snd_pcm_hw_params_set_format(pcm_handle, params, SND_PCM_FORMAT_FLOAT_LE);
	snd_pcm_hw_params_set_channels(pcm_handle, params, chans);
    snd_pcm_hw_params_set_rate_near(pcm_handle, params, &sample_rate, 0);

	snd_pcm_uframes_t buffer_size = 1024;
	snd_pcm_hw_params_set_buffer_size_near(pcm_handle, params, &buffer_size);

	snd_pcm_uframes_t peroid_size = 256;
	snd_pcm_hw_params_set_period_size_near(pcm_handle, params, &peroid_size, 0);

	if ((pcm = snd_pcm_hw_params(pcm_handle, params)) < 0) {
        fprintf(stderr, "Cannot set parameters: %s\n", snd_strerror(pcm));
        return NULL;
    }

    if ((pcm = snd_pcm_prepare(pcm_handle)) < 0) {
        fprintf(stderr, "Cannot prepare interface: %s\n", snd_strerror(pcm));
        return NULL;
    }

	float prev_out[input->count];
	float prev_inp[input->count];
	float phases[input->count];
	float phase_incs[input->count];
	float alphas[input->count];
	DELAY_STATE* delay_states[input->count];
	CHORUS_STATE* chorus_states[input->count];

	for (int i = 0; i < input->count; i++) {
        phases[i] = input->waves[i].phase;
        phase_incs[i] = 1.0f / (float)sample_rate;
        prev_out[i] = 0;
        prev_inp[i] = 0;
        alphas[i] = (2.0f * M_PI * input->waves[i].cutt_freq) / sample_rate;
		delay_states[i] = init_delay(1.0f, sample_rate); // Max 1 second delay
        chorus_states[i] = init_chorus();
    }

	unsigned long frames_remaining = input->waves[0].sexs*sample_rate;
	float buffer[256*chans];

    while (frames_remaining > 0 && !stop_req) {
        unsigned long frames = (frames_remaining > 256) ? 256 : frames_remaining;
        memset(buffer, 0, sizeof(buffer));
        for (int i = 0; i < frames && !stop_req; i++) {
        	float l_mix = 0.0f; float r_mix = 0.0f;
        	for (int w = 0; w < input->count; w++) {
				float l_sample, r_sample;
				if (strcmp(input->waves[w].wave_form, "sine") == 0) {
					l_sample = gen_sin(phases[w]);
				} else if (strcmp(input->waves[w].wave_form, "square") == 0) {
					l_sample = square(phases[w]);
				} else if (strcmp(input->waves[w].wave_form, "saw") == 0) {
					l_sample = saw_mod(phases[w]);
				}

				r_sample = 0.0f;
				if (strcmp(input->waves[w].channels, "stereo") == 0) {
					float right_phase = phases[w] + input->waves[w].phase_offset;
                    if (right_phase >= 1.0f) right_phase -= 1.0f;

                    if (strcmp(input->waves[w].wave_form_r, "sine") == 0) {
                        r_sample = gen_sin(right_phase);
                    } else if (strcmp(input->waves[w].wave_form_r, "square") == 0) {
                        r_sample = square(right_phase);
                    } else if (strcmp(input->waves[w].wave_form_r, "saw") == 0) {
                        r_sample = saw_mod(right_phase);
                    }
				}
				if (input->waves[w].cutt_freq != -1) {
    				if (strcmp(input->waves[w].filter, "lpf") == 0) {
						l_sample = low_pass(l_sample, &prev_out[w], alphas[w], &prev_inp[w]);
						if (strcmp(input->waves[w].channels, "stereo") == 0) {
							r_sample = low_pass(r_sample, &prev_out[w], alphas[w], &prev_inp[w]);
						}
    				} else if (strcmp(input->waves[w].filter, "hpf") == 0) {
						l_sample = high_pass(l_sample, &prev_out[w], alphas[w], &prev_inp[w]);
						if (strcmp(input->waves[w].channels, "stereo") == 0) {
							r_sample = high_pass(r_sample, &prev_out[w], alphas[w], &prev_inp[w]);
						}
    				}
				}

				l_sample = amplify(l_sample, input->waves[w].amp);
				r_sample = amplify(r_sample, input->waves[w].amp);

				// chorus effects
				if (input->waves[w].chorus_rate > 0.0f) {
                    l_sample = process_chorus(l_sample, chorus_states[w],input->waves[w].chorus_rate, input->waves[w].chorus_depth, sample_rate);
                    if (strcmp(input->waves[w].channels, "stereo") == 0) {
                        r_sample = process_chorus(r_sample, chorus_states[w], input->waves[w].chorus_rate, input->waves[w].chorus_depth, sample_rate);
                    }
                }


                // panning gains
                float angle = input->waves[w].pan * M_PI_4;
                float left_gain = cosf(angle) * 0.707f;
                float right_gain = sinf(angle) * 0.707f;

				if (strcmp(input->waves[w].channels, "left") == 0) {
                    right_gain = 0.0f;
                } else if (strcmp(input->waves[w].channels, "right") == 0) {
                    left_gain = 0.0f;
                } else if (strcmp(input->waves[w].channels, "mono") == 0) {
                    r_sample = l_sample;
                }

				if (strcmp(input->type, "add") == 0) {
					l_mix += l_sample * left_gain;
					r_mix += r_sample * right_gain;
				} else if (strcmp(input->type, "sub") == 0) {
					l_mix -= l_sample * left_gain;
					r_mix -= r_sample * right_gain;
				}

				phases[w] += input->waves[w].freq * phase_incs[w];
				if (phases[w] >= 1.0f) phases[w] -= 1.0f;
        	}

			if (input->waves[0].delay > 0.0f) {
                float delayed_left = process_delay(l_mix, delay_states[0],
                                                 input->waves[0].delay,
                                                 input->waves[0].feedback);
                float delayed_right = process_delay(r_mix, delay_states[0],
                                                  input->waves[0].delay * 1.5f,
                                                  input->waves[0].feedback);

                l_mix = l_mix * 0.7f + delayed_left * 0.3f;
                r_mix = r_mix * 0.7f + delayed_right * 0.3f;
            }

            // Soft clipping to prevent distortion
            l_mix = tanhf(l_mix);
            r_mix = tanhf(r_mix);

            // Fill interleaved buffer
            if (chans == 2) {
                buffer[i*2] = l_mix;
                buffer[i*2 + 1] = r_mix;
            } else {
                buffer[i] = (l_mix + r_mix) * 0.5f;
            }

			
        }

        if (stop_req) break;

		snd_pcm_sframes_t frames_written = snd_pcm_writei(pcm_handle, buffer, frames);

		if (frames_written < 0) {
            frames_written = snd_pcm_recover(pcm_handle, frames_written, 0);
        }
        if (frames_written < 0) {
            printf("Error writing to PCM device: %s\n", snd_strerror(frames_written));
            break;
        }

		frames_remaining -= frames_written;
    }

    // Cleanup
    if (stop_req) {
        snd_pcm_drop(pcm_handle);  // Immediately stop playback
    } else {
        snd_pcm_drain(pcm_handle); // Drain if normal completion
    }

    for (int i = 0; i < input->count; i++) {
        free_delay(delay_states[i]);
        free(chorus_states[i]);
    }

    snd_pcm_drain(pcm_handle);
    snd_pcm_close(pcm_handle);

    free(input->waves);
    free(input);
    playing = false;
    stop_req = false;

    return NULL;
}
