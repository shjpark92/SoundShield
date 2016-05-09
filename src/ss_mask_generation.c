#include <stdio.h>
#include <string.h>
#include "ss_api.h"
#include "ss_mask_generation.h"

#define MASK_RESOURCE_PATH      "../mask/wav/filtered/currently_playing/"
#define MASK_FILE_EXTENSION     ".wav"
#define FILE_NAME_LENGTH        3
#define RESOURCE_PATH_LENGTH    50

const char MASK_FILE_NAME[NUMBER_OF_BANDS][FILE_NAME_LENGTH] = { "b0", "b1", "b2", "b3", "b4", 
                                                                "b5", "b6", "b7", "b8", "b9" };
int SS_MaskInit(void) {
    short bandIndex;
    char maskResourcePath[RESOURCE_PATH_LENGTH];

    for(bandIndex = 0; bandIndex < NUMBER_OF_BANDS; ++bandIndex) {
        sprintf(maskResourcePath, "%s", MASK_RESOURCE_PATH);
        strcat(maskResourcePath, MASK_FILE_NAME[bandIndex]);
        strcat(maskResourcePath, MASK_FILE_EXTENSION);
        puts(maskResourcePath);

        if (!(BAND[bandIndex] = sf_open(maskResourcePath, SFM_READ, &sfinfo))) {
            printf("Not able to open input file: %s.wav\n", MASK_FILE_NAME[bandIndex]);
            sf_perror (NULL);
            return  1;
        }
    }
    LEFT_INDEX = 0;
    RIGHT_INDEX = 1;
    bufferToFill = PING;

    readWavFiles();

    bufferToOutput = PING;
    bufferToFill = PONG;

    return 0;
}

void readWavFiles(void) {
    int readCounter, bufferOffset = BUFFER_LEN;
    float *bandPtr[NUMBER_OF_BANDS];
    short bandIndex;

    for(bandIndex = 0; bandIndex < NUMBER_OF_BANDS; ++bandIndex) {
        bandPtr[bandIndex] = bufferToFill == PING ? PING_INTERLEAVED + (bufferOffset * bandIndex) 
                                            : PONG_INTERLEAVED + (bufferOffset * bandIndex);
    }
    for(bandIndex = 0; bandIndex < NUMBER_OF_BANDS; ++bandIndex) {
        readCounter = sf_read_float(BAND[bandIndex], bandPtr[bandIndex], BUFFER_LEN);
        if(sf_error(BAND[bandIndex]) != 0) {
            printf("BAND%d error %d", bandIndex, sf_error(BAND[bandIndex]));
        }
        if(readCounter != BUFFER_LEN || readCounter == 0) {
            sf_seek(BAND[bandIndex], 0, SEEK_SET);
            readCounter = sf_read_float(BAND[bandIndex], bandPtr[bandIndex] + readCounter, BUFFER_LEN - readCounter);
        }
    }
}

void SS_InputDataToUI(float inValueToUI) {
    if(IN_AMP_INDEX < IO_BUFFER_SIZE) {
        IN_AMP_BUFFER[IN_AMP_INDEX] = inValueToUI;
        IN_AMP_INDEX++;
    }
    else {
      IN_AMP_READY = 0x01;
    }
}

float SS_SynthesizeRight(FrequencyBands magnitudeContainer) {
    float accumulate = 0.0;
    short bandIndex;
    int bufferOffset = BUFFER_LEN;

    if(bufferToOutput == PING) {
        for(bandIndex = 0; bandIndex < NUMBER_OF_BANDS; ++bandIndex) {
            magnitudeContainer.peakMagnitude[bandIndex] = magnitudeContainer.peakMagnitude[bandIndex] < 1 ? 
                                                        1 : magnitudeContainer.peakMagnitude[bandIndex];

            accumulate += PING_INTERLEAVED[RIGHT_INDEX + (bufferOffset * bandIndex)]
                             * magnitudeContainer.peakMagnitude[bandIndex] * VOLUME_S[bandIndex];
        }
    }
    else {
        for(bandIndex = 0; bandIndex < NUMBER_OF_BANDS; ++bandIndex) {
            magnitudeContainer.peakMagnitude[bandIndex] = magnitudeContainer.peakMagnitude[bandIndex] < 1 ? 
                                                        1 : magnitudeContainer.peakMagnitude[bandIndex];

            accumulate += PONG_INTERLEAVED[RIGHT_INDEX + (bufferOffset * bandIndex)]
                            * magnitudeContainer.peakMagnitude[bandIndex] * VOLUME_S[bandIndex];
        }
    }
    accumulate = accumulate * VOLUME_MASTER * ONE_OVER_BANDS;
    RIGHT_INDEX = RIGHT_INDEX < BUFFER_LEN - 1 ? RIGHT_INDEX + 2 : 1;

    if(AMP_INDEX < IO_BUFFER_SIZE) {
      AMP_BUFFER[AMP_INDEX] = accumulate;
    }
    return accumulate;
}

float SS_SynthesizeLeft(FrequencyBands magnitudeContainer){
    float accumulate = 0.0;
    short bandIndex;
    int bufferOffset = BUFFER_LEN;

    if(bufferToOutput == PING) {
        for(bandIndex = 0; bandIndex < NUMBER_OF_BANDS; ++bandIndex) {
            magnitudeContainer.peakMagnitude[bandIndex] = magnitudeContainer.peakMagnitude[bandIndex] < 1 ? 
                                                        1 : magnitudeContainer.peakMagnitude[bandIndex];

            accumulate += PING_INTERLEAVED[LEFT_INDEX + (bufferOffset * bandIndex)]
                            * magnitudeContainer.peakMagnitude[bandIndex] * VOLUME_S[bandIndex];
        }
    }
    else {
        for(bandIndex = 0; bandIndex < NUMBER_OF_BANDS; ++bandIndex) {
            magnitudeContainer.peakMagnitude[bandIndex] = magnitudeContainer.peakMagnitude[bandIndex] < 1 ? 
                                                        1 : magnitudeContainer.peakMagnitude[bandIndex];

            accumulate += PONG_INTERLEAVED[LEFT_INDEX + (bufferOffset * bandIndex)]
                            * magnitudeContainer.peakMagnitude[bandIndex] * VOLUME_S[bandIndex];
        }
    }
    accumulate = accumulate * VOLUME_MASTER * ONE_OVER_BANDS;
    LEFT_INDEX = LEFT_INDEX < BUFFER_LEN - 2 ? LEFT_INDEX + 2 : 0;

    if(AMP_INDEX < IO_BUFFER_SIZE) {
        AMP_BUFFER[AMP_INDEX] = (AMP_BUFFER[AMP_INDEX] + accumulate) * 0.5;
      AMP_INDEX++;
    }
    else {
      AMP_READY = 0x01;
    }
    return accumulate;
}

FrequencyBands SS_FrequencyBandInit(FrequencyBands freqBinContainer) {
    short bandIndex = 0;

    for(bandIndex = 0; bandIndex < NUMBER_OF_BANDS; ++bandIndex) {
        freqBinContainer.peakRawMagnitude[bandIndex] = 0.0;
        freqBinContainer.bandScale[bandIndex] = 0.0;
        freqBinContainer.peakFrequency[bandIndex] = 0.0;
        freqBinContainer.peakMagnitude[bandIndex] = 0.0;
        freqBinContainer.peakPowerDensity[bandIndex] = 0.0;
    }
    return freqBinContainer;
}
