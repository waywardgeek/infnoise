/* Driver for the Infinite Noise Multiplier USB stick */

// Required to include clock_gettime
#define _POSIX_C_SOURCE 200809L

#include <stdlib.h>
#include <share.h>

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "VisualStudio\ftdi\ftd2xx.h"
#include "infnoise.h"
#include "Keccak\KeccakF-1600-interface.h"

// Pipes in Windows basically don't work, so if you want output from a program to redirect to a file
// you are forced to write to the file directly, rather than do infnoise > foo.
FILE *outFile;

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
            uint8_t bit = even? oddBit : evenBit;
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
        entropy = (uint32_t)(inmExpectedEntropyPerBit*BUFLEN/INM_ACCURACY);
    }
    if(!writeDevRandom) {
        if(fwrite(bytes, 1, length, outFile) != length) {
            fputs("Unable to write output from Infinite Noise Multiplier\n", stderr);
            exit(1);
        }
		fflush(outFile);
    } else {
		fprintf(stderr, "/dev/random not supported in Windows");
		exit(1);
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
static bool initializeUSB(FT_HANDLE *ftdic, char **message) {
    *message = NULL;

    // Open FTDI device based on FT240X vendor & product IDs
	if (FT_Open(0, ftdic) != FT_OK) {
		*message = "Can't find Infinite Noise Multiplier\n";
		return false;
	}

    // Set high baud rate
	if (FT_SetBaudRate(*ftdic, 30000) != FT_OK) {
        *message = "Setting baud rate failed\n";
        return false;
    }

    // Enable syncrhonous bitbang mode
	if (FT_SetBitMode(*ftdic, MASK, BITMODE_SYNCBB) != FT_OK) {
        *message = "Can't enable bit-bang mode\n";
        return false;
    }
    
    // Just test to see that we can write and read.
    uint8_t buf[64] = {0,};
	uint32_t bytesWritten;
    if(FT_Write(*ftdic, buf, 64, &bytesWritten) != FT_OK || bytesWritten != 64) {
        *message = "USB write failed\n";
        return false;
    }
	uint32_t bytesRead;
    if(FT_Read(*ftdic, buf, 64, &bytesRead) != FT_OK || bytesRead != 64) {
        *message = "USB read failed\n";
        return false;
    }
    return true;
}

/*
// Return the differnece in the times as a double in microseconds.
static double diffTime(struct timespec *start, struct timespec *end) {
    uint32_t seconds = end->tv_sec - start->tv_sec;
    int32_t nanoseconds = end->tv_nsec - start->tv_nsec;
    return seconds*1e6 + nanoseconds/1000.0;
}
*/

int main(int argc, char **argv)
{
    FT_HANDLE ftdic;
    bool raw = false;
    bool debug = false;
    bool writeDevRandom = false;
    bool noOutput = false;
    uint32_t outputMultiplier = 2;
    uint32_t xArg;

    // Process arguments
    for(xArg = 1; xArg < (uint32_t)(argc-1); xArg++) {
        if(!strcmp(argv[xArg], "--raw")) {
            raw = true;
        } else if(!strcmp(argv[xArg], "--debug")) {
            debug = true;
        } else if(!strcmp(argv[xArg], "--dev-random")) {
            writeDevRandom = true;
        } else if(!strcmp(argv[xArg], "--no-output")) {
            noOutput = true;
        } else if(!strcmp(argv[xArg], "--multiplier") && xArg+1 < (uint32_t)argc) {
            xArg++;
            outputMultiplier = atoi(argv[xArg]);
            if(outputMultiplier == 0) {
                fputs("Multiplier must be > 0\n", stderr);
                return 1;
            }
		} else {
            fputs("Usage: infnoise [options] outFile\n"
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
	if (argc < 2) {
		fprintf(stderr, "No output file specified\n");
		return 1;
	}
	outFile = _fsopen(argv[xArg], "wb", _SH_DENYWR);
	if(outFile == NULL) {
		fprintf(stderr, "Unable to open file %s\n", argv[xArg]);
		return 1;
	}

/*    if(writeDevRandom) {
        inmWriteEntropyStart(BUFLEN/8, debug);
    }
*/
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
/*
        struct timespec start;
        clock_gettime(CLOCK_REALTIME, &start);
*/
		uint32_t numBytes;
        if(FT_Write(ftdic, outBuf, BUFLEN, &numBytes) != FT_OK || numBytes != BUFLEN) {
            fputs("USB write failed\n", stderr);
            return -1;
        }
        if(FT_Read(ftdic, inBuf, BUFLEN, &numBytes) != FT_OK || numBytes != BUFLEN) {
            fputs("USB read failed\n", stderr);
            return -1;
        }
/*
        struct timespec end;
        clock_gettime(CLOCK_REALTIME, &end);
        uint32_t us = diffTime(&start, &end);
        //printf("diffTime:%u us\n", us);
*/
//        if(us <= MAX_MICROSEC_FOR_SAMPLES) {
            uint8_t bytes[BUFLEN/8];
            uint32_t entropy = extractBytes(bytes, inBuf, raw);
            if(!noOutput && inmHealthCheckOkToUseData() && inmEntropyOnTarget(entropy, BUFLEN)) {
                processBytes(keccakState, bytes, entropy, raw, writeDevRandom, outputMultiplier);
            }
            good++;
  /*      } else {
            bad++;
        }
*/
        //if(((good + bad) & 0xff) == 0) {
            //printf("Good %lu, bad %lu\n", good, bad);
        //}
			fflush(stdout);
			fflush(stderr);
    }
    return 0;
}
