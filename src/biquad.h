// btoven
// Simple implementation of Biquad filters -- Tom St Denis

#ifndef BIQUAD_H
#define BIQUAD_H

#include <math.h>
#include <stdlib.h>
#include <stdint.h>
#include "audioformat.h"

#ifndef M_LN2
#define M_LN2 0.69314718055994530942
#endif

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

typedef BTOVEN_STORETYPE smp_type;

/* this holds the data required to update samples thru a filter */
typedef struct {
	double a0, a1, a2, a3, a4;
	smp_type x1, x2, y1, y2;
} biquad_filter;

extern smp_type biquad(smp_type sample, biquad_filter * b);
extern biquad_filter *biquad_new(int type, double dbGain, // gain of filter
                                 double freq,             // center frequency
                                 double srate,            // sampling rate
                                 double bandwidth);       // bandwidth in octaves

// filter types
enum {
	LPF, // low pass filter
	HPF, // High pass filter
	BPF, // band pass filter
	NOTCH, // Notch Filter
	PEQ, // Peaking band EQ filter
	LSH, // Low shelf filter
	HSH // High shelf filter
};

#endif // BIQUAD_H
