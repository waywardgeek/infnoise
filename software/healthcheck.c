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

#define INM_MIN_SAMPLE_SIZE 100

static uint8_t inmN;
static uint32_t inmPrevBits;
static uint64_t inmNumBitsCounted, inmNumBitsSampled;
static uint64_t *inmOneCounts, *inmZeroCounts, *inmSameAsLastTimeCounts;
static bool *inmPrevResults;
static double inmK, inmExpectedEntropyPerBit;
// The total probability of generating the string of states we did is
// 1/(2^inmNumBitsOfEntropy * inmCurrentProbability).
static uint64_t inmNumBitsOfEntropy;
static double inmCurrentProbability;

// Free memory used by the health checker.
void inmHealthCheckerStop(void) {
    if(inmOneCounts != NULL) {
        free(inmOneCounts);
    }
    if(inmZeroCounts != NULL) {
        free(inmZeroCounts);
    }
    if(inmSameAsLastTimeCounts != NULL) {
        free(inmSameAsLastTimeCounts);
    }
    if(inmPrevResults != NULL) {
        free(inmPrevResults);
    }
}

// Initialize the health checker.  N is the number of bits used to predict the next bit.
// At least 8 bits must be used, and no more than 30.  In general, we should use bits
// large enough so that INM output will be uncorrelated with bits N samples back in time.
bool inmHealthCheckerStart(uint8_t N, double K) {
    if(N < 8 || N > 30) {
        return false;
    }
    inmNumBitsOfEntropy = 0;
    inmCurrentProbability = 1.0;
    inmK = K;
    inmN = N;
    inmPrevBits = 0;
    inmNumBitsCounted = 0;
    inmNumBitsSampled = 0;
    inmOneCounts = calloc(1u << N, sizeof(uint64_t));
    inmZeroCounts = calloc(1u << N, sizeof(uint64_t));
    inmSameAsLastTimeCounts = calloc(1u << N, sizeof(uint64_t));
    inmPrevResults = calloc(1u << N, sizeof(bool));
    inmExpectedEntropyPerBit = log(K)/log(2.0);
    if(inmOneCounts == NULL || inmZeroCounts == NULL || inmSameAsLastTimeCounts == NULL
        || inmPrevResults == NULL) {
        inmHealthCheckerStop();
        return false;
    }
    return true;
}

// Reset the statistics.
static void resetStats(void) {
    printf("Resetting with numSampled=%lu and numCounted=%lu\n", inmNumBitsSampled, inmNumBitsCounted);
    inmNumBitsSampled = 0;
    inmNumBitsCounted = 0;
    inmCurrentProbability = 1.0;
    inmNumBitsOfEntropy = 0;
}

// This should be called for each bit generated.
bool inmHealthCheckerAddBit(bool bit) {
    if(inmOneCounts[inmPrevBits] > INM_MIN_SAMPLE_SIZE ||
            inmZeroCounts[inmPrevBits] > INM_MIN_SAMPLE_SIZE) {
        uint64_t total = inmZeroCounts[inmPrevBits] + inmOneCounts[inmPrevBits];
        if(bit) {
            if(inmOneCounts[inmPrevBits] != 0) {
                inmCurrentProbability *= (double)inmOneCounts[inmPrevBits]/total;
            }
        } else {
            if(inmZeroCounts[inmPrevBits] != 0) {
                inmCurrentProbability *= (double)inmZeroCounts[inmPrevBits]/total;
            }
        }
        while(inmCurrentProbability <= 0.5) {
            inmCurrentProbability *= 2.0;
            inmNumBitsOfEntropy++;
        }
        //printf("probability:%f\n", inmCurrentProbability);
        inmNumBitsCounted++;
    }
    inmNumBitsSampled++;
    if(bit) {
        inmOneCounts[inmPrevBits]++;
    } else {
        inmZeroCounts[inmPrevBits]++;
    }
    if(bit == inmPrevResults[inmPrevBits]) {
        inmSameAsLastTimeCounts[inmPrevBits]++;
    }
    inmPrevResults[inmPrevBits] = bit;
    inmPrevBits = (inmPrevBits << 1) & ((1 << inmN)-1);
    if(bit) {
        inmPrevBits |= 1;
    }
    //printf("prevBits: %x\n", inmPrevBits);
    if(inmNumBitsSampled < 10000) {
        return true; // Not enough data yet to test
    }
    if(inmNumBitsSampled == 10000 && inmNumBitsCounted < 9900) {
        // Wait until we have enough data to start measuring entropy
        resetStats();
        return true;
    }
    if(inmNumBitsOfEntropy > 10000) {
        // Check the entropy is in line with expectations
        uint64_t expectedEntropy = inmExpectedEntropyPerBit*inmNumBitsCounted;
        if(inmNumBitsOfEntropy > expectedEntropy*1.1 || inmNumBitsOfEntropy < expectedEntropy/1.1) {
            printf("entropy:%lu, expected entropy:%lu, num bits counted:%lu, num bits sampled:%lu\n",
                inmNumBitsOfEntropy, expectedEntropy, inmNumBitsCounted, inmNumBitsSampled);
            return false;
        }
    }
    return true;
}

// Once we have enough samples, we know that entropyPerBit = log(K)/log(2), so
// K must be 2^entryopPerBit.
double inmHealthCheckerEstimateK(void) {
    if(inmNumBitsOfEntropy <= 10000) {
        return inmK;
    }
    double entropyPerBit = (double)inmNumBitsOfEntropy/inmNumBitsCounted;
    return pow(2.0, entropyPerBit);
}

// Compare the ability to predict with 1 fewer bits and see how much less accurate we are.
static void checkLSBStatsForNBits(uint8_t N) {
    uint32_t i, j;
    uint64_t totalGuesses = 0;
    uint64_t totalRight = 0.0;
    for(i = 0; i < (1 << N); i++) {
        uint64_t total = 0;
        uint64_t zeros = 0;
        uint64_t ones = 0;
        for(j = 0; j < (1 << (inmN - N)); j++) {
            uint32_t pos = i + j*(1 << N);
            total += inmZeroCounts[pos] + inmOneCounts[pos];
            zeros += inmZeroCounts[pos];
            ones += inmOneCounts[pos];
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
    //double K = 1.82;
    double K = 1.9;
    inmHealthCheckerStart(16, K);
    srand(time(NULL));
    double A = (double)rand()/RAND_MAX; // Simulating INM
    double noiseAmplitude = 1.0/(1 << 12);
    while(true) {
        bool bit = computeRandBit(&A, K, noiseAmplitude);
        if(!inmHealthCheckerAddBit(bit)) {
            printf("Failed health check!\n");
            resetStats();
            //return 1;
        } else if(inmNumBitsCounted > 0 && (inmNumBitsCounted & 0xfffff) == 0) {
            printf("Estimated K: %f\n", inmHealthCheckerEstimateK());
            checkLSBStats();
            //resetStats();
        }
    }
    inmHealthCheckerStop();
    return 0;
}
