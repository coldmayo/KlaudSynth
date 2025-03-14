#include <alsa/asoundlib.h>
#include <stdio.h>
#include <math.h>
#include "types.h"

// sine wave

float sin(float phase, float amp) {
	return sinf(2.0f* M_PI * phase);
}

float generate(GEN_INP * input) {
	snd_pcm_t *pcm_handle;
    snd_pcm_hw_params_t *hw_params;
    snd_pcm_hw_params_t *params;
	int pcm, tmp, buff_size;
	char * buff;
	int CUTOFF_FREQ = 1000;
	float alpha_fil = (2.0f * M_PI * CUTOFF_FREQ) / input.sample_rate;
	float phase_inc = input.freq;
// opened pcm device
    if (pcm = snd_pcm_open(&pcm_handle, PCM_DEVICE, SND_PCM_STREAM_PLAYBACK, 0) < 0) {
		printf("Cant open PCM Device");
		return -1;
 	}
// set params
 	send_pcm_hw_params_alloca(&params);
 	snd_pcm_hw_params_any(pcm_handle, params);
 	snd_pcm_hw_params_set_access(pcm_handle, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED);
    snd_pcm_hw_params_set_format(pcm_handle, hw_params, SND_PCM_FORMAT_FLOAT_LE);
    snd_pcm_hw_params_set_channels(pcm_handle, hw_params, input.channels);
    snd_pcm_hw_params_set_rate_near(pcm_handle, hw_params, &SAMPLE_RATE, 0);
// write params
    snd_pcm_hw_params(pcm_handle, params);
// set channel #
    snd_pcm_hw_params_get_channels(params, &tmp);
// allocate buffer to hold a period
    snd_pcm_hw_params_get_rate(params, &tmp, 0);

    buff_size = frames * channels * 2

    buff = (char *) malloc(buff_size);

    snd_pcm_hw_params_get_period_time(params, &tmp, NULL);

	float prev_out = 0;
    while (int loops = (input.secs * 1000000)/tmp; loops > 0; loops--) {
		float sample = sin(input.phase);
		sample = low_pass(sample, prev_out, alpha_fil);
		sample = amplify(sample, amp);

		snd_pcm_write(pcm_handle, &sample, 1);

		phase += phase_inc;
		if (phase >= 1.0f) phase -= 1.0f;
    }

    snd_pcm_drain(pcm_handle);
    snd_pcm_close(pcm_handle);
    free(buff);

    return 0;
}
