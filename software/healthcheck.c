/*
Measure the entropy level dynamically from the Infinite Noise Multiplier.

The theory behind this is simple.  The next bit from the INM TRNG can be guessed, based on
the previous bits, by measuring how often a 0 or 1 occurs given the previous bits.  Update
these statistics dynamically, and use them to determine how hard it would be to predict
the current state.

For example, if 0100 is followed by 1 80% of the time, and we read a 1, the probability of
the input string being what it is decreases by multiplying it by 0.8.  If we read a 0, we
multiply the likelyhood of the current state by 0.2.

Because INMs generate about log(K)/log(2) bits per clock when K is the gain used in the
INM (between 1 and 2), we know how much entropy there should be coming from the device.
If the measured entropy diverges too strongly from the theoretical entropy, we should shut
down the entropy source, since it is not working correctly.

An assumption made is that bits far enough away are not correlated.  This is directly
confirmed.

*/

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include "healthcheck.h"

#define INM_MIN_DATA 80000
#define INM_MIN_SAMPLE_SIZE 100
#define INM_ACCURACY 1.05
#define INM_MAX_SEQUENCE 20
#define INM_MAX_COUNT (1 << 14)
// Matches the Keccac sponge size
#define INM_MAX_ENTROPY 1600

static uint8_t inmN;
static uint32_t inmPrevBits;
static uint32_t inmNumBitsSampled;
static uint32_t *inmOnesEven, *inmZerosEven;
static uint32_t *inmOnesOdd, *inmZerosOdd;
static double inmK, inmExpectedEntropyPerBit;
// The total probability of generating the string of states we did is
// 1/(2^inmNumBitsOfEntropy * inmCurrentProbability).
static uint32_t inmNumBitsOfEntropy;
static double inmCurrentProbability;
static uint64_t inmTotalBits;
static bool inmPrevBit;
static uint32_t inmEntropyLevel;
static uint32_t inmNumSequentialZeros, inmNumSequentialOnes;

// Free memory used by the health check.
void inmHealthCheckStop(void) {
    if(inmOnesEven != NULL) {
        free(inmOnesEven);
    }
    if(inmZerosEven != NULL) {
        free(inmZerosEven);
    }
    if(inmOnesOdd != NULL) {
        free(inmOnesOdd);
    }
    if(inmZerosOdd != NULL) {
        free(inmZerosOdd);
    }
}

// Reset the statistics.
static void resetStats(void) {
    inmNumBitsSampled = 0;
    inmCurrentProbability = 1.0;
    inmNumBitsOfEntropy = 0;
    inmEntropyLevel = 0;
}

// Initialize the health check.  N is the number of bits used to predict the next bit.
// At least 8 bits must be used, and no more than 30.  In general, we should use bits
// large enough so that INM output will be uncorrelated with bits N samples back in time.
bool inmHealthCheckStart(uint8_t N, double K) {
    if(N < 1 || N > 30) {
        return false;
    }
    inmNumBitsOfEntropy = 0;
    inmCurrentProbability = 1.0;
    inmK = K;
    inmN = N;
    inmPrevBits = 0;
    inmOnesEven = calloc(1u << N, sizeof(uint32_t));
    inmZerosEven = calloc(1u << N, sizeof(uint32_t));
    inmOnesOdd = calloc(1u << N, sizeof(uint32_t));
    inmZerosOdd = calloc(1u << N, sizeof(uint32_t));
    inmExpectedEntropyPerBit = log(K)/log(2.0);
    inmTotalBits = 0;
    inmPrevBit = false;
    inmNumSequentialZeros = 0;
    inmNumSequentialOnes = 0;
    resetStats();
    if(inmOnesEven == NULL || inmZerosEven == NULL || inmOnesOdd == NULL || inmZerosOdd == NULL) {
        inmHealthCheckStop();
        return false;
    }
    return true;
}

// If running continuously, it is possible to start overflowing the 32-bit counters for
// zeros and ones.  Check for this, and scale the stats if needed.
static void scaleStats(void) {
    uint32_t i;
    printf("Scaling stats...\n");
    for(i = 0; i < (1 << inmN); i++) {
        inmZerosEven[i] >>= 1;
        inmOnesEven[i] >>= 1;
        inmZerosOdd[i] >>= 1;
        inmOnesOdd[i] >>= 1;
    }
}

// If running continuously, it is possible to start overflowing the 32-bit counters for
// zeros and ones.  Check for this, and scale the stats if needed.
static void scaleEntropy(void) {
    //printf("Scaling entropy...\n");
    inmNumBitsOfEntropy = inmNumBitsOfEntropy*(uint64_t)INM_MIN_DATA/(2*inmNumBitsSampled);
    inmNumBitsSampled = INM_MIN_DATA/2;
}

// This should be called for each bit generated.
bool inmHealthCheckAddBit(bool bit, bool even) {
    inmTotalBits++;
    if((inmTotalBits & 0xfffff) == 0) {
        printf("Generated %lu bots.  Estimated entropy per bit: %f, estimated K: %f\n",
            inmTotalBits, inmHealthCheckEstimateEntropyPerBit(), inmHealthCheckEstimateK());
        if(inmTotalBits > 3000000) {
            exit(0);
        }
    }
    inmPrevBits = (inmPrevBits << 1) & ((1 << inmN)-1);
    if(inmPrevBit) {
        inmPrevBits |= 1;
    }
    inmPrevBit = bit;
    if(inmNumBitsSampled > 100) {
        if(bit) {
            inmNumSequentialOnes++;
            inmNumSequentialZeros = 0;
            if(inmNumSequentialOnes > INM_MAX_SEQUENCE) {
                printf("Maximum sequence of %d 1's exceeded\n", INM_MAX_SEQUENCE);
                //exit(1);
            }
        } else {
            inmNumSequentialZeros++;
            inmNumSequentialOnes = 0;
            if(inmNumSequentialZeros > INM_MAX_SEQUENCE) {
                printf("Maximum sequence of %d 0's exceeded\n", INM_MAX_SEQUENCE);
                //exit(1);
            }
        }
    }
    uint32_t zeros, ones;
    if(even) {
        zeros = inmZerosEven[inmPrevBits];
        ones = inmOnesEven[inmPrevBits];
    } else {
        zeros = inmZerosOdd[inmPrevBits];
        ones = inmOnesOdd[inmPrevBits];
    }
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
        if(inmHealthCheckOkToUseData() && inmEntropyLevel < INM_MAX_ENTROPY) {
            inmEntropyLevel++;
        }
    }
    //printf("probability:%f\n", inmCurrentProbability);
    inmNumBitsSampled++;
    if(bit) {
        if(even) {
            inmOnesEven[inmPrevBits]++;
            if(inmOnesEven[inmPrevBits] == INM_MAX_COUNT) {
                scaleStats();
            }
        } else {
            inmOnesOdd[inmPrevBits]++;
            if(inmOnesOdd[inmPrevBits] == INM_MAX_COUNT) {
                scaleStats();
            }
        }
    } else {
        if(even) {
            inmZerosEven[inmPrevBits]++;
            if(inmZerosEven[inmPrevBits] == INM_MAX_COUNT) {
                scaleStats();
            }
        } else {
            inmZerosOdd[inmPrevBits]++;
            if(inmZerosOdd[inmPrevBits] == INM_MAX_COUNT) {
                scaleStats();
            }
        }
    }
    if(inmNumBitsSampled == INM_MIN_DATA) {
        scaleEntropy();
    }
    return true;
}

// Once we have enough samples, we know that entropyPerBit = log(K)/log(2), so
// K must be 2^entryopPerBit.
double inmHealthCheckEstimateK(void) {
    double entropyPerBit = (double)inmNumBitsOfEntropy/inmNumBitsSampled;
    return pow(2.0, entropyPerBit);
}

// Once we have enough samples, we know that entropyPerBit = log(K)/log(2), so
// K must be 2^entryopPerBit.
double inmHealthCheckEstimateEntropyPerBit(void) {
    return (double)inmNumBitsOfEntropy/inmNumBitsSampled;
}

// Return true if the health checker has enough data to verify proper operation of the INM.
bool inmHealthCheckOkToUseData(void) {
    return inmNumBitsSampled >= INM_MIN_DATA;
}

// Just return the entropy level added so far in bytes;
uint32_t inmHealthCheckGetEntropyLevel(void) {
    return inmEntropyLevel/8;
}

// Reduce the entropy level by numBytes.
void inmHealthCheckReduceEntropyLevel(uint32_t numBytes) {
    if(numBytes*8 > inmEntropyLevel) {
        fprintf(stderr, "Entropy pool underflow\n");
        exit(1);
    }
    inmEntropyLevel -=  numBytes*8;
}

#ifdef TEST_HEALTHCHECK

// Print the tables of statistics.
static void inmDumpStats(void) {
    uint32_t i;
    for(i = 0; i < 1 << inmN; i++) {
        //if(inmOnes[i] > 0 || inmZeros[i] > 0) {
            printf("%x onesEven:%u zerosEven:%u onesOdd:%u zerosOdd:%u\n",
                i, inmOnesEven[i], inmZerosEven[i], inmOnesOdd[i], inmZerosOdd[i]);
        //}
    }
}

// Compare the ability to predict with 1 fewer bits and see how much less accurate we are.
static void checkLSBStatsForNBits(uint8_t N) {
    uint32_t i, j;
    uint32_t totalGuesses = 0;
    uint32_t totalRight = 0.0;
    for(i = 0; i < (1 << N); i++) {
        uint32_t total = 0;
        uint32_t zeros = 0;
        uint32_t ones = 0;
        for(j = 0; j < (1 << (inmN - N)); j++) {
            uint32_t pos = i + j*(1 << N);
            total += inmZerosEven[pos] + inmOnesEven[pos];
            zeros += inmZerosEven[pos];
            ones += inmOnesEven[pos];
        }
        if(zeros >= ones) {
            totalRight += zeros;
        } else {
            totalRight += ones;
        }
        totalGuesses += total;
    }
    printf("Probability of guessing correctly with %u bits: %f\n", N, (double)totalRight/totalGuesses);
}

// Compare the ability to predict with 1 fewer bits and see how much less accurate we are.
static void checkLSBStats(void) {
    uint32_t N;
    for(N = 1; N <= inmN; N++) {
        checkLSBStatsForNBits(N);
    }
}

/* This could be built with one opamp for the multiplier, a comparator with
   rail-to-rail outputs, and switches and caps and resistors.*/
static inline bool updateA(double *A, double K, double noise) {
    if(*A > 1.0) {
        *A = 1.0;
    } else if (*A < 0.0) {
        *A = 0.0;
    }
    *A += noise;
    if(*A > 0.5) {
        *A = K**A - (K-1);
        return true;
    }
    *A += noise;
    *A = K**A;
    return false;
}

static inline bool computeRandBit(double *A, double K, double noiseAmplitude) {
    double noise = noiseAmplitude*(((double)rand()/RAND_MAX) - 0.5);
    return updateA(A, K, noise);
}

int main() {
    //double K = sqrt(2.0);
    double K = 1.82;
    uint8_t N = 16;
    inmHealthCheckStart(N, K);
    srand(time(NULL));
    double A = (double)rand()/RAND_MAX; // Simulating INM
    double noiseAmplitude = 1.0/(1 << 10);
    uint32_t i;
    for(i = 0; i < 32; i++) {
        // Throw away some initial bits.
        computeRandBit(&A, K, noiseAmplitude);
    }
    for(i = 0; i < 1 << 28; i++) {
        bool bit = computeRandBit(&A, K, noiseAmplitude);
        if(!inmHealthCheckAddBit(bit, true)) {
            printf("Failed health check!\n");
            return 1;
        }
        if(inmTotalBits > 0 && (inmTotalBits & 0xfffffff) == 0) {
            printf("Estimated entropy per bit: %f, estimated K: %f\n", inmHealthCheckEstimateEntropyPerBit(),
                inmHealthCheckEstimateK());
            checkLSBStats();
        }
    }
    inmDumpStats();
    inmHealthCheckStop();
    return 0;
}
#endif
