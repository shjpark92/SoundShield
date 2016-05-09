#ifndef _SS_CALLBACK_H_
#define _SS_CALLBACK_H_

#include <stdio.h>
#include "portaudio.h"

int SS_Callback(const void *inputBuffer, void *outputBuffer,
                unsigned long framesPerBuffer,
                const PaStreamCallbackTimeInfo* timeInfo,
                PaStreamCallbackFlags statusFlags, void *userData );

#endif //_SS_CALLBACK_H_
