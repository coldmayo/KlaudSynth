#include <alsa/asoundlib.h>
#include <alsa/pcm.h>
#include <stdio.h>
#include <math.h>
#include "types.h"
#include "filter.h"
#include "amp.h"

// sine wave

float gen_sin(float phase, float amp) {
	return sinf(2.0f* M_PI * phase);
}

// sqaure wave

float square(float phase, float amp) {
	float val = sinf(2.0f*M_PI*phase);
	if (val > 0) {
		return 1;
	} else {
		return 0;
	}
}

float generate(GEN_INP input) {
	snd_pcm_t *pcm_handle;
    snd_pcm_hw_params_t *params;
	int pcm, tmp, buff_size;
	int chans;
	float alpha_fil = (2.0f * M_PI * input.cutt_freq) / input.sample_rate;
	float phase_inc = input.freq;
	unsigned int sample_rate = input.sample_rate;
// opened pcm device
    if ((pcm = snd_pcm_open(&pcm_handle, input.pcm_device, SND_PCM_STREAM_PLAYBACK, 0)) < 0) {
		printf("Cant open PCM Device");
		return -1;
 	}

    chans = 1;
    if (strcmp(input.channels, "stereo") == 0) {
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
        return -1;
    }

    if ((pcm = snd_pcm_prepare(pcm_handle)) < 0) {
        fprintf(stderr, "Cannot prepare interface: %s\n", snd_strerror(pcm));
        return -1;
    }

	float prev_out = 0;
	float prev_inp = 0;
	float phasei = input.phase;
    while (1) {
        float sample;
		if (strcmp(input.wave_form, "sine") == 0) {
			sample = gen_sin(phasei, input.amp);
		} else if (strcmp(input.wave_form, "square") == 0) {
			sample = square(phasei, input.amp);
		}
		
		sample = low_pass(sample, &prev_out, alpha_fil, &prev_inp);
		sample = amplify(sample, input.amp);

		snd_pcm_writei(pcm_handle, &sample, 1);

		phasei += phase_inc;
		if (phasei >= 1.0f) phasei -= 1.0f;
    }

    snd_pcm_drain(pcm_handle);
    snd_pcm_close(pcm_handle);

    return 0;
}
