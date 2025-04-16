#include <alsa/asoundlib.h>
#include <alsa/pcm.h>
#include <stdio.h>
#include <math.h>
#include "types.h"
#include "filter.h"
#include "amp.h"

// sine wave

float gen_sin(float phase) {
	return sinf(2.0f* M_PI * phase);
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

	for (int i = 0; i < input->count; i++) {
        phases[i] = input->waves[i].phase;
        phase_incs[i] = input->waves[i].freq / (float)sample_rate;
        prev_out[i] = 0;
        prev_inp[i] = 0;
        alphas[i] = (2.0f * M_PI * input->waves[i].cutt_freq) / sample_rate;
    }

	unsigned long frames_remaining = input->waves[0].sexs*sample_rate;
	float buffer[256*chans];

    while (frames_remaining > 0) {
        unsigned long frames = (frames_remaining > 256) ? 256 : frames_remaining;
        memset(buffer, 0, sizeof(buffer));
        for (int i = 0; i < frames; i++) {
        	float mixed_sample = 0.0f;
        	for (int w = 0; w < input->count; w++) {
				float sample;
				if (strcmp(input->waves[w].wave_form, "sine") == 0) {
					sample = gen_sin(phases[w]);
				} else if (strcmp(input->waves[w].wave_form, "square") == 0) {
					sample = square(phases[w]);
				}
				if (input->waves[w].cutt_freq != -1) {
    				if (strcmp(input->waves[w].filter, "lpf") == 0) {
						sample = low_pass(sample, &prev_out[w], alphas[w], &prev_inp[w]);
    				} else if (strcmp(input->waves[w].filter, "hpf") == 0) {
						sample = high_pass(sample, &prev_out[w], alphas[w], &prev_inp[w]);
    				}
				}
				sample = amplify(sample, input->waves[w].amp);
				if (strcmp(input->type, "add") == 0) {
					mixed_sample += sample;
				} else if (strcmp(input->type, "sub") == 0) {
					mixed_sample -= sample;
				}
				phases[w] += phase_incs[w];
				if (phases[w] >= 1.0f) phases[w] -= 1.0f;
        	}
			

			if (chans == 2) { // not working for stereo, fix this!
				buffer[i*2] = buffer[i*2+1] = mixed_sample;
			} else {
				buffer[i] = mixed_sample;
			}

			
        }
		snd_pcm_sframes_t frames_written =snd_pcm_writei(pcm_handle, buffer, frames);

		frames_remaining -= frames_written;
    }

    snd_pcm_drain(pcm_handle);
    snd_pcm_close(pcm_handle);

    return NULL;
}
