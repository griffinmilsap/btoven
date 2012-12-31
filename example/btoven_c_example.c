// btoven
// Motalen 2011

#include <btoven.h>
#include <portaudio.h>
#include <mpg123.h>
#include <stdio.h>
#include <string.h>

#ifdef __GNUC__
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#endif // __GNUC__

#ifdef __MSC_VER__
#include <conio.h>
#endif // __MSC_VER__

//#define DONT_PRINT

// Globals
const double SAMPLE_RATE = 44100.0;
const int FRAMES_PER_BUFFER = 1024;
const int NUM_CHANNELS = 2;
mpg123_handle *mh = NULL;
int test = 0;
btoven_state state;

// Version info
void printVersion()
{
	char version[32];
	btoven_get_version_string( version, 32 );

	printf( "btoven\n" );
	printf( "Version %s\n", version );
	printf( "Motalen 2011\n" );
}

// Help
void printHelp()
{
	printf( "btoven_c_example\n" );
	printf( "Supply no arguments for realtime audio input\n" );
	printf( "-f <filename> - Use an MP3 formatted input file\n" );
}

// Portaudio Error Handling
bool checkPaErr( PaError err, const char* msg )
{
	if( err != paNoError )
	{
		printf( msg );
		printf( Pa_GetErrorText( err ) );
		printf( "\n" );
		return true;
	}
	else
		return false;
}

// btoven Error Handling
bool checkBtovenError( btoven_error err )
{
	if( err != BTOVEN_OK )
	{
		printf( btoven_error_string( err ) );
		printf( "\n" );
		return true;
	}
	else
		return false;
}

#ifdef __GNUC__
int _kbhit(void)
{
  struct termios oldt, newt;
  int ch;
  int oldf;

  tcgetattr(STDIN_FILENO, &oldt);
  newt = oldt;
  newt.c_lflag &= ~(ICANON | ECHO);
  tcsetattr(STDIN_FILENO, TCSANOW, &newt);
  oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
  fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);

  ch = getchar();

  tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
  fcntl(STDIN_FILENO, F_SETFL, oldf);

  if(ch != EOF)
  {
    ungetc(ch, stdin);
    return 1;
  }

  return 0;
}
#endif // __GNUC__

int main( int argc, char* argv[] )
{
	size_t i, j, idx, block_size;
	char* filename = NULL;
	btoven_audioformat fmt;
	btoven_trackhandle h;
	PaStream *stream = NULL;
	PaStreamParameters inputParameters, outputParameters;
	char bar[32] = "";
	int16_t* block = NULL;

	printf( "========================\n" );
	printVersion();
	printf( "Example C Implementation\n" );
	printf( "========================\n\n" );
	
	// Parse arguments
	for( i = 1; i < ( size_t )argc; i++ )
	{
		char* arg = argv[i];
		if( !strcmp( arg, "--help" ) || !strcmp( arg, "-h" ) || !strcmp( arg, "?" ) ) 
		{
			printHelp();
			return 0;
		}
		else if( !strcmp( arg, "-f" ) )
			filename = argv[ ++i ];
		else
			++i;
	}
	
	// Prepare MPG123 to read the file
	if( filename )
	{
		int err = MPG123_OK;
		long rate = 0;
		int channels = 0, encoding = 0;

		printf( "User specified input file.\n" );
		printf( "Initializing mpg123 for decoding: %s\n", filename );
		
		// Initialize file and engine
		if(  ( err = mpg123_init() ) != MPG123_OK 
			|| ( mh = mpg123_new( NULL, &err ) ) == NULL
			|| ( err = mpg123_open( mh, filename ) ) != MPG123_OK )
		{
			( mh == NULL ) ? mpg123_plain_strerror( err ) : mpg123_strerror( mh );
		 	return err;
		}
		printf( "MPG123 Initialized.\n" );
		
		// Grab the format
		if( mpg123_getformat( mh, &rate, &channels, &encoding ) != MPG123_OK )
		{
			printf( "Trouble with mpg123: %s\n",
				( mh == NULL ) ? 
					mpg123_plain_strerror( err ) : mpg123_strerror( mh ) );
			return -1;
		}
		
		// Set the format to what we want
		mpg123_format_none( mh );
		mpg123_format( mh, ( long )SAMPLE_RATE, MPG123_STEREO, MPG123_ENC_SIGNED_16 );
	}
	
	// Initialize btoven
	printf( "Initializing btoven...\n" );
	if( checkBtovenError( btoven_initialize( NULL ) ) )
		return -1;

	fmt.enc = BTOVEN_ENC_SIGNED_16;
	fmt.rate = ( uint32_t )SAMPLE_RATE;
	fmt.channels = 2;
	fmt.interleaved = true;
	btoven_create_track( fmt, &h );
	printf( "btoven_trackhandle: %u\n", h );
	
	// Set up portaudio to play the file
	if( checkPaErr( Pa_Initialize(), "Error Initializing Portaudio.\n" ) )
		return -1;
	
	inputParameters.device = Pa_GetDefaultInputDevice();
	inputParameters.channelCount = 2; // Stereo
	inputParameters.sampleFormat = paInt16;
	inputParameters.suggestedLatency = 
		Pa_GetDeviceInfo( inputParameters.device )->defaultLowInputLatency;
	inputParameters.hostApiSpecificStreamInfo = NULL;
	
	outputParameters.device = Pa_GetDefaultOutputDevice();
	outputParameters.channelCount = 2; // Stereo
	outputParameters.sampleFormat = paInt16;
	outputParameters.suggestedLatency = 
		Pa_GetDeviceInfo( outputParameters.device )->defaultLowOutputLatency;
	outputParameters.hostApiSpecificStreamInfo = NULL;
	
	if( filename ) {
		if( checkPaErr( Pa_OpenStream( &stream, NULL, &outputParameters, SAMPLE_RATE, 
		FRAMES_PER_BUFFER, paClipOff, NULL, NULL ), "Error Opening Output Stream" ) )
			return -1;
	} else {
		if( checkPaErr( Pa_OpenStream( &stream, &inputParameters, NULL, SAMPLE_RATE, 
		FRAMES_PER_BUFFER, paClipOff, NULL, NULL ), "Error Opening Input Stream" ) )
			return -1;
	}
	
	if( checkPaErr( Pa_StartStream( stream ), "Error Starting stream.\n" ) )
		return -1;
	
	// Enter the processing loop
	block_size = FRAMES_PER_BUFFER * NUM_CHANNELS;
	block = ( int16_t* )malloc( sizeof( int16_t ) * block_size );
	if( filename )
		printf( "Reading decoded audio into btoven with playback.\n" );
	else
		printf( "Streaming microphone input into btoven.\n" );
	printf( "Press any key to exit.\n" );
	while( !_kbhit() )
	{
		if( filename )
		{
			int err = MPG123_OK;
			size_t length = 0;
			err = mpg123_read( mh, ( unsigned char* )block,
				block_size * sizeof( int16_t ), &length );
			if( err != MPG123_OK )
			{
				if( err == MPG123_DONE )
					printf( "Music has ended.\n" );
				else
					printf( "MPG123 error code: %d\n", err );
				break;
			}
		}
		else
			if( checkPaErr( Pa_ReadStream( stream, block, FRAMES_PER_BUFFER ),
				"Error Reading from Stream.\n" ) )
				return -1;
			
		// Feed btoven, display output
		btoven_process( h, FRAMES_PER_BUFFER, block );
		state = btoven_read_state( h );
#ifndef DONT_PRINT
		printf( "btoven ---------------------------\n" );
		printf( "bpm: %d \t Confidence = 0%% \n", state.bpm );
		
		for( j = 0; j < state.cur_intensity; j += 200 )
			strcat( bar, "=" );
		printf( "cur_intensity: %s \n", bar );
		
		strcpy( bar, "" );
		for( j = 0; j < state.sec_intensity; j += 200 )
			strcat( bar, "=" );
		printf( "sec_intensity: %s \n", bar );
		
		idx = state.percent_to_next / 32;
		printf( "percent_to_next: [" );
		for( j = 0; j < 8; j++ )
			if( j == idx ) printf( "|" ); else printf( " " );
		printf( "] %d%%\n", ( state.percent_to_next * 100 ) / 256 );
		
		printf( "pitch:   [" );
		idx = state.pitch / 8;
		for( j = 0; j < 32; j++ )
			if( j == idx ) printf( "|" ); else printf( " " );
		printf( "] %d\n", state.pitch ); 
		
		printf( "pitch_2: [" );
		idx = state.pitch_2 / 8;
		for( j = 0; j < 32; j++ )
			if( j == idx ) printf( "|" ); else printf( " " );
		printf( "] %d\n", state.pitch_2 ); 
		
		printf( "pitch_3: [" );
		idx = state.pitch_3 / 8;
		for( j = 0; j < 32; j++ )
			if( j == idx ) printf( "|" ); else printf( " " );
		printf( "] %d\n", state.pitch_3 ); 
		
		if( state.beat ) printf( "BEAT!\n" ); else printf( "\n" );
		if( state.transient_low ) printf( "Low Transient!\n" ); else printf( "\n" );
		if( state.transient_mid ) printf( "Mid Transient!\n" ); else printf( "\n" );
		if( state.transient_high ) printf( "High Transient!\n" ); else printf( "\n" );
#endif // DONT_PRINT
		
		if( filename )
			if( checkPaErr( Pa_WriteStream( stream, block, FRAMES_PER_BUFFER ),
				"Error Writing Stream.\n" ) )
				return -1;
	}
	printf( "Cleaning up btoven\n" );
	btoven_cleanup();
	printf( "btoven terminated.\n" );
	
	printf( "Terminating PortAudio\n" );
	if( checkPaErr( Pa_StopStream( stream ), "Error Stopping Stream...\n" ) )
		return -1;
	if( checkPaErr( Pa_CloseStream( stream ), "Error Closing Stream...\n" ) )
		return -1;
		
	Pa_Terminate();
	printf( "PortAudio terminated.\n" );
	
	if( filename )
	{
		printf( "Ending mpg123\n" );
		mpg123_close( mh );
		mpg123_delete( mh );
		mpg123_exit();
		mh = NULL;
		printf( "mpg123 terminated\n" );
	}

	// Everything went okay
	printf( "btoven_c_example executed successfully\n" );
	return 0;
}
	
