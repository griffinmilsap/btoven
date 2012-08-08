// btoven
// Motalen 2011

// This file describes the processing algorithm without vector optimization.
#include <math.h>
#include <string.h>
#include <stdio.h>
#include <kiss_fft.h>
#include "environment.h"
#include "processing.h"
#include "biquad.h"

#ifndef BTOVEN_OPTIMIZATION // This is the development version of the algorithm

// Uncomment these to debug different steps of the algorithm
//#define DEBUG_PCM
//#define DEBUG_ENVELOPE
//#define DEBUG_TRANSIENTS
//#define DEBUG_BPM
//#define DEBUG_WINDOWING
//#define DEBUG_SPECTRUM

float _btoven_envelope = 0.2f; // seconds
double _btoven_hps_alpha = 0.0f; // Weight HPS History (0 - 1)
double _btoven_transient_threshold = 5.0;
void transient( btoven_track* t, int band );
void printbars_storetype( BTOVEN_STORETYPE* data, size_t minidx, size_t maxidx, size_t step, BTOVEN_STORETYPE max );
void printbars_mathtype( BTOVEN_MATHTYPE* data, size_t minidx, size_t maxidx, size_t step, BTOVEN_MATHTYPE max );
void printbars_double( double* data, size_t minidx, size_t maxidx, size_t step, double max );
void printbars_float( float* data, size_t minidx, size_t maxidx, size_t step, float max );

// This processing step produces a half-wave rectified envelope for the defined
// frequency subbands, sampled at the defined system clock speed
void btoven_proc_envelope( btoven_track* t )
{
	size_t chan, frame, band, i, j;
	void** input;
	uint32_t env;

#ifdef DEBUG_PCM	
	LOG( "---PCM---\n" );
	for( chan = 0; chan < t->af.channels; chan++ )
	{
		LOG( "Channel %d: ", chan + 1 );
		for( frame = 0; frame < t->pcm_chunk_size; frame += 10 )
			LOG( "%d, ", t->pcm_chunk[chan][frame] );
		LOG( "\n" );
	}
#endif // DEBUG_PCM

	// Average Channels into mono
	for( frame = 0; frame < t->pcm_chunk_size; frame++ ) 
	{
		BTOVEN_MATHTYPE val = 0;
		for( chan = 0; chan < t->af.channels; chan++ ) 
			val += t->pcm_chunk[ chan ][ frame ];
		t->mono_pcm[ frame ] = val / t->af.channels;
	}
	
	input = ( void** ) &( t->mono_pcm );
	btoven_audiobuffer_push( t->mono_buf, &_btoven_storetype_format, input, t->pcm_chunk_size );

	// Filter, Extract Envelope, and Sample into History
	for( band = 0; band < _btoven_std_num_subbands; band++ )
	{
		// We'll push back the history
		memmove( &t->envelope[band][1], &t->envelope[band][0], sizeof( BTOVEN_STORETYPE ) * ( t->env_size - 1 ) );
		t->envelope[band][0] = t->envelope[band][1];
		
		// Deal with our current chunk
		env = 0;
		for( frame = t->pcm_chunk_size - 1; frame > 0; frame-- )
		{
			// Apply the filter
			BTOVEN_MATHTYPE mval = abs( biquad( t->mono_pcm[ frame ], t->filterbank[ band ] ) );
			
			if( frame <= ( size_t )( _btoven_envelope * _btoven_cfg->sys_clock ) )
				env += abs( mval );
		}
		
		// Calculate this envelope value
		env /= ( size_t )( _btoven_envelope * _btoven_cfg->sys_clock );
		env = ( env > BTOVEN_STORETYPEMAX ) ? BTOVEN_STORETYPEMAX : ( BTOVEN_STORETYPE )env;
		t->envelope[ band ][0] = ( BTOVEN_STORETYPE )env;
	}

#ifdef DEBUG_ENVELOPE
	LOG( "---Envelope---\n" );
	for( i = 0; i < 20; i++ )
	{
		char bar[32] = "";
		for( j = 0; j < t->envelope[5][i]; j += ( 32768 / 20 ) )
			strcat( bar, "=" );
		LOG( "%s\n", bar );
	}
#endif // DEBUG_ENVELOPE 
}

void btoven_proc_bpm( btoven_track* t )
{
	// Reset the Resonators
	uint32_t bpm, idx;
	size_t band, sh, i, j, cur_intensity_size;
	uint32_t max_energy = 0, bpm_energy_std_dev = 0, bpm_energy_mean = 0;
	uint32_t sel_resonator = 0;
	uint32_t max_data = 0;
	uint32_t time_to_next = 0; //indices
	uint8_t percent_to_next;

	for( bpm = 0; bpm < _btoven_cfg->num_bpm; bpm++ )
		comb_reset( t->resonators[bpm] );
		
	// For every band...
	t->state.sec_intensity = 0;
	t->state.cur_intensity = 0;
	cur_intensity_size = _btoven_cfg->sys_clock / _btoven_cfg->analysis_dec;
	for( band = 0; band < _btoven_std_num_subbands; band++ )
	{
		// Apply a half-wave rectified derivation filter 
		t->transient_mean[band] = 0.0;
		for( sh = 0; sh < t->env_size - 1; sh++ )
		{
			BTOVEN_MATHTYPE mval;
			t->state.sec_intensity += t->envelope[band][sh];
			if( sh < cur_intensity_size ) t->state.cur_intensity += t->envelope[band][sh];
			mval = ( BTOVEN_MATHTYPE )t->envelope[band][sh] - ( BTOVEN_MATHTYPE )t->envelope[band][sh+1];
			mval = ( mval > 0 ) ? mval : 0;
			t->transients[band][sh] = mval;
			t->transient_mean[band] += t->transients[band][sh];
		}
		t->transient_mean[band] /= ( t->env_size - 1 );
				
		// Preform a comb filter on the rectified data, testing for each BPM
		for( bpm = 0; bpm < _btoven_cfg->num_bpm; bpm++ )
			comb( t->transients[band], t->resonators[bpm] );
	}
	t->state.sec_intensity /= ( t->env_size * _btoven_std_num_subbands );
	t->state.cur_intensity /= ( cur_intensity_size * _btoven_std_num_subbands );
	
#ifdef DEBUG_TRANSIENTS
	LOG( "---Transients---\n" );
	for( i = 0; i < _btoven_std_num_subbands - 1; i++ )
		printbars_mathtype( t->transients[i], 0, 100, 1, 10000 );
#endif // DEBUG_TRANSIENTS

	// Determine Transient Standard Deviations
	for( band = 0; band < _btoven_std_num_subbands; band++ )
	{
		t->transient_std_dev[band] = 0.0;
		for( i = 0; i < t->env_size - 1; i++ )
		{
			double dval = t->transients[band][i] - t->transient_mean[band];
			t->transient_std_dev[band] += ( dval * dval ); 
		}
		t->transient_std_dev[band] = sqrt( t->transient_std_dev[band] / ( t->env_size - 1 ) );
	}
	
	// Determine if we've had a transient recently
	for( band = 0; band < _btoven_std_num_subbands; band++ )
	{
		double threshold = _btoven_transient_threshold * t->transient_std_dev[band];
		for( i = 0; i < ( _btoven_cfg->sys_clock / _btoven_cfg->analysis_dec ); i++ )
			if( abs( t->transients[band][i] - t->transient_mean[band] ) > threshold )
				transient( t, band );
	}
	
#ifdef DEBUG_BPM
	LOG( "---BPM---\n" );
	for( i = 0; i < _btoven_cfg->num_bpm; i+=5 )
	{
		char bar[32] = "";
		LOG( "%d: ", t->resonators[i]->bpm );
		for( j = 0; j < t->resonators[i]->max_energy; j += 400 )
			strcat( bar, "=" );
		LOG( "%s\n", bar );
	}
#endif // DEBUG_BPM
	
	// Determine the bpm with the most energy as well as confidence
	// TODO: Determine the bpm confidence
	for( bpm = 0; bpm < _btoven_cfg->num_bpm; bpm++ )
	{
		if( t->resonators[bpm]->max_energy > max_energy )
		{
			max_energy = t->resonators[bpm]->max_energy;
			t->state.bpm = t->resonators[bpm]->bpm;
			sel_resonator = bpm;
		}
	}

	// Determine the percentage of time until the next beat
	for( idx = 0; idx < 2 * t->resonators[sel_resonator]->T; idx++ )
	{
		if( t->resonators[sel_resonator]->acc_data[idx] > max_data )
		{
			time_to_next = idx;
			max_data = t->resonators[sel_resonator]->acc_data[idx];
		}
	}
	
	time_to_next = ( t->resonators[sel_resonator]->T - time_to_next );
	percent_to_next = ( uint8_t )( 255.0f * ( ( float )time_to_next / ( float )t->resonators[sel_resonator]->T ) );
	if( percent_to_next > t->state.percent_to_next )
		t->state.beat = true;
	t->state.percent_to_next = percent_to_next;
		
}

void transient( btoven_track* t, int band )
{
	if( band == 0 )
		t->state.transient_low = true;
	else if( _btoven_std_num_subbands - 1 )
		t->state.transient_high = true;
	else
		t->state.transient_mid = true;
}

void btoven_proc_pitch( btoven_track* t )
{
	// Minimum index to search for a pitch is around 60 hz to 2000 hz
	int minIndex = ( 60 * t->spectrum_size ) / t->af.rate;
	int maxIndex = ( 2000 * t->spectrum_size ) / t->af.rate;
	int harmonics = 3;
	int i, j, maxHIndex, idx;
	int maxLocation[3] = { minIndex, minIndex, minIndex };

	// Window and transform the current pcm to frequency domain
	btoven_audiobuffer_pop( t->mono_buf, &t->fft_input, t->spectrum_size );
	btoven_audiobuffer_flush( t->mono_buf );
	kiss_fftr( t->fft_cfg, window( t->fft_input, t->win ), t->spectrum ); 
	for( i = 0; i < ( t->spectrum_size / 2 ); i++ )
		t->hps[i] = ( t->hps[i] * _btoven_hps_alpha ) 
			+ ( t->spectrum[i].r * t->spectrum[i].r ) 
			+ ( t->spectrum[i].i * t->spectrum[i].i );
		
#ifdef DEBUG_WINDOWING
	LOG( "---fft_input---\n" );
	printbars_storetype( t->fft_input, 0, t->spectrum_size, 40, 32767 );
	
	LOG( "---window---\n" );
	printbars_float( t->win->window, 0, t->spectrum_size, 40, 1.0f );
	
	LOG( "---windowed_data---\n" );
	printbars_mathtype( t->win->windowed_data, 0, t->spectrum_size, 40, 32767 );
#endif // DEBUG_WINDOWING
	
#ifdef DEBUG_SPECTRUM
	LOG( "---Spectrum---\n" );
	printbars_double( t->hps, 0, t->spectrum_size / 2, 20, 10000 );
#endif // DEBUG_SPECTRUM
	
	maxHIndex = ( t->spectrum_size / 2 ) / harmonics;
	if( maxIndex < maxHIndex ) 
		maxHIndex = maxIndex;

	// generate the Harmonic Product Spectrum values and keep track of the
	// maximum amplitude value to assign to a pitch.
	for( j = minIndex; j <= maxHIndex; j++ ) {
		for( i = 1; i <= harmonics; i++ ) 
			t->hps[j] *= t->hps[j*i];
		if( t->hps[j] > t->hps[ maxLocation[0] ] )
		{
			maxLocation[2] = maxLocation[1];
			maxLocation[1] = maxLocation[0];
			maxLocation[0] = j;
		}
	}

	// Correct for octave too high errors.  If the maximum subharmonic
	// of the measured maximum is approximately 1/2 of the maximum
	// measured frequency, AND if the ratio of the sub-harmonic
	// to the total maximum is greater than 0.2, THEN the pitch value
	// is assigned to the subharmonic.
	for( idx = 0; idx < 3; idx++ )
	{
		int max2 = minIndex;
		int maxsearch = ( maxLocation[idx] * 3 ) / 4;
		for( i = minIndex + 1; i < maxsearch; i++ )
			if( t->hps[i] > t->hps[max2] )
				max2 = i;
		if( abs( max2 * 2 - maxLocation[idx] ) < 4 ) 
			if( t->hps[max2] / t->hps[maxLocation[idx]] > 0.2f ) 
				maxLocation[idx] = max2;
	}
			
	t->state.pitch = ( ( maxLocation[0] - minIndex ) * 255 ) / ( maxIndex - minIndex );
	t->state.pitch_2 = ( ( maxLocation[1] - minIndex ) * 255 ) / ( maxIndex - minIndex );
	t->state.pitch_3 = ( ( maxLocation[2] - minIndex ) * 255 ) / ( maxIndex - minIndex );
}


#define BAR_HEIGHT 10
void printbars_storetype( BTOVEN_STORETYPE* data, size_t minidx, size_t maxidx, size_t step, BTOVEN_STORETYPE max )
{	
	size_t r, i;
	for( r = BAR_HEIGHT; r > 0; r-- )
	{
		char row[512] = "";
		for( i = minidx; i < maxidx; i += step )
		{
			if( abs( data[i] ) >= ( max / BAR_HEIGHT ) * r )
				strcat( row, "+" );
			else
				strcat( row, " " );
		}
		LOG( "%s\n", row );
	}
}

void printbars_mathtype( BTOVEN_MATHTYPE* data, size_t minidx, size_t maxidx, size_t step, BTOVEN_MATHTYPE max )
{	
	size_t r, i;
	for( r = BAR_HEIGHT; r > 0; r-- )
	{
		char row[512] = "";
		for( i = minidx; i < maxidx; i += step )
		{
			if( abs( data[i] ) >= ( max / BAR_HEIGHT ) * r )
				strcat( row, "+" );
			else
				strcat( row, " " );
		}
		LOG( "%s\n", row );
	}
}

void printbars_float( float* data, size_t minidx, size_t maxidx, size_t step, float max )
{	
	size_t r, i;
	for( r = BAR_HEIGHT; r > 0; r-- )
	{
		char row[512] = "";
		for( i = minidx; i < maxidx; i += step )
		{
			if( data[i] >= ( max / ( float )BAR_HEIGHT ) * ( float )r )
				strcat( row, "+" );
			else
				strcat( row, " " );
		}
		LOG( "%s\n", row );
	}
}

void printbars_double( double* data, size_t minidx, size_t maxidx, size_t step, double max )
{	
	size_t r, i;
	for( r = BAR_HEIGHT; r > 0; r-- )
	{
		char row[1200] = "";
		for( i = minidx; i < maxidx; i += step )
		{
			if( data[i] >= ( max / ( float )BAR_HEIGHT ) * ( float )r )
				strcat( row, "+" );
			else
				strcat( row, " " );
		}
		LOG( "%s\n", row );
	}
}
	 

#endif // BTOVEN_OPTIMIZATION
