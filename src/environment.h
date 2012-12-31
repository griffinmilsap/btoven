// btoven
// Motalen 2011
#ifndef _ENVIRONMENT_H
#define _ENVIRONMENT_H

#include <btoven.h>
#include <kiss_fftr.h>
#include "audiobuffer.h"
#include "biquad.h"
#include "comb.h"
#include "window.h"

#ifdef ANDROID
#include <android/log.h>
#endif // ANDROID

#define BTOVEN_VER_MAJOR 0
#define BTOVEN_VER_MINOR 1
#define BTOVEN_VER_PATCH 0

typedef struct {
	bool valid;
	btoven_audiobuffer* ab;
	btoven_audioformat af;
	btoven_state state;
	void** new_pcm;
	BTOVEN_STORETYPE** pcm_chunk;
	size_t pcm_chunk_size;
	BTOVEN_STORETYPE** envelope;
	size_t env_size;
	BTOVEN_STORETYPE* mono_pcm;
	btoven_audiobuffer* mono_buf;
	biquad_filter** filterbank;
	comb_filter** resonators;
	BTOVEN_MATHTYPE** transients;
	double* transient_mean;
	double* transient_std_dev;
	kiss_fftr_cfg fft_cfg;
	size_t spectrum_size;
	BTOVEN_STORETYPE* fft_input;
	kiss_fft_cpx* spectrum;
	double* hps;
	btoven_window* win;
	uint32_t clock;
} btoven_track;

// Our global btoven config.  If this is NULL, btoven hasn't been initialized yet.
extern const btoven_config* _btoven_cfg;

// This is an array of audiobuffers with an indexed by trackhandle
extern btoven_track* _btoven_tracks;
#define TRACK( h ) _btoven_tracks[ h ]

// The number of existing tracks (valid or not)
extern btoven_trackhandle _btoven_num_tracks;
#define NUM_TRACKS _btoven_num_tracks

extern uint32_t _btoven_std_subbands[]; // Hz
extern uint8_t _btoven_std_num_subbands;
extern float _btoven_history_length; // Seconds

#ifdef ANDROID
	#define LOG( format, args... ) __android_log_print( ANDROID_LOG_DEBUG, "btoven", format, ##args );
#else // ANDROID
	#ifdef _MSC_VER
		#define LOG( format, ... ) printf( format, __VA_ARGS__ );
	#else // _MSC_VER
		#define LOG( format, args... ) printf( format, ##args );
	#endif // _MSC_VER
#endif // ANDROID

#endif // _ENVIRONMENT_H
