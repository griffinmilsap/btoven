// btoven
// Copyright Motalen 2011

#include <string.h>
#include "comb.h"

// Computes a comb filter on an input dataset
void comb( BTOVEN_MATHTYPE* input, comb_filter* c )
{
	size_t i;
	for( i = c->size - c->T - 1; i > 0; i-- )
	{
		c->data[i] = ( BTOVEN_MATHTYPE )( c->alpha * ( double )c->data[ i + c->T ] + ( ( 1.0f - c->alpha ) * ( double )input[i] ) );
		c->acc_data[i] += c->data[i];
		if( c->max_energy < c->acc_data[i] && i < ( 2 * c->T ) )
			c->max_energy = c->acc_data[i];
	}
}

void comb_reset( comb_filter* c )
{
	c->max_energy = 0;
	memset( c->acc_data, 0, sizeof( BTOVEN_MATHTYPE ) * c->size );
}

// Sets up a new comb filter
comb_filter* comb_new( uint8_t bpm, double half_energy_time, uint32_t rate, size_t size )
{
	comb_filter *c;

	// Allocate memory as necessary
	c = ( comb_filter* )malloc( sizeof( comb_filter ) );
	if ( c == NULL )
		return NULL;
		
	c->data = ( BTOVEN_MATHTYPE* )calloc( size, sizeof( BTOVEN_MATHTYPE ) );
	c->acc_data = ( BTOVEN_MATHTYPE* )malloc( sizeof( BTOVEN_MATHTYPE ) * size );
	
	c->bpm = bpm;
	c->size= size;
	c->T = ( uint32_t )( ( 60.0f / ( double )bpm ) * ( double )rate );
	c->alpha = pow( 0.5f, ( double )c->T / ( rate * half_energy_time ) );
	comb_reset( c );

	return c;
}

void comb_delete( comb_filter* c )
{
	if( c->data ) free( c->data );
	if( c->acc_data ) free( c->acc_data );
	if( c ) free( c );
}

