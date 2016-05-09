#include <stdio.h>
#include <math.h>
#include "ss_psd.h"

void SS_PSDDraw(double signalAverage[], int channelsToProcess) {
    int barLength;
    float psdB;
    unsigned currentChannel;
    
    // 10log10(signal average) calcuation of mask computation
    for (currentChannel = 0; currentChannel < channelsToProcess; currentChannel++) {
        psdB = 10 * log10(signalAverage[currentChannel]);
        barLength = 64 + floor(psdB - 0.5);
        
        if (barLength < 0) {
            barLength = 0;
        }
        printf("ch%d: %4.1f dB ", currentChannel, psdB);
        
        while(barLength > 0) {
            putchar('#');
            barLength--;
        }
        putchar('\n');
    }
}
