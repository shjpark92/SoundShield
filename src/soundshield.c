#include <stdio.h>
#include "Python.h"
#include "portaudio.h"
#include "libwebsockets.h"
#include "sndfile.h"
#include "ss_config.h"
#include "ss_callback.h"
#include "ss_mask_generation.h"
#include "ss_server.h"

#define SAMPLE_RATE             44100
#define FRAMES_PER_CALLBACK     2048

/************* main *************
 * Function Input: None
 * Function Output: None
 * -------------------
 * PortAudio variables
 *
 * PaStream -- *stream
 * PaStreamParameters -- inputParameters
 * PaStreamParameters -- outputParameters
 * PaError -- err
 ********************************/
int main(void) {
    PaError err = paNoError;
    PaStream *stream;
    PaStreamParameters inputParameters, outputParameters;
    SoundShieldData CONFIG;
    SoundShieldData *config = &CONFIG;
    SoundShieldParam ssParam; 
    short c, bandIndex;

    /* Initialize PortAudio */
    printf("SoundShield Initializing...\n"); 
    fflush(stdout);
    err = Pa_Initialize();
    if(err != paNoError) goto error;

    printf("-----------------------------------------------\n");
    printf("I/O Configuration:\n");
    printf("Input format = %lu\n", INPUT_FORMAT);
    printf("Output format = %lu\n", OUTPUT_FORMAT);
    printf("Input device ID  = %d\n", INPUT_DEVICE);
    printf("Output device ID = %d\n", OUTPUT_DEVICE); 
    fflush(stdout);

    /* Call Python script to process mask */
    Py_Initialize();
    PyRun_SimpleString("import os\n"
                     "os.system('python3 mask-preprocess.py')\n");
    Py_Finalize();
   
    /* Grab the input configuration */ 
    ssParam = SS_InputInit(config);
    if(ssParam.error != 0) goto error;
    inputParameters = ssParam.parameter;
    
    /* Grab the output configuration */
    ssParam = SS_OutputInit(config);
    if(ssParam.error != 0) goto error;
    outputParameters = ssParam.parameter;
    
    /* Page in real-time data */
    if(SS_MaskInit() != 0) goto error;

    /* Give the user to choice to configure multiple i/o channels if available */
    SS_ConfigureIOChannels(&inputParameters.channelCount, &outputParameters.channelCount);
    printf("-----------------------------------------------\n" );
    printf("SoundShield Configuration:\n");
    fflush(stdout);
    
    /* Set the config values according to the values acquired from initialization */
    config->framesPerCallback = FRAMES_PER_CALLBACK;
    config->numInputChannels = inputParameters.channelCount;
    config->numOutputChannels = outputParameters.channelCount;
    config->numCallbacks = 0;

    printf("Input channels = %d\n", config->numInputChannels);
    printf("Output channels = %d\n", config->numOutputChannels);
    printf("Frames Per Callback = %d\n", config->framesPerCallback);
    printf("Press ENTER to begin SoundShield.\n(Press ENTER again to terminate SoundShield)\n");
    fflush(stdout);
    c = getchar();
    
    err = Pa_OpenStream(&stream, &inputParameters, &outputParameters, 
                        SAMPLE_RATE, config->framesPerCallback, paClipOff, 
                        SS_Callback, //NULL,  /* We are using blocking API. No callback is required */
                        config);//NULL); /* No need for userData/config since no there is no callback */
    if(err != paNoError) goto error;

    err = Pa_StartStream(stream);
    if(err != paNoError) goto error;
    
    if(openServer() == 0) {
        printf("Server - Success");
    } 
    else {
        printf("Server - Error");
    }

    fflush(stdout);
    c = getchar();

    /* Stop and Close PortAudio stream */
    printf("Stopping and Closing stream...\n");
    fflush(stdout);

    err = Pa_CloseStream(stream);
    if(err != paNoError) goto error;

    if(err == 1) {
        err = paNoError;
        goto done;
    }
    else if(err != paNoError) {
        goto error;
    }
		
    printf("number of callbacks = %d\n", config->numCallbacks);

done:
    for(bandIndex = 0; bandIndex < NUMBER_OF_BANDS; ++bandIndex) {
        sf_close(BAND[bandIndex]);
    }
    
    Pa_Terminate();
    printf("User has terminated SoundShield.\n");
    fflush(stdout);
    printf("Hit ENTER again to quit.\n");
    fflush(stdout);
    getchar();
    return 0;

error:
    if(stream) {
       Pa_AbortStream(stream);
       Pa_CloseStream(stream);
    }
    
    for(bandIndex = 0; bandIndex < NUMBER_OF_BANDS; ++bandIndex) {
        sf_close(BAND[bandIndex]);
    }

    Pa_Terminate();
    fprintf(stderr, "An error occured while using the Portaudio stream\n");
    fprintf(stderr, "Error number: %d\n", err);
    fprintf(stderr, "Error message: %s\n", Pa_GetErrorText(err));
    printf("Hit ENTER to quit.\n");  
    fflush(stdout);
    getchar();
    return -1;
}
