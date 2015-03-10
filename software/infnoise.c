/* Driver for the Infinite Noise Multiplier USB stick */

// Required to include clock_gettime
#define _POSIX_C_SOURCE 200809L

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <ftdi.h>
#include "infnoise.h"
#include "KeccakF-1600-interface.h"

// The FT240X has a 512 byte buffer.  Must be multiple of 64
// We also write this in one go to the Keccak sponge, which is at most 1600 bits
#define BUFLEN 512

// This is how many previous bits are used to predict the next bit from the INM
#define PREDICTION_BITS 14

// This is the maximum time we allow to pass to perform the I/O operations, since long
// delays can reduce entropy from the INM.
#define MAX_MICROSEC_FOR_SAMPLES 5000

// This is the gain of each of the two op-amp stages in the INM
#define DESIGN_K 1.84

// This defines which pins on the FT240X are used
#ifdef VERSION1

// The V1 version is the original raw board with the edge connector instead of a real USB plug
#define COMP1 2
#define COMP2 0
#define SWEN1 4
#define SWEN2 1

#else

// This is the production version with a real USB plug
#define COMP1 1
#define COMP2 4
#define SWEN1 2
#define SWEN2 0

#endif

// The remaining 8 bits are driven with 0 .. 15 to help track the cause of misfires
#define ADDR0 3
#define ADDR1 5
#define ADDR2 6
#define ADDR3 7

// All data bus bits of the FT240X are outputs, except COMP1 and COMP2
#define MASK (0xff & ~(1 << COMP1) & ~(1 << COMP2))

// Convert an address value 0 to 15 to an 8-bit value using ADDR0 .. ADDR3.
static uint8_t makeAddress(uint8_t addr) {
    uint8_t value = 0;
    if(addr & 1) {
        value |= 1 << ADDR0;
    }
    if(addr & 2) {
        value |= 1 << ADDR1;
    }
    if(addr & 4) {
        value |= 1 << ADDR2;
    }
    if(addr & 8) {
        value |= 1 << ADDR3;
    }
    return value;
}

// Extract a value form 0 to 15 from the ADDR0 .. ADDR3 bits.
static uint8_t extractAddress(uint8_t value) {
    uint8_t addr = 0;
    if(value & (1 << ADDR0)) {
        addr |= 1;
    }
    if(value & (1 << ADDR1)) {
        addr |= 2;
    }
    if(value & (1 << ADDR2)) {
        addr |= 4;
    }
    if(value & (1 << ADDR3)) {
        addr |= 8;
    }
    return addr;
}


// Extract the INM output from the data received.  Basically, either COMP1 or COMP2
// changes, not both, so alternate reading bits from them.  We get 1 INM bit of output
// per byte read.  Feed bits from the INM to the health checker.  Return the expected
// bits of entropy.
static uint32_t extractBytes(uint8_t *bytes, uint8_t *inBuf, bool raw) {
    inmClearEntropyLevel();
    //printf("New batch\n");
    uint32_t i;
    for(i = 0; i < BUFLEN/8; i++) {
        uint32_t j;
        uint8_t byte = 0;
        for(j = 0; j < 8; j++) {
            //printf("%x ", inBuf[i*8 + j] & ~MASK);
            uint8_t val = inBuf[i*8 + j];
            uint8_t evenBit = (val >> COMP2) & 1;
            uint8_t oddBit = (val >> COMP1) & 1;
            bool even = j & 1; // Use the even bit if j is odd
            uint8_t bit = even? evenBit : oddBit;
            byte = (byte << 1) | bit;
            // This is a good place to feed the bit from the INM to the health checker.
            uint8_t addr = extractAddress(val);
            //printf("Address: %u, adding evenBit:%u oddBit:%u even:%u\n", addr, evenBit, oddBit, even);
            if(!inmHealthCheckAddBit(evenBit, oddBit, even, addr)) {
                fputs("Health check of Infinite Noise Multiplier failed!\n", stderr);
                exit(1);
            }
        }
        //printf("extracted byte:%x\n", byte);
        bytes[i] = byte;
    }
    return inmGetEntropyLevel();
}

// Write the bytes to either stdout, or /dev/random.  Use the lower of the measured
// entropy and the provable lower bound on average entropy.
static void outputBytes(uint8_t *bytes, uint32_t length, uint32_t entropy, bool writeDevRandom) {
    if(entropy > inmExpectedEntropyPerBit*BUFLEN/INM_ACCURACY) {
        entropy = inmExpectedEntropyPerBit*BUFLEN/INM_ACCURACY;
    }
    if(!writeDevRandom) {
        if(fwrite(bytes, 1, length, stdout) != length) {
            fputs("Unable to write output from Infinite Noise Multiplier\n", stderr);
            exit(1);
        }
    } else {
        inmWaitForPoolToHaveRoom();
        inmWriteEntropyToPool(bytes, length, entropy);
    }
}

// Whiten the output, if requested, with a Keccak sponge.  Output bytes only if the health
// checker says it's OK.  Using outputMultiplier > 1 is a nice way to generate a lot more
// cryptographically secure pseudo-random data than the INM generates.  This allows a user
// to generate hundreds of MiB per second if needed, for use as cryptogrpahic keys.
static void processBytes(uint8_t *keccakState, uint8_t *bytes, uint32_t entropy, bool raw,
        bool writeDevRandom, uint32_t outputMultiplier) {
    if(raw) {
        // In raw mode, we just output raw data from the INM.
        outputBytes(bytes, BUFLEN/8, entropy, writeDevRandom);
        return;
    }
    // Note that BUFLEN has to be less than 1600 by enough to make the sponge secure,
    // since outputing all 1600 bits would tell an attacker the Keccak state, allowing
    // him to predict any further output, when outputMultiplier > 1, until the next call
    // to processBytes.  All 512 bits are absorbed before sqeezing data out to insure that
    // we instantly recover (reseed) from a state compromise, which is when an attacker
    // gets a snapshot of the keccak state.  BUFLEN must be a multiple of 64, since
    // Keccak-1600 uses 64-bit "lanes".
    KeccakAbsorb(keccakState, bytes, BUFLEN/64);
    uint8_t dataOut[16*8];
    while(outputMultiplier > 0) {
        // Write up to 1024 bits at a time.
        uint32_t numLanes = 16;
        if(outputMultiplier < 4) {
            numLanes = outputMultiplier*4;
        }
        KeccakExtract(keccakState, dataOut, numLanes);
        // Extract does not do a permute, so do it here.
        KeccakPermutation(keccakState);
        uint32_t entropyThisTime = entropy;
        if(entropyThisTime > numLanes*64) {
            entropyThisTime = numLanes*64;
        }
        outputBytes(dataOut, numLanes*8, entropyThisTime, writeDevRandom);
        outputMultiplier -= numLanes/4;
        entropy -= entropyThisTime;
    }
}

// Initialize the Infinite Noise Multiplier USB ineterface.
static bool initializeUSB(struct ftdi_context *ftdic, char **message) {
    *message = NULL;

    // Initialize FTDI context
    ftdi_init(ftdic);
    // Open FTDI device based on FT240X vendor & product IDs
    if(ftdi_usb_open(ftdic, 0x0403, 0x6015) < 0) {
        *message = "Can't find Infinite Noise Multiplier\n";
        return false;
    }

    // Set high baud rate
    int rc = ftdi_set_baudrate(ftdic, 30000);
    if(rc == -1) {
        *message = "Invalid baud rate\n";
        return false;
    } else if(rc == -2) {
        *message = "Setting baud rate failed\n";
        return false;
    } else if(rc == -3) {
        *message = "Infinite Noise Multiplier unavailable\n";
        return false;
    }

    // Enable syncrhonous bitbang mode
    rc = ftdi_set_bitmode(ftdic, MASK, BITMODE_SYNCBB);
    if(rc == -1) {
        *message = "Can't enable bit-bang mode\n";
        return false;
    } else if(rc == -2) {
        *message = "Infinite Noise Multiplier unavailable\n";
        return false;
    }
    
    // Just test to see that we can write and read.
    uint8_t buf[64] = {0,};
    if(ftdi_write_data(ftdic, buf, 64) != 64) {
        *message = "USB write failed\n";
        return false;
    }
    if(ftdi_read_data(ftdic, buf, 64) != 64) {
        *message = "USB read failed\n";
        return false;
    }
    return true;
}

// Return the differnece in the times as a double in microseconds.
static double diffTime(struct timespec *start, struct timespec *end) {
    uint32_t seconds = end->tv_sec - start->tv_sec;
    int32_t nanoseconds = end->tv_nsec - start->tv_nsec;
    return seconds*1e6 + nanoseconds/1000.0;
}

int main(int argc, char **argv)
{
    struct ftdi_context ftdic;
    bool raw = false;
    bool debug = false;
    bool writeDevRandom = false;
    bool noOutput = false;
    uint32_t outputMultiplier = 1; // 256 bits out for every 512 read
    uint32_t xArg;
    bool multiplierAssigned = false;

    // Process arguments
    for(xArg = 1; xArg < argc; xArg++) {
        if(!strcmp(argv[xArg], "--raw")) {
            raw = true;
        } else if(!strcmp(argv[xArg], "--debug")) {
            debug = true;
        } else if(!strcmp(argv[xArg], "--dev-random")) {
            writeDevRandom = true;
        } else if(!strcmp(argv[xArg], "--no-output")) {
            noOutput = true;
        } else if(!strcmp(argv[xArg], "--multiplier") && xArg+1 < argc) {
            xArg++;
            multiplierAssigned = true;
            outputMultiplier = atoi(argv[xArg]);
            if(outputMultiplier == 0) {
                fputs("Multiplier must be > 0\n", stderr);
                return 1;
            }
        } else {
            fputs("Usage: infnoise [options]\n"
                            "Options are:\n"
                            "    --debug - turn on some debug output\n"
                            "    --dev-random - write entropy to /dev/random instead of stdout\n"
                            "    --raw - do not whiten the output\n"
                            "    --multiplier <value> - write 256 bits * value for each 512 bits written to\n"
                            "      the Keccak sponge\n"
                            "    --no-output - do not write random output data\n", stderr);
            return 1;
        }
    }

    if(!multiplierAssigned && writeDevRandom) {
        outputMultiplier = 2; // Don't throw away entropy when writing to /dev/random unless told to do so
    }

    if(writeDevRandom) {
        inmWriteEntropyStart(BUFLEN/8, debug);
    }
    if(!inmHealthCheckStart(PREDICTION_BITS, DESIGN_K, debug)) {
        fputs("Can't intialize health checker\n", stderr);
        return 1;
    }
    uint8_t keccakState[KeccakPermutationSizeInBytes];
    KeccakInitializeState(keccakState);

    char *message;
    if(!initializeUSB(&ftdic, &message)) {
        // Sometimes have to do it twice - not sure why
        //ftdi_usb_close(&ftdic);
        if(!initializeUSB(&ftdic, &message)) {
            fputs(message, stderr);
            return 1;
        }
    }

    // Endless loop: set SW1EN and SW2EN alternately
    uint32_t i;
    uint8_t outBuf[BUFLEN], inBuf[BUFLEN];
    for(i = 0; i < BUFLEN; i++) {
        // Alternate Ph1 and Ph2 - maybe should have both off in between
        outBuf[i] = i & 1?  (1 << SWEN2) : (1 << SWEN1);
        outBuf[i] |= makeAddress(i & 0xf);
    }

    uint64_t good = 0, bad = 0;
    while(true) {
        struct timespec start;
        clock_gettime(CLOCK_REALTIME, &start);

        if(ftdi_write_data(&ftdic, outBuf, BUFLEN) != BUFLEN) {
            fputs("USB write failed\n", stderr);
            return -1;
        }
        if(ftdi_read_data(&ftdic, inBuf, BUFLEN) != BUFLEN) {
            fputs("USB read failed\n", stderr);
            return -1;
        }
        struct timespec end;
        clock_gettime(CLOCK_REALTIME, &end);
        uint32_t us = diffTime(&start, &end);
        //printf("diffTime:%u us\n", us);
        if(us <= MAX_MICROSEC_FOR_SAMPLES) {
            uint8_t bytes[BUFLEN/8];
            uint32_t entropy = extractBytes(bytes, inBuf, raw);
            if(!noOutput && inmHealthCheckOkToUseData() && inmEntropyOnTarget(entropy, BUFLEN)) {
                processBytes(keccakState, bytes, entropy, raw, writeDevRandom, outputMultiplier);
            }
            good++;
        } else {
            bad++;
        }
        //if(((good + bad) & 0xff) == 0) {
            //printf("Good %lu, bad %lu\n", good, bad);
        //}
    }
    return 0;
}
