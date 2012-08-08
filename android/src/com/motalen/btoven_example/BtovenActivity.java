package com.motalen.btoven_example;

import com.motalen.btoven.Btoven;

import android.app.Activity;
import android.content.res.AssetManager;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;

public class BtovenActivity extends Activity {
	
	static AssetManager assetManager;
	public static final String TAG = "BtovenExample";
	private boolean active = false;
	int AssetTrack = Btoven.INVALID_TRACKHANDLE;
	int URITrack = Btoven.INVALID_TRACKHANDLE;
	
	@Override
    protected void onCreate(Bundle icicle) {
		super.onCreate( icicle );
		setContentView( R.layout.main );
		
		// Get the application assets
		assetManager = getAssets();
		
		// Activate the engine if necessary
		if( !active ) {
			Log.d( TAG, "Creating Engine!" );
			Btoven.CreateEngine();
			active = true;
		}
		
		// Create an asset player
		( ( Button )findViewById( R.id.assetButton) ).setOnClickListener( new OnClickListener() {
			public void onClick( View view ) {
				Log.d( TAG, "Asset Button Clicked!" );
				AssetTrack = Btoven.CreateAssetTrack( assetManager, "music.flac" );
				Btoven.PlayTrack( AssetTrack );
            }
		} );
		
		// Create a URI player
		( ( Button )findViewById( R.id.uriButton) ).setOnClickListener( new OnClickListener() {
			public void onClick( View view ) {
				Log.d( TAG, "URI Button Clicked!" );
				//URITrack = Btoven.CreateURITrack( "http://www.thoughtbubblemedia.com/stuff/music/Everything.mp3" );
				URITrack = Btoven.CreateURITrack( "http://www.stoben.com/stuff/music/Bang%20(Sub)Lime.flac" );
				Btoven.PlayTrack( URITrack );
            }
		} );
	}
	
	@Override
	protected void onResume() {
		super.onResume();
		Btoven.PlayTrack( AssetTrack );
		Btoven.PlayTrack( URITrack );
		Log.d( TAG, "Activity Resumed" );
	}
	
	@Override
	protected void onPause() {
		Log.d( TAG, "Activity Paused" );
		Btoven.PauseTrack( AssetTrack );
		Btoven.PauseTrack( URITrack );
		super.onPause();
	}
	
	@Override
    protected void onDestroy() {
        Log.d( TAG, "Activity Being Destroyed" );
        Btoven.Shutdown();
        active = false;
        super.onDestroy();
    }
}
