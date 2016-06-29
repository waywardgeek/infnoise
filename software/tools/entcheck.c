// Measure the entropy level of an input sample.

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

#define INM_MIN_DATA 80000u
#define INM_MIN_SAMPLE_SIZE 100u
#define INM_MAX_COUNT (1u << 14u)

static uint8_t inmN;
static uint8_t inmNumStreams;
static uint32_t inmPrevBits;
static uint32_t inmNumBitsSampled;
static uint32_t **inmOnes, **inmZeros;
// The total probability of generating the string of states we did is
// 1/(2^inmNumBitsOfEntropy * inmCurrentProbability).
static uint32_t inmNumBitsOfEntropy;
static double inmCurrentProbability;
static uint64_t inmTotalBits;
static bool inmPrevBit;
static uint32_t inmTotalOnes, inmTotalZeros;
static bool inmDebug;

// Reset the statistics.
static void resetStats(void) {
    inmNumBitsSampled = 0u;
    inmCurrentProbability = 1.0;
    inmNumBitsOfEntropy = 0u;
    inmTotalOnes = 0u;
    inmTotalZeros = 0u;
}

// Initialize the entropy check.  N is the number of bits used to predict the next bit.
// At least 8 bits must be used, and no more than 30.  In general, we should use bits
// large enough so that INM output will be uncorrelated with bits N samples back in time.
bool inmEntCheckStart(uint8_t N, uint8_t numStreams, bool debug) {
    if(N < 1u || N > 30u) {
        return false;
    }
    inmDebug = debug;
    inmNumBitsOfEntropy = 0u;
    inmCurrentProbability = 1.0;
    inmN = N;
    inmNumStreams = numStreams;
    inmPrevBits = 0u;
    inmOnes = calloc(numStreams, sizeof(uint32_t *));
    inmZeros = calloc(numStreams, sizeof(uint32_t *));
    if(inmOnes == NULL || inmZeros == NULL) {
        return false;
    }
    uint8_t i;
    for(i = 0u; i < numStreams; i++) {
        inmOnes[i] = calloc(1u << N, sizeof(uint32_t));
        inmZeros[i] = calloc(1u << N, sizeof(uint32_t));
        if(inmOnes[i] == NULL || inmZeros[i] == NULL) {
            return false;
        }
    }
    inmTotalBits = 0u;
    inmPrevBit = false;
    resetStats();
    return true;
}

// If running continuously, it is possible to start overflowing the 32-bit counters for
// zeros and ones.  Check for this, and scale the stats if needed.
static void scaleStats(void) {
    uint32_t i, j;
    for(i = 0u; i < inmNumStreams; i++) {
        for(j = 0u; j < (1u << inmN); j++) {
            inmZeros[i][j] >>= 1u;
            inmOnes[i][j] >>= 1u;
        }
    }
}

// If running continuously, it is possible to start overflowing the 32-bit counters for
// zeros and ones.  Check for this, and scale the stats if needed.
static void scaleEntropy(void) {
    if(inmNumBitsSampled == INM_MIN_DATA) {
        inmNumBitsOfEntropy >>= 1u;
        inmNumBitsSampled >>= 1u;
    }
}

// If running continuously, it is possible to start overflowing the 32-bit counters for
// zeros and ones.  Check for this, and scale the stats if needed.
static void scaleZeroOneCounts(void) {
    uint64_t maxVal = inmTotalZeros >= inmTotalOnes? inmTotalZeros : inmTotalOnes;
    if(maxVal == INM_MIN_DATA) {
        inmTotalZeros >>= 1u;
        inmTotalOnes >>= 1u;
    }
}

// This should be called for each bit generated.
bool inmEntCheckAddBit(bool bit) {
    uint8_t stream = inmTotalBits % inmNumStreams;
    inmTotalBits++;
    inmPrevBits = (inmPrevBits << 1u) & ((1u << inmN)-1u);
    if(inmPrevBit) {
        inmPrevBits |= 1;
    }
    inmPrevBit = bit;
    if(inmNumBitsSampled > 100u) {
        if(bit) {
            inmTotalOnes++;
        } else {
            inmTotalZeros++;
        }
    }
    uint32_t *zeros = inmZeros[stream];
    uint32_t *ones = inmOnes[stream];
    uint32_t numZeros, numOnes;
    numZeros = zeros[inmPrevBits];
    numOnes = ones[inmPrevBits];
    uint32_t total = numZeros + numOnes;
    if(bit) {
        if(numOnes != 0u) {
            inmCurrentProbability *= (double)numOnes/total;
        }
    } else {
        if(numZeros != 0u) {
            inmCurrentProbability *= (double)numZeros/total;
        }
    }
    while(inmCurrentProbability <= 0.5) {
        inmCurrentProbability *= 2.0;
        inmNumBitsOfEntropy++;
    }
    //printf("probability:%f\n", inmCurrentProbability);
    inmNumBitsSampled++;
    if(bit) {
        ones[inmPrevBits]++;
        if(ones[inmPrevBits] == INM_MAX_COUNT) {
            scaleStats();
        }
    } else {
        zeros[inmPrevBits]++;
        if(zeros[inmPrevBits] == INM_MAX_COUNT) {
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
    uint32_t i, j;
    for(i = 0u; i < inmNumStreams; i++) {
        printf("*************************************** stream %u\n", i);
        for(j = 0u; j < 1u << inmN; j++) {
            printf("%x ones:%u(%.2g%%) zeros:%u(%.2g%%)\n", j, inmOnes[i][j],
                inmOnes[i][j]*100.0/inmTotalBits, inmZeros[i][j],
                inmZeros[i][j]*100.0/inmTotalBits);
        }
    }
}

// Print usage and exit
static void usage(void) {
    fprintf(stderr, "Usage: entcheck [options]\n"
           "    -N numBits    -- Use N previous bits to predict the next bit\n"
           "    -s numStreams -- Use s streams to predict the next bit\n"
           "\n"
           "entcheck simply uses the previous N bits (16 by default) to predict the next bit.\n"
           "It estimates the entropy based on 'surprise', or log2 of the probability of seeing\n"
           "the string of 0's and 1's.  Sometimes some bits are special, such as the output of\n"
           "an 8 bit DAC, where we want different tables depending on which bit we're predicting.\n"
           "Set numStreams to the DAC width in this case.\n");
    exit(1);
}

int main(int argc, char **argv) {
    uint8_t N = 12u;
    uint8_t numStreams = 2u;
    int xArg;
    for(xArg = 1u; xArg < argc; xArg++) {
        if(!strcmp(argv[xArg], "-N")) {
            xArg++;
            if(xArg == argc) {
                usage();
            }
            N = atoi(argv[xArg]);
            if(N == 0u) {
                usage();
            }
        } else if(!strcmp(argv[xArg], "-s")) {
            xArg++;
            if(xArg == argc) {
                usage();
            }
            numStreams = atoi(argv[xArg]);
            if(numStreams == 0u || numStreams > 32u) {
                usage();
            }
        } else {
            usage();
        }
    }
    if(!inmEntCheckStart(N, numStreams, true)) {
        fprintf(stderr, "Unable to allocate memory\n");
        return 1;
    }
    int value = getchar();
    while(value != EOF) {
        uint32_t i;
        for(i = 0u; i < 8u; i++) {
            bool bit = value & (1u << (7u-i))? true : false;
	    inmEntCheckAddBit(bit);
        }
        value = getchar();
        if((inmTotalBits & 0xffffu) == 0u) {
            printf("Added %llu bits, estimated entropy per bit:%f\n", (long long)inmTotalBits,
                inmEntCheckEstimateEntropyPerBit());
            //resetStats();
        }
    }
    if(inmDebug) {
        inmDumpStats();
    }
    return 0;
}
