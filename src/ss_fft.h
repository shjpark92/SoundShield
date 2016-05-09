#ifndef _SS_FFT_H_
#define _SS_FFT_H_

#include <stdio.h>
#include "fftw3.h"
#include "ss_mask_generation.h"

FrequencyBands SS_CalculateFFT(float inputFFTBuffer[], int samplesPerBuffer);

#endif // _SS_FFT_H_
