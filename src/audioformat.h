// btoven
// Copyright Motalen 2011
#ifndef _AUDIOFORMAT_H
#define _AUDIOFORMAT_H

#include <btoven.h>
#include <math.h>
#include <limits.h>

extern btoven_audioformat _btoven_storetype_format;

// We'll convert and store the PCM in signed 16 bit slots because anything more detailed 
// isn't really statistically significant *INSERT CITATION
#define BTOVEN_STORETYPE int16_t
#define BTOVEN_STORETYPEU uint16_t
#define BTOVEN_STORETYPEMAX INT16_MAX
#define BTOVEN_MATHTYPE int32_t

// Also, we need a function type to get PCM from a void** matrix indexed by channel and frame
typedef BTOVEN_STORETYPE ( *btoven_getPCM )( void**, int, int );

// These macros will create an array of specifically named cast function pointers
#define BTOVEN_BEGIN_ENCODINGS btoven_getPCM __btoven__getPCM_array[] = {
#define BTOVEN_ENCODING( code ) __btoven__get__##code##__,
#define BTOVEN_END_ENCODINGS };
extern btoven_getPCM __btoven__getPCM_array[];

// This macro defines the get function for a specific format and transforms it
// to a signed 16 bit representation of itself.
// FIXME: "ret = ( STORETYPE ) ( num >> shiftamt )" is implementation specific.
// The cast may not lop off the correct bits depending on how this is implemented
#define BTOVEN_ENCODING_PROTOTYPE( code ) \
BTOVEN_STORETYPE __btoven__get__##code##__( void** pcm, int channel, int frame );
#define BTOVEN_ENCODING_DEFINITION( code, encoding ) \
BTOVEN_STORETYPE __btoven__get__##code##__( void** pcm, int channel, int frame ) { \
	BTOVEN_STORETYPE ret = 0; \
	BTOVEN_STORETYPEU utemp = 0; \
	encoding num = ( ( encoding** )pcm )[ channel ][ frame ]; \
	uint32_t shiftamt = 0; \
	uint8_t sign = ( ( ( encoding ) 0 ) - 1 ) < 0; \
	if( sizeof( encoding ) < sizeof( BTOVEN_STORETYPE ) ) { \
		shiftamt = ( sizeof( BTOVEN_STORETYPE ) - sizeof( encoding ) ) * CHAR_BIT; \
		if( sign ) { \
			ret = ( ( BTOVEN_STORETYPE )num ) << shiftamt; \
		} else { \
			utemp = ( ( BTOVEN_STORETYPEU )num ) << shiftamt; \
			ret = ( BTOVEN_STORETYPE ) ( utemp - BTOVEN_STORETYPEMAX ); \
		} \
	} else { \
		shiftamt = ( sizeof( encoding ) - sizeof( BTOVEN_STORETYPE ) ) * CHAR_BIT; \
		if( sign ) { \
			ret = ( BTOVEN_STORETYPE ) ( num >> shiftamt ); \
		} else { \
			utemp = ( BTOVEN_STORETYPEU )( num >> shiftamt ); \
			ret = ( BTOVEN_STORETYPE ) ( utemp - BTOVEN_STORETYPEMAX ); \
		} \
	} \
	return ret; \
}

// We also need a special function to support floating point types because bit twiddling
// on floating point numbers is a big no-no.
#define BTOVEN_ENCODING_DEFINITION_FLOAT( code, encoding ) \
BTOVEN_STORETYPE __btoven__get__##code##__( void** pcm, int channel, int frame ) { \
	return ( BTOVEN_STORETYPE ) ( ( ( encoding** )pcm )[ channel ][ frame ] * BTOVEN_STORETYPEMAX ); \
}

// Front end function for retreival of PCM data
#define BTOVEN_DECODE( encoding ) __btoven__getPCM_array[ btoven_enc2idx( encoding ) ]

// Define return type for these types for your platform.  If these types aren't defined, you 
// may need to define your own
#ifdef INT8_MAX
BTOVEN_ENCODING_PROTOTYPE( BTOVEN_ENC_SIGNED_8 )
BTOVEN_ENCODING_PROTOTYPE( BTOVEN_ENC_UNSIGNED_8 )
#endif // INT8_MAX
#ifdef INT16_MAX
BTOVEN_ENCODING_PROTOTYPE( BTOVEN_ENC_SIGNED_16 )
BTOVEN_ENCODING_PROTOTYPE( BTOVEN_ENC_UNSIGNED_16 )
#endif // INT16_MAX
#ifdef INT24_MAX
BTOVEN_ENCODING_PROTOTYPE( BTOVEN_ENC_SIGNED_24 )
BTOVEN_ENCODING_PROTOTYPE( BTOVEN_ENC_UNSIGNED_24 )
#endif // INT24_MAX
#ifdef INT32_MAX
BTOVEN_ENCODING_PROTOTYPE( BTOVEN_ENC_SIGNED_32 )
BTOVEN_ENCODING_PROTOTYPE( BTOVEN_ENC_UNSIGNED_32 )
#endif // INT32_MAX
#ifdef BTOVEN_FLOAT32
BTOVEN_ENCODING_PROTOTYPE( BTOVEN_ENC_FLOAT_32 )
#endif // BTOVEN_FLOAT32
#ifdef BTOVEN_FLOAT64
BTOVEN_ENCODING_PROTOTYPE( BTOVEN_ENC_FLOAT_64 )
#endif // BTOVEN_FLOAT64

// Internal audioformat related functionality
BTOVEN_STORETYPE btoven_decodePCM( btoven_audioformat *af, void** pcm, uint8_t channel, uint32_t frame );
int32_t btoven_enc2idx( btoven_enc enc );

#endif // _AUDIOFORMAT_H

