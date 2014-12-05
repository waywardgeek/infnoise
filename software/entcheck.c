/*
Measure the entropy level of an input sample.

*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include "infnoise.h"

#define INM_MIN_DATA 80000
#define INM_MIN_SAMPLE_SIZE 100
#define INM_MAX_SEQUENCE 20
#define INM_MAX_COUNT (1 << 14)

static uint8_t inmN;
static uint32_t inmPrevBits;
static uint32_t inmNumBitsSampled;
static uint32_t *inmOnes, *inmZeros;
// The total probability of generating the string of states we did is
// 1/(2^inmNumBitsOfEntropy * inmCurrentProbability).
static uint32_t inmNumBitsOfEntropy;
static double inmCurrentProbability;
static uint64_t inmTotalBits;
static bool inmPrevBit;
static uint32_t inmTotalOnes, inmTotalZeros;
static bool inmDebug;

// Free memory used by the entropy check.
void inmEntCheckStop(void) {
    if(inmOnes != NULL) {
        free(inmOnes);
    }
    if(inmZeros != NULL) {
        free(inmZeros);
    }
}

// Reset the statistics.
static void resetStats(void) {
    inmNumBitsSampled = 0;
    inmCurrentProbability = 1.0;
    inmNumBitsOfEntropy = 0;
    inmTotalOnes = 0;
    inmTotalZeros = 0;
}

// Initialize the entropy check.  N is the number of bits used to predict the next bit.
// At least 8 bits must be used, and no more than 30.  In general, we should use bits
// large enough so that INM output will be uncorrelated with bits N samples back in time.
bool inmEntCheckStart(uint8_t N, bool debug) {
    if(N < 1 || N > 30) {
        return false;
    }
    inmDebug = debug;
    inmNumBitsOfEntropy = 0;
    inmCurrentProbability = 1.0;
    inmN = N;
    inmPrevBits = 0;
    inmOnes = calloc(1u << N, sizeof(uint32_t));
    inmZeros = calloc(1u << N, sizeof(uint32_t));
    inmTotalBits = 0;
    inmPrevBit = false;
    resetStats();
    if(inmOnes == NULL || inmZeros == NULL) {
        inmEntCheckStop();
        return false;
    }
    return true;
}

// If running continuously, it is possible to start overflowing the 32-bit counters for
// zeros and ones.  Check for this, and scale the stats if needed.
static void scaleStats(void) {
    uint32_t i;
    for(i = 0; i < (1 << inmN); i++) {
        inmZeros[i] >>= 1;
        inmOnes[i] >>= 1;
    }
}

// If running continuously, it is possible to start overflowing the 32-bit counters for
// zeros and ones.  Check for this, and scale the stats if needed.
static void scaleEntropy(void) {
    if(inmNumBitsSampled == INM_MIN_DATA) {
        inmNumBitsOfEntropy >>= 1;
        inmNumBitsSampled >>= 1;
    }
}

// If running continuously, it is possible to start overflowing the 32-bit counters for
// zeros and ones.  Check for this, and scale the stats if needed.
static void scaleZeroOneCounts(void) {
    uint64_t maxVal = inmTotalZeros >= inmTotalOnes? inmTotalZeros : inmTotalOnes;
    if(maxVal == INM_MIN_DATA) {
        inmTotalZeros >>= 1;
        inmTotalOnes >>= 1;
    }
}

// This should be called for each bit generated.
bool inmEntCheckAddBit(bool bit) {
    inmTotalBits++;
    inmPrevBits = (inmPrevBits << 1) & ((1 << inmN)-1);
    if(inmPrevBit) {
        inmPrevBits |= 1;
    }
    inmPrevBit = bit;
    if(inmNumBitsSampled > 100) {
        if(bit) {
            inmTotalOnes++;
        } else {
            inmTotalZeros++;
        }
    }
    uint32_t zeros, ones;
    zeros = inmZeros[inmPrevBits];
    ones = inmOnes[inmPrevBits];
    uint32_t total = zeros + ones;
    if(bit) {
        if(ones != 0) {
            inmCurrentProbability *= (double)ones/total;
        }
    } else {
        if(zeros != 0) {
            inmCurrentProbability *= (double)zeros/total;
        }
    }
    while(inmCurrentProbability <= 0.5) {
        inmCurrentProbability *= 2.0;
        inmNumBitsOfEntropy++;
    }
    //printf("probability:%f\n", inmCurrentProbability);
    inmNumBitsSampled++;
    if(bit) {
        inmOnes[inmPrevBits]++;
        if(inmOnes[inmPrevBits] == INM_MAX_COUNT) {
            scaleStats();
        }
    } else {
        inmZeros[inmPrevBits]++;
        if(inmZeros[inmPrevBits] == INM_MAX_COUNT) {
            scaleStats();
        }
    }
    scaleEntropy();
    scaleZeroOneCounts();
    return true;
}

// Once we have enough samples, we know that entropyPerBit = log(K)/log(2), so
// K must be 2^entryopPerBit.
double inmEntCheckEstimateEntropyPerBit(void) {
    return (double)inmNumBitsOfEntropy/inmNumBitsSampled;
}

// Print the tables of statistics.
static void inmDumpStats(void) {
    uint32_t i;
    for(i = 0; i < 1 << inmN; i++) {
        //if(inmOnes[i] > 0 || inmZeros[i] > 0) {
            printf("%x ones:%u zeros:%u\n", i, inmOnes[i], inmZeros[i]);
        //}
    }
}

int main() {
    uint8_t N = 16;
    inmEntCheckStart(N, false);
    int value = getchar();
    while(value != EOF) {
        int i;
        for(i = 0; i < 8; i++) {
	    inmEntCheckAddBit(value & 1);
            value >>= 1;
        }
        value = getchar();
        if((inmTotalBits & 0xffff) == 0) {
            printf("Added %llu bits, estimated entropy per bit:%f\n", (long long)inmTotalBits,
                inmEntCheckEstimateEntropyPerBit());
        }
    }
    if(inmDebug) {
        inmDumpStats();
    }
    inmEntCheckStop();
    return 0;
}
