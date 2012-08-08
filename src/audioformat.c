// btoven
// Copyright Motalen 2011
#include "audioformat.h"

btoven_audioformat _btoven_storetype_format = { BTOVEN_ENC_SIGNED_16, 0, 1, false };

// Define return type for these types for your platform.  If these types aren't defined, you 
// may need to define your own
#ifdef INT8_MAX
BTOVEN_ENCODING_DEFINITION( BTOVEN_ENC_SIGNED_8, int8_t )
BTOVEN_ENCODING_DEFINITION( BTOVEN_ENC_UNSIGNED_8, uint8_t )
#endif // INT8_MAX
#ifdef INT16_MAX
BTOVEN_ENCODING_DEFINITION( BTOVEN_ENC_SIGNED_16, int16_t )
BTOVEN_ENCODING_DEFINITION( BTOVEN_ENC_UNSIGNED_16, uint16_t )
#endif // INT16_MAX
#ifdef INT24_MAX
BTOVEN_ENCODING_DEFINITION( BTOVEN_ENC_SIGNED_24, int24_t )
BTOVEN_ENCODING_DEFINITION( BTOVEN_ENC_UNSIGNED_24, uint24_t )
#endif // INT24_MAX
#ifdef INT32_MAX
BTOVEN_ENCODING_DEFINITION( BTOVEN_ENC_SIGNED_32, int32_t )
BTOVEN_ENCODING_DEFINITION( BTOVEN_ENC_UNSIGNED_32, uint32_t )
#endif // INT32_MAX
#ifdef BTOVEN_FLOAT32
BTOVEN_ENCODING_DEFINITION_FLOAT( BTOVEN_ENC_FLOAT_32, BTOVEN_FLOAT32 )
#endif // BTOVEN_FLOAT32
#ifdef BTOVEN_FLOAT64
BTOVEN_ENCODING_DEFINITION_FLOAT( BTOVEN_ENC_FLOAT_64, BTOVEN_FLOAT64 )
#endif // BTOVEN_FLOAT64

// List the supported formats and populate the get function pointer array
BTOVEN_BEGIN_ENCODINGS
#ifdef INT8_MAX
BTOVEN_ENCODING( BTOVEN_ENC_SIGNED_8 )
BTOVEN_ENCODING( BTOVEN_ENC_UNSIGNED_8 )
#endif // INT8_MAX
#ifdef INT16_MAX
BTOVEN_ENCODING( BTOVEN_ENC_SIGNED_16 )
BTOVEN_ENCODING( BTOVEN_ENC_UNSIGNED_16 )
#endif // INT16_MAX
#ifdef INT24_MAX
BTOVEN_ENCODING( BTOVEN_ENC_SIGNED_24 )
BTOVEN_ENCODING( BTOVEN_ENC_UNSIGNED_24 )
#endif // INT24_MAX
#ifdef INT32_MAX
BTOVEN_ENCODING( BTOVEN_ENC_SIGNED_32 )
BTOVEN_ENCODING( BTOVEN_ENC_UNSIGNED_32 )
#endif // INT32_MAX
#ifdef BTOVEN_FLOAT32
BTOVEN_ENCODING( BTOVEN_ENC_FLOAT_32 )
#endif // BTOVEN_FLOAT32
#ifdef BTOVEN_FLOAT64
BTOVEN_ENCODING( BTOVEN_ENC_FLOAT_64 )
#endif // BTOVEN_FLOAT64
BTOVEN_END_ENCODINGS

/*---------------------------------------------------------------------------------------
btoven_set_encoding
Input: btoven_audioformat &af - audioformat to augment
       btoven_enc enc - encoding to try
Output: BTOVEN_ERROR - Error Code
Description: Sets the format's encoding, and checks if its supported
 Returns BTOVEN_ENCODING_NOT_SUPPORTED if the encoding is not supported
---------------------------------------------------------------------------------------*/
btoven_error btoven_set_encoding( btoven_audioformat *af, btoven_enc enc )
{
	if( btoven_enc_supported( enc ) )
                af->enc = enc; 
        else
                return BTOVEN_ENCODING_NOT_SUPPORTED; 
        return BTOVEN_OK; 
}

/*---------------------------------------------------------------------------------------
 btoven_enc_size
 Input: N/A
 Output: uint32_t -- Size (in bytes) of the format's encoding
 Description: Sometimes it is handy to know the size in bytes of the encoding
 ---------------------------------------------------------------------------------------*/
uint32_t btoven_enc_size( btoven_enc enc )
{ 
        if( enc & BTOVEN_ENC_8 )
                return ( ( 8 / CHAR_BIT ) + 0.99f );
        else if( enc & BTOVEN_ENC_16 )
                return ( ( 16 / CHAR_BIT ) + 0.99f );
        else if( enc & BTOVEN_ENC_24 )
                return ( ( 24 / CHAR_BIT ) + 0.99f );
        else if( enc & BTOVEN_ENC_32 )
                return ( ( 32 / CHAR_BIT ) + 0.99f );
        else if( enc & BTOVEN_ENC_64 )
                return ( ( 64 / CHAR_BIT ) + 0.99f );
        return 0;
}

/*---------------------------------------------------------------------------------------
 btoven_enc_supported
 Input: btoven_enc enc -- The requested encoding
 Output: bool -- True if the encoding is supported, false if it isn't
 Description: Find out if the format supported on this platform
 ---------------------------------------------------------------------------------------*/
bool btoven_enc_supported( btoven_enc enc )
{ 
        // We can use the same logic as in the header to decide if the encoding is supported
        switch( enc )
        {
                case BTOVEN_ENC_SIGNED_8:
                case BTOVEN_ENC_UNSIGNED_8:
#ifdef INT8_MAX
                        return true;
#else // INT8_MAX
                        return false;
#endif // INT8_MAX
                        break;
                case BTOVEN_ENC_SIGNED_16:
                case BTOVEN_ENC_UNSIGNED_16:
#ifdef INT16_MAX
                        return true;
#else // INT16_MAX
                        return false;
#endif // INT16_MAX
                        break;
                case BTOVEN_ENC_SIGNED_24:
                case BTOVEN_ENC_UNSIGNED_24:
#ifdef INT24_MAX
                        return true;
#else // INT24_MAX
                        return false;
#endif // INT24_MAX
                        break;
                case BTOVEN_ENC_SIGNED_32:
                case BTOVEN_ENC_UNSIGNED_32:
#ifdef INT32_MAX
                        return true;
#else // INT32_MAX
                        return false;
#endif // INT32_MAX
                        break;
                case BTOVEN_ENC_FLOAT_32:
#ifdef BTOVEN_FLOAT32
                        return true;
#else // BTOVEN_FLOAT32
                        return false;
#endif // BTOVEN_FLOAT32
                        break;
                case BTOVEN_ENC_FLOAT_64:
#ifdef BTOVEN_FLOAT64
                        return true;
#else // BTOVEN_FLOAT64
                        return false;
#endif // BTOVEN_FLOAT64
                        break;
                default:
                        return false;
        }
}

/*---------------------------------------------------------------------------------------
 btoven_getPCM
 Input: btoven_audioformat& -- The audio format that the audio to decode is in
        void** -- The raw data, encoding given by mEncoding
        uint32_t -- The requested frame
        uint32_t -- The requested channel
 Output: BTOVEN_STORETYPE -- The requested PCM value
 Description: Decodes the PCM in the given array as denoted by the encoding
 ---------------------------------------------------------------------------------------*/
BTOVEN_STORETYPE btoven_decodePCM( btoven_audioformat *af, void** pcm, uint8_t channel, uint32_t frame )
{
        // We are unable to do bounds checking on the frame, but we can check the channel!
        if( channel >= af->channels )
                return 0;
        
        // Return the requested data
        if( af->interleaved )
                return BTOVEN_DECODE( af->enc )( pcm, 0, ( frame * af->channels ) + channel );
        else 
                return BTOVEN_DECODE( af->enc )( pcm, channel, frame );
        return 0;
}

/*---------------------------------------------------------------------------------------
 btoven_enc2idx
 Input: btoven_enc -- The requested encoding
 Output: int32_t -- The index of the decoding function in the macro array
 Description: Returns -1 if the requested encoding isn't supported
 ---------------------------------------------------------------------------------------*/
int32_t btoven_enc2idx( btoven_enc enc )
{
		int32_t ret = 0;

        // Ensure the encoding is supported first
        if( !btoven_enc_supported( enc ) )
                return -1; 
        
        // Find which index represents this encoding
#ifdef INT8_MAX
        if( enc == BTOVEN_ENC_SIGNED_8 ) return ret; else ret++;
        if( enc == BTOVEN_ENC_UNSIGNED_8 ) return ret; else ret++;
#endif // INT8_MAX
#ifdef INT16_MAX
        if( enc == BTOVEN_ENC_SIGNED_16 ) return ret; else ret++;
        if( enc == BTOVEN_ENC_UNSIGNED_16 ) return ret; else ret++;
#endif // INT16_MAX
#ifdef INT24_MAX
        if( enc == BTOVEN_ENC_SIGNED_24 ) return ret; else ret++;
        if( enc == BTOVEN_ENC_UNSIGNED_24 ) return ret; else ret++;
#endif // INT24_MAX
#ifdef INT32_MAX
        if( enc == BTOVEN_ENC_SIGNED_32 ) return ret; else ret++;
        if( enc == BTOVEN_ENC_UNSIGNED_32 ) return ret; else ret++;
#endif // INT32_MAX
#ifdef BTOVEN_FLOAT32
        if( enc == BTOVEN_ENC_FLOAT_32 ) return ret; else ret++;
#endif // BTOVEN_FLOAT32
#ifdef BTOVEN_FLOAT64
        if( enc == BTOVEN_ENC_FLOAT_64 ) return ret; else ret++;
#endif // BTOVEN_FLOAT64
        
        // Shouldn't ever get here, but if we do, something bad happened.
        return -1;
}

