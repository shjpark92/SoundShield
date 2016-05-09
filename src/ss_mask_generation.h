#ifndef _SS_MASK_GENERATION_H_
#define _SS_MASK_GENERATION_H_

#include <stdio.h>
#include "sndfile.h"

#define NUMBER_OF_BANDS 10
#define ONE_OVER_BANDS 	0.1
//#define BUFFER_LEN 	  21600000
//#define BUFFER_LEN	  41943040 // 5 MB
//#define BUFFER_LEN 	  8192
#define BUFFER_LEN 		4096
#define PING    		1
#define PONG    		0

SNDFILE *BAND[NUMBER_OF_BANDS];
SF_INFO sfinfo;

short LEFT_INDEX;
short RIGHT_INDEX;
short bufferToFill, bufferToOutput;

float PING_INTERLEAVED[BUFFER_LEN * NUMBER_OF_BANDS];
float PONG_INTERLEAVED[BUFFER_LEN * NUMBER_OF_BANDS];

typedef struct FrequencyBands {
    float peakRawMagnitude[NUMBER_OF_BANDS];
    float bandScale[NUMBER_OF_BANDS];
    float peakFrequency[NUMBER_OF_BANDS];
    float peakMagnitude[NUMBER_OF_BANDS];
    float peakPowerDensity[NUMBER_OF_BANDS];
} FrequencyBands;

int SS_MaskInit();

void readWavFiles();

void SS_InputDataToUI(float inValueToUI);

float SS_SynthesizeRight(FrequencyBands magnitudeContainer);

float SS_SynthesizeLeft(FrequencyBands magnitudeContainer);

FrequencyBands SS_FrequencyBandInit(FrequencyBands freqBinContainer);

#endif // _SS_MASK_GENERATION_H_
