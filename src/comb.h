// btoven
// Copyright Motalen 2011

#ifndef _COMB_H
#define _COMB_H

#include <math.h>
#include <stdlib.h>
#include <stdint.h>
#include "audioformat.h"

typedef struct {
	double alpha;
	uint32_t T;
	uint8_t bpm;
	BTOVEN_MATHTYPE* data;
	BTOVEN_MATHTYPE* acc_data;
	size_t size;
	BTOVEN_MATHTYPE max_energy;
} comb_filter;

void comb( BTOVEN_MATHTYPE* input, comb_filter* c );
void comb_reset( comb_filter* c );
comb_filter* comb_new( uint8_t bpm, double half_energy_time, uint32_t rate, size_t size );
void comb_delete( comb_filter* c );

#endif // _COMB_H
