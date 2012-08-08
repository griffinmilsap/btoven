/*---------------------------------------------------------------------------------------
Filename: jniwrapper.h
Description: JNI wrapper for btoven

Copyright 2012 Motalen
----------------------------------------------------------------------------------------*/
#ifndef JNIWRAPPER_H
#define JNIWRAPPER_H

#include <jni.h>
#include <android/log.h>

#include "btoven.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

typedef jint jtrackhandle;

JNIEXPORT jint JNICALL JNI_OnLoad( JavaVM* vm, void* reserved );

// LEVEL 1 ---------
// Call CreateEngine once before calling any other functions. (PROCEED TO LEVEL 2)
JNIEXPORT void JNICALL Java_com_motalen_btoven_Btoven_CreateEngine( JNIEnv* env, jclass cl );

// LEVEL 2 ---------
// Create a track
JNIEXPORT jtrackhandle JNICALL Java_com_motalen_btoven_Btoven_CreateAssetTrack( JNIEnv* env, jclass cl, jobject assetManager, jstring asset );
JNIEXPORT jtrackhandle JNICALL Java_com_motalen_btoven_Btoven_CreateURITrack( JNIEnv* env, jclass cl, jstring uri );

// Play/Pause/Stop a track
JNIEXPORT void JNICALL Java_com_motalen_btoven_Btoven_PlayTrack( JNIEnv* env, jclass cl, jtrackhandle handle ); // Ensure track is initialized first!
JNIEXPORT void JNICALL Java_com_motalen_btoven_Btoven_PauseTrack( JNIEnv* env, jclass cl, jtrackhandle handle );
JNIEXPORT void JNICALL Java_com_motalen_btoven_Btoven_StopTrack( JNIEnv* env, jclass cl, jtrackhandle handle );

// Is the track 
JNIEXPORT jboolean JNICALL Java_com_motalen_btoven_Btoven_IsFinished( JNIEnv* env, jclass cl, jtrackhandle handle ); // Finished Playing?
JNIEXPORT jboolean JNICALL Java_com_motalen_btoven_Btoven_IsPlaying( JNIEnv* env, jclass cl, jtrackhandle handle ); // Playing?

// Get the state from btoven
JNIEXPORT void JNICALL Java_com_motalen_btoven_Btoven_ProcessTrack( JNIEnv* env, jclass cl, jtrackhandle handle );
JNIEXPORT jint JNICALL Java_com_motalen_btoven_Btoven_GetBPM( JNIEnv* env, jclass cl, jtrackhandle handle ); // 0 - 256
JNIEXPORT jint JNICALL Java_com_motalen_btoven_Btoven_GetBPMConfidence( JNIEnv* env, jclass cl, jtrackhandle handle ); // 0 - 256
JNIEXPORT jint JNICALL Java_com_motalen_btoven_Btoven_GetPercentToNext( JNIEnv* env, jclass cl, jtrackhandle handle ); // 0 - 256
JNIEXPORT jboolean JNICALL Java_com_motalen_btoven_Btoven_GetBeat( JNIEnv* env, jclass cl, jtrackhandle handle );
JNIEXPORT jboolean JNICALL Java_com_motalen_btoven_Btoven_GetTransientLow( JNIEnv* env, jclass cl, jtrackhandle handle );
JNIEXPORT jboolean JNICALL Java_com_motalen_btoven_Btoven_GetTransientMid( JNIEnv* env, jclass cl, jtrackhandle handle );
JNIEXPORT jboolean JNICALL Java_com_motalen_btoven_Btoven_GetTransientHigh( JNIEnv* env, jclass cl, jtrackhandle handle );
JNIEXPORT jlong JNICALL Java_com_motalen_btoven_Btoven_GetCurrentIntensity( JNIEnv* env, jclass cl, jtrackhandle handle ); // 0 - 65535^2
JNIEXPORT jlong JNICALL Java_com_motalen_btoven_Btoven_GetSectionalIntensity( JNIEnv* env, jclass cl, jtrackhandle handle ); // 0  - 65535^2
JNIEXPORT jint JNICALL Java_com_motalen_btoven_Btoven_GetPitch( JNIEnv* env, jclass cl, jtrackhandle handle ); // 0 - 256 -- relative...
JNIEXPORT jint JNICALL Java_com_motalen_btoven_Btoven_GetPitch2( JNIEnv* env, jclass cl, jtrackhandle handle ); // 0 - 256
JNIEXPORT jint JNICALL Java_com_motalen_btoven_Btoven_GetPitch3( JNIEnv* env, jclass cl, jtrackhandle handle ); // 0 - 256

// Destroy a track, free resources for a new track
JNIEXPORT void JNICALL Java_com_motalen_btoven_Btoven_DestroyTrack( JNIEnv* env, jclass cl, jtrackhandle handle );

// Call this when exiting the application -- destroys the whole engine (PROCEED TO LEVEL 1)
JNIEXPORT void JNICALL Java_com_motalen_btoven_Btoven_Shutdown( JNIEnv* env, jclass cl ); 

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // JNIWRAPPER_H
