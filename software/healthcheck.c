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
#define INM_ACCURACY 1.1

static uint8_t inmN;
static uint32_t inmPrevBits;
static uint32_t inmNumBitsCounted, inmNumBitsSampled;
static uint32_t *inmOnes, *inmZeros;
static double inmK, inmExpectedEntropyPerBit;
// The total probability of generating the string of states we did is
// 1/(2^inmNumBitsOfEntropy * inmCurrentProbability).
static uint32_t inmNumBitsOfEntropy;
static double inmCurrentProbability;

// Print the tables of statistics.
static void inmDumpStats(void) {
    uint32_t i;
    for(i = 0; i < 1 << inmN; i++) {
        if(inmOnes[i] > 0 || inmZeros[i] > 0) {
            printf("%x ones:%u zeros:%u\n", i, inmOnes[i], inmZeros[i]);
        }
    }
}

// Free memory used by the health checker.
void inmHealthCheckerStop(void) {
    if(inmOnes != NULL) {
        free(inmOnes);
    }
    if(inmZeros != NULL) {
        free(inmZeros);
    }
}

// Initialize the health checker.  N is the number of bits used to predict the next bit.
// At least 8 bits must be used, and no more than 30.  In general, we should use bits
// large enough so that INM output will be uncorrelated with bits N samples back in time.
bool inmHealthCheckerStart(uint8_t N, double K) {
    if(N < 1 || N > 30) {
        return false;
    }
    inmNumBitsOfEntropy = 0;
    inmCurrentProbability = 1.0;
    inmK = K;
    inmN = N;
    inmPrevBits = 0;
    inmNumBitsCounted = 0;
    inmNumBitsSampled = 0;
    inmOnes = calloc(1u << N, sizeof(uint32_t));
    inmZeros = calloc(1u << N, sizeof(uint32_t));
    inmExpectedEntropyPerBit = log(K)/log(2.0);
    if(inmOnes == NULL || inmZeros == NULL) {
        inmHealthCheckerStop();
        return false;
    }
    return true;
}

// Reset the statistics.
static void resetStats(void) {
    printf("Resetting with numSampled=%u and numCounted=%u\n", inmNumBitsSampled, inmNumBitsCounted);
    inmNumBitsSampled = 0;
    inmNumBitsCounted = 0;
    inmCurrentProbability = 1.0;
    inmNumBitsOfEntropy = 0;
}

// This should be called for each bit generated.
bool inmHealthCheckerAddBit(bool bit) {
    if(inmOnes[inmPrevBits] > INM_MIN_SAMPLE_SIZE ||
            inmZeros[inmPrevBits] > INM_MIN_SAMPLE_SIZE) {
        uint32_t total = inmZeros[inmPrevBits] + inmOnes[inmPrevBits];
        if(bit) {
            if(inmOnes[inmPrevBits] != 0) {
                inmCurrentProbability *= (double)inmOnes[inmPrevBits]/total;
            }
        } else {
            if(inmZeros[inmPrevBits] != 0) {
                inmCurrentProbability *= (double)inmZeros[inmPrevBits]/total;
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
        inmOnes[inmPrevBits]++;
    } else {
        inmZeros[inmPrevBits]++;
    }
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
        uint32_t expectedEntropy = inmExpectedEntropyPerBit*inmNumBitsCounted;
        if(inmNumBitsOfEntropy > expectedEntropy*INM_ACCURACY || inmNumBitsOfEntropy < expectedEntropy/INM_ACCURACY) {
            printf("entropy:%u, expected entropy:%u, num bits counted:%u, num bits sampled:%u\n",
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

// Once we have enough samples, we know that entropyPerBit = log(K)/log(2), so
// K must be 2^entryopPerBit.
double inmHealthCheckerEstimateEntropyPerBit(void) {
    if(inmNumBitsOfEntropy <= 10000) {
        return inmExpectedEntropyPerBit;
    }
    return (double)inmNumBitsOfEntropy/inmNumBitsCounted;
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
            total += inmZeros[pos] + inmOnes[pos];
            zeros += inmZeros[pos];
            ones += inmOnes[pos];
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

// If running continuously, it is possible to start overflowing the 32-bit counters for
// zeros and ones.  Check for this, and scale the stats if needed.
static void reduceStatsIfNeeded(void) {
    uint32_t i;
    uint32_t maxValue = 0;
    for(i = 0; i < (1 << inmN); i++) {
        uint32_t zeros = inmZeros[i];
        uint32_t ones = inmOnes[i];
        if(zeros >= maxValue) {
            maxValue = zeros;
        }
        if(ones > maxValue) {
            maxValue = ones;
        }
    }
    if(maxValue > (1 << 30)) {
        printf("Scaling stats...\n");
        for(i = 0; i < (1 << inmN); i++) {
            inmZeros[i] >>= 1;
            inmOnes[i] >>= 1;
        }
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
    double K = 1.82;
    inmHealthCheckerStart(14, K);
    srand(time(NULL));
    double A = (double)rand()/RAND_MAX; // Simulating INM
    double noiseAmplitude = 1.0/(1 << 12);
    uint32_t i;
    for(i = 0; i < 1 << 26; i++) {
        bool bit = computeRandBit(&A, K, noiseAmplitude);
        if(!inmHealthCheckerAddBit(bit)) {
            printf("Failed health check!\n");
            resetStats();
            //return 1;
        } else if(inmNumBitsCounted > 0 && (inmNumBitsCounted & 0xfffff) == 0) {
            printf("Estimated entropy per bit: %f, estimated K: %f\n", inmHealthCheckerEstimateEntropyPerBit(),
                inmHealthCheckerEstimateK());
            checkLSBStats();
            reduceStatsIfNeeded();
            //resetStats();
        }
    }
    inmDumpStats();
    inmHealthCheckerStop();
    return 0;
}
