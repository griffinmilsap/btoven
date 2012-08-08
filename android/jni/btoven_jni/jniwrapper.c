/*---------------------------------------------------------------------------------------
Filename: jniwrapper.c
Description: JNI wrapper for btoven_android

2012 Motalen
----------------------------------------------------------------------------------------*/
#include "jniwrapper.h"
#include "btoven_interface.h"
#include <android/log.h>

#define LOG( format, args... ) __android_log_print( ANDROID_LOG_DEBUG, "btoven_jni", format, ##args );
// FIXME: None of these track states are initialized
static btoven_state btTrackState[ BT_MAX_NUM_TRACKS ];

JNIEXPORT jint JNICALL JNI_OnLoad( JavaVM* vm, void* reserved )
{
	LOG( "JNI_OnLoad called" );
	// TODO: Register the methods
	return JNI_VERSION_1_6;
}

JNIEXPORT void JNICALL 
Java_com_motalen_btoven_Btoven_CreateEngine( JNIEnv* env, jclass cl )
{
	btCreateEngine();
}

JNIEXPORT jtrackhandle JNICALL 
Java_com_motalen_btoven_Btoven_CreateAssetTrack( JNIEnv* env, jclass cl, jobject assetManager, jstring asset )
{
	jtrackhandle ret = BT_INVALID_TRACKHANDLE;

	// Get necessary data from JNI
	const jbyte *utf8 = ( *env )->GetStringUTFChars( env, asset, NULL );
	AAssetManager* mgr = AAssetManager_fromJava( env, assetManager );

	// Call the interface
	if( mgr && utf8 ) ret = btCreateAssetTrack( mgr, utf8 );

	// Release data
	( *env )->ReleaseStringUTFChars( env, asset, utf8 );

	return ret;
}

JNIEXPORT jtrackhandle JNICALL 
Java_com_motalen_btoven_Btoven_CreateURITrack( JNIEnv* env, jclass cl, jstring uri )
{
	jtrackhandle ret = BT_INVALID_TRACKHANDLE;

	// Get necessary data from JNI
	const jbyte *utf8 = ( *env )->GetStringUTFChars( env, uri, NULL );

	// Call the interface
	if( utf8 ) ret = btCreateURITrack( utf8 );

	// Release data
	( *env )->ReleaseStringUTFChars( env, uri, utf8 );

	return ret;
}

JNIEXPORT void JNICALL 
Java_com_motalen_btoven_Btoven_PlayTrack( JNIEnv* env, jclass cl, jtrackhandle handle )
{
	btPlayTrack( handle );
}

JNIEXPORT void JNICALL 
Java_com_motalen_btoven_Btoven_PauseTrack( JNIEnv* env, jclass cl, jtrackhandle handle )
{
	btPauseTrack( handle );
}

JNIEXPORT void JNICALL 
Java_com_motalen_btoven_Btoven_StopTrack( JNIEnv* env, jclass cl, jtrackhandle handle )
{
	btStopTrack( handle );
}

// FIXME: No bounds checking in any of the following could result in unhelpful errors
// Consider making the "IsValidTrackhandle" function public to the btoven_android API
JNIEXPORT void JNICALL
Java_com_motalen_btoven_Btoven_ProcessTrack( JNIEnv* env, jclass cl, jtrackhandle handle )
{
	btTrackState[ handle ] = btProcessTrack( handle );
}

JNIEXPORT jint JNICALL 
Java_com_motalen_btoven_Btoven_GetBPM( JNIEnv* env, jclass cl, jtrackhandle handle )
{
	return btTrackState[ handle ].bpm;
}

JNIEXPORT jint JNICALL 
Java_com_motalen_btoven_Btoven_GetBPMConfidence( JNIEnv* env, jclass cl, jtrackhandle handle )
{
	return btTrackState[ handle ].bpm_confidence;
}

JNIEXPORT jint JNICALL 
Java_com_motalen_btoven_Btoven_GetPercentToNext( JNIEnv* env, jclass cl, jtrackhandle handle )
{
	return btTrackState[ handle ].percent_to_next;
}

JNIEXPORT jboolean JNICALL 
Java_com_motalen_btoven_Btoven_GetBeat( JNIEnv* env, jclass cl, jtrackhandle handle )
{
	return ( btTrackState[ handle ].beat ) ? JNI_TRUE : JNI_FALSE;
}

JNIEXPORT jboolean JNICALL 
Java_com_motalen_btoven_Btoven_GetTransientLow( JNIEnv* env, jclass cl, jtrackhandle handle )
{
	return ( btTrackState[ handle ].transient_low ) ? JNI_TRUE : JNI_FALSE;
}

JNIEXPORT jboolean JNICALL 
Java_com_motalen_btoven_Btoven_GetTransientMid( JNIEnv* env, jclass cl, jtrackhandle handle )
{
	return ( btTrackState[ handle ].transient_mid ) ? JNI_TRUE : JNI_FALSE;
}

JNIEXPORT jboolean JNICALL 
Java_com_motalen_btoven_Btoven_GetTransientHigh( JNIEnv* env, jclass cl, jtrackhandle handle )
{
	return ( btTrackState[ handle ].transient_high ) ? JNI_TRUE : JNI_FALSE;
}

JNIEXPORT jlong JNICALL 
Java_com_motalen_btoven_Btoven_GetCurrentIntensity( JNIEnv* env, jclass cl, jtrackhandle handle )
{
	return btTrackState[ handle ].cur_intensity;
}

JNIEXPORT jlong JNICALL 
Java_com_motalen_btoven_Btoven_GetSectionalIntensity( JNIEnv* env, jclass cl, jtrackhandle handle )
{
	return btTrackState[ handle ].sec_intensity;
}

JNIEXPORT jint JNICALL 
Java_com_motalen_btoven_Btoven_GetPitch( JNIEnv* env, jclass cl, jtrackhandle handle )
{
	return btTrackState[ handle ].pitch;
}

JNIEXPORT jint JNICALL 
Java_com_motalen_btoven_Btoven_GetPitch2( JNIEnv* env, jclass cl, jtrackhandle handle )
{
	return btTrackState[ handle ].pitch_2;
}

JNIEXPORT jint JNICALL 
Java_com_motalen_btoven_Btoven_GetPitch3( JNIEnv* env, jclass cl, jtrackhandle handle )
{
	return btTrackState[ handle ].pitch_3;
}

JNIEXPORT jboolean JNICALL 
Java_com_motalen_btoven_Btoven_IsFinished( JNIEnv* env, jclass cl, jtrackhandle handle )
{
	if( btIsFinished( handle ) )
		return JNI_TRUE;
	return JNI_FALSE;
}

JNIEXPORT jboolean JNICALL 
Java_com_motalen_btoven_Btoven_IsPlaying( JNIEnv* env, jclass cl, jtrackhandle handle )
{
	if( btIsPlaying( handle ) )
		return JNI_TRUE;
	return JNI_FALSE;
}

JNIEXPORT void JNICALL 
Java_com_motalen_btoven_Btoven_DestroyTrack( JNIEnv* env, jclass cl, jtrackhandle handle )
{
	btDestroyTrack( handle );
}

JNIEXPORT void JNICALL 
Java_com_motalen_btoven_Btoven_Shutdown( JNIEnv* env, jclass cl )
{
	btShutdown();
}

