#define _USE_MATH_DEFINES
#include <alsa/asoundlib.h>
#include <stdio.h>
#include <math.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "includes/types.h"
#include "includes/filter.h"
#include "includes/effects.h"

// Renamed to avoid math.h collision
double osc_sine(double phase) {
	return sin(phase); // phase is already in radians from the loop
}

double osc_square(double phase) {
	return (sin(phase) >= 0) ? 1.0 : -1.0;
}

double osc_saw(double phase) {
	// phase is 0 to 2*PI, convert to -1 to 1
	return (2.0 * (phase / (2.0 * M_PI))) - 1.0;
}

double osc_triangle(double phase) {
	double p = phase / (2.0 * M_PI);
	if (p < 0.25) return 4.0 * p;
	if (p < 0.75) return 2.0 - 4.0 * p;
	return -4.0 + 4.0 * p;
}

double update_envelope(ENVELOPE *env, double sample_rate, bool note_on) {
	if (!note_on && env->state != RELEASE && env->state != IDLE) {
		env->state = RELEASE;
	}

	switch (env->state) {
		case ATTACK:
			env->curr_gain += 1.0 / (env->attack * sample_rate);
			if (env->curr_gain >= 1.0) {
				env->curr_gain = 1.0;
				env->state = DECAY;
			}
			break;
		case DECAY:
			env->curr_gain -= (1.0 - env->sustain) / (env->decay * sample_rate);
			if (env->curr_gain <= env->sustain) {
				env->curr_gain = env->sustain;
				env->state = SUSTAIN;
			}
			break;
		case SUSTAIN:
			env->curr_gain = env->sustain;
			break;
		case RELEASE:
			env->curr_gain -= env->sustain / (env->release * sample_rate);
			if (env->curr_gain <= 0.0) {
				env->curr_gain = 0.0;
				env->state = IDLE;
			}
			break;
		default:
			env->curr_gain = 0.0;
			break;
	}
	return env->curr_gain;
}

void * generate(void * arg) {
	SOUND * sound_data = (SOUND*)arg;

	FILE *data_file = fopen("audio_samples.txt", "w");
	if (!data_file) {
		perror("Failed to open data file");
		return NULL;
	}

	snd_pcm_t *pcm_handle = NULL; // Initialize to NULL
	snd_pcm_hw_params_t *params;

	// 1. Open the device FIRST
	if (snd_pcm_open(&pcm_handle, "default", SND_PCM_STREAM_PLAYBACK, 0) < 0) {
		fprintf(stderr, "Cant open PCM Device\n");
		return NULL;
	}

	snd_pcm_hw_params_alloca(&params);
	snd_pcm_hw_params_any(pcm_handle, params);

	// 3. Now you can set the properties
	snd_pcm_uframes_t period_size = 256;
	snd_pcm_uframes_t buffer_size = period_size * 4;

	snd_pcm_hw_params_set_access(pcm_handle, params, SND_PCM_ACCESS_RW_INTERLEAVED);
	snd_pcm_hw_params_set_format(pcm_handle, params, SND_PCM_FORMAT_FLOAT_LE);

	unsigned int rate = sound_data->sample_rate;
	int chans = (sound_data->channels == STEREO) ? 2 : 1;
	snd_pcm_hw_params_set_channels(pcm_handle, params, chans);
	snd_pcm_hw_params_set_rate_near(pcm_handle, params, &rate, 0);

	// Set buffer/period sizes AFTER basic params are set
	snd_pcm_hw_params_set_period_size_near(pcm_handle, params, &period_size, 0);
	snd_pcm_hw_params_set_buffer_size_near(pcm_handle, params, &buffer_size);

	// 4. Apply the params to the hardware
	snd_pcm_hw_params(pcm_handle, params);
	if (snd_pcm_prepare(pcm_handle) < 0) {
    	fprintf(stderr, "Failed to prepare PCM\n");
    	return NULL;
	}	

	// Assume 5 seconds of audio if no duration specified
	unsigned long frames_remaining = 5 * rate;
	float buffer[1024 * 2]; // Max size for stereo

	int pitch;

	while (frames_remaining > 0) {
    	int frames_to_process = frames_remaining > 256 ? 256 : frames_remaining;
		memset(buffer, 0, frames_to_process * chans * sizeof(float));

		float left_gain = sqrt(0.5 * (1.0 - sound_data->pan)) * sound_data->master_volume;
		float right_gain = sqrt(0.5 * (1.0 + sound_data->pan)) * sound_data->master_volume;

		for (int i = 0; i < frames_to_process; i++) {
			float l_mix = 0.0f;
			float r_mix = 0.0f;

			double raw_sample = 0.0;


			for (int w = 0; w < sound_data->num_waves; w++) {
				WAVE * wave = sound_data->waves[w];
				pitch = wave->pitch * pow(2.0, (double)wave->octave);
				double increment = (2.0 * M_PI * pitch) / rate;
				if (!wave->is_active) continue;

				// Phase update
				wave->current_phase += increment;
				if (wave->current_phase >= 2.0 * M_PI) wave->current_phase -= 2.0 * M_PI;

				switch (wave->type) {
					case WAVE_SINE:   raw_sample = osc_sine(wave->current_phase); break;
					case WAVE_SAW:    raw_sample = osc_saw(wave->current_phase); break;
					case WAVE_SQUARE: raw_sample = osc_square(wave->current_phase); break;
					case WAVE_TRI:    raw_sample = osc_triangle(wave->current_phase); break;
				}

				// Filter Processing
				for (int f = 0; f < wave->num_filters; f++) {
					FILTER * fil = wave->filters[f];

					double fs = (double)rate;
    				double fc = fil->cutt_freq;

    				if (fc < 10.0) fc = 10.0;
    				if (fc > fs / 2.1) fc = fs / 2.1;

					switch (fil->type) {
						case LPF_F: {
							double alpha = (2.0 * M_PI * fc / fs) / (1.0 + (2.0 * M_PI * fc / fs));
							raw_sample = LPF(alpha, raw_sample, &fil->prev_out[0]);
							break;
						}
						case HPF_F: {
							double alpha = 1.0 / (1.0 + (2.0 * M_PI * fc / fs));
							raw_sample = HPF(alpha, raw_sample, &fil->prev_out[0], &fil->prev_inp[0]);
							break;
						}
						case BPF_F: {
							// Biquads require indices 0 and 1 for history
							raw_sample = BPF(raw_sample, &fil->prev_inp[0], &fil->prev_inp[1], &fil->prev_out[0], &fil->prev_out[1], fil->resonance, fc, rate);
							break;
						}
						case HPF_F_2: {
    						raw_sample = HPF_2(raw_sample, &fil->prev_inp[0], &fil->prev_inp[1], &fil->prev_out[0], &fil->prev_out[1], fil->resonance, fc, rate);
    						break;
						}
						case LPF_F_2: {
    						raw_sample = LPF_2(raw_sample, &fil->prev_inp[0], &fil->prev_inp[1], &fil->prev_out[0], &fil->prev_out[1], fil->resonance, fc, rate);
    						break;
						}
						default:
							// Pass-through if type is unknown
							break;
					}
				}

				l_mix += (float)(raw_sample * wave->volume);
				r_mix += (float)(raw_sample * wave->volume);
			}

			double l_sample = (double)l_mix;
			double r_sample = (double)r_mix;
			// Global filter handling
				for (int f = 0; f < sound_data->num_filts; f++) {
					FILTER * fil = sound_data->glob_filters[f];

					double fs = (double)rate;
    				double fc = fil->cutt_freq;

    				if (fc < 10.0) fc = 10.0;
    				if (fc > fs / 2.1) fc = fs / 2.1;

					switch (fil->type) {
						case LPF_F: {
							double alpha = (2.0 * M_PI * fc / fs) / (1.0 + (2.0 * M_PI * fc / fs));
							l_sample = LPF(alpha, l_sample, &fil->prev_out[0]);
							r_sample = LPF(alpha, r_sample, &fil->prev_out[0]);
							break;
						}
						case HPF_F: {
							double alpha = 1.0 / (1.0 + (2.0 * M_PI * fc / fs));
							l_sample = HPF(alpha, l_sample, &fil->prev_out[0], &fil->prev_inp[0]);
							r_sample = HPF(alpha, r_sample, &fil->prev_out[0], &fil->prev_inp[0]);
							break;
						}
						case BPF_F: {
							// require indices 0 and 1 for history
							l_sample = BPF(l_sample, &fil->prev_inp[0], &fil->prev_inp[1], &fil->prev_out[0], &fil->prev_out[1], fil->resonance, fc, rate);
							r_sample = BPF(r_sample, &fil->prev_inp[0], &fil->prev_inp[1], &fil->prev_out[0], &fil->prev_out[1], fil->resonance, fc, rate);
							break;
						}
						case HPF_F_2: {
    						l_sample = HPF_2(l_sample, &fil->prev_inp[0], &fil->prev_inp[1], &fil->prev_out[0], &fil->prev_out[1], fil->resonance, fc, rate);
							r_sample = HPF_2(r_sample, &fil->prev_inp[0], &fil->prev_inp[1], &fil->prev_out[0], &fil->prev_out[1], fil->resonance, fc, rate);
    						break;
						}
						case LPF_F_2: {
    						r_sample = LPF_2(r_sample, &fil->prev_inp[0], &fil->prev_inp[1], &fil->prev_out[0], &fil->prev_out[1], fil->resonance, fc, rate);
							l_sample = LPF_2(l_sample, &fil->prev_inp[0], &fil->prev_inp[1], &fil->prev_out[0], &fil->prev_out[1], fil->resonance, fc, rate);
    						break;
						}
						default:
							// Pass-through if type is unknown
							break;
					}
				}

				for (int x = 0; x < sound_data->num_fx; x++) {
					FX * fx = sound_data->fxs[x];

					switch(fx->type) {
						case CHORUS:
							r_sample = chorus(r_sample, fx->chorus_state, fx->mix, fx->lfo, rate);
							l_sample = chorus(l_sample, fx->chorus_state, fx->mix, fx->lfo, rate);
							break;
						case FLANGER:
							r_sample = flanger(r_sample, fx->flanger_state, fx->mix, fx->feedback, rate);
							l_sample = flanger(l_sample, fx->flanger_state, fx->mix, fx->feedback, rate);
							break;
						case DELAY:
							r_sample = delay(r_sample, fx->delay_state, fx->mix, fx->feedback);
							l_sample = delay(l_sample, fx->delay_state, fx->mix, fx->feedback);
							break;
						case REVERB:
							r_sample = reverb(r_sample, fx->reverb_state, fx->mix);
							l_sample = reverb(l_sample, fx->reverb_state, fx->mix);
							break;
						case PHASER:
							r_sample = phaser(r_sample, fx->lfo, fx->phaser_state, fx->feedback, fx->mix, rate);
							l_sample = phaser(l_sample, fx->lfo, fx->phaser_state, fx->feedback, fx->mix, rate);
							break;
						default:
							break;
					}
				}

				l_mix = (float)l_sample;
				r_mix = (float)r_sample;
			double env_vol = update_envelope(sound_data->env, rate, true);
			float final_gain = (float)(env_vol * sound_data->master_volume);

			float l_pan = sqrt(0.5 * (1.0 - sound_data->pan));
			float r_pan = sqrt(0.5 * (1.0 + sound_data->pan));

			l_mix *= (final_gain * l_pan);
			r_mix *= (final_gain * r_pan);

			if (sound_data->channels == STEREO) {
    			buffer[i*2] = l_mix;
                buffer[i*2 + 1] = r_mix;
                fprintf(data_file, "%.6f %.6f\n", l_mix, r_mix); 
                
			} else {
				buffer[i] = tanhf((l_mix + r_mix) * 0.5f);
				fprintf(data_file, "%.6f %.6f\n", 0.0f, buffer[i]);	
			}
		}

		snd_pcm_sframes_t written = 0;
		while (written < frames_to_process) {
			snd_pcm_sframes_t n = snd_pcm_writei(
				pcm_handle,
				buffer + written * chans,
				frames_to_process - written
			);

			if (n == -EPIPE) {
				snd_pcm_prepare(pcm_handle);
				continue;
			} else if (n < 0) {
				fprintf(stderr, "ALSA error: %s\n", snd_strerror(n));
				break;
			}

			written += n;
		}
		frames_remaining -= written;
	}

	snd_pcm_close(pcm_handle);
	if (data_file) fclose(data_file);
	return NULL;
}
