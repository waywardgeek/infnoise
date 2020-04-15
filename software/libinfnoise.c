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
#include "KeccakF-1600-interface.h"

#if defined(__OpenBSD__) || defined(__NetBSD__) || defined(__DragonFly__) || defined(__FreeBSD__)
#include <fcntl.h>
#endif

uint8_t keccakState[KeccakPermutationSizeInBytes];
uint8_t outBuf[BUFLEN];

void prepareOutputBuffer() {
    uint32_t i;

    // Endless loop: set SW1EN and SW2EN alternately
    for (i = 0u; i < BUFLEN; i+=2) {
        // Alternate Ph1 and Ph2
        outBuf[i] = (1 << SWEN1);
        outBuf[i+1] = (1 << SWEN2);
    }
}

bool initInfnoise(struct infnoise_context *context, char *serial, bool keccak, bool debug) {
    context->message="";
    context->entropyThisTime=0;
    context->errorFlag=false;
    context->keccakBytesGiven=0;
    context->bytesWritten=0;

    prepareOutputBuffer();

    // initialize health check
    if (!inmHealthCheckStart(PREDICTION_BITS, DESIGN_K, debug)) {
        context->message = "Can't initialize health checker";
        return false;
    }

    // initialize USB
    if (!initializeUSB(&context->ftdic, &context->message, serial)) {
        // Sometimes have to do it twice - not sure why
        if (!initializeUSB(&context->ftdic, &context->message, serial)) {
            return false;
        }
    }

    // initialize keccak
    if (keccak) {
        KeccakInitialize();
        KeccakInitializeState(keccakState);
    }

    // let healthcheck collect some data
    uint32_t maxWarmupRounds = 5000;
    uint32_t warmupRounds = 0;

    //bool errorFlag = false;
    while (!inmHealthCheckOkToUseData()) {
        readData(context, NULL, true, 1);
        warmupRounds++;
    }

    if (warmupRounds > maxWarmupRounds) {
        context->message = "Unable to collect enough entropy to initialize health checker.";
        return false;
    }
    return true;
}

void deinitInfnoise(struct infnoise_context *context)
{
    inmHealthCheckStop();
    ftdi_usb_close(&context->ftdic);
    ftdi_deinit(&context->ftdic);
}


// Extract the INM output from the data received.  Basically, either COMP1 or COMP2
// changes, not both, so alternate reading bits from them.  We get 1 INM bit of output
// per byte read.  Feed bits from the INM to the health checker.  Return the expected
// bits of entropy.
uint32_t extractBytes(uint8_t *bytes, uint32_t length, uint8_t *inBuf, const char **message, bool *errorFlag) {
    inmClearEntropyLevel();
    uint32_t i;
    for (i = 0u; i < length; i++) {
        uint32_t j;
        uint8_t byte = 0u;
        for (j = 0u; j < 8u; j++) {
            uint8_t val = inBuf[i * 8u + j];
            uint8_t evenBit = (val >> COMP2) & 1u;
            uint8_t oddBit = (val >> COMP1) & 1u;
            bool even = j & 1u; // Use the even bit if j is odd
            uint8_t bit = even ? evenBit : oddBit;
            byte = (byte << 1u) | bit;

            // This is a good place to feed the bit from the INM to the health checker.
            if (!inmHealthCheckAddBit(evenBit, oddBit, even)) {
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
    return seconds * 1.0e6 + nanoseconds / 1000.0;
}

bool isSuperUser(void) {
    return (geteuid() == 0);
}


// let's do it recursive, because if sth. fails we can easily wipe the malloc()
infnoise_devlist_node_t* inf_get_devstrings(struct ftdi_context* ftdic,
                                            struct ftdi_device_list* curdev,
                                            const char** message,
                                            infnoise_devlist_node_t* bgn,
                                            infnoise_devlist_node_t* end) {
    if( curdev != NULL ) {
        infnoise_devlist_node_t* cur;
        cur = (infnoise_devlist_node_t*) malloc (sizeof(infnoise_devlist_node_t));
        cur->next = NULL;
        int rc = ftdi_usb_get_strings(ftdic, curdev->dev,
                                      cur->manufacturer, sizeof(cur->manufacturer),
                                      cur->description, sizeof(cur->description),
                                      cur->serial, sizeof(cur->serial));
        if (rc < 0) {
            *message = ftdi_get_error_string(ftdic);
            free( cur );
            return NULL;
        }
        else {
            // in case bgn is NULL, then implicitly end is NULL, also the other way around
            if(bgn == NULL) {
                bgn = cur;
            }
            else {
                end->next = cur;
            }
            infnoise_devlist_node_t* ret;
            ret = inf_get_devstrings(ftdic, curdev->next, message, bgn, cur);
            // a next dev triggered issue? -> wipe current
            if( ret == NULL ) {
                free( cur );
            }
            return ret;
        }
    }
    return bgn;
}


// Return a list of all infinite noise multipliers found.
infnoise_devlist_node_t* listUSBDevices(const char **message) {
    struct ftdi_context ftdic;
    if(ftdi_init(&ftdic) < 0) {
        *message = "Failed to init";
        return NULL;
    }

    infnoise_devlist_node_t* retlist = NULL;
    struct ftdi_device_list *devlist = NULL;
    if (ftdi_usb_find_all(&ftdic, &devlist, INFNOISE_VENDOR_ID, INFNOISE_PRODUCT_ID) < 0
        || devlist == NULL) {
        if (!isSuperUser()) {
            *message = "Can't find Infinite Noise Multiplier.  Try running as super user?";
        } else {
            *message = "Can't find Infinite Noise Multiplier.";
        }
    }
    else {
        retlist = inf_get_devstrings(&ftdic, devlist, message, NULL, NULL);
        ftdi_list_free2(devlist);
    }

    ftdi_deinit(&ftdic);
    return retlist;
}

// Initialize the Infinite Noise Multiplier USB interface.
bool initializeUSB(struct ftdi_context *ftdic, const char **message, char *serial) {
    ftdi_init(ftdic);
    struct ftdi_device_list *devlist;

    // search devices
    int rc = ftdi_usb_find_all(ftdic, &devlist, INFNOISE_VENDOR_ID, INFNOISE_PRODUCT_ID);
    if (rc < 0) {
        *message = "Can't find Infinite Noise Multiplier";
        return false;
    }
    ftdi_list_free2(devlist);

    // only one found, or no serial given
    if (serial == NULL) {
        // more than one found AND no serial given
        if (rc >= 2) {
            *message = "Multiple Infnoise TRNGs found and serial not specified, using the first one!";
        }
        if (ftdi_usb_open(ftdic, INFNOISE_VENDOR_ID, INFNOISE_PRODUCT_ID) < 0) {
            if (!isSuperUser()) {
                *message = "Can't open Infinite Noise Multiplier. Try running as super user?";
            } else {
#ifdef LINUX
                *message = "Can't open Infinite Noise Multiplier.";
#endif
#if defined(__APPLE__)

                *message = "Can't open Infinite Noise Multiplier. sudo kextunload -b com.FTDI.driver.FTDIUSBSerialDriver ? sudo kextunload -b  com.apple.driver.AppleUSBFTDI ?";
#endif
            }
            return false;
        }
    } else {
        // serial specified
        if (ftdi_usb_open_desc(ftdic, INFNOISE_VENDOR_ID, INFNOISE_PRODUCT_ID, NULL, serial) < 0) {
            if (!isSuperUser()) {
                *message = "Can't find Infinite Noise Multiplier. Try running as super user?";
            } else {
                *message = "Can't find Infinite Noise Multiplier with given serial";
            }
            return false;
        }
    }

    // Set high baud rate
    switch (ftdi_set_baudrate(ftdic, 30000)) {
    case -1:
        *message = "Invalid baud rate";
        return false;
    case -2:
        *message = "Setting baud rate failed";
        return false;
    case -3:
        *message = "Infinite Noise Multiplier unavailable";
        return false;
    default:
        break;
    }

    switch (ftdi_set_bitmode(ftdic, MASK, BITMODE_SYNCBB)) {
    case -1:
        *message = "Can't enable bit-bang mode";
        return false;
    case -2:
        *message = "Infinite Noise Multiplier unavailable\n";
        return false;
    default:
        break;
    }

    // Just test to see that we can write and read.
    uint8_t buf[64u] = {0u,};
    if (ftdi_write_data(ftdic, buf, sizeof(buf)) != sizeof(buf)) {
        *message = "USB write failed";
        return false;
    }
    if (ftdi_read_data(ftdic, buf, sizeof(buf)) != sizeof(buf)) {
        *message = "USB read failed";
        return false;
    }
    return true;
}


// Whiten the output, if requested, with a Keccak sponge. Output bytes only if the health
// checker says it's OK.  Using outputMultiplier > 1 is a nice way to generate a lot more
// cryptographically secure pseudo-random data than the INM generates.  If
// outputMultiplier is 0, we output only as many bits as we measure in entropy.
// This allows a user to generate hundreds of MiB per second if needed, for use
// as cryptographic keys.
uint32_t processBytes(uint8_t *bytes, uint8_t *result, uint32_t *entropy,
                      uint32_t *bytesGiven, uint32_t *bytesWritten,
                      bool raw, uint32_t outputMultiplier) {
    //Use the lower of the measured entropy and the provable lower bound on
    //average entropy.
    if (*entropy > inmExpectedEntropyPerBit * BUFLEN / INM_ACCURACY) {
        *entropy = inmExpectedEntropyPerBit * BUFLEN / INM_ACCURACY;
    }
    if (raw) {
        // In raw mode, we just output raw data from the INM.
        if (result != NULL) {
            memcpy(result, bytes, BUFLEN / 8u * sizeof(uint8_t));
        }
        return BUFLEN / 8u;
    }

    // Note that BUFLEN has to be less than 1600 by enough to make the sponge secure,
    // since outputting all 1600 bits would tell an attacker the Keccak state, allowing
    // him to predict any further output, when outputMultiplier > 1, until the next call
    // to processBytes.  All 512 bits are absorbed before squeezing data out to ensure that
    // we instantly recover (reseed) from a state compromise, which is when an attacker
    // gets a snapshot of the keccak state.  BUFLEN must be a multiple of 64, since
    // Keccak-1600 uses 64-bit "lanes".
    uint8_t resultSize;
    if (outputMultiplier <= 2) {
        resultSize = 64u;
    } else {
        resultSize = 128u;
    }

    uint8_t dataOut[resultSize];
    KeccakAbsorb(keccakState, bytes, BUFLEN / 64u);

    if (outputMultiplier == 0u) {
        // Output all the bytes of entropy we have
        KeccakExtract(keccakState, dataOut, (*entropy + 63u) / 64u);
        if (result != NULL) {
            memcpy(result, dataOut, *entropy / 8u * sizeof(uint8_t));
        }
        return *entropy / 8u;
    }

    // Output 256*outputMultipler bits (in chunks of 1024)
    // only the first 1024 now,
    if (*bytesGiven == 0u) {
        *bytesGiven = outputMultiplier*256u / 8u;
        *bytesWritten = 0u;

        // Output up to 1024 bits at a time.
        uint32_t bytesToWrite = 1024u / 8u;
        if (bytesToWrite > *bytesGiven) {
            bytesToWrite = *bytesGiven;
        }

        KeccakExtract(keccakState, result, bytesToWrite / 8u);
        KeccakPermutation(keccakState);
        *bytesWritten = bytesToWrite;
        *bytesGiven -= bytesToWrite;
    }
    return *bytesWritten;
}

uint32_t readData(struct infnoise_context *context, uint8_t *result, bool raw, uint32_t outputMultiplier) {
    // check if data can be squeezed from the keccak sponge from previous state (or we need to collect some new entropy to get bytesGiven >0)
    if (context->keccakBytesGiven > 0u) {
        // squeeze the sponge!

        // Output up to 1024 bits at a time.
        uint32_t bytesToWrite = 1024u / 8u;

        if (bytesToWrite > context->keccakBytesGiven) {
            bytesToWrite = context->keccakBytesGiven;
        }

        KeccakExtract(keccakState, result, bytesToWrite / 8u);
        KeccakPermutation(keccakState);

        context->bytesWritten += bytesToWrite;
        context->keccakBytesGiven -= bytesToWrite;
        return bytesToWrite;
    } else { // collect new entropy
        uint8_t inBuf[BUFLEN];
        struct timespec start;
        clock_gettime(CLOCK_REALTIME, &start);

        // write clock signal
        if (ftdi_write_data(&context->ftdic, outBuf, sizeof(outBuf)) != sizeof(outBuf)) {
            context->message = "USB write failed";
            context->errorFlag = true;
        }

        // and read 512 byte from the internal buffer (in synchronous bitbang mode)
        if (ftdi_read_data(&context->ftdic, inBuf, sizeof(inBuf)) != sizeof(inBuf)) {
            context->message = "USB read failed";
            context->errorFlag = true;
            return 0;
        }

        struct timespec end;
        clock_gettime(CLOCK_REALTIME, &end);
        uint32_t us = diffTime(&start, &end);

        if (us <= MAX_MICROSEC_FOR_SAMPLES) {
            uint8_t bytes[BUFLEN / 8u];
            context->entropyThisTime = extractBytes(bytes, sizeof(bytes), inBuf, &context->message, &context->errorFlag);
            if (context->errorFlag) {
		            // todo: message?
                return 0;
            }
            // call health check and return bytes if OK
            if (inmHealthCheckOkToUseData() && inmEntropyOnTarget(context->entropyThisTime, BUFLEN)) {
                return processBytes(bytes, result, &context->entropyThisTime, &context->keccakBytesGiven, &context->bytesWritten,
                raw, outputMultiplier);
            }
        }
    }
    return 0;
}
