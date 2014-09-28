#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

/* This could be built with one opamp for the multiplier, a comparator with
   rail-to-rail outputs, and switches and caps and resistors.*/
static inline uint8_t updateA(long double *A, long double K, long double noise) {
    if(*A > 1.0) {
        *A = 1.0;
    } else if (*A < 0.0) {
        *A = 0.0;
    }
    *A += noise;
    if(*A > 0.5) {
        *A = K**A - (K-1);
        return 1;
    }
    *A += noise;
    *A = K**A;
    return 0;
}

static inline uint32_t computeRandBits(long double *A, uint32_t N, long double K, long double noiseAmplitude) {
    uint32_t bits = 0;
    for(uint32_t i = 0; i < N; i++) {
        //printf("%f\n", (double)*A);
        long double noise = noiseAmplitude*(((double)rand()/RAND_MAX) - 0.5);
        uint8_t result = updateA(A, K, noise);
        bits = (bits << 1) | result;
    }
    return bits;
}

static uint32_t computeMax(uint32_t *values, uint32_t numRuns, uint32_t N, long double K,
        long double noiseAmplitude, uint32_t *maxValue) {
    long double A = 0.0;

    memset(values, 0, (1 << N)*sizeof(uint32_t));
    computeRandBits(&A, 30, K, noiseAmplitude); // Randomize state
    for(uint32_t i = 0; i < numRuns; i++) {
        uint32_t value = computeRandBits(&A, N, K, noiseAmplitude); // Randomize state
        //printf("%u\n", value);
        values[value]++;
    }
    *maxValue = 0;
    uint32_t maxOccurance = 0;
    for(uint32_t i = 0; i < 1 << N; i++) {
        uint32_t occurance = values[i];
        if(occurance > maxOccurance) {
            maxOccurance = occurance;
            *maxValue = i;
        }
    }
    printf("Max occurance at K=%.2f: %u.  Most common value was %x\n", (double)K, maxOccurance, *maxValue);
    return maxOccurance;
}

int main() {
    uint32_t N = 18;
    long double noiseAmplitude = 1.0/(1 << 20);
    uint32_t numRuns = 1 << 26;
    //long double K = sqrt(2.0);
    long double K = 1.8;
    uint32_t *values = calloc(1 << N, sizeof(uint32_t));

    srand(time(NULL));
    uint32_t maxValue1, maxValue2;
    computeMax(values, numRuns, N, 2.0, noiseAmplitude, &maxValue1);
    computeMax(values, numRuns, N, 2.0, noiseAmplitude, &maxValue2);
    uint32_t max1 = values[maxValue1];
    printf("max1 = %u, total bits = %f\n", max1, log((double)numRuns/max1)/log(2.0));
    computeMax(values, numRuns, N, K, noiseAmplitude, &maxValue1);
    computeMax(values, numRuns, N, K, noiseAmplitude, &maxValue2);
    uint32_t max2 = values[maxValue1];
    printf("max2 = %u, total bits = %f\n", max2, log((double)numRuns/max2)/log(2.0));
    printf("Computed bits/clock: %f\n", log(numRuns/max2)/log(numRuns/max1));
    printf("Estimated bits/clock: %f\n", log(K)/log(2.0));
}
