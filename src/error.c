// btoven
// 2011 Motalen

#include <btoven.h>

const char* btoven_error_string( btoven_error err )
{
	switch( err )
	{
		case BTOVEN_OK:
			return "System OK.";
			break;
		case BTOVEN_ALREADY_INITIALIZED:
			return "System already initialized, invalid request.";
			break;
		case BTOVEN_NOT_INITIALIZED:
			return "System not yet initialized, invalid request.";
			break;
		case BTOVEN_ENCODING_NOT_SUPPORTED:
			return "The requested encoding is not supported by btoven on this platform.";
			break;
		case BTOVEN_INVALID_TRACK_HANDLE:
			return "The trackhandle is invalid.";
			break;
		case BTOVEN_OTHER:
		default:
			return "Flagrant System Error.";
			break;
	}
}
