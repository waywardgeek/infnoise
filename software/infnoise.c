/* Driver for the Infinite Noise Multiplier USB stick */

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <ftdi.h>
#include "healthcheck.h"
#include "KeccakF-1600-interface.h"

// Must be multiple of 64
//#define BUFLEN 64
// The FT240X has a 512 byte buffer
#define BUFLEN 512

#define COMP1 1
#define COMP2 4
#define SWEN1 2
#define SWEN2 0
// Add bits are outputs, except COMP1 and COMP2
#define MASK (0xff & ~(1 << COMP1) & ~(1 << COMP2))

// Extract the INM output from the data received.  Basically, either COMP1 or COMP2
// changes, not both, so alternate reading bits from them.  We get 1 INM bit of output
// per byte read.  Feed bits from the INM to the health checker.
static void extractBytes(uint8_t *bytes, uint8_t *inBuf, bool raw) {
    uint32_t i;
    for(i = 0; i < BUFLEN/8; i++) {
        uint32_t j;
        uint8_t byte = 0;
        for(j = 0; j < 8; j++) {
            //printf("%x ", inBuf[i*8 + j] & ~MASK);
            uint8_t val = inBuf[i*8 + j];
            uint8_t bit;
            if(j & 1) {
                bit = (val >> COMP1) & 1;
            } else {
                bit = (val >> COMP2) & 1;
            }
            byte = (byte << 1) | bit;
            // This is a good place to feed the bit from the INM to the health checker.
            //printf("Adding bit %u\n", bit);
            if(!raw && !inmHealthCheckAddBit(bit, j & 1)) {
                fprintf(stderr, "Health check of Infinite Noise Multiplier failed!\n");
                exit(1);
            }
        }
        //printf("extracted byte:%x\n", byte);
        bytes[i] = byte;
    }
}

// Send the new bytes through the health checker and also into the Keccak sponge.
// Output bytes from the sponge only if the health checker says it's OK, and only
// output half the entropy we get from the INM, just to be paranoid.
static void processBytes(uint8_t *keccakState, uint8_t *bytes, bool raw) {
    if(raw) {
        // In raw mode, we disable the health check and whitening, and just output raw
        // data from the INM.
        if(fwrite(bytes, 1, BUFLEN/8, stdout) != BUFLEN/8) {
            fprintf(stderr, "Unable to write output from Infinite Noise Multiplier\n");
            exit(1);
        }
        return;
    }
    uint32_t i;
    for(i = 0; i < BUFLEN/64; i++) {
        // It's always OK to absorb bytes, even if they are not verified for randomness
        KeccakAbsorb(keccakState, bytes + i*8, 1);
        if(inmHealthCheckOkToUseData() && inmHealthCheckGetEntropyLevel() >= 16) {
            // Only output byes if we have enough entropy and health check passes
            // Also, we output data at 1/2 the rate of entropy added to the sponge
            //uint32_t j;
            //for(j = 0; j < 1 << 24; j++) {
            uint8_t dataOut[8];
            KeccakPermutation(keccakState);
            KeccakExtract(keccakState, dataOut, 1);
            if(fwrite(dataOut, 1, 8, stdout) != 8) {
                fprintf(stderr, "Unable to write output from Infinite Noise Multiplier\n");
                exit(1);
            }
            //}
            inmHealthCheckReduceEntropyLevel(16);
        }
    }
}

int main(int argc, char **argv)
{
    struct ftdi_context ftdic;
    bool raw = false;

    // Process arguments
    if(argc > 2 || (argc == 2 && strcmp(argv[1], "--raw"))) {
        fprintf(stderr, "Usage: infnoise [--raw]\n");
        return 1;
    }
    if(argc == 2) {
        raw = true;
    }

    // Initialize FTDI context
    ftdi_init(&ftdic);
    if(!inmHealthCheckStart(14, 1.82)) {
        puts("Can't intialize health checker\n");
        return 1;
    }
    uint8_t keccakState[KeccakPermutationSizeInBytes];
    KeccakInitializeState(keccakState);

    /* Open FTDI device based on FT232R vendor & product IDs */
    if(ftdi_usb_open(&ftdic, 0x0403, 0x6015) < 0) {
        puts("Can't find Infinite Noise Multiplier");
        return 1;
    }

    // Set high baud rate
    int rc = 0;
    //rc = ftdi_set_baudrate(&ftdic, 3000000);
    rc = ftdi_set_baudrate(&ftdic, 500000);
    if(rc == -1) {
        puts("Invalid baud rate\n");
        return -1;
    } else if(rc == -2) {
        puts("Setting baud rate failed\n");
        return -1;
    } else if(rc == -3) {
        puts("Infinite Noise Multiplier unavailable\n");
        return -1;
    }

    // Enable syncrhonous bitbang mode
    rc = ftdi_set_bitmode(&ftdic, MASK, BITMODE_SYNCBB);
    if(rc == -1) {
        puts("Can't enable bit-bang mode\n");
        return -1;
    } else if(rc == -2) {
        puts("Infinite Noise Multiplier unavailable\n");
        return -1;
    }

    // Endless loop: set SW1EN and SW2EN alternately
    uint32_t i;
    uint8_t outBuf[BUFLEN], inBuf[BUFLEN];
    for(i = 0; i < BUFLEN; i++) {
        // Alternate Ph1 and Ph2 - maybe should have both off in between
        outBuf[i] = i & 1?  (1 << SWEN2) : (1 << SWEN1);
    }
    //outBuf[BUFLEN-1] = 0;

    while(true) {
        /*
        for(i = 0; i < BUFLEN; i++) {
            if(ftdi_write_data(&ftdic, outBuf + i, 1) != 1) {
                puts("USB write failed\n");
                return -1;
            }
            if(ftdi_read_data(&ftdic, inBuf + i, 1) != 1) {
                puts("USB read failed\n");
                return -1;
            }
        }
        */
        if(ftdi_write_data(&ftdic, outBuf, BUFLEN) != BUFLEN) {
            puts("USB write failed\n");
            return -1;
        }
        if(ftdi_read_data(&ftdic, inBuf, BUFLEN) != BUFLEN) {
            puts("USB read failed\n");
            return -1;
        }
        uint8_t bytes[BUFLEN/8];
        extractBytes(bytes, inBuf, raw);
        processBytes(keccakState, bytes, raw);
    }
    return 0;
}
