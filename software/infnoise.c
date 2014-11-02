/* Driver for the Infinite Noise Multiplier USB stick */

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <ftdi.h>
#include "infnoise.h"
#include "KeccakF-1600-interface.h"

// The FT240X has a 512 byte buffer.  Must be multiple of 64
//#define BUFLEN 512
#define BUFLEN (64*8)
#define PREDICTION_BITS 14
#define LINUX_POOL_SIZE (4096/8)

#ifdef VERSION1
#define DESIGN_K 1.82
#define COMP1 2
#define COMP2 0
#define SWEN1 4
#define SWEN2 1
#else
#define DESIGN_K 1.82
//#define DESIGN_K 1.736
#define COMP1 1
#define COMP2 4
#define SWEN1 2
#define SWEN2 0
#endif

// Add bits are outputs, except COMP1 and COMP2
#define MASK (0xff & ~(1 << COMP1) & ~(1 << COMP2))

// Extract the INM output from the data received.  Basically, either COMP1 or COMP2
// changes, not both, so alternate reading bits from them.  We get 1 INM bit of output
// per byte read.  Feed bits from the INM to the health checker.  Return the expected
// bits of entropy.
static uint32_t extractBytes(uint8_t *bytes, uint8_t *inBuf, bool raw) {
    inmClearEntropyLevel();
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
            //printf("Adding evenBit:%u oddBit:%u even:%u\n", evenBit, oddBit, even);
            if(!inmHealthCheckAddBit(evenBit, oddBit, even)) {
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

// Send the new bytes through the health checker and also into the Keccak sponge.
// Output bytes from the sponge only if the health checker says it's OK
static void processBytes(uint8_t *keccakState, uint8_t *bytes, uint32_t entropy, bool raw,
        bool writeDevRandom, uint32_t outputMultiplier) {
    if(raw) {
        // In raw mode, we just output raw data from the INM.
        outputBytes(bytes, BUFLEN/8, entropy, writeDevRandom);
        return;
    }
    // Note that BUFLEN has to be less than 1600 by enough to make the sponge secure.
    // BUFLEN must also be a multiple of 64.  512 and 1024 are reasonable values.
    KeccakAbsorb(keccakState, bytes, BUFLEN/64);
    uint8_t dataOut[BUFLEN/8];
    uint32_t i;
    for(i = 0; i < outputMultiplier; i++) {
        KeccakExtract(keccakState, dataOut, BUFLEN/64);
        // Extract does not do a permute, so do it here.
        KeccakPermutation(keccakState);
        outputBytes(dataOut, BUFLEN/8, entropy, writeDevRandom);
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

int main(int argc, char **argv)
{
    struct ftdi_context ftdic;
    bool raw = false;
    bool debug = false;
    bool writeDevRandom = false;
    bool noOutput = false;
    uint32_t outputMultiplier = 1;
    uint32_t xArg;

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
                            "    --no-output - do not write random output data\n", stderr);
            return 1;
        }
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
    }

    while(true) {
        if(ftdi_write_data(&ftdic, outBuf, BUFLEN) != BUFLEN) {
            fputs("USB write failed\n", stderr);
            return -1;
        }
        if(ftdi_read_data(&ftdic, inBuf, BUFLEN) != BUFLEN) {
            fputs("USB read failed\n", stderr);
            return -1;
        }
        uint8_t bytes[BUFLEN/8];
        uint32_t entropy = extractBytes(bytes, inBuf, raw);
        if(!noOutput && inmHealthCheckOkToUseData() && inmEntropyOnTarget(entropy, BUFLEN)) {
            processBytes(keccakState, bytes, entropy, raw, writeDevRandom, outputMultiplier);
        }
    }
    return 0;
}
