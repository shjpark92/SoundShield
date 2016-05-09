#ifndef _SS_CONFIG_H_
#define _SS_CONFIG_H_

#include <stdio.h>
#include <stdbool.h>
#include "portaudio.h"

typedef struct SoundShieldData {
    int numInputChannels;
    int numOutputChannels;
    int framesPerCallback;
    int numCallbacks;
} SoundShieldData;

typedef struct SoundShieldParam {
    PaStreamParameters parameter;
    bool error;
} SoundShieldParam;

#define USE_FLOAT_INPUT        (1)
#define USE_FLOAT_OUTPUT       (1)

#if USE_FLOAT_INPUT
#define INPUT_FORMAT  paFloat32
typedef float INPUT_SAMPLE;
#else
#define INPUT_FORMAT  paInt16
typedef short INPUT_SAMPLE;
#endif

#if USE_FLOAT_OUTPUT
#define OUTPUT_FORMAT  paFloat32
typedef float OUTPUT_SAMPLE;
#else
#define OUTPUT_FORMAT  paInt16
typedef short OUTPUT_SAMPLE;
#endif

#define INPUT_DEVICE           (Pa_GetDefaultInputDevice())
#define OUTPUT_DEVICE          (Pa_GetDefaultOutputDevice())

SoundShieldParam SS_InputInit(SoundShieldData *config);

SoundShieldParam SS_OutputInit(SoundShieldData *config);

void SS_ConfigureIOChannels(int *inputChannelCount, int *outputChannelCount);

#endif // _SS_CONFIG_H_