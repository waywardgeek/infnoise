/* Library for the Infinite Noise Multiplier USB stick */

// Required to include clock_gettime
#define _POSIX_C_SOURCE 200809L

#define INFNOISE_VENDOR_ID 0x0403
#define INFNOISE_PRODUCT_ID 0x6015

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <ftdi.h>
#include "libinfnoise_private.h"
#include "libinfnoise.h"
#include "KeccakF-1600-interface.h"

uint8_t keccakState[KeccakPermutationSizeInBytes];
bool initInfnoise(struct ftdi_context *ftdic,char *serial, char **message, bool keccak, bool debug) {
    prepareOutputBuffer();

    // initialize health check
    if (!inmHealthCheckStart(PREDICTION_BITS, DESIGN_K, debug)) {
        *message="Can't initialize health checker";
        return false;
    }

    // initialize USB
    if(!initializeUSB(ftdic, message, serial)) {
        // Sometimes have to do it twice - not sure why
        if(!initializeUSB(ftdic, message, serial)) {
            return false;
        }
    }

    // initialize keccak
    if (keccak) {
        KeccakInitialize();
        KeccakInitializeState(keccakState);
    }

    // let healthcheck collect some data
    uint32_t maxWarmupRounds = 500;
    uint32_t warmupRounds = 0;
    bool errorFlag = false;
    while(!inmHealthCheckOkToUseData()) {
        readData_private(ftdic, NULL, message, &errorFlag, false, true, 0, false);
        warmupRounds++;
    }
    if (warmupRounds > maxWarmupRounds) {
        *message = "Unable to collect enough entropy to initialize health checker.";
        return false;
    }
    return true;
}

uint8_t outBuf[BUFLEN];
void prepareOutputBuffer() {
    uint32_t i;

    // Endless loop: set SW1EN and SW2EN alternately
    for(i = 0u; i < BUFLEN; i++) {
        // Alternate Ph1 and Ph2
        outBuf[i] = i & 1?  (1 << SWEN2) : (1 << SWEN1);
    }
}

// Extract the INM output from the data received.  Basically, either COMP1 or COMP2
// changes, not both, so alternate reading bits from them.  We get 1 INM bit of output
// per byte read.  Feed bits from the INM to the health checker.  Return the expected
// bits of entropy.
uint32_t extractBytes(uint8_t *bytes, uint8_t *inBuf, char **message, bool *errorFlag) {
    inmClearEntropyLevel();
    uint32_t i;
    for(i = 0u; i < BUFLEN/8u; i++) {
        uint32_t j;
        uint8_t byte = 0u;
        for(j = 0u; j < 8u; j++) {
            uint8_t val = inBuf[i*8u + j];
            uint8_t evenBit = (val >> COMP2) & 1u;
            uint8_t oddBit = (val >> COMP1) & 1u;
            bool even = j & 1u; // Use the even bit if j is odd
            uint8_t bit = even? evenBit : oddBit;
            byte = (byte << 1u) | bit;

            // This is a good place to feed the bit from the INM to the health checker.
            if(!inmHealthCheckAddBit(evenBit, oddBit, even)) {
                *message = "Health check of Infinite Noise Multiplier failed!";
                *errorFlag = true;
                return 0;
            }
        }
        bytes[i] = byte;
    }
    return inmGetEntropyLevel();
}

// Return the difference in the times as a double in microseconds.
double diffTime(struct timespec *start, struct timespec *end) {
    uint32_t seconds = end->tv_sec - start->tv_sec;
    int32_t nanoseconds = end->tv_nsec - start->tv_nsec;
    return seconds*1.0e6 + nanoseconds/1000.0;
}

// Write the bytes to either stdout, or /dev/random.
bool outputBytes(uint8_t *bytes,  uint32_t length, uint32_t entropy, bool writeDevRandom, char **message) {
    if(!writeDevRandom) {
        if(fwrite(bytes, 1, length, stdout) != length) {
            *message = "Unable to write output from Infinite Noise Multiplier";
            return false;
        }
    } else {
#ifdef MACOS
        *message = "macOS doesn't support writes to entropy pool";
        entropy = 0; // suppress warning
        return false;
#endif
#ifdef LINUX
        inmWaitForPoolToHaveRoom();
        inmWriteEntropyToPool(bytes, length, entropy);
#endif
    }
    return true;
}

bool isSuperUser(void) {
        return (geteuid() == 0);
}

// Whiten the output, if requested, with a Keccak sponge. Output bytes only if the health
// checker says it's OK.  Using outputMultiplier > 1 is a nice way to generate a lot more
// cryptographically secure pseudo-random data than the INM generates.  If
// outputMultiplier is 0, we output only as many bits as we measure in entropy.
// This allows a user to generate hundreds of MiB per second if needed, for use
// as cryptographic keys.
uint32_t processBytes(uint8_t *bytes, uint8_t *result, uint32_t entropy,
        bool raw, bool writeDevRandom, uint32_t outputMultiplier, bool noOutput,
        char **message, bool *errorFlag) {
    //Use the lower of the measured entropy and the provable lower bound on
    //average entropy.
    if(entropy > inmExpectedEntropyPerBit*BUFLEN/INM_ACCURACY) {
        entropy = inmExpectedEntropyPerBit*BUFLEN/INM_ACCURACY;
    }
    if(raw) {
        // In raw mode, we just output raw data from the INM.
        if (!noOutput) {
            if (!outputBytes(bytes, BUFLEN/8u, entropy, writeDevRandom, message)) {
                *errorFlag = true;
                return 0; // write failed
            }
        } else {
            if (result != NULL) {
                memcpy(result, bytes, BUFLEN/8u * sizeof(uint8_t));
            }
        }
        return BUFLEN/8u;
    }

    // Note that BUFLEN has to be less than 1600 by enough to make the sponge secure,
    // since outputting all 1600 bits would tell an attacker the Keccak state, allowing
    // him to predict any further output, when outputMultiplier > 1, until the next call
    // to processBytes.  All 512 bits are absorbed before squeezing data out to ensure that
    // we instantly recover (reseed) from a state compromise, which is when an attacker
    // gets a snapshot of the keccak state.  BUFLEN must be a multiple of 64, since
    // Keccak-1600 uses 64-bit "lanes".
    KeccakAbsorb(keccakState, bytes, BUFLEN/64u);
    uint8_t dataOut[16u*8u];
    if(outputMultiplier == 0u) {
        // Output all the bytes of entropy we have
        KeccakExtract(keccakState, dataOut, (entropy + 63u)/64u);
        if (!noOutput) {
            if (!outputBytes(dataOut, entropy/8u, entropy & 0x7u, writeDevRandom, message)) {
                *errorFlag = true;
                return 0;
            }
        } else {
            if (result != NULL) {
                memcpy(result, dataOut, entropy/8u * sizeof(uint8_t));
            }
        }
        return entropy/8u;
    }

    // Output 256*outputMultipler bits.
    uint32_t numBits = outputMultiplier*256u;
    uint32_t bytesWritten = 0u;

    while(numBits > 0u) {
        // Write up to 1024 bits at a time.
        uint32_t bytesToWrite = 1024u/8u;
        if(bytesToWrite > numBits/8u) {
            bytesToWrite = numBits/8u;
        }
        KeccakExtract(keccakState, dataOut, bytesToWrite/8u);
        uint32_t entropyThisTime = entropy;
        if(entropyThisTime > 8u*bytesToWrite) {
            entropyThisTime = 8u*bytesToWrite;
        }
        if (!noOutput) {
            if (!outputBytes(dataOut, bytesToWrite, entropyThisTime, writeDevRandom, message)) {
                *errorFlag = true;
                return 0;
            }
        } else {
            //memcpy(result + bytesWritten, dataOut, bytesToWrite * sizeof(uint8_t)); //doesn't work?
            // alternative: loop through dataOut and append array elements to result..
            if (result != NULL) {
                for (uint32_t i =0; i < bytesToWrite; i++ ) {
                    result[bytesWritten + i] = dataOut[i];
                }
            }
        }
        bytesWritten += bytesToWrite;
        numBits -= bytesToWrite*8u;
        entropy -= entropyThisTime;
        if(numBits > 0u) {
            KeccakPermutation(keccakState);
        }
    }
    if(bytesWritten != outputMultiplier*(256u/8u)) {
        *message = "Internal error outputing bytes";
        *errorFlag = true;
        return 0;
    }
    return bytesWritten;
}

// Return a list of all infinite noise multipliers found.
bool listUSBDevices(struct ftdi_context *ftdic, char** message) {
    ftdi_init(ftdic);

    struct ftdi_device_list *devlist;
    struct ftdi_device_list *curdev;
    char manufacturer[128], description[128], serial[128];
    int i=0;

    // search devices
    int rc = ftdi_usb_find_all(ftdic, &devlist, INFNOISE_VENDOR_ID, INFNOISE_PRODUCT_ID);

    if (rc < 0) {
        if (!isSuperUser()) {
            *message = "Can't find Infinite Noise Multiplier.  Try running as super user?";
        } else {
            *message = "Can't find Infinite Noise Multiplier";
        }
    }

    for (curdev = devlist; curdev != NULL; i++) {
        //printf("Device: %d, ", i);
        rc = ftdi_usb_get_strings(ftdic, curdev->dev, manufacturer, 128, description, 128, serial, 128);
        if (rc < 0) {
            if (!isSuperUser()) {
                *message = "Can't find Infinite Noise Multiplier.  Try running as super user?";
                return false;
            }
            //*message = "ftdi_usb_get_strings failed: %d (%s)\n", rc, ftdi_get_error_string(ftdic));
            return false;
        }

        // print to stdout
        printf("Manufacturer: %s, Description: %s, Serial: %s", manufacturer, description, serial);
        curdev = curdev->next;
    }

    return true;
}

// Initialize the Infinite Noise Multiplier USB interface.
bool initializeUSB(struct ftdi_context *ftdic, char **message, char *serial) {
    ftdi_init(ftdic);
    struct ftdi_device_list *devlist;

    // search devices
    int rc = 0;
    if ((rc = ftdi_usb_find_all(ftdic, &devlist, INFNOISE_VENDOR_ID, INFNOISE_PRODUCT_ID)) < 0) {
        *message = "Can't find Infinite Noise Multiplier";
        return false;
    }

    // only one found, or no serial given
    if (rc >= 0) {
        if (serial == NULL) {
            // more than one found AND no serial given
            if (rc >= 2) {
                *message = "Multiple Infnoise TRNGs found and serial not specified, using the first one!";
            }
            if (ftdi_usb_open(ftdic, INFNOISE_VENDOR_ID, INFNOISE_PRODUCT_ID) < 0) {
                if(!isSuperUser()) {
                    *message = "Can't open Infinite Noise Multiplier. Try running as super user?";
                } else {
#ifdef LINUX
                    *message = "Can't open Infinite Noise Multiplier.";
#endif
#ifdef MACOS
                    *message = "Can't open Infinite Noise Multiplier. sudo kextunload -b com.FTDI.driver.FTDIUSBSerialDriver ?";
#endif
                }
                return false;
            }
        } else {
            // serial specified
            rc = ftdi_usb_open_desc(ftdic, INFNOISE_VENDOR_ID, INFNOISE_PRODUCT_ID, NULL, serial);
            if (rc < 0) {
                if(!isSuperUser()) {
                    *message = "Can't find Infinite Noise Multiplier. Try running as super user?";
                } else {
                    *message = "Can't find Infinite Noise Multiplier with given serial";
                }
                return false;
            }
        }
    }

    // Set high baud rate
    rc = ftdi_set_baudrate(ftdic, 30000);
    if(rc == -1) {
        *message = "Invalid baud rate";
        return false;
    } else if(rc == -2) {
        *message = "Setting baud rate failed";
        return false;
    } else if(rc == -3) {
        *message = "Infinite Noise Multiplier unavailable";
        return false;
    }
    rc = ftdi_set_bitmode(ftdic, MASK, BITMODE_SYNCBB);
    if(rc == -1) {
        *message = "Can't enable bit-bang mode";
        return false;
    } else if(rc == -2) {
        *message = "Infinite Noise Multiplier unavailable\n";
        return false;
    }

    // Just test to see that we can write and read.
    uint8_t buf[64u] = {0u,};
    if(ftdi_write_data(ftdic, buf, 64) != 64) {
        *message = "USB write failed";
        return false;
    }
    if(ftdi_read_data(ftdic, buf, 64) != 64) {
        *message = "USB read failed";
        return false;
    }
    return true;
}

uint32_t readRawData(struct ftdi_context *ftdic, uint8_t *result, char **message, bool *errorFlag) {
    return readData_private(ftdic, result, message, errorFlag, false, true, 0, false);
}

uint32_t readData(struct ftdi_context *ftdic, uint8_t *result, char **message, bool *errorFlag, uint32_t outputMultiplier) {
    return readData_private(ftdic, result, message, errorFlag, false, false, outputMultiplier, false);
}

uint32_t readData_private(struct ftdi_context *ftdic, uint8_t *result, char **message, bool *errorFlag,
                        bool noOutput, bool raw, uint32_t outputMultiplier, bool devRandom) {
    uint8_t inBuf[BUFLEN];
    struct timespec start;
    clock_gettime(CLOCK_REALTIME, &start);

    // write clock signal
    if(ftdi_write_data(ftdic, outBuf, BUFLEN) != BUFLEN) {
        *message = "USB write failed";
        *errorFlag = true;
    }

    // and read 512 byte from the internal buffer (in synchronous bitbang mode)
    if(ftdi_read_data(ftdic, inBuf, BUFLEN) != BUFLEN) {
        *message = "USB read failed";
        *errorFlag = true;
    }

    struct timespec end;
    clock_gettime(CLOCK_REALTIME, &end);
    uint32_t us = diffTime(&start, &end);
    if(us <= MAX_MICROSEC_FOR_SAMPLES) {
        uint8_t bytes[BUFLEN/8u];
        uint32_t entropy = extractBytes(bytes, inBuf, message, errorFlag);

        // call health check and process bytes if OK
        if (inmHealthCheckOkToUseData() && inmEntropyOnTarget(entropy, BUFLEN)) {
            uint32_t byteswritten = processBytes(bytes, result, entropy, raw, devRandom, outputMultiplier, noOutput, message, errorFlag);
            return byteswritten;
        }
    }
    return 0;
}

#ifdef LIB_EXAMPLE_PROGRAM
// example use of libinfnoise - with keccak
int main() {
    char *serial=NULL; // use any device, can be set to a specific serial

    // initialize USB
    struct ftdi_context ftdic;
    initInfnoise(&ftdic, serial);

    // parameters for readData(..):
    bool rawOutput = true;
    uint32_t multiplier = 10u;

    // calculate output size based on the parameters:
    // when using the multiplier, we need a result array of 32*MULTIPLIER - otherwise 64(BUFLEN/8) bytes
    uint32_t resultSize;
    if (multiplier == 0 || rawOutput == true) {
        resultSize = BUFLEN/8u;
    } else {
        resultSize = multiplier*32u;
    }

    uint64_t totalBytesWritten = 0u;

    // read and print in a loop
    while (totalBytesWritten < 100000) {
        uint8_t result[resultSize];
        uint64_t bytesWritten = 0u;
        bytesWritten = readData(&ftdic, keccakState, result, multiplier);

        // check for -1, indicating an error
        totalBytesWritten += bytesWritten;

        // make sure to only read as many bytes as readData returned. Only those have passed the health check in this round (usually all)
        fwrite(result, 1, bytesWritten, stdout);
    }
}
#endif
