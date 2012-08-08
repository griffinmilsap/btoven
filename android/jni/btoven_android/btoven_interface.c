// btoven
// Motalen 2011

#include <assert.h>
#include <jni.h>
#include <string.h>
#include <android/log.h>
#include <pthread.h>

#include "btoven_interface.h"

// Engine Interfaces
static SLObjectItf engineObject = NULL;
static SLEngineItf engineEngine;
static SLThreadSyncItf sync;
static SLObjectItf outputMixObject = NULL;
static pthread_mutex_t mutex;

// Prototypes
bool SL_ERRCHECK( SLresult res );
bool BT_ERRCHECK( btoven_error err );
void OutputPlayerCallback( SLAndroidSimpleBufferQueueItf bq, void *context );
void InputPlayerCallback(SLAndroidSimpleBufferQueueItf bq, void *context );
void PlayStatusCallback( SLPlayItf play, void* context, SLuint32 event );
bool TrackHandleValid( btTrackHandle handle );
SLDataFormat_PCM DetermineFormat( btTrack* t );
btTrackHandle FindAvailableTrack();
btTrackHandle RealizeTrack( btTrackHandle h, SLDataSource audioSrc );
void CreateAudioOutput( btTrack* t );
void Lock();
void Unlock();

// Check OpenSL Errors
#define LOG( format, args... ) __android_log_print( ANDROID_LOG_DEBUG, "btoven_android", format, ##args );
bool SL_ERRCHECK( SLresult res ) 
{
	if( res != SL_RESULT_SUCCESS )
	{
		LOG( "OpenSL Error: Result = %d\n", res );
		return true;
	}
	return false;
}

// Check btoven Errors
bool BT_ERRCHECK( btoven_error err )
{
	if( err != BTOVEN_OK )
	{
		LOG( "btoven Error: %s\n", btoven_error_string( err ) );
		return true;
	}
	return false;
}

// Output Callback
void OutputPlayerCallback( SLAndroidSimpleBufferQueueItf bq, void *context )
{
	Lock();
	btTrack* t = ( btTrack* )context;
	t->Distance--;
	if( t->Distance <= ( BT_RING_BUFFER_SIZE / 4 ) )
		SL_ERRCHECK( ( *( t->IPlay ) )->SetPlayState( t->IPlay, SL_PLAYSTATE_PLAYING ) );
		
	if( t->Finishing && t->Distance == 0 ) {
		t->Finished = true;
		SL_ERRCHECK( ( *( t->OPlay ) )->SetPlayState( t->OPlay, SL_PLAYSTATE_PAUSED ) );
	}
	Unlock();
	
	// Wait until the distance is proper
	while( t->Distance == 0 ) usleep( 1000 ); // TODO: TIMEOUT
	
	// Enqueue more data if we have it
	Lock();
	if ( t->Distance > 0 ) {
		SL_ERRCHECK( ( *( t->OBufferQueue ) )->Enqueue( t->OBufferQueue, t->RingBuffer[ t->RingRead ], btBufferSizeBytes ) );
		t->RingRead = ( t->RingRead + 1 ) % BT_RING_BUFFER_SIZE;
		t->PlayHead = ( t->PlayHead + 1 ) % BT_RING_BUFFER_SIZE;
		t->ProcessedBlock = false;
	}
	Unlock();
}

// PCM Decode Input Callback
void InputPlayerCallback( SLAndroidSimpleBufferQueueItf bq, void *context )
{
	Lock();
	btTrack* t = ( btTrack* )context;
	t->Distance++;

	if( !( t->Initialized ) ) {
		if( t->Distance == ( BT_RING_BUFFER_SIZE / 2 ) ) {
			CreateAudioOutput( t );
			// Enqueue Output Buffers
			while( t->Distance != 0 ) {
				SL_ERRCHECK( ( *( t->OBufferQueue ) )->Enqueue( t->OBufferQueue, t->RingBuffer[ t->RingRead ], btBufferSizeBytes ) );
				t->RingRead = ( t->RingRead + 1 ) % BT_RING_BUFFER_SIZE;
				( t->Distance )--;
			}
			t->Initialized = true;
		}
	} else {
		if( t->Distance >= ( BT_RING_BUFFER_SIZE / 4 ) )
			SL_ERRCHECK( ( *( t->IPlay ) )->SetPlayState( t->IPlay, SL_PLAYSTATE_PAUSED ) );
	}
	SL_ERRCHECK( ( *( t->IBufferQueue ) )->Enqueue( t->IBufferQueue, t->RingBuffer[ t->RingWrite ], btBufferSizeBytes ) );
	t->RingWrite = ( t->RingWrite + 1 ) % BT_RING_BUFFER_SIZE;
	Unlock();
}

// Called when the decoding has an event (such as track end)
void PlayStatusCallback( SLPlayItf play, void* context, SLuint32 event )
{
	LOG( "PlayStatus Callback!" );
	btTrack* t = ( btTrack* )context;
	if( event == SL_PLAYEVENT_HEADATEND )
	{
		LOG( "Track Ended!" );
		// Write zeros for the rest of the write buffers
		Lock();
		t->Finishing = 1;
		Unlock();
	}
}

/*
// Recorded PCM Input Callback
void InputRecorderCallback(SLAndroidSimpleBufferQueueItf bq, void *context)
{
	// TODO: Recorder
	// Feed PCMBuffer[WriteBuf] into BTOVEN
	// Enqueue PCMBuffer[WriteBuf]
	// WriteBuf = ( WriteBuf ) ? 0 : 1;
	
    // for streaming recording, here we would call Enqueue to give recorder the next buffer to fill
    // but instead, this is a one-time buffer so we stop recording
    SLresult result;
    result = (*recorderRecord)->SetRecordState(recorderRecord, SL_RECORDSTATE_STOPPED);
    if (SL_RESULT_SUCCESS == result) {
        recorderSize = RECORDER_FRAMES * sizeof(short);
        recorderSR = SL_SAMPLINGRATE_16;
    }
}
*/

void btCreateEngine()
{
	int i;
	char version[32];

	// Create engine
	SL_ERRCHECK( slCreateEngine( &engineObject, 0, NULL, 0, NULL, NULL ) );

	// Realize the engine
	SL_ERRCHECK( ( *engineObject )->Realize(engineObject, SL_BOOLEAN_FALSE) );

	// Get the engine threadsync interface (Which we can't get, so we'll use a pthread mutex)
	LOG( "Getting the threadsync interface" );
	if( SL_ERRCHECK( ( *engineObject )->GetInterface( engineObject, SL_IID_THREADSYNC, &sync ) ) ) {
		LOG( "Unable to get threadsync interface --> using pthread mutex instead!" );
		pthread_mutex_init( &mutex, NULL );
	}

	// Get the engine interface, which is needed in order to create other objects
	SL_ERRCHECK( ( *engineObject )->GetInterface( engineObject, SL_IID_ENGINE, &engineEngine ) );

	// Create output mix
	SL_ERRCHECK( ( *engineEngine )->CreateOutputMix( engineEngine, &outputMixObject, 0, NULL, NULL ) );

	// Realize the output mix
	SL_ERRCHECK( ( *outputMixObject )->Realize( outputMixObject, SL_BOOLEAN_FALSE ) );

	// Set all tracks as unallocated and uninitialized
	for( i = 0; i < BT_MAX_NUM_TRACKS; i++ )
	{
		btTracks[i].OObject = NULL;
		btTracks[i].IObject = NULL;
		btTracks[i].Resource = NULL;
		btTracks[i].Allocated = false;
		btTracks[i].Initialized = false;
		btTracks[i].Finishing = 0; // FIXME: FALSE
		btTracks[i].Finished = false;
	}
	
	// Initialize btoven
	BT_ERRCHECK( btoven_initialize( &btoven_mobile_config ) );
	btoven_get_version_string( version, 32 );
	LOG( "BTOVEN Version %s Initialized: %d", version, btoven_initialized() );
}

bool TrackHandleValid( btTrackHandle handle )
{
	// Simple Sanity Check -- Can put more interesting checks in later.
	if( handle < 0 || handle > BT_MAX_NUM_TRACKS )
		return false;
	return true;
}

// Return an available track handle or BT_INVALID_TRACKHANDLE if none are available
btTrackHandle FindAvailableTrack()
{
	btTrackHandle h;

	// Find a valid unused track index
	for( h = 0; h < BT_MAX_NUM_TRACKS; h++ )
		if( !btTracks[h].Allocated )
			return h;

	return BT_INVALID_TRACKHANDLE;
}

btoven_state btProcessTrack( btTrackHandle h )
{
	if( !TrackHandleValid( h ) ) return;
	btTrack* t = &( btTracks[h] );

	// Wait for data to feed btoven
	while( btIsPlaying( h ) && t->ProcessedBlock ) usleep( 1000 ); // TODO: TIMEOUT
	
	Lock();
	// Feed BTOVEN 
	BT_ERRCHECK( btoven_process( t->Handle, BT_BUFFER_SAMPLES / t->Format.channels, t->RingBuffer[ t->PlayHead ] ) );
	t->ProcessedBlock = true;
	Unlock();
	
	// Grab and Save the state
	return btoven_read_state( t->Handle );
}

btTrackHandle btCreateAssetTrack( AAssetManager* assetManager, const char* asset )
{
	// Find an available trackhandle
	btTrackHandle h = FindAvailableTrack();
	if( h == BT_INVALID_TRACKHANDLE )
		return BT_INVALID_TRACKHANDLE;

	// Create an input data source for this track
	btTrack* t = &( btTracks[h] );
	AAsset* assetObject = AAssetManager_open( assetManager, asset, AASSET_MODE_UNKNOWN );
	if ( NULL == assetObject )
	{
		LOG( "Cannot open requested asset: %s\n", asset );
		return BT_INVALID_TRACKHANDLE;
	}
	t->Resource = malloc( strlen( asset ) + 1 );
	strcpy( t->Resource, asset );
	
	// Open asset as file descriptor
	off_t start, length;
	int fd = AAsset_openFileDescriptor( assetObject, &start, &length );
	AAsset_close(assetObject);

	// Configure audio source
	SLDataLocator_AndroidFD loc_fd = { SL_DATALOCATOR_ANDROIDFD, fd, start, length };
	SLDataFormat_MIME format_mime = { SL_DATAFORMAT_MIME, NULL, SL_CONTAINERTYPE_UNSPECIFIED };
	SLDataSource audioSrc = { &loc_fd, &format_mime };

	// Realize the track
	return RealizeTrack( h, audioSrc );
}

// Requires the INTERNET permission depending on the uri parameter
btTrackHandle btCreateURITrack( const char* uri )
{
	// Find an available trackhandle
	btTrackHandle h = FindAvailableTrack();
	if( h == BT_INVALID_TRACKHANDLE )
		return BT_INVALID_TRACKHANDLE;
	btTrack* t = &( btTracks[h] );

	// Configure audio source
	SLDataLocator_URI loc_uri = { SL_DATALOCATOR_URI, ( SLchar* )uri };
	SLDataFormat_MIME format_mime = { SL_DATAFORMAT_MIME, NULL, SL_CONTAINERTYPE_UNSPECIFIED };
	SLDataSource audioSrc = { &loc_uri, &format_mime };
	t->Resource = malloc( strlen( uri ) + 1 );
	strcpy( t->Resource, uri );
	
	// Realize the track
	return RealizeTrack( h, audioSrc );
}

// Given an audio source (either URI or ASSET), realize the player
btTrackHandle RealizeTrack( btTrackHandle h, SLDataSource audioSrc )
{
	int i;
	
	// Acquire the track
	if( !TrackHandleValid( h ) ) return BT_INVALID_TRACKHANDLE;
	btTrack* t = &( btTracks[ h ] );
	
	// Set the Object as Allocated
	t->Allocated = true;
	
	// Configure audio sink (Request these PCMFormat specifications -- We may not be so lucky)
	SLDataLocator_AndroidSimpleBufferQueue loc_bufq = { SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE, BT_RING_BUFFER_SIZE };
	SLDataFormat_PCM format_pcm = { SL_DATAFORMAT_PCM, 2, SL_SAMPLINGRATE_44_1,
		SL_PCMSAMPLEFORMAT_FIXED_16, SL_PCMSAMPLEFORMAT_FIXED_16,
		SL_SPEAKER_FRONT_LEFT|SL_SPEAKER_FRONT_RIGHT, SL_BYTEORDER_LITTLEENDIAN };
	SLDataSink audioSnk = { &loc_bufq, &format_pcm };

	// Create audio player
	const SLInterfaceID ids[3] = { SL_IID_ANDROIDSIMPLEBUFFERQUEUE, SL_IID_PLAY, SL_IID_METADATAEXTRACTION };
	const SLboolean req[3] = { SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE };
	SL_ERRCHECK( ( *engineEngine )->CreateAudioPlayer( engineEngine, &( t->IObject ), &audioSrc, &audioSnk, 3, ids, req ) );
	
	// Allocate RingBuffer
	t->RingRead = 0; t->RingWrite = 0;
	t->PlayHead = 0; t->Distance = 0;
	t->ProcessedBlock = true;
	for( i = 0; i < BT_RING_BUFFER_SIZE; i++ ) 
		t->RingBuffer[i] = ( btPCM* )malloc( btBufferSizeBytes );

	// Realize the player -- We might fail here if the URI is invalid
	if( SL_ERRCHECK( ( *( t->IObject ) )->Realize( t->IObject, SL_BOOLEAN_FALSE ) ) )
	{
		LOG( "Error Realizing AudioPlayer for Resource: %s\n", t->Resource );
		btDestroyTrack( h );
		return BT_INVALID_TRACKHANDLE;
	}

	// Get the play interface
	SL_ERRCHECK( ( *( t->IObject ) )->GetInterface( t->IObject, SL_IID_PLAY, &( t->IPlay ) ) );

	// Get the buffer queue interface
	SL_ERRCHECK( ( *( t->IObject ) )->GetInterface( t->IObject, SL_IID_BUFFERQUEUE, &( t->IBufferQueue ) ) );

	// Get the metadata interface
	SL_ERRCHECK( ( *( t->IObject ) )->GetInterface( t->IObject, SL_IID_METADATAEXTRACTION, &( t->IMetaData ) ) );

	// Register callback on the buffer queue
	SL_ERRCHECK( ( *( t->IBufferQueue ) )->RegisterCallback( t->IBufferQueue, InputPlayerCallback, t ) );

	// Enqueue the ring buffer
	for( i = 0; i < BT_RING_BUFFER_SIZE / 2; i++ ) {
		t->RingWrite = ( t->RingWrite + 1 ) % BT_RING_BUFFER_SIZE;
		SL_ERRCHECK( ( *( t->IBufferQueue ) )->Enqueue( t->IBufferQueue, t->RingBuffer[i], btBufferSizeBytes ) );
	}

	// Set a callback on the play interface for play events
	SL_ERRCHECK( ( *( t->IPlay ) )->RegisterCallback( t->IPlay, PlayStatusCallback, t ) );
	SL_ERRCHECK( ( *( t->IPlay ) )->SetCallbackEventsMask( t->IPlay, SL_PLAYEVENT_HEADATEND ) );

	// Set the Audio Input to Play
	SL_ERRCHECK( ( *( t->IPlay ) )->SetPlayState( t->IPlay, SL_PLAYSTATE_PLAYING ) );
	
	// Wait until the track is initialized
	while( !( t->Initialized ) ) usleep( 10000 ); // TODO: TIMEOUT
		
	LOG( "Track Initialized!" );
	return h;
}

// HOLY CODE-BLOAT BATMAN!
SLDataFormat_PCM DetermineFormat( btTrack* t )
{
	SLuint32 mdCount = 0;
	SLuint32 i;
	SLDataFormat_PCM ret = { SL_DATAFORMAT_PCM, 0, 0, 0, 0, 0, 0 };
	
	// Scan through the metadata items
	SL_ERRCHECK( ( *( t->IMetaData ) )->GetItemCount( t->IMetaData, &mdCount ) );
	for( i = 0; i < mdCount; ++i )
	{
		SLMetadataInfo *key = NULL;
		SLMetadataInfo *value = NULL;
		SLuint32 itemSize = 0;
		
		// Get the size of and malloc memory for the metadata item
		SL_ERRCHECK( ( *( t->IMetaData ) )->GetKeySize( t->IMetaData, i, &itemSize ) );
		key = malloc( itemSize );
		if( key )
		{
			// Extract the key into the memory
			SL_ERRCHECK( ( *( t->IMetaData ) )->GetKey( t->IMetaData, i, itemSize, key ) );
			SL_ERRCHECK( ( *( t->IMetaData ) )->GetValueSize( t->IMetaData, i, &itemSize ) );
			value = malloc( itemSize );
			if( value )
			{ 
				// Extract the value into memory
				SL_ERRCHECK( ( *( t->IMetaData ) )->GetValue( t->IMetaData, i, itemSize, value ) );
				LOG( "Key: %s, Value: %d", key->data, *( ( SLuint32* )value->data ) );
				if( strcmp( key->data, ANDROID_KEY_PCMFORMAT_NUMCHANNELS ) == 0 )
					ret.numChannels = *( ( SLuint32* )value->data );
				else if( strcmp( key->data, ANDROID_KEY_PCMFORMAT_SAMPLERATE ) == 0 )
					ret.samplesPerSec = *( ( SLuint32* )value->data ) * 1000;
				else if( strcmp( key->data, ANDROID_KEY_PCMFORMAT_BITSPERSAMPLE ) == 0 )
					ret.bitsPerSample = *( ( SLuint32* )value->data );
				else if( strcmp( key->data, ANDROID_KEY_PCMFORMAT_CONTAINERSIZE ) == 0 )
					ret.containerSize = *( ( SLuint32* )value->data );
				else if( strcmp( key->data, ANDROID_KEY_PCMFORMAT_CHANNELMASK ) == 0 )
					ret.channelMask = *( ( SLuint32* )value->data );
				else if( strcmp( key->data, ANDROID_KEY_PCMFORMAT_ENDIANNESS ) == 0 )
					ret.endianness = *( ( SLuint32* )value->data );
				free(value);
			}
			free(key);
		}
	}
	
	return ret;
}


// Create an output from metadata
void CreateAudioOutput( btTrack* t )
{
	btoven_enc enc;

	// Configure audio source
	SLDataLocator_AndroidSimpleBufferQueue loc_bufq = { SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE, BT_RING_BUFFER_SIZE };
	SLDataFormat_PCM format_pcm = DetermineFormat( t );
	SLDataSource audioSrc = { &loc_bufq, &format_pcm };

	// Configure audio sink
	SLDataLocator_OutputMix loc_outmix = { SL_DATALOCATOR_OUTPUTMIX, outputMixObject };
	SLDataSink audioSnk = { &loc_outmix, NULL };

	// Create audio player
	LOG( "AudioOutput: Create Audio Player" );
	const SLInterfaceID ids[2] = { SL_IID_BUFFERQUEUE, SL_IID_VOLUME };
	const SLboolean req[2] = { SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE };
	SL_ERRCHECK( ( *engineEngine )->CreateAudioPlayer( engineEngine, &( t->OObject ), &audioSrc, &audioSnk, 2, ids, req ) );

	// Realize the player
	SL_ERRCHECK( ( *( t->OObject ) )->Realize( t->OObject, SL_BOOLEAN_FALSE ) );

	// Get the play interface
	SL_ERRCHECK( ( *( t->OObject ) )->GetInterface( t->OObject, SL_IID_PLAY, &( t->OPlay ) ) );

	// Get the buffer queue interface
	SL_ERRCHECK( ( *( t->OObject ) )->GetInterface( t->OObject, SL_IID_BUFFERQUEUE, &( t->OBufferQueue ) ) );

	// Register callback on the buffer queue
	SL_ERRCHECK( ( *( t->OBufferQueue ) )->RegisterCallback( t->OBufferQueue, OutputPlayerCallback, t ) );

	// Get the volume interface
	SL_ERRCHECK( ( *( t->OObject ) )->GetInterface( t->OObject , SL_IID_VOLUME, &( t->OVolume ) ) );

	// Create a btoven Track for processing this data
	switch( format_pcm.containerSize ) {
		case 8:
			enc = BTOVEN_ENC_SIGNED_8;
			break;
		case 16:
			enc = BTOVEN_ENC_SIGNED_16;
			break;
		case 32:
			enc = BTOVEN_ENC_SIGNED_32;
			break;
		default:
			enc = BTOVEN_ENC_SIGNED_16;
			LOG( "BTOVEN warning -- Nonstandard PCM container size" );
	}
	//BT_ERRCHECK( btoven_set_encoding( &( t->Format ), enc ) ); //FIXME
	t->Format.enc = BTOVEN_ENC_SIGNED_16;
	t->Format.rate = ( uint32_t )format_pcm.samplesPerSec / 1000;
	t->Format.channels = ( uint32_t )format_pcm.numChannels;
	t->Format.interleaved = true;
	LOG( "Creating a BTOVEN Track..." );
	BT_ERRCHECK( btoven_create_track( t->Format, &( t->Handle ) ) );
}

void btPlayTrack( btTrackHandle handle )
{
	if( !TrackHandleValid( handle ) )
		return;

	btTrack* t = &( btTracks[ handle ] );
	Lock();
	if( t->Initialized )
		SL_ERRCHECK( ( *( t->OPlay ) )->SetPlayState( t->OPlay, SL_PLAYSTATE_PLAYING ) );
	Unlock();
}

void btPauseTrack( btTrackHandle handle )
{
	if( !TrackHandleValid( handle ) )
		return;

	btTrack* t = &( btTracks[ handle ] );
	Lock();
	if( t->Initialized )
		SL_ERRCHECK( ( *( t->OPlay ) )->SetPlayState( t->OPlay, SL_PLAYSTATE_PAUSED ) );
	Unlock();
}

// Consider waiting to return from this function until initialized
void btStopTrack( btTrackHandle handle )
{
	int i;
	btTrack* t = NULL;

	if( !TrackHandleValid( handle ) )
		return;

	// Stop the players
	t = &( btTracks[ handle ] );
	Lock();
	if( t->Initialized )
	{
		SL_ERRCHECK( ( *( t->OPlay ) )->SetPlayState( t->OPlay, SL_PLAYSTATE_STOPPED ) );
		SL_ERRCHECK( ( *( t->IPlay ) )->SetPlayState( t->IPlay, SL_PLAYSTATE_STOPPED ) );
	}

	// Reset the Input and prepare to play again from start
	t->Finishing = 0;
	t->Finished = false;
	t->Initialized = false;
	SL_ERRCHECK( ( *( t->IPlay ) )->SetPlayState( t->IPlay, SL_PLAYSTATE_PLAYING ) );

	// Reset btoven
	//btoven_reset_track( t->Handle );
	Unlock();
	
	// Wait until the track is initialized again
	while( !( t->Initialized ) )
		usleep( 10000 ); // TODO: TIMEOUT
}

bool btIsFinished( btTrackHandle handle )
{
	if( !TrackHandleValid( handle ) )
		return false;
	
	return ( btTracks[ handle ].Finished ) ? true : false;
}

bool btIsPlaying( btTrackHandle handle )
{
	SLuint32 playState;
	btTrack* t;

	if( !TrackHandleValid( handle ) )
		return;

	t = &( btTracks[ handle ] );

	if( t->Initialized )
	{
		SL_ERRCHECK( ( *( t->OPlay ) )->GetPlayState( t->OPlay, &playState ) );
		if( playState == SL_PLAYSTATE_PLAYING )
			return true;
		else
			return false;
	}
}

void btDestroyTrack( btTrackHandle handle )
{
	int i;
	btTrack* t;
	if( !TrackHandleValid( handle ) )
		return;
	
	t = &( btTracks[ handle ] );
	if( !t->Allocated )
		return;

	// Destroy Input AudioPlayer
	if( t->IObject != NULL ) {
		( *( t->IObject ) )->Destroy( t->IObject );
		t->IObject = NULL;
		t->IPlay = NULL;
		t->IBufferQueue = NULL;
		t->IMetaData = NULL;
	}

	// Destroy Output AudioPlayer
	if( t->OObject != NULL ) {
		( *( t->OObject ) )->Destroy( t->OObject);
		t->OObject = NULL;
		t->OPlay = NULL;
		t->OBufferQueue = NULL;
		t->OVolume = NULL;
	}

	// Free and reset the resource
	free( t->Resource );
	t->Resource = NULL;

	// Free the ring buffer
	for( i = 0; i < BT_RING_BUFFER_SIZE; i++ )
		free( t->RingBuffer[i] );
	t->RingRead = 0; t->PlayHead = 0;
	t->RingWrite = 0; t->Distance = 0;

	// Reset the flags
	t->Allocated = false;
	t->Initialized = false;
	t->Finishing = 0;
	t->Finished = false;
	
	// Kill the BTOVEN track
	BT_ERRCHECK( btoven_delete_track( t->Handle ) );
}

/* TODO:
// create audio recorder
jboolean Java_com_example_nativeaudio_NativeAudio_createAudioRecorder(JNIEnv* env, jclass clazz)
{
    SLresult result;

    // configure audio source
    SLDataLocator_IODevice loc_dev = {SL_DATALOCATOR_IODEVICE, SL_IODEVICE_AUDIOINPUT,
            SL_DEFAULTDEVICEID_AUDIOINPUT, NULL};
    SLDataSource audioSrc = {&loc_dev, NULL};

    // configure audio sink
    SLDataLocator_AndroidSimpleBufferQueue loc_bq = {SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE, 2};
    SLDataFormat_PCM format_pcm = {SL_DATAFORMAT_PCM, 1, SL_SAMPLINGRATE_16,
        SL_PCMSAMPLEFORMAT_FIXED_16, SL_PCMSAMPLEFORMAT_FIXED_16,
        SL_SPEAKER_FRONT_CENTER, SL_BYTEORDER_LITTLEENDIAN};
    SLDataSink audioSnk = {&loc_bq, &format_pcm};

    // create audio recorder
    // (requires the RECORD_AUDIO permission)
    const SLInterfaceID id[1] = {SL_IID_ANDROIDSIMPLEBUFFERQUEUE};
    const SLboolean req[1] = {SL_BOOLEAN_TRUE};
    result = (*engineEngine)->CreateAudioRecorder(engineEngine, &recorderObject, &audioSrc,
            &audioSnk, 1, id, req);
    if (SL_RESULT_SUCCESS != result) {
        return JNI_FALSE;
    }

    // realize the audio recorder
    result = (*recorderObject)->Realize(recorderObject, SL_BOOLEAN_FALSE);
    if (SL_RESULT_SUCCESS != result) {
        return JNI_FALSE;
    }

    // get the record interface
    result = (*recorderObject)->GetInterface(recorderObject, SL_IID_RECORD, &recorderRecord);
    assert(SL_RESULT_SUCCESS == result);

    // get the buffer queue interface
    result = (*recorderObject)->GetInterface(recorderObject, SL_IID_ANDROIDSIMPLEBUFFERQUEUE,
            &recorderBufferQueue);
    assert(SL_RESULT_SUCCESS == result);

    // register callback on the buffer queue
    result = (*recorderBufferQueue)->RegisterCallback(recorderBufferQueue, bqRecorderCallback,
            NULL);
    assert(SL_RESULT_SUCCESS == result);

    return JNI_TRUE;
}


// set the recording state for the audio recorder
void Java_com_example_nativeaudio_NativeAudio_startRecording(JNIEnv* env, jclass clazz)
{
    SLresult result;

    // in case already recording, stop recording and clear buffer queue
    result = (*recorderRecord)->SetRecordState(recorderRecord, SL_RECORDSTATE_STOPPED);
    assert(SL_RESULT_SUCCESS == result);
    result = (*recorderBufferQueue)->Clear(recorderBufferQueue);
    assert(SL_RESULT_SUCCESS == result);

    // the buffer is not valid for playback yet
    recorderSize = 0;

    // enqueue an empty buffer to be filled by the recorder
    // (for streaming recording, we would enqueue at least 2 empty buffers to start things off)
    result = (*recorderBufferQueue)->Enqueue(recorderBufferQueue, recorderBuffer,
            RECORDER_FRAMES * sizeof(short));
    // the most likely other result is SL_RESULT_BUFFER_INSUFFICIENT,
    // which for this code example would indicate a programming error
    assert(SL_RESULT_SUCCESS == result);

    // start recording
    result = (*recorderRecord)->SetRecordState(recorderRecord, SL_RECORDSTATE_RECORDING);
    assert(SL_RESULT_SUCCESS == result);

}
*/

// shut down the native audio system
void btShutdown()
{
	int i;

	// Destroy All Tracks
	for( i = 0; i < BT_MAX_NUM_TRACKS; i++ )
		btDestroyTrack( i );

	// Destroy Recorder
	/* TODO: Recorder
	if (recorderObject != NULL) {
		(*recorderObject)->Destroy(recorderObject);
		recorderObject = NULL;
		recorderRecord = NULL;
		recorderBufferQueue = NULL;
	}
	*/

	// Destroy output mix object, and invalidate all associated interfaces
	if( outputMixObject != NULL ) {
		( *outputMixObject )->Destroy( outputMixObject );
		outputMixObject = NULL;
	}

	// Destroy engine object, and invalidate all associated interfaces
	if( engineObject != NULL ) {
		( *engineObject )->Destroy( engineObject );
		engineObject = NULL;
		engineEngine = NULL;
		sync = NULL;
	}
	
	// Destroy the mutex
	pthread_mutex_destroy( &mutex );
	
	// Destroy btoven
	btoven_cleanup( &btoven_mobile_config );
}

void Lock() 
{
	if( sync )
		SL_ERRCHECK( ( *sync )->EnterCriticalSection( sync ) );
	else
		pthread_mutex_lock( &mutex );
}

void Unlock()
{ 
	if( sync )
		SL_ERRCHECK( ( *sync )->ExitCriticalSection( sync ) );
	else
		pthread_mutex_unlock( &mutex );
}
