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

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include "libinfnoise_private.h"

#define INM_MIN_DATA 80000u
#define INM_MIN_SAMPLE_SIZE 100u
#define INM_MAX_SEQUENCE 20u
#define INM_MAX_COUNT (1u << 14u)

// Print the tables of statistics.
void inmDumpStats(struct infnoise_health_state *hc) {
    uint32_t i;
    for(i = 0u; i < 1u << hc->N; i++) {
        printf("%x onesEven:%u zerosEven:%u onesOdd:%u zerosOdd:%u\n",
            i, hc->onesEven[i], hc->zerosEven[i], hc->onesOdd[i], hc->zerosOdd[i]);
    }
}

// Free memory used by the health check.
void inmHealthCheckStop(struct infnoise_health_state *hc) {
    if(hc->onesEven != NULL) {
        free(hc->onesEven);
        hc->onesEven = NULL;
    }
    if(hc->zerosEven != NULL) {
        free(hc->zerosEven);
        hc->zerosEven = NULL;
    }
    if(hc->onesOdd != NULL) {
        free(hc->onesOdd);
        hc->onesOdd = NULL;
    }
    if(hc->zerosOdd != NULL) {
        free(hc->zerosOdd);
        hc->zerosOdd = NULL;
    }
}

// Reset the statistics.
static void resetStats(struct infnoise_health_state *hc) {
    hc->numBitsSampled = 0u;
    hc->currentProbability = 1.0;
    hc->numBitsOfEntropy = 0u;
    hc->entropyLevel = 0u;
    hc->totalOnes = 0u;
    hc->totalZeros = 0u;
    hc->evenMisfires = 0u;
    hc->oddMisfires = 0u;
}

// Initialize the health check.  N is the number of bits used to predict the next bit.
// At least 8 bits must be used, and no more than 30.  In general, we should use bits
// large enough so that INM output will be uncorrelated with bits N samples back in time.
bool inmHealthCheckStart(struct infnoise_health_state *hc, uint8_t N, double K, bool debug) {
    if(N < 1u || N > 30u) {
        return false;
    }
    hc->debug = debug;
    hc->numBitsOfEntropy = 0u;
    hc->currentProbability = 1.0;
    hc->K = K;
    hc->N = N;
    hc->prevBits = 0u;
    hc->onesEven = calloc((size_t)1u << N, sizeof(*hc->onesEven));
    hc->zerosEven = calloc((size_t)1u << N, sizeof(*hc->zerosEven));
    hc->onesOdd = calloc((size_t)1u << N, sizeof(*hc->onesOdd));
    hc->zerosOdd = calloc((size_t)1u << N, sizeof(*hc->zerosOdd));
    hc->expectedEntropyPerBit = log(K)/log(2.0);
    hc->totalBits = 0u;
    hc->prevBit = false;
    hc->numSequentialZeros = 0u;
    hc->numSequentialOnes = 0u;
    resetStats(hc);
    if(hc->onesEven == NULL || hc->zerosEven == NULL || hc->onesOdd == NULL || hc->zerosOdd == NULL) {
        inmHealthCheckStop(hc);
        return false;
    }
    return true;
}

// If running continuously, it is possible to start overflowing the 32-bit counters for
// zeros and ones.  Check for this, and scale the stats if needed.
static void scaleStats(struct infnoise_health_state *hc) {
    uint32_t i;
    for(i = 0u; i < (1u << hc->N); i++) {
        hc->zerosEven[i] >>= 1u;
        hc->onesEven[i] >>= 1u;
        hc->zerosOdd[i] >>= 1u;
        hc->onesOdd[i] >>= 1u;
    }
}

// If running continuously, it is possible to start overflowing the 32-bit counters for
// zeros and ones.  Check for this, and scale the stats if needed.
static void scaleEntropy(struct infnoise_health_state *hc) {
    if(hc->numBitsSampled == INM_MIN_DATA) {
        hc->numBitsOfEntropy >>= 1u;
        hc->numBitsSampled >>= 1u;
        hc->evenMisfires >>= 1u;
        hc->oddMisfires >>= 1u;
    }
}

// If running continuously, it is possible to start overflowing the 32-bit counters for
// zeros and ones.  Check for this, and scale the stats if needed.
static void scaleZeroOneCounts(struct infnoise_health_state *hc) {
    uint64_t maxVal = hc->totalZeros >= hc->totalOnes? hc->totalZeros : hc->totalOnes;
    if(maxVal == INM_MIN_DATA) {
        hc->totalZeros >>= 1u;
        hc->totalOnes >>= 1u;
    }
}

// This should be called for each bit generated.
bool inmHealthCheckAddBit(struct infnoise_health_state *hc, bool evenBit, bool oddBit, bool even) {
    bool bit;
    if(even) {
        bit = evenBit;
        hc->evenMisfires += (evenBit != hc->prevEven);
    } else {
        bit = oddBit;
        hc->oddMisfires += (oddBit != hc->prevOdd);
    }
    hc->prevEven = evenBit;
    hc->prevOdd = oddBit;
    hc->totalBits++;
    if(hc->debug && (hc->totalBits & 0xfffffllu) == 0u) {
        fprintf(stderr, "Generated %llu bits.  %s to use data.  Estimated entropy per bit: %f, estimated K: %f\n",
            (long long)hc->totalBits, inmHealthCheckOkToUseData(hc)? "OK" : "NOT OK", inmHealthCheckEstimateEntropyPerBit(hc),
            inmHealthCheckEstimateK(hc));
        fprintf(stderr, "num1s:%f%%, even misfires:%f%%, odd misfires:%f%%\n",
            hc->totalOnes*100.0/(hc->totalZeros + hc->totalOnes),
            hc->evenMisfires*100.0/hc->numBitsSampled, hc->oddMisfires*100.0/hc->numBitsSampled);
        fflush(stderr);
    }
    hc->prevBits = (hc->prevBits << 1) & ((1 << hc->N)-1);
    if(hc->prevBit) {
        hc->prevBits |= 1;
    }
    hc->prevBit = bit;
    if(hc->numBitsSampled > 100u) {
        if(bit) {
            hc->totalOnes++;
            hc->numSequentialOnes++;
            hc->numSequentialZeros = 0u;
            if(hc->numSequentialOnes > INM_MAX_SEQUENCE) {
                fprintf(stderr, "Maximum sequence of %d 1's exceeded\n", INM_MAX_SEQUENCE);
                hc->numSequentialOnes = 0u;
                return false;
            }
        } else {
            hc->totalZeros++;
            hc->numSequentialZeros++;
            hc->numSequentialOnes = 0u;
            if(hc->numSequentialZeros > INM_MAX_SEQUENCE) {
                fprintf(stderr, "Maximum sequence of %d 0's exceeded\n", INM_MAX_SEQUENCE);
                hc->numSequentialZeros = 0u;
                return false;
            }
        }
    }
    uint32_t zeros, ones;
    if(even) {
        zeros = hc->zerosEven[hc->prevBits];
        ones = hc->onesEven[hc->prevBits];
    } else {
        zeros = hc->zerosOdd[hc->prevBits];
        ones = hc->onesOdd[hc->prevBits];
    }
    uint32_t total = zeros + ones;
    if(bit) {
        if(ones != 0u) {
            hc->currentProbability *= (double)ones/total;
        }
    } else {
        if(zeros != 0u) {
            hc->currentProbability *= (double)zeros/total;
        }
    }
    while(hc->currentProbability <= 0.5) {
        hc->currentProbability *= 2.0;
        hc->numBitsOfEntropy++;
        if(inmHealthCheckOkToUseData(hc)) {
            hc->entropyLevel++;
        }
    }
    //printf("probability:%f\n", hc->currentProbability);
    hc->numBitsSampled++;
    if(bit) {
        if(even) {
            hc->onesEven[hc->prevBits]++;
            if(hc->onesEven[hc->prevBits] == INM_MAX_COUNT) {
                scaleStats(hc);
            }
        } else {
            hc->onesOdd[hc->prevBits]++;
            if(hc->onesOdd[hc->prevBits] == INM_MAX_COUNT) {
                scaleStats(hc);
            }
        }
    } else {
        if(even) {
            hc->zerosEven[hc->prevBits]++;
            if(hc->zerosEven[hc->prevBits] == INM_MAX_COUNT) {
                scaleStats(hc);
            }
        } else {
            hc->zerosOdd[hc->prevBits]++;
            if(hc->zerosOdd[hc->prevBits] == INM_MAX_COUNT) {
                scaleStats(hc);
            }
        }
    }
    scaleEntropy(hc);
    scaleZeroOneCounts(hc);
    return true;
}

// Once we have enough samples, we know that entropyPerBit = log(K)/log(2), so
// K must be 2^entryopPerBit.
double inmHealthCheckEstimateK(struct infnoise_health_state *hc) {
    double entropyPerBit = (double)hc->numBitsOfEntropy/hc->numBitsSampled;
    return pow(2.0, entropyPerBit);
}

// Once we have enough samples, we know that entropyPerBit = log(K)/log(2), so
// K must be 2^entryopPerBit.
double inmHealthCheckEstimateEntropyPerBit(struct infnoise_health_state *hc) {
    return (double)hc->numBitsOfEntropy/hc->numBitsSampled;
}

// Return true if the health checker has enough data to verify proper operation of the INM.
bool inmHealthCheckOkToUseData(struct infnoise_health_state *hc) {
    double entropy = inmHealthCheckEstimateEntropyPerBit(hc);
    return hc->totalBits >= INM_MIN_DATA && entropy*INM_ACCURACY >= hc->expectedEntropyPerBit &&
        entropy/INM_ACCURACY <= hc->expectedEntropyPerBit;
}

// Just return the entropy level added so far in bytes;
uint32_t inmGetEntropyLevel(struct infnoise_health_state *hc) {
    return hc->entropyLevel;
}

// Reduce the entropy level by numBytes.
void inmClearEntropyLevel(struct infnoise_health_state *hc) {
    hc->entropyLevel = 0u;
}

// Check that the entropy of the last group of bits was high enough for use.
bool inmEntropyOnTarget(struct infnoise_health_state *hc, uint32_t entropy, uint32_t numBits) {
    uint32_t expectedEntropy = (uint32_t)(numBits*hc->expectedEntropyPerBit);
    return expectedEntropy < entropy*INM_ACCURACY;
}

#ifdef TEST_HEALTHCHECK
#include "infnoise.h"

// Compare the ability to predict with 1 fewer bits and see how much less accurate we are.
static void checkLSBStatsForNBits(struct infnoise_health_state *hc, uint8_t N) {
    uint32_t i, j;
    uint32_t totalGuesses = 0u;
    uint32_t totalRight = 0.0;
    for(i = 0u; i < (1u << N); i++) {
        uint32_t zeros = 0u;
        uint32_t ones = 0u;
        for(j = 0u; j < (1u << (hc->N - N)); j++) {
            uint32_t pos = i + j*(1u << N);
            zeros += hc->zerosEven[pos];
            ones += hc->onesEven[pos];
        }
        if(zeros >= ones) {
            totalRight += zeros;
        } else {
            totalRight += ones;
        }
        totalGuesses += zeros + ones;
    }
    printf("Probability of guessing correctly with %u bits: %f\n", N, (double)totalRight/totalGuesses);
}

// Compare the ability to predict with 1 fewer bits and see how much less accurate we are.
static void checkLSBStats(struct infnoise_health_state *hc) {
    uint32_t N;
    for(N = 1u; N <= hc->N; N++) {
        checkLSBStatsForNBits(hc, N);
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

static void initOpts(struct opt_struct *opts) {
    opts->outputMultiplier = 0u;
    opts->daemon =
        opts->debug =
        opts->devRandom =
        opts->noOutput =
        opts->listDevices =
        opts->raw = false;
    opts->version = false;
    opts->help = false;
    opts->none = false;
    opts->pidFileName =
        opts->serial = NULL;
}

int main() {
    struct opt_struct opts;
    initOpts(&opts);
    //double K = sqrt(2.0);
    double K = 1.82;
    uint8_t N = 16u;
    struct infnoise_health_state hc;
    memset(&hc, 0, sizeof(hc));
    inmHealthCheckStart(&hc, N, K, false);
    srand(time(NULL));
    double A = (double)rand()/RAND_MAX; // Simulating INM
    double noiseAmplitude = 1.0/(1u << 10);
    uint32_t i;

    for(i = 0u; i < 32u; i++) {
        // Throw away some initial bits.
        computeRandBit(&A, K, noiseAmplitude);
    }
    bool evenBit = false;
    bool oddBit = false;
    for(i = 0u; i < 1u << 28u; i++) {
        bool bit = computeRandBit(&A, K, noiseAmplitude);
        bool even = !(i & 1);
        if(even) {
            evenBit = bit;
        } else {
            oddBit = bit;
        }
        if(!inmHealthCheckAddBit(&hc, evenBit, oddBit, even)) {
            fprintf(stderr, "Failed health check!\n");
            return 1;
        }
        if(hc.totalBits > 0u && (hc.totalBits & 0xfffffff) == 0) {
            printf("Estimated entropy per bit: %f, estimated K: %f\n", inmHealthCheckEstimateEntropyPerBit(&hc),
                inmHealthCheckEstimateK(&hc));
            checkLSBStats(&hc);
        }
    }
    inmDumpStats(&hc);
    inmHealthCheckStop(&hc);
    return 0;
}
#endif
