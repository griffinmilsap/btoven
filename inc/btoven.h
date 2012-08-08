// btoven
// Copyright Motalen 2011
#ifndef _BTOVEN_H
#define _BTOVEN_H

#ifdef __cplusplus
extern "C" {
#else // __cplusplus
typedef unsigned char bool;
#define true 1
#define false 0
#endif

// Standard library requirements
#include <stdint.h>
#include <stddef.h>

// AudioFormat related code
// If this isn't true on your system you'll have to define these manually.
// Undefine them to not support floating point types
#define BTOVEN_FLOAT32 float
#define BTOVEN_FLOAT64 double

// Define encoding format flags
typedef enum 
{
	BTOVEN_ENC_UNKNOWN = 0x00,
	BTOVEN_ENC_FLOAT = 0x80,
	BTOVEN_ENC_SIGNED = 0x60,
	BTOVEN_ENC_8 = 0x01,
	BTOVEN_ENC_16 = 0x02,
	BTOVEN_ENC_24 = 0x04,
	BTOVEN_ENC_32 = 0x08,
	BTOVEN_ENC_64 = 0x10,
	BTOVEN_ENC_SIGNED_8 = ( BTOVEN_ENC_SIGNED | BTOVEN_ENC_8 ),
	BTOVEN_ENC_UNSIGNED_8 = BTOVEN_ENC_8,
	BTOVEN_ENC_SIGNED_16 = ( BTOVEN_ENC_SIGNED | BTOVEN_ENC_16 ),
	BTOVEN_ENC_UNSIGNED_16 = BTOVEN_ENC_16,
	BTOVEN_ENC_SIGNED_24 = ( BTOVEN_ENC_SIGNED | BTOVEN_ENC_24 ),
	BTOVEN_ENC_UNSIGNED_24 = BTOVEN_ENC_24,
	BTOVEN_ENC_SIGNED_32 = ( BTOVEN_ENC_SIGNED | BTOVEN_ENC_32 ),
	BTOVEN_ENC_UNSIGNED_32 = BTOVEN_ENC_32,
	BTOVEN_ENC_FLOAT_32 = ( BTOVEN_ENC_FLOAT | BTOVEN_ENC_32 ),
	BTOVEN_ENC_FLOAT_64 = ( BTOVEN_ENC_FLOAT | BTOVEN_ENC_64 ),
	BTOVEN_ENC_ANY = ( BTOVEN_ENC_SIGNED_8 | BTOVEN_ENC_SIGNED_16 
		| BTOVEN_ENC_SIGNED_24 | BTOVEN_ENC_SIGNED_32
		| BTOVEN_ENC_UNSIGNED_8 | BTOVEN_ENC_UNSIGNED_16 
		| BTOVEN_ENC_UNSIGNED_24 | BTOVEN_ENC_UNSIGNED_32 
		| BTOVEN_ENC_FLOAT_32 | BTOVEN_ENC_FLOAT_64 )
} btoven_enc;

typedef struct {
	btoven_enc enc;
	uint32_t rate;
	uint8_t channels;
	bool interleaved;
} btoven_audioformat;

// btoven Errors
typedef enum {
	BTOVEN_OK = 0,
	BTOVEN_ALREADY_INITIALIZED,
	BTOVEN_NOT_INITIALIZED,
	BTOVEN_ENCODING_NOT_SUPPORTED,
	BTOVEN_INVALID_TRACK_HANDLE,
	BTOVEN_OTHER
} btoven_error;

// Environment related functionality
typedef size_t btoven_trackhandle;

typedef struct {
	uint32_t sys_clock; // Hz
	uint32_t analysis_dec; // Clocks per Analysis 
	uint32_t low_bpm;
	uint32_t high_bpm;
	uint32_t num_bpm;
	double half_energy; // Seconds
} btoven_config;

// Some handy premade configurations
extern const btoven_config btoven_pc_config;
extern const btoven_config btoven_mobile_config;

// State related functionality
typedef struct {
	uint8_t bpm;
	uint8_t bpm_confidence;
	uint8_t percent_to_next;
	bool beat;
	bool transient_low;
	bool transient_mid;
	bool transient_high;
	uint32_t cur_intensity;
	uint32_t sec_intensity;
	uint8_t pitch;
	uint8_t pitch_2;
	uint8_t pitch_3;
} btoven_state;

extern const btoven_state btoven_initial_state;

// Start State Functions
// The following functions should only be called from the "Start" state
btoven_error btoven_initialize( const btoven_config *cfg );
bool btoven_initialized();

// Initialized State Functions
// The following functions should only be called from the "Initialized" state
btoven_error btoven_create_track( btoven_audioformat fmt, btoven_trackhandle* h );
void btoven_reset_track( btoven_trackhandle th );
btoven_error btoven_process( btoven_trackhandle th, uint32_t numFrames, ... );
btoven_state btoven_read_state( btoven_trackhandle th );
btoven_error btoven_delete_track( btoven_trackhandle th );
void btoven_cleanup();
void btoven_get_version_string( char* buf, uint32_t size );

// Functions related to btoven_audioformat 
btoven_error btoven_set_encoding( btoven_audioformat *af, btoven_enc enc );
uint32_t btoven_enc_size( btoven_enc enc );
bool btoven_enc_supported( btoven_enc enc );

// Functions related to btoven_error
const char* btoven_error_string( btoven_error err );

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // _BTOVEN_H


