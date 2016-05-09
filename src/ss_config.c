#include <stdio.h>
#include "portaudio.h"
#include "ss_config.h"

SoundShieldParam SS_InputInit(SoundShieldData *config) {
    SoundShieldParam targetParameter;
    PaStreamParameters inputParameters;
    const PaDeviceInfo *inputDeviceInfo;
    targetParameter.error = false;
    
    /* Set the parameter to the default INPUT_DEVICE */
    printf("\nInitializing Input...");
    inputParameters.device = INPUT_DEVICE;
    if (inputParameters.device == paNoDevice) {
        fprintf(stderr,"\nError: No default input device.\n");
        goto error;
    }
    printf("Done!\n");
    
    /* A default input device exists */
    inputDeviceInfo = Pa_GetDeviceInfo(inputParameters.device);
    printf("Input sampling rate: %.2f \n", inputDeviceInfo->defaultSampleRate);
    
    /* Define remaining parameters */
    inputParameters.channelCount = inputDeviceInfo->maxInputChannels;
    inputParameters.sampleFormat = INPUT_FORMAT | paNonInterleaved;
    //inputParameters.sampleFormat = INPUT_FORMAT;
    //inputParameters.suggestedLatency = Pa_GetDeviceInfo(inputParameters.device)->defaultHighInputLatency;
    inputParameters.suggestedLatency = Pa_GetDeviceInfo(inputParameters.device)->defaultLowInputLatency;
    inputParameters.hostApiSpecificStreamInfo = NULL;
    targetParameter.parameter = inputParameters;
    
    return targetParameter;
    
error:
    targetParameter.error = true;
    return targetParameter;
}

SoundShieldParam SS_OutputInit(SoundShieldData *config) {
    SoundShieldParam targetParameter;
    PaStreamParameters outputParameters;
    const PaDeviceInfo *outputDeviceInfo;
    targetParameter.error = false;
    
    /* Set the parameter to the default OUTPUT_DEVICE */
    printf("\nInitializing Output...");
    outputParameters.device = OUTPUT_DEVICE;
    if (outputParameters.device == paNoDevice) {
        fprintf(stderr,"\nError: No default output device.\n");
        goto error;
    }
    printf("Done!\n");
    
    /* A default output device exists */
    outputDeviceInfo = Pa_GetDeviceInfo(outputParameters.device);
    printf("Output sampling rate: %.2f \n", outputDeviceInfo->defaultSampleRate);
    
    /* Define remaining parameters */
    outputParameters.channelCount = outputDeviceInfo->maxOutputChannels;
    outputParameters.sampleFormat = OUTPUT_FORMAT | paNonInterleaved;
    //outputParameters.sampleFormat = OUTPUT_FORMAT;
    //outputParameters.suggestedLatency = Pa_GetDeviceInfo(outputParameters.device)->defaultHighOutputLatency;
    outputParameters.suggestedLatency = Pa_GetDeviceInfo(outputParameters.device)->defaultLowOutputLatency;
    outputParameters.hostApiSpecificStreamInfo = NULL;
    targetParameter.parameter = outputParameters;
    
    return targetParameter;
    
error:
    targetParameter.error = true;
    return targetParameter;
}

// TODO: Later offer device ID selection via GUI
void SS_ConfigureIOChannels(int *inputChannelCount, int *outputChannelCount) {
    printf("\nOptimizing channel configuration...");
    
    *inputChannelCount = *inputChannelCount > 1 ? 2 : 1;
    *outputChannelCount = *outputChannelCount > 1 ? 2 : 1;
    printf("Done!\n");

    if(*inputChannelCount == 2 && *outputChannelCount == 2) {
        printf("Surround sound has been enabled!\n");
    } 
    else {
        printf("Surround sound is NOT enabled!\n");
    }
}

