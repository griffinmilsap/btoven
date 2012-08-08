// btoven
// Motalen 2011
#ifndef _PROCESSING_H
#define _PROCESSING_H

// Uncomment these to determine the optimization to be used
//#define BTOVEN_OPTIMIZATION 1  // Software Implementation

void btoven_proc_envelope( btoven_track* t );
void btoven_proc_bpm( btoven_track* t );
void btoven_proc_pitch( btoven_track* t );

#endif // _PROCESSING_H
