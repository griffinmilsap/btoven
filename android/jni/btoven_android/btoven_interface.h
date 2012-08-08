// btoven
// Motalen 2012
#ifndef BTOVEN_INTERFACE
#define BTOVEN_INTERFACE

#include <sys/types.h>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>

#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>

#include "btoven.h"

// Prevent Name Mangling
#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#define BT_BUFFER_SAMPLES ( 2048 )
#define BT_RING_BUFFER_SIZE ( 5 )
#define BT_MAX_NUM_TRACKS ( 4 )

#define BT_INVALID_TRACKHANDLE ( -1 )
typedef int btTrackHandle;
typedef short btPCM;

static int btBufferSizeBytes = BT_BUFFER_SAMPLES * sizeof( btPCM );

// Defines all necessary constructs for an Audio Track
typedef struct {
	// Output interfaces
	SLObjectItf                   OObject;
	SLPlayItf                     OPlay;
	SLAndroidSimpleBufferQueueItf OBufferQueue;
	SLVolumeItf                   OVolume;

	// Input interfaces
	SLObjectItf                   IObject;
	SLAndroidSimpleBufferQueueItf IBufferQueue;
	SLPlayItf                     IPlay;
	SLMetadataExtractionItf       IMetaData;
	
	// New Buffering System
	btPCM* RingBuffer[ BT_RING_BUFFER_SIZE ];
	int RingRead;
	int RingWrite;
	int PlayHead;
	int Distance;
	bool ProcessedBlock;

	// Other Data
	char* Resource;
	btoven_audioformat Format;
	btoven_trackhandle Handle;
	bool Initialized;
	bool Allocated;
	int Finishing;
	bool Finished;
} btTrack;

static btTrack btTracks[ BT_MAX_NUM_TRACKS ];

typedef struct {
	// Recorder Interfaces
	SLObjectItf                   IObject;
	SLRecordItf                   IRecord;
	SLAndroidSimpleBufferQueueItf IBufferQueue;
	
	// BTOVEN
	btoven_trackhandle Handle;
	
	// Buffers
	btPCM* PCMBuffer[2];
	int WriteBuf;
} btRecordTrack;

static btRecordTrack btRecord;

// Call CreateEngine once before calling any other functions.
void btCreateEngine();

// Create a track from a file in the asset manager
btTrackHandle btCreateAssetTrack( AAssetManager* assetManager, const char* asset );
btTrackHandle btCreateURITrack( const char* uri ); // MAY REQUIRE INTERNET PERMISSION

// Play/Pause/Stop a track
void btPlayTrack( btTrackHandle handle );
void btPauseTrack( btTrackHandle handle );
void btStopTrack( btTrackHandle handle );

// Is the track 
bool btIsFinished( btTrackHandle handle ); // Finished Playing?
bool btIsPlaying( btTrackHandle handle ); // Playing?

// Get the state from btoven
btoven_state btProcessTrack( btTrackHandle handle );
void btGetRecordState();

// Destroy a track, free resources for a new track
void btDestroyTrack( btTrackHandle handle );

// Call this when exiting the application -- destroys the whole engine
void btShutdown(); 

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // BTOVEN_INTERFACE

