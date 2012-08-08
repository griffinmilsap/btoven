// btoven
// Motalen 2011

#include <btoven.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include "environment.h"
#include "processing.h"

#define DEFAULT_NUM_TRACKS 8

// Standard filtering parameters
// TODO: Prefixing underscore is reserved for compiler.  Post-fix underscores.
uint32_t _btoven_std_subbands[] = { 200, 400, 800, 1600, 3200 }; // Hz
uint8_t _btoven_std_num_subbands = ( sizeof( _btoven_std_subbands ) / sizeof( uint32_t ) ) + 1;
float _btoven_history_length = 5.0f; // Seconds

const btoven_track _btoven_invalid_track = { false };
const btoven_config* _btoven_cfg = NULL;
btoven_track* _btoven_tracks = NULL;
size_t _btoven_num_tracks = 0;

const btoven_config btoven_pc_config = { 
	400, // Clock the system at 400 hz
	20,  // Run detailed analysis every 20 clocks (20 Hz)
	60,  // Lowest BPM we want to find is 60 BPM
	240, // Highest BPM we want to find is 240BPM
	90,  // Test 90 logarithmically distributed BPMs
	1.5f // Weight history past 1.5 seconds less
};

const btoven_config btoven_mobile_config = {
	100, // Clock the system at 100 hz
	5,  // Run detailed analysis every 5 clocks (20 Hz)
	60,  // Lowest BPM we want to find is 60 BPM
	240, // Highest BPM we want to find is 240BPM
	10,  // Test 30 logarithmically distributed BPMs (This is really tough on mobile).
	1.5f // Weight history past 1.5 seconds less
};

const btoven_state btoven_initial_state = {
	0,                   // Initially, there is no BPM
	0,                   // No BPM confidence
	0,                   // No progress toward next beat
	false,               // No beats have occurred yet
	false,               // No transients on the horizon
	false,
	false,
	0,                   // No current intensity
	0,                   // No sectional intensity yet either.
	0,                   // No current pitch
	0,                   // No secondary pitch
	0                    // No tertiary pitch
};

// Retreive library version string
void btoven_get_version_string( char* buf, uint32_t size ) {
	char tmp[32];
	size_t n = sprintf( tmp, "%d.%d.%d", BTOVEN_VER_MAJOR, BTOVEN_VER_MINOR, BTOVEN_VER_PATCH );
	if( buf && n < size )
		strcpy( buf, tmp );
}

bool btoven_initialized()
{
	return _btoven_cfg ? true : false;
}

// Puts the system into "Initialized State" and allocates required structures.
btoven_error btoven_initialize( const btoven_config *cfg )
{
	// If we're already initialized, we don't need to re-initialize.
	btoven_trackhandle i;
	if( btoven_initialized() )
		return BTOVEN_ALREADY_INITIALIZED;
	
	// Initialize the configuration -- Now in "Initialized State"	
	if( !cfg ) cfg = &btoven_pc_config;
	_btoven_cfg = cfg;
	
	// Initialize the track array
	_btoven_tracks = ( btoven_track* )malloc( sizeof( btoven_track ) * DEFAULT_NUM_TRACKS );
	_btoven_num_tracks = DEFAULT_NUM_TRACKS;
	for( i = 0; i < _btoven_num_tracks; i++ )
		_btoven_tracks[i].valid = false;
	
	return BTOVEN_OK;
}

// Puts the system back into "Start State" and frees memory
void btoven_cleanup()
{
	// Delete all tracks
	btoven_trackhandle i;
	for( i = 0; i < _btoven_num_tracks; i++ )
		btoven_delete_track( i );
			
	kiss_fft_cleanup();
	free( _btoven_tracks );
	_btoven_tracks = NULL;
	_btoven_cfg = NULL;
}

// Doubles the size of the tracks array, retaining existing data
void resize_tracks()
{
	btoven_trackhandle i;
	btoven_track* tmp = ( btoven_track* )malloc( sizeof( btoven_track ) * _btoven_num_tracks );
	memcpy( tmp, _btoven_tracks, sizeof( btoven_track ) * _btoven_num_tracks );
	free( _btoven_tracks );
	_btoven_tracks = ( btoven_track* )malloc( sizeof( btoven_track ) * _btoven_num_tracks * 2 );
	memcpy( _btoven_tracks, tmp, sizeof( btoven_track ) * _btoven_num_tracks );
	free( tmp );
	for( i = _btoven_num_tracks; i < _btoven_num_tracks * 2; i++ )
		_btoven_tracks[i].valid = false;
	_btoven_num_tracks *= 2;
}

// Attempts to find an empty track which can be used for analysis
bool find_empty_track( btoven_trackhandle* h )
{
	btoven_trackhandle i;
	for( i = 0; i < _btoven_num_tracks; i++ )
		if( !( _btoven_tracks[i].valid ) )
			return _btoven_tracks[ *h = i ].valid;
	return false;
}

btoven_error btoven_create_track( btoven_audioformat fmt, btoven_trackhandle* h )
{
	size_t i;
	btoven_track* t;

	if( !btoven_initialized() )
		return BTOVEN_NOT_INITIALIZED;
	
	// Find a place to create the track and make it if it doesn't exist	
	if( find_empty_track( h ) )
	{
		resize_tracks();
		find_empty_track( h );
	}
	t = &_btoven_tracks[*h];
	
	// Set values appropriately
	t->af = fmt;
	t->ab = btoven_create_audiobuffer( &( t->af ) );
	t->valid = true;
	
	t->new_pcm = ( void** )malloc( t->af.channels * sizeof( void* ) );
	t->pcm_chunk_size = t->af.rate / _btoven_cfg->sys_clock;
	t->pcm_chunk = ( BTOVEN_STORETYPE** )malloc( sizeof( BTOVEN_STORETYPE* ) * t->af.channels );
	for( i = 0; i < t->af.channels; i++ ) 
		t->pcm_chunk[i] = ( BTOVEN_STORETYPE* )malloc( sizeof( BTOVEN_STORETYPE ) * t->pcm_chunk_size );

	// Prepare the fft
	t->spectrum_size = ( t->pcm_chunk_size * ( _btoven_cfg->sys_clock / _btoven_cfg->analysis_dec ) );
	t->fft_cfg = kiss_fftr_alloc( t->spectrum_size, 0, NULL, NULL );
	t->win = window_new( BTOVEN_TRIANGLE_WINDOW, t->spectrum_size );
	t->mono_pcm = ( BTOVEN_STORETYPE* )malloc( sizeof( BTOVEN_STORETYPE ) * t->pcm_chunk_size );
	t->mono_buf = btoven_create_audiobuffer( &_btoven_storetype_format );
	t->fft_input = ( BTOVEN_STORETYPE* )malloc( sizeof( BTOVEN_STORETYPE ) * t->spectrum_size );
	t->spectrum = ( kiss_fft_cpx* )malloc( sizeof( kiss_fft_cpx ) * ( ( t->spectrum_size / 2 ) + 1 ) );
	t->hps = ( double* )malloc( sizeof( double ) * ( t->spectrum_size / 2 ) );
	for( i = 0; i <  ( t->spectrum_size / 2 ); i++ )
		t->hps[i] = 1.0;

	// Allocate envelope and transient data
	t->env_size = ( size_t )( _btoven_history_length * _btoven_cfg->sys_clock );
	t->envelope = ( BTOVEN_STORETYPE** )malloc( sizeof( BTOVEN_STORETYPE* ) * _btoven_std_num_subbands );
	t->transients = ( BTOVEN_MATHTYPE** )malloc( sizeof( BTOVEN_MATHTYPE* ) * _btoven_std_num_subbands );
	t->transient_mean = ( double* )malloc( sizeof( double ) * _btoven_std_num_subbands );
	t->transient_std_dev = ( double* )malloc( sizeof( double ) * _btoven_std_num_subbands );
	for( i = 0; i < _btoven_std_num_subbands; i++ )
	{
		t->envelope[i] = ( BTOVEN_STORETYPE* )calloc( t->env_size, sizeof( BTOVEN_STORETYPE ) );
		t->transients[i] = ( BTOVEN_MATHTYPE* )calloc( t->env_size, sizeof( BTOVEN_MATHTYPE ) );
	}
	
	// Set up the filterbank
	t->filterbank = ( biquad_filter** )malloc( sizeof( biquad_filter* ) * _btoven_std_num_subbands );
	t->filterbank[0] = biquad_new( LPF, 0, _btoven_std_subbands[0], t->af.rate, 1.0f );
	for( i = 0; i < ( _btoven_std_num_subbands - 2 ); i++ )
	{
		double cf = ( _btoven_std_subbands[i] + _btoven_std_subbands[ i + 1 ] ) / 2.0f;
		double bw = ( _btoven_std_subbands[ i + 1 ] - _btoven_std_subbands[i] ) / cf;
		t->filterbank[ i + 1 ] = biquad_new( BPF, 0, cf, t->af.rate, bw );
	}
	t->filterbank[ _btoven_std_num_subbands - 1 ] 
		= biquad_new( HPF, 0, _btoven_std_subbands[ _btoven_std_num_subbands - 2 ], t->af.rate, 1.0f );
		
	// Allocate resonators
	t->resonators = ( comb_filter** )malloc( sizeof( comb_filter* ) * _btoven_cfg->num_bpm );
	for( i = 0; i < _btoven_cfg->num_bpm; i++ )
	{
		double frac = ( exp( i / ( double )_btoven_cfg->num_bpm) - 1.0f ) / ( exp( 1.0f ) - 1.0f );
		uint8_t bpm = ( uint8_t )( _btoven_cfg->low_bpm + ( frac * ( _btoven_cfg->high_bpm - _btoven_cfg->low_bpm ) ) );
		t->resonators[i] = comb_new( bpm, 1.5f, _btoven_cfg->sys_clock, t->env_size );
	}
	
	t->state = btoven_initial_state;
	t->clock = 0;
	
	return BTOVEN_OK;
}

btoven_error btoven_delete_track( btoven_trackhandle h )
{
	size_t i;
	btoven_track* t = &_btoven_tracks[h];
	if( !btoven_initialized() )
		return BTOVEN_NOT_INITIALIZED;
	if( !t->valid )
		return BTOVEN_INVALID_TRACK_HANDLE;
		
	// Delete all allocated memory for this track.
	btoven_delete_audiobuffer( &( t->ab ) );	
	free( t->new_pcm );
	
	for( i = 0; i < t->af.channels; i++ )
		free( t->pcm_chunk[i] );
	free( t->pcm_chunk );
	
	window_delete( t->win );
	free( t->fft_cfg );
	btoven_delete_audiobuffer( &( t->mono_buf ) );
	free( t->fft_input );
	free( t->spectrum );
	free( t->hps );
	
	for( i = 0; i < _btoven_cfg->num_bpm; i++ )
		comb_delete( t->resonators[i] );
	free( t->resonators );
	
	for( i = 0; i < _btoven_std_num_subbands; i++ )
	{
		free( t->envelope[i] );
		free( t->filterbank[i] );
		free( t->transients[i] );
	}
	free( t->envelope );
	free( t->filterbank );
	free( t->transients );
	free( t->transient_mean );
	free( t->transient_std_dev );
	free( t->mono_pcm );
	
	t->valid = false;
		
	return BTOVEN_OK;		
}

btoven_state btoven_read_state( btoven_trackhandle h )
{
	btoven_state s = _btoven_tracks[h].state;
	_btoven_tracks[h].state.beat = false;
	_btoven_tracks[h].state.transient_low = false;
	_btoven_tracks[h].state.transient_mid = false;
	_btoven_tracks[h].state.transient_high = false;
	return s;
}

// Process the incoming PCM
btoven_error btoven_process( btoven_trackhandle h, uint32_t num_frames, const void* const  pcm, ... )
{
	uint8_t i;
	btoven_track* t = &_btoven_tracks[h];
	if( !btoven_initialized() )
		return BTOVEN_NOT_INITIALIZED;
	if( !t->valid )
		return BTOVEN_INVALID_TRACK_HANDLE;

	//memcpy( t->new_pcm[0], pcm, num_frames * t->af.channels * btoven_enc_size( t->af.enc ) );
	if( !t->af.interleaved ) 
	{
		va_list ap;
		va_start( ap, pcm );
		for( i = 1; i < t->af.channels; i++ )
			t->new_pcm[i] = va_arg( ap, void* );
	}
	btoven_audiobuffer_push( t->ab, &( t->af ), t->new_pcm, num_frames );
	
	// While we still have data left to process, loop through and perform the analysis
	while( btoven_audiobuffer_pop( t->ab, t->pcm_chunk, t->pcm_chunk_size ) )
	{	
		// Apply step one of the bpm algorithm here
		btoven_proc_envelope( t );
		t->clock++;
	}	
	
	// Perform a more detailed analysis every so often
	if( t->clock >= _btoven_cfg->analysis_dec )
	{
		t->clock = 0;
		btoven_proc_bpm( t );
		btoven_proc_pitch( t );
	}
	
	return BTOVEN_OK;
}

