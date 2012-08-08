// btoven
// Copyright Motalen 2011

#include <stdlib.h>
#include "window.h"

float* create_rectangle_window( size_t size );
float* create_triangle_window( size_t size );

btoven_window* window_new( btoven_window_type type, size_t size )
{
	// Allocate the window
	btoven_window* w = ( btoven_window* )malloc( sizeof( btoven_window ) );
	if( w == NULL )
		return NULL;
		
	// Initialize window parameters
	w->type = type;
	w->size = size;
	w->windowed_data = ( BTOVEN_MATHTYPE* )malloc( size * sizeof( BTOVEN_MATHTYPE ) );
	
	// Allocate and create the window function
	switch( type )
	{
		case BTOVEN_TRIANGLE_WINDOW:
			w->window = create_triangle_window( w->size );
			break;
		case BTOVEN_RECTANGLE_WINDOW:
		default:
			w->window = create_rectangle_window( w->size );
			break;
	}
	
	return w;
}

// Delete the window that we allocated
void window_delete( btoven_window* w )
{
	if( w->window ) free( w->window );
	if( w->windowed_data ) free( w->windowed_data );
	if( w ) free( w );
}

// Window some data
BTOVEN_MATHTYPE* window( const BTOVEN_STORETYPE* data, btoven_window* w )
{
	size_t i;
	for( i = 0; i < w->size; i++ )
		w->windowed_data[i] = ( BTOVEN_MATHTYPE )( data[i] * w->window[i] );
	return w->windowed_data;
}

// Allocate and fill a rectangular window
float* create_rectangle_window( size_t size )
{
	size_t i;
	float* win = ( float* )malloc( size * sizeof( float ) );
	if( win == NULL )
		return NULL;
	
	for( i = 0; i < size; i++ )
		win[i] = 1.0f;
		
	return win;
}

// Allocate and fill a triangular window
float* create_triangle_window( size_t size )
{
	size_t i;
	float* win = ( float* )malloc( size * sizeof( float ) );
	if( win == NULL )
		return NULL;
	
	for( i = 0; i < size / 2; i++ )
		win[i] = i / ( float )( size / 2 ); 
	for( i = size / 2; i < size; i++ )
		win[i] = ( size - i ) / ( float )( size / 2 );
	
	return win;
}

