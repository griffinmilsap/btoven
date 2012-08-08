// btoven
// Copyright Motalen 2011

#ifndef _WINDOW_H
#define _WINDOW_H

#include <stdint.h>
#include <btoven.h>
#include "audioformat.h"

typedef enum {
	BTOVEN_RECTANGLE_WINDOW = 0,
	BTOVEN_TRIANGLE_WINDOW
} btoven_window_type;

typedef struct {
	BTOVEN_MATHTYPE* windowed_data;
	float* window;
	size_t size;
	btoven_window_type type;
} btoven_window;

btoven_window* window_new( btoven_window_type type, size_t size );
void window_delete( btoven_window* w );
BTOVEN_MATHTYPE* window( const BTOVEN_STORETYPE* data, btoven_window* w );
	
#endif // _WINDOW_H
