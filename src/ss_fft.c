#include <stdio.h>
#include <math.h>
#include "fftw3.h"
#include "ss_fft.h"
#include "ss_mask_generation.h"

#define PEAK_PICKING_BANDS       8 
#define PREVIOUS_SAMPLE_COUNT   32

float previousInputBuffer[PREVIOUS_SAMPLE_COUNT][NUMBER_OF_BANDS] = { 0.0 };
short previousSample = 31;

/* SAMPLES_TO_ALLOCATE = 1023 total samples */
const short SAMPLES_TO_ALLOCATE[NUMBER_OF_BANDS] = { 1, 2, 4, 7, 15, 30, 59, 119, 238, 548 };
const short SAMPLES_TO_ALLOCATE_TEST[NUMBER_OF_BANDS] = { 1, 2, 4, 7, 15, 30, 59, 119, 25, 50 };

FrequencyBands SS_CalculateFFT(float inputFFTBuffer[], int samplesPerBuffer) {
    fftwf_plan plan;
    short sample = 0;
    short bandIndex = 0, allocateIndex = 0, 
          previousIndex = 0, sampleIndex = 1;
    float fftValue, previousSum = 0.0;
    float currentRawMagnitude = 0.0;
    float currentFrequency = 0.0;
    float currentPowerDensity = 0.0;
    float resolution = 44100.0 / samplesPerBuffer;
    float fftOutput[samplesPerBuffer - 1][2];
    float threeTapMagnitude[NUMBER_OF_BANDS], threeTapPSD[NUMBER_OF_BANDS];

    FrequencyBands freqBands;
    freqBands = SS_FrequencyBandInit(freqBands);
   
    for(sample = 0; sample < samplesPerBuffer; ++sample) {
        fftOutput[sample][0] = 0.0;
        fftOutput[sample][1] = 0.0;
    }
 
    plan = fftwf_plan_dft_1d(samplesPerBuffer, (fftwf_complex*)inputFFTBuffer, 
                            (fftwf_complex*)fftOutput, FFTW_FORWARD, FFTW_ESTIMATE);
    fftwf_execute(plan);

    /* Peak peaking for the first 7 bins */
    for(bandIndex = 0; bandIndex < PEAK_PICKING_BANDS; ++bandIndex) {
        for(allocateIndex = 0; allocateIndex < SAMPLES_TO_ALLOCATE[bandIndex]; ++allocateIndex) {
            fftValue = (fftOutput[sampleIndex][0] * fftOutput[sampleIndex][0])
                        + (fftOutput[sampleIndex][1] * fftOutput[sampleIndex][1]);
            
            currentRawMagnitude = sqrt(fftValue);
            currentPowerDensity = fftValue / samplesPerBuffer;
            currentFrequency = sampleIndex * resolution;
            
            if(currentRawMagnitude > freqBands.peakMagnitude[bandIndex]) {
                freqBands.peakMagnitude[bandIndex] = currentRawMagnitude;
                freqBands.peakPowerDensity[bandIndex] = currentPowerDensity;
                freqBands.peakFrequency[bandIndex] = currentFrequency;
            }
            sampleIndex++;
        }

        /* Store to 3 tap delay line */
        threeTapMagnitude[bandIndex] = freqBands.peakMagnitude[bandIndex];
        threeTapPSD[bandIndex] = freqBands.peakPowerDensity[bandIndex];
    }

    /* Accumulate and average the last 2 bins */
    for(bandIndex = PEAK_PICKING_BANDS; bandIndex < NUMBER_OF_BANDS; ++bandIndex) {
        for(allocateIndex = 0; allocateIndex < SAMPLES_TO_ALLOCATE[bandIndex]; ++allocateIndex) {
            fftValue = (fftOutput[sampleIndex][0] * fftOutput[sampleIndex][0])
                        + (fftOutput[sampleIndex][1] * fftOutput[sampleIndex][1]);

            freqBands.peakMagnitude[bandIndex] += sqrt(fftValue);
            freqBands.peakPowerDensity[bandIndex] += fftValue / samplesPerBuffer;
            freqBands.peakFrequency[bandIndex] += sampleIndex * resolution;
            sampleIndex++;
        }
        freqBands.peakFrequency[bandIndex] /= SAMPLES_TO_ALLOCATE_TEST[bandIndex];
        freqBands.peakPowerDensity[bandIndex] /= SAMPLES_TO_ALLOCATE_TEST[bandIndex];
        freqBands.peakMagnitude[bandIndex] /= SAMPLES_TO_ALLOCATE_TEST[bandIndex];

        /* Store to 3 tap delay line */
        threeTapMagnitude[bandIndex] = freqBands.peakMagnitude[bandIndex];
        threeTapPSD[bandIndex] = freqBands.peakPowerDensity[bandIndex];
    }

    /* Compute 3 tap delay */
    for(bandIndex = 0; bandIndex < NUMBER_OF_BANDS; ++bandIndex) {
        if(bandIndex == 0){
            freqBands.peakMagnitude[bandIndex] += (threeTapMagnitude[bandIndex + 1] * 0.5);
            freqBands.peakMagnitude[bandIndex] *= 0.6666;
            freqBands.peakPowerDensity[bandIndex] += threeTapPSD[bandIndex + 1];
            freqBands.peakPowerDensity[bandIndex] *= 0.5;
        }
        else if(bandIndex == 9){
            freqBands.peakMagnitude[bandIndex] += (threeTapMagnitude[bandIndex - 1] * 0.5);
            freqBands.peakMagnitude[bandIndex] *= 0.6666;
            freqBands.peakPowerDensity[bandIndex] += threeTapPSD[bandIndex - 1];
            freqBands.peakPowerDensity[bandIndex] *= 0.5;
        }
        else{
            freqBands.peakMagnitude[bandIndex] += (threeTapMagnitude[bandIndex - 1] * 0.5)
                                                    + (threeTapMagnitude[bandIndex + 1] * 0.5);
            freqBands.peakMagnitude[bandIndex] *= 0.5;
            freqBands.peakPowerDensity[bandIndex] += threeTapPSD[bandIndex - 1]
                                                    + threeTapPSD[bandIndex + 1];
            freqBands.peakMagnitude[bandIndex] *= 0.5;
        }

        /* Store to previous input/sample buffer */
        previousInputBuffer[0][bandIndex] = freqBands.peakMagnitude[bandIndex];

        /* Average the bands */
        previousSum = 0.0;
        previousIndex = 0;

        do {
            /* Accumulate */
            previousSum += previousInputBuffer[previousIndex][bandIndex];
            ++previousIndex;
        } while(previousIndex < PREVIOUS_SAMPLE_COUNT);

        previousSum /= PREVIOUS_SAMPLE_COUNT;
        freqBands.peakMagnitude[bandIndex] = previousSum;

        /* Shift linear buffer */
        --previousIndex;
        do {
            previousInputBuffer[previousIndex][bandIndex] = previousInputBuffer[previousIndex - 1][bandIndex];
            --previousIndex;
        } while(previousIndex > 0);
    }  

    //previousSample = previousSample < 1 ? 31 : --previousSample;

    /*
    printf("Peak Magnitude\n");
    for(bandIndex = 0; bandIndex < NUMBER_OF_BANDS; ++bandIndex) {
        printf("Band %i: %g,%g Hz\n", bandIndex + 1, freqBands.peakMagnitude[bandIndex], 
                                        freqBands.peakFrequency[bandIndex]);
    }
    */

    fftwf_destroy_plan(plan);
    
    return freqBands;
}
