// btoven
// Motalen 2011

#include "audiobuffer.h"
#include "environment.h"
#include <string.h>
#include <stdlib.h>

// Defines the default number of frames to allocate space for
#define DEFAULT_SIZE 2048

// Little helper function which allocates a pcm buffer to the requested size
void alloc_pcm( BTOVEN_STORETYPE*** pcm, uint8_t channels, size_t frames )
{
	uint8_t chan;
	( *pcm ) = ( BTOVEN_STORETYPE** )malloc( sizeof( BTOVEN_STORETYPE* ) * channels );
	for( chan = 0; chan < channels; chan++ )
		( *pcm )[ chan ] = ( BTOVEN_STORETYPE* )malloc( sizeof( BTOVEN_STORETYPE ) * frames );
}

// Little helper function which frees an pcm buffer and sets it to NULL
void free_pcm( BTOVEN_STORETYPE*** pcm, uint8_t channels )
{
	uint8_t chan;
	for( chan = 0; chan < channels; chan++ )
		free( ( *pcm )[ chan ] );
	free( *pcm );
	( *pcm ) = NULL;
}

// Little helper function which copys pcm data from one buffer to another
void copy_pcm( BTOVEN_STORETYPE** destpcm, BTOVEN_STORETYPE** srcpcm, uint8_t channels, size_t frames, size_t off )
{
	uint8_t chan;
	for( chan = 0; chan < channels; chan++ )
		memmove( &( destpcm[ chan ][ 0 ] ), &( srcpcm[ chan ][ off ] ), sizeof( BTOVEN_STORETYPE ) * frames );
}

// Resizes the internal buffer if necessary in order to be able to push num_frames without overflow
void resize_buffer_if_necessary( btoven_audiobuffer* const buf, uint32_t num_frames )
{
	if( buf->cursize + num_frames > buf->allocsize )
	{
		BTOVEN_STORETYPE** pcm = NULL;
		alloc_pcm( &pcm, buf->channels, buf->cursize );
		copy_pcm( pcm, buf->pcm, buf->channels, buf->cursize, 0 );
		free_pcm( &( buf->pcm ), buf->channels );
		while( buf->cursize + num_frames > buf->allocsize )
			buf->allocsize *= 2;
		alloc_pcm( &( buf->pcm ), buf->channels, buf->allocsize );
		copy_pcm( buf->pcm, pcm, buf->channels, buf->cursize, 0 );
		free_pcm( &pcm, buf->channels );
	}
}

// Create a new audiobuffer
btoven_audiobuffer* btoven_create_audiobuffer( btoven_audioformat* fmt )
{
	btoven_audiobuffer* buf = ( btoven_audiobuffer* )malloc( sizeof( btoven_audiobuffer ) );
	buf->allocsize = DEFAULT_SIZE;
	buf->cursize = 0;
	buf->channels = fmt->channels;
	alloc_pcm( &( buf->pcm ), buf->channels, buf->allocsize );
	return buf;
}

// Delete an audiobuffer (retains its position in audiobuffer array)
void btoven_delete_audiobuffer( btoven_audiobuffer** buf )
{
	// Don't attempt to access a NULL object!
	if( !( *buf ) )
		return;
	
	// We want to delete any allocated space for the buffer.
	free_pcm( &( ( *buf )->pcm ), ( *buf )->channels );
	free( *buf );
	( *buf ) = NULL;
}

void btoven_audiobuffer_push( btoven_audiobuffer* const buf, btoven_audioformat* fmt, uint32_t num_frames, ... )
{
	va_list data;
	va_start( data, num_frames );
	btoven_audiobuffer_vpush( buf, fmt, num_frames, data );
}

void btoven_audiobuffer_vpush( btoven_audiobuffer* const buf, btoven_audioformat* fmt, uint32_t num_frames, va_list data )
{
	size_t channel, frame;
	
	// Copy the new data into the buffer
	resize_buffer_if_necessary( buf, num_frames );
	const void* sample_data = va_arg( data, const void* );
	for( channel = 0; channel < buf->channels; channel++ )
	{
		if( sample_data == NULL ) break;  // IF WE BREAK HERE, THIS INDICATES A SEVERE ERROR.
		for( frame = 0; frame < num_frames; frame++ )
			buf->pcm[ channel ][ buf->cursize + frame ] = ( fmt->interleaved ) ? 
				btoven_decodePCM( fmt, sample_data, ( fmt->channels * frame ) + channel ) :
				btoven_decodePCM( fmt, sample_data, frame );
		if( !fmt->interleaved ) sample_data = va_arg( data, const void* );
	}
	buf->cursize += num_frames;
}

// Pop data out of the audiobuffer
bool btoven_audiobuffer_pop( btoven_audiobuffer* const buf, BTOVEN_STORETYPE** data, uint32_t num_frames )
{
	if( num_frames > buf->cursize )
		return false;
	copy_pcm( data, buf->pcm, buf->channels, num_frames, 0 );
	copy_pcm( buf->pcm, buf->pcm, buf->channels, buf->allocsize - num_frames, num_frames );
	buf->cursize -= num_frames;
	return true;
}

// Tell the audiobuffer we can overwrite.
void btoven_audiobuffer_flush( btoven_audiobuffer* const buf )
{
	buf->cursize = 0;
}


