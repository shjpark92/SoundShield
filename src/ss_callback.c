#include <stdio.h>
#include "portaudio.h"
#include "ss_callback.h"
#include "ss_config.h"
#include "ss_fft.h"
#include "ss_mask_generation.h"
#include "ss_psd.h"

int SS_Callback(const void *inputBuffer, void *outputBuffer,
                       unsigned long framesPerBuffer,
                       const PaStreamCallbackTimeInfo* timeInfo,
                       PaStreamCallbackFlags statusFlags, void *userData ) {
    INPUT_SAMPLE *in;
    OUTPUT_SAMPLE *out;
    short inChannel = 0, outChannel = 0;
    short inputDone = 0x00, outputDone = 0x00;
    short finished = 0;
    unsigned int sample;
    SoundShieldData *config = (SoundShieldData *) userData;
    FrequencyBands magnitudeContainer;

    short numberOfInputChannels = config->numInputChannels;
    short numberofOutputChannels = config->numOutputChannels;

    float inputFFTBuffer[framesPerBuffer - 1][2];    
    float *p_FFTBuffer = inputFFTBuffer[0];    

    // TODO: Maybe consider moving this to ss_psd.c?
    float powerAccumuated[numberOfInputChannels];
    float sampleValue;

    float inValueToUI;
   
    // Printing timeInfo content
    //printf("%.2f\n", timeInfo->currentTime); 

    // This may get called with NULL inputBuffer during initial setup.
    if(inputBuffer == NULL) {
        return finished;
    }

    // Increment the number of callbacks occurred
    config->numCallbacks += 1;

    // Input analysis
    while(!inputDone) {
        in = ((INPUT_SAMPLE**)inputBuffer)[inChannel];        
        for(sample = 0; sample < framesPerBuffer; sample++) {
            
            SS_InputDataToUI(*in);

            // Populate the input buffer for the FFT
            inputFFTBuffer[sample][0] = *in;
            inputFFTBuffer[sample][1] = 0; 
            in += 1;
            
            // PSD/Mask Computation
            // interleaved
            sampleValue = in[inChannel + sample * numberOfInputChannels];
            
            // not interleaved
            //sampleValue = in[sample];
            
            // Square the sample value to obtain the power
            powerAccumuated[inChannel] += sampleValue * sampleValue;
        }
        
        // Finalize Mask Computation by taking the mean
        //powerAccumuated[inChannel] /= framesPerBuffer;
        
        if(inChannel < (config->numInputChannels - 1)) inChannel++;
        else inputDone ^= 0x01;
    }
    
    // Pass finalized mask computation to visualize PSD magnitude on the terminal
    //SS_PSDDraw(powerAccumuated, numberOfInputChannels);
    
    // Input graph dump
    //inValueToUI /= framesPerBuffer;
    //printf("inValueTOUI: %f\n", inValueToUI);
    //SS_InputDataToUI(inValueToUI);

    // FFT
    magnitudeContainer = SS_CalculateFFT(p_FFTBuffer, framesPerBuffer);

    // Output analysis
    while(!outputDone) {
        out = ((OUTPUT_SAMPLE**)outputBuffer)[outChannel];

        for(sample = 0; sample < framesPerBuffer; sample++) {
            // This line directly converts in to out (mic input goes to speaker output)
            //*out = (*in); in++;
            if(outChannel != 1) {
                *out = SS_SynthesizeRight(magnitudeContainer);
                out += 1;
            }
            else {
                *out = SS_SynthesizeLeft(magnitudeContainer);
                out += 1;
            }
        }

        if(outChannel < (config->numOutputChannels - 1)) outChannel++;
        else outputDone ^= 0x01;
    }

    if(LEFT_INDEX == 0x00) {
        readWavFiles();
        bufferToOutput ^= 0x01;
        bufferToFill ^= 0x01;
    } 
     
    return finished;
}

