/*---------------------------------------------------------------------------------------
Filename: Btoven.java
Description: Java interface to the btoven library in native code

Copyright 2012 Motalen
----------------------------------------------------------------------------------------*/
package com.motalen.btoven;

import android.content.res.AssetManager;

public class Btoven
{
	// Load the library at once
	static { 
		System.loadLibrary( "btoven" );
		System.loadLibrary( "btoven_android" );
		System.loadLibrary( "btoven_android_jni" );
	}
	
	public static final int INVALID_TRACKHANDLE = -1;

	// Create the engine
	// Note: This MUST be called before tracks can be instantiated
	public native static void CreateEngine();

	// Create a track, return the handle to it
	public native static int CreateAssetTrack( AssetManager assetManager, String asset );
	public native static int CreateURITrack( String uri );

	// Play/Pause/Stop a track
	public native static void PlayTrack( int handle ); // Ensure track is initialized first!
	public native static void PauseTrack( int handle );
	public native static void StopTrack( int handle );

	// Query Track State
	public native static boolean IsFinished( int handle ); // Finished Playing?
	public native static boolean IsPlaying( int handle ); // Playing?

	// Destroy a track, free resources for a new track
	public native static void DestroyTrack( int handle );

	// Call this to release the OpenSL engine and destroy all tracks
	// Note: This MUST be called to release the OpenSL engine
	public native static void Shutdown(); 
	
	// Analysis retrieval
	public native static int GetBPM( int handle );
	public native static int GetBPMConfidence( int handle ); 
	public native static int GetPercentToNext( int handle ); 
	public native static boolean GetBeat( int handle );
	public native static boolean GetTransientLow( int handle );
	public native static boolean GetTransientMid( int handle );
	public native static boolean GetTransientHigh( int handle );
	public native static long GetCurrentIntensity( int handle ); 
	public native static long GetSectionalIntensity( int handle );
	public native static int GetPitch( int handle ); 
	public native static int GetPitch2( int handle ); 
	public native static int GetPitch3( int handle );
	
	// Declare an interface for processing tracks
	public native static void ProcessTrack( int handle ); // Get track state, initialize processing
}
