// btoven
// Motalen 2011
#ifndef _AUDIO_BUFFER_H
#define _AUDIO_BUFFER_H

#include <btoven.h>
#include "audioformat.h"

typedef struct {
	BTOVEN_STORETYPE** pcm;
	uint32_t allocsize;
	uint32_t cursize;
	uint8_t channels;
} btoven_audiobuffer;

// Create a new audiobuffer
btoven_audiobuffer* btoven_create_audiobuffer( btoven_audioformat* fmt );
// Delete an audiobuffer (retains its position in audiobuffer array)
void btoven_delete_audiobuffer( btoven_audiobuffer** buffer );
// Push data into the audiobuffer
void btoven_audiobuffer_push( btoven_audiobuffer* const buf, btoven_audioformat* fmt, void** data, uint32_t num_frames );
// Pop data out of the audiobuffer
bool btoven_audiobuffer_pop( btoven_audiobuffer* const buffer, BTOVEN_STORETYPE** data, uint32_t num_frames );
// Flush the audiobuffer
void btoven_audiobuffer_flush( btoven_audiobuffer* const buffer );

#endif // _AUDIO_BUFFER_H
