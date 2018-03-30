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
#include "infnoise.h"

#define INM_MIN_DATA 80000u
#define INM_MIN_SAMPLE_SIZE 100u
#define INM_MAX_SEQUENCE 20u
#define INM_MAX_COUNT (1u << 14u)

double inmK, inmExpectedEntropyPerBit;

static uint8_t inmN;
static uint32_t inmPrevBits;
static uint32_t inmNumBitsSampled;
static uint32_t *inmOnesEven, *inmZerosEven;
static uint32_t *inmOnesOdd, *inmZerosOdd;
// The total probability of generating the string of states we did is
// 1/(2^inmNumBitsOfEntropy * inmCurrentProbability).
static uint32_t inmNumBitsOfEntropy;
static double inmCurrentProbability;
static uint64_t inmTotalBits;
static bool inmPrevBit;
static uint32_t inmEntropyLevel;
static uint32_t inmNumSequentialZeros, inmNumSequentialOnes;
static uint32_t inmTotalOnes, inmTotalZeros;
static uint32_t inmEvenMisfires, inmOddMisfires;
static bool inmPrevEven, inmPrevOdd;
static bool inmDebug;

// Print the tables of statistics.
void inmDumpStats(void) {
    uint32_t i;
    for(i = 0u; i < 1u << inmN; i++) {
        printf("%x onesEven:%u zerosEven:%u onesOdd:%u zerosOdd:%u\n",
            i, inmOnesEven[i], inmZerosEven[i], inmOnesOdd[i], inmZerosOdd[i]);
    }
}

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
    inmNumBitsSampled = 0u;
    inmCurrentProbability = 1.0;
    inmNumBitsOfEntropy = 0u;
    inmEntropyLevel = 0u;
    inmTotalOnes = 0u;
    inmTotalZeros = 0u;
    inmEvenMisfires = 0u;
    inmOddMisfires = 0u;
}

// Initialize the health check.  N is the number of bits used to predict the next bit.
// At least 8 bits must be used, and no more than 30.  In general, we should use bits
// large enough so that INM output will be uncorrelated with bits N samples back in time.
bool inmHealthCheckStart(uint8_t N, double K, struct opt_struct *opts) {
    if(N < 1u || N > 30u) {
        return false;
    }
    inmDebug = opts->debug;
    inmNumBitsOfEntropy = 0u;
    inmCurrentProbability = 1.0;
    inmK = K;
    inmN = N;
    inmPrevBits = 0u;
    inmOnesEven = calloc(1u << N, sizeof(uint32_t));
    inmZerosEven = calloc(1u << N, sizeof(uint32_t));
    inmOnesOdd = calloc(1u << N, sizeof(uint32_t));
    inmZerosOdd = calloc(1u << N, sizeof(uint32_t));
    inmExpectedEntropyPerBit = log(K)/log(2.0);
    inmTotalBits = 0u;
    inmPrevBit = false;
    inmNumSequentialZeros = 0u;
    inmNumSequentialOnes = 0u;
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
    for(i = 0u; i < (1u << inmN); i++) {
        inmZerosEven[i] >>= 1u;
        inmOnesEven[i] >>= 1u;
        inmZerosOdd[i] >>= 1u;
        inmOnesOdd[i] >>= 1u;
    }
}

// If running continuously, it is possible to start overflowing the 32-bit counters for
// zeros and ones.  Check for this, and scale the stats if needed.
static void scaleEntropy(void) {
    if(inmNumBitsSampled == INM_MIN_DATA) {
        inmNumBitsOfEntropy >>= 1u;
        inmNumBitsSampled >>= 1u;
        inmEvenMisfires >>= 1u;
        inmOddMisfires >>= 1u;
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
bool inmHealthCheckAddBit(bool evenBit, bool oddBit, bool even) {
    bool bit;
    if(even) {
        bit = evenBit;
        if(evenBit != inmPrevEven) {
            inmEvenMisfires++;
            //printf("even misfire\n");
        }
    } else {
        bit = oddBit;
        if(oddBit != inmPrevOdd) {
            inmOddMisfires++;
            //printf("odd misfire\n");
        }
    }
    inmPrevEven = evenBit;
    inmPrevOdd = oddBit;
    inmTotalBits++;
    if(inmDebug && (inmTotalBits & 0xfffffllu) == 0u) {
        fprintf(stderr, "Generated %llu bits.  %s to use data.  Estimated entropy per bit: %f, estimated K: %f\n",
            (long long)inmTotalBits, inmHealthCheckOkToUseData()? "OK" : "NOT OK", inmHealthCheckEstimateEntropyPerBit(),
            inmHealthCheckEstimateK());
        fprintf(stderr, "num1s:%f%%, even misfires:%f%%, odd misfires:%f%%\n",
            inmTotalOnes*100.0/(inmTotalZeros + inmTotalOnes),
            inmEvenMisfires*100.0/inmNumBitsSampled, inmOddMisfires*100.0/inmNumBitsSampled);
		fflush(stderr);
	}
    inmPrevBits = (inmPrevBits << 1) & ((1 << inmN)-1);
    if(inmPrevBit) {
        inmPrevBits |= 1;
    }
    inmPrevBit = bit;
    if(inmNumBitsSampled > 100u) {
        if(bit) {
            inmTotalOnes++;
            inmNumSequentialOnes++;
            inmNumSequentialZeros = 0u;
            if(inmNumSequentialOnes > INM_MAX_SEQUENCE) {
                fprintf(stderr, "Maximum sequence of %d 1's exceeded\n", INM_MAX_SEQUENCE);
                exit(1);
            }
        } else {
            inmTotalZeros++;
            inmNumSequentialZeros++;
            inmNumSequentialOnes = 0u;
            if(inmNumSequentialZeros > INM_MAX_SEQUENCE) {
                fprintf(stderr, "Maximum sequence of %d 0's exceeded\n", INM_MAX_SEQUENCE);
                exit(1);
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
        if(ones != 0u) {
            inmCurrentProbability *= (double)ones/total;
        }
    } else {
        if(zeros != 0u) {
            inmCurrentProbability *= (double)zeros/total;
        }
    }
    while(inmCurrentProbability <= 0.5) {
        inmCurrentProbability *= 2.0;
        inmNumBitsOfEntropy++;
        if(inmHealthCheckOkToUseData()) {
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
    scaleEntropy();
    scaleZeroOneCounts();
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
    double entropy = inmHealthCheckEstimateEntropyPerBit();
    return inmTotalBits >= INM_MIN_DATA && entropy*INM_ACCURACY >= inmExpectedEntropyPerBit &&
        entropy/INM_ACCURACY <= inmExpectedEntropyPerBit;
}

// Just return the entropy level added so far in bytes;
uint32_t inmGetEntropyLevel(void) {
    return inmEntropyLevel;
}

// Reduce the entropy level by numBytes.
void inmClearEntropyLevel(void) {
    inmEntropyLevel = 0u;
}

// Check that the entropy of the last group of bits was high enough for use.
bool inmEntropyOnTarget(uint32_t entropy, uint32_t numBits) {
    uint32_t expectedEntropy = (uint32_t)(numBits*inmExpectedEntropyPerBit);
    return expectedEntropy < entropy*INM_ACCURACY;
}

#ifdef TEST_HEALTHCHECK

// Compare the ability to predict with 1 fewer bits and see how much less accurate we are.
static void checkLSBStatsForNBits(uint8_t N) {
    uint32_t i, j;
    uint32_t totalGuesses = 0u;
    uint32_t totalRight = 0.0;
    for(i = 0u; i < (1u << N); i++) {
        uint32_t total = 0u;
        uint32_t zeros = 0u;
        uint32_t ones = 0u;
        for(j = 0u; j < (1u << (inmN - N)); j++) {
            uint32_t pos = i + j*(1u << N);
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
    for(N = 1u; N <= inmN; N++) {
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
    inmHealthCheckStart(N, K, &opts);
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
        if(!inmHealthCheckAddBit(evenBit, oddBit, even)) {
            fprintf(stderr, "Failed health check!\n");
            return 1;
        }
        if(inmTotalBits > 0u && (inmTotalBits & 0xfffffff) == 0) {
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
