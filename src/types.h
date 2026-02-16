#ifndef synth_main_h_INCLUDED
#define synth_main_h_INCLUDED

#include <stdint.h>
#include <stdbool.h>

#define MAX_LEN 64
#define FILTER_STAGES 3
#define NUM_PHASER_STAGES 3
#define MAX_CHORUS_SAMPLES 64
#define MAX_DELAY_SAMPLES 64
#define MAX_REVERB_SAMPLES 64
#define NUM_COMBS 4
#define NUM_ALLPASS 4  // Added missing definition

// notes and their pitch (in Hz)
static const double C       = 261.63;
static const double C_sharp = 277.18;
static const double D       = 293.66;
static const double D_sharp = 311.13;
static const double E       = 329.63;
static const double F       = 349.23;
static const double F_sharp = 369.99;
static const double G       = 392.00;
static const double G_sharp = 415.30;
static const double A       = 440.00;
static const double A_sharp = 466.16;
static const double B       = 493.88;

// WaveType moved up so LFO and WAVE can see it
typedef enum {
	WAVE_SINE,
	WAVE_SAW,
	WAVE_TRI,
	WAVE_SQUARE
} WaveType;

typedef enum {
	HPF_F,
	LPF_F,
	BPF_F,
	NOTCH_F,
	LOW_SHELF_F,
	HIGH_SHELF_F,
	ALL_PASS_F,
	PEAKING_EQ_F
} FILTER_TYPES;

typedef struct {
	FILTER_TYPES type;
	double cutt_freq;
	double resonance;
	double drive;
	int index;
	double alpha;
	double gain;
	double center_freq;
	double prev_out[FILTER_STAGES];
	double prev_inp[FILTER_STAGES];
} FILTER;

typedef enum {
	PAN,
	PITCH,
	F_CUTOFF,
	RESONANCE,
	VOLUME,
	VOLUME_WAVE
} LFO_DEST;

typedef struct {
	WaveType type;
	LFO_DEST dest;
	double curr_phase;
	double rate;
	double fade;
	double pitch;
	double depth;
	double sample_rate;
	int filt_index;
} LFO;

typedef struct {
	double buffer[MAX_CHORUS_SAMPLES];
	int write_head;
	double lfo_phase;
} CHORUS_STATE;

typedef struct {
	double buffer[MAX_DELAY_SAMPLES];
	int write_head;
	double delay_samples;
} DELAY_STATE;

typedef struct {
	double buffer[MAX_DELAY_SAMPLES];
	int write_head;
	double depth_samples;
	double base_delay_samples;
	double lfo_value;
	int buffer_size;
} FLANGER_STATE;

typedef struct {
	double input_1, input_2;
	double output_1, output_2;
	double buffer[MAX_REVERB_SAMPLES]; // Added buffer if used as state
	int write_head;
} ALL_PASS_STATE; // Renamed to match MASTER_REVERB usage

typedef struct {
	double buffer[MAX_REVERB_SAMPLES];
	int write_head;
} REVERB_STATE;

typedef struct {
	REVERB_STATE comb_states[NUM_COMBS];
	double comb_delay_ms[NUM_COMBS];
	double comb_gains[NUM_COMBS];
	ALL_PASS_STATE ap_states[NUM_ALLPASS]; // Matches renamed struct
	double ap_center_freq[NUM_ALLPASS];
	double ap_Q[NUM_ALLPASS];
	int sample_rate;
	double mix;
} MASTER_REVERB;

typedef struct {
	double min_freq;
	double max_freq;
	double lfo_phase;
	double Q;
	double phase;
	ALL_PASS_STATE filter[NUM_PHASER_STAGES];
} PHASER_STATE;

typedef enum {
	ATTACK,
	DECAY,
	SUSTAIN,
	RELEASE,
	IDLE
} ENV_STATE;

typedef struct {
	double attack;
	double decay;
	double release;
	double sustain;
	ENV_STATE state;
	double curr_gain;
} ENVELOPE;

typedef enum {
	CHORUS,
	FLANGER,
	DELAY,
	REVERB,
	PHASER
} FX_TYPES;

typedef struct {
	FX_TYPES type;
	double time;
	double feedback;
	double mix;
	int index;
	LFO * lfo;
	CHORUS_STATE * chorus_state;
	DELAY_STATE * delay_state;
	FLANGER_STATE * flanger_state;
	REVERB_STATE * reverb_state;
	PHASER_STATE * phaser_state;
} FX;

typedef struct {
	char name[MAX_LEN];
	WaveType type;
	double volume;
	double velocity;
	double octave;
	double pitch;
	double pulse_width;
	double target_freq;
	double curr_freq;
	double unison;
	int curr_lfo;
	int index;
	int num_filters;
	int num_fx;
	bool is_active;
	int curr_filter;
	int curr_fx;
	int num_lfos;
	FILTER ** filters;
	FX ** fxs;
	LFO ** lfos;
	double current_phase;
} WAVE;

typedef enum {
	UP,
	DOWN,
	UP_DOWN,
	RANDOM,
	AS_PLAYED
} ARP_modes;

typedef enum {
	ARP_TAB,
	LFO_TAB,
	WAVE_TAB,
	FX_TAB,
	FILTER_TAB,
	SOUNDS_TAB
} TAB;

typedef enum {
	STEREO,
	MONO
} ChanType;

typedef struct {
	int num_waves;
	int sample_rate;
	int curr_wave;
	int volume;
	double master_volume;
	double fx_send;
	double stereo_spread;
	double current_tempo;
	double glide_time;
	double pan;
	ChanType channels;
	ENVELOPE * env;
	WAVE ** waves;
} SOUND;

typedef struct {
	int notes[16];
	int num_notes;
	int curr_step;
	uint8_t last_tick;
	double octave;
	double swing;
	ARP_modes mode;
	double tempo;
	SOUND * sound;
} ARP;

#endif // synth_main_h_INCLUDED
