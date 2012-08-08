/*---------------------------------------------------------------------------------------
Filename: BtovenTrack.java
Description: A Definition of a processing thread for use with the btoven JNI interface

Copyright 2012 Motalen
----------------------------------------------------------------------------------------*/
package com.motalen.btoven;

import android.content.res.AssetManager;
import android.util.Log;

// TODO: Make BtovenTrack a ProcessThread
public class BtovenTrack
{
	private int mTrackHandle = Btoven.INVALID_TRACKHANDLE;
	private Thread mThread = null;
	
	// Constructors
	
	// Create a new track from a file:// or http:// uri
	public BtovenTrack( String uri ) 
	{
		mTrackHandle = Btoven.CreateURITrack( uri );
	}
	
	// Create a new track from a file packed with the APK in the assets folder
	public BtovenTrack( AssetManager mgr, String filename )
	{
		mTrackHandle = Btoven.CreateAssetTrack( mgr, filename );
	}
	
	// Getters/Setters
	
	// Returns true if the track is currently playing
	public boolean Playing() { return Btoven.IsPlaying( mTrackHandle ); }
	
	// Returns true if the track has played all the way through and is no longer playing
	public boolean Finished() { return Btoven.IsFinished( mTrackHandle ); }
	
	// Get the tempo in Beats per Minute
	public int BPM() { return Btoven.GetBPM( mTrackHandle ); }
	
	// Get Btoven's confidence in the calculated tempo.
	// 0 = Not confident, 255 = Super confident
	public int BPMConfidence() { return Btoven.GetBPMConfidence( mTrackHandle ); } 
	
	// Get the percentage to the next predicted beat
	// 0 = 0 percent to next beat, 255 = 100% to next beat 
	public int PercentToNext() { return Btoven.GetPercentToNext( mTrackHandle ); } 
	
	// Get the current intensity (squared) in the track
	// 0 = Silent, 4294836225 = Most possible energy
	public long CurrentIntensity() { return Btoven.GetCurrentIntensity( mTrackHandle ); }
	
	// Get the averaged intensity over the last 5 seconds of the track
	// 0 = Silent, 4294836225 = Most possible energy
	public long SectionalIntensity() { return Btoven.GetSectionalIntensity( mTrackHandle ); }
	
	// Get the most dominant pitch in the audio
	// 0 = 60 Hz, 255 = 2000 Hz - Linearly distributed
	public int Pitch() { return Btoven.GetPitch( mTrackHandle ); } 
	
	// Get the second most dominant pitch in the audio
	// 0 = 60 Hz, 255 = 2000 Hz - Linearly distributed
	public int Pitch2() { return Btoven.GetPitch2( mTrackHandle ); } 
	
	// Get the third most dominant pitch in the audio
	// 0 = 60 Hz, 255 = 2000 Hz - Linearly distributed
	public int Pitch3() { return Btoven.GetPitch3( mTrackHandle ); }
	
	// Methods
	
	// Play the track
	public void Play()
	{
		if( !this.Playing() )
		{
			mThread = new Thread( new Runnable() {
				@Override
				public void run() 
				{
					while( Btoven.IsPlaying( mTrackHandle ) )
					{
						Btoven.ProcessTrack( mTrackHandle );
						if( Btoven.GetBeat( mTrackHandle ) ) onBeat();
						boolean low = Btoven.GetTransientLow( mTrackHandle );
						boolean mid = Btoven.GetTransientMid( mTrackHandle );
						boolean high = Btoven.GetTransientHigh( mTrackHandle );
						if( low || mid || high ) onTransient( low, mid, high );
					}
					if( Btoven.IsFinished( mTrackHandle ) ) onFinished();
				} 
			} );
			Btoven.PlayTrack( mTrackHandle );
			mThread.start();
		}
	}
	
	// Pause the track
	public void Pause()
	{
		if( this.Playing() )
		{
			Btoven.PauseTrack( mTrackHandle );
			try {
				while( mThread.isAlive() )
					Thread.sleep( 10 );
			} catch( InterruptedException e ) {
				e.printStackTrace();
			}
		}
	}
	
	// Stop the track
	public void Stop() 
	{
		if( this.Playing() )
		{
			Btoven.StopTrack( mTrackHandle );
			try {
				while( mThread.isAlive() )
					Thread.sleep( 10 );
			} catch( InterruptedException e ) {
				e.printStackTrace();
			}
		}
	}
	
	public void onFinished() { };
    public void onBeat() { }
    public void onTransient( boolean low, boolean medium, boolean high ) { }

}