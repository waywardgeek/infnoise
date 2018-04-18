/* Driver for the Infinite Noise Multiplier USB stick */

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
#include <ftdi.h>
#include "libinfnoise_private.h"
#include "libinfnoise.h"
#include "KeccakF-1600-interface.h"

// Extract the INM output from the data received.  Basically, either COMP1 or COMP2
// changes, not both, so alternate reading bits from them.  We get 1 INM bit of output
// per byte read.  Feed bits from the INM to the health checker.  Return the expected
// bits of entropy.
uint32_t extractBytes(uint8_t *bytes, uint8_t *inBuf) {
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
                fputs("Health check of Infinite Noise Multiplier failed!\n", stderr);
                exit(1);
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
void outputBytes(uint8_t *bytes, uint32_t length, uint32_t entropy, bool writeDevRandom) {
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

// Whiten the output, if requested, with a Keccak sponge. Output bytes only if the health
// checker says it's OK.  Using outputMultiplier > 1 is a nice way to generate a lot more
// cryptographically secure pseudo-random data than the INM generates.  If
// outputMultiplier is 0, we output only as many bits as we measure in entropy.
// This allows a user to generate hundreds of MiB per second if needed, for use
// as cryptographic keys.
uint32_t processBytes(uint8_t *keccakState, uint8_t *bytes, uint8_t *result, uint32_t entropy, bool raw,
        bool writeDevRandom, uint32_t outputMultiplier, bool noOutput) {
    //Use the lower of the measured entropy and the provable lower bound on
    //average entropy.
    if(entropy > inmExpectedEntropyPerBit*BUFLEN/INM_ACCURACY) {
        entropy = inmExpectedEntropyPerBit*BUFLEN/INM_ACCURACY;
    }
    if(raw) {
        // In raw mode, we just output raw data from the INM.
        if (!noOutput) {
            outputBytes(bytes, BUFLEN/8u, entropy, writeDevRandom);
        } else {
            memcpy(result, bytes, BUFLEN/8u * sizeof(uint8_t));
            //result=bytes;
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
	    outputBytes(dataOut, entropy/8u, entropy & 0x7u, writeDevRandom);
	} else {
            memcpy(result, dataOut, entropy/8u * sizeof(uint8_t));
	}
        return entropy/8u;
    } // todo: write to result array

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
            outputBytes(dataOut, bytesToWrite, entropyThisTime, writeDevRandom);
	} else {
            // append data in result array until we have finished squeezing the keccak sponge
	    // its important to have an result array of the approriate size: outputMultiplier*32
            //fprintf(stderr, "bytes written: %d\n", bytesWritten);
            //fprintf(stderr, "bytes to write: %d\n", bytesToWrite);

            //memcpy(result + bytesWritten, dataOut, bytesToWrite * sizeof(uint8_t)); //doesn't work
            // alternative: loop through dataOut and append array elements to result..
            for (uint32_t i =0; i < bytesToWrite; i++ ) {
                 fprintf(stderr, "                 result[%d] = dataOut[%d];\n", bytesWritten + i, i);
                 result[bytesWritten + i] = dataOut[i];
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
        fprintf(stderr, "Internal error outputing bytes\n");
        exit(1);
    }
    fprintf(stderr, "bytes written: %d\n", bytesWritten);
    return bytesWritten;
}
void add_to_list(struct inm_devlist *list, struct infnoise_device **dev) {
    struct inm_devlist_node *tmp = malloc(sizeof(struct inm_devlist_node ) );
    tmp->device = (*dev);
    printf("added serial1: %s\n", (*dev)->serial);
    tmp->next = list->head;
    printf("added serial2: %s\n", tmp->device->serial);
    list->head = tmp;
    printf("added serial3: %s\n", list->head->device->serial);
}



// Return a list of all infinite noise multipliers found.
bool listUSBDevices(struct ftdi_context *ftdic, struct inm_devlist *result, char** message) {
    ftdi_init(ftdic);

    struct ftdi_device_list *devlist;
    struct ftdi_device_list *curdev;
    char manufacturer[128], description[128], serial[128];
    int i=0;

    // search devices
    int rc = ftdi_usb_find_all(ftdic, &devlist, INFNOISE_VENDOR_ID, INFNOISE_PRODUCT_ID);

    if (rc < 0) {
        if (!isSuperUser()) {
            *message = "Can't find Infinite Noise Multiplier.  Try running as super user?\n";
        } else {
            *message = "Can't find Infinite Noise Multiplier\n";
        }
    }

    for (curdev = devlist; curdev != NULL; i++) {
        //printf("Device: %d, ", i);
        rc = ftdi_usb_get_strings(ftdic, curdev->dev, manufacturer, 128, description, 128, serial, 128);
        if (rc < 0) {
            if (!isSuperUser()) {
                *message = "Can't find Infinite Noise Multiplier.  Try running as super user?\n";
            }
            //todo: fprintf(stderr, "ftdi_usb_get_strings failed: %d (%s)\n", rc, ftdi_get_error_string(ftdic));
	    return false;
       	}

	// build struct of device descriptor & add to list
        printf("Manufacturer: %s, Description: %s, Serial: %s\n", manufacturer, description, serial);

	struct infnoise_device *result_dev = malloc(sizeof(struct infnoise_device));

	result_dev->index = i;
        result_dev->manufacturer = manufacturer;
        result_dev->product = description;
        result_dev->serial = serial;

        //printf("debug: %s\n", result_dev);
        add_to_list(result, &result_dev);

    struct inm_devlist_node *tmp;
    for ( tmp = result->head; tmp != NULL; tmp=tmp->next) {
        if (tmp->device->serial != NULL) {
            printf("%s\n", tmp->device->serial);
        }
           //tmp = tmp->next;
        }
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
        *message = "Can't find Infinite Noise Multiplier\n";
        return false;
    }

    // only one found, or no serial given
    if (rc >= 0) {
	if (serial == NULL) {
            // more than one found AND no serial given
            if (rc >= 2) {
		*message = "Multiple Infnoise TRNGs found and serial not specified, using the first one!\n";
            }
            if (ftdi_usb_open(ftdic, INFNOISE_VENDOR_ID, INFNOISE_PRODUCT_ID) < 0) {
                if(!isSuperUser()) {
                    *message = "Can't open Infinite Noise Multiplier. Try running as super user?\n";
                } else {
                    *message = "Can't open Infinite Noise Multiplier\n";
                }
                return false;
	    }
        } else {
            // serial specified
            rc = ftdi_usb_open_desc(ftdic, INFNOISE_VENDOR_ID, INFNOISE_PRODUCT_ID, NULL, serial);
            if (rc < 0) {
                if(!isSuperUser()) {
                    *message = "Can't find Infinite Noise Multiplier. Try running as super user?\n";
                } else {
                    *message = "Can't find Infinite Noise Multiplier with given serial\n";
                }
                return false;
	    }
        }
    }

    // Set high baud rate
    rc = ftdi_set_baudrate(ftdic, 30000);
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
    rc = ftdi_set_bitmode(ftdic, MASK, BITMODE_SYNCBB);
    if(rc == -1) {
        *message = "Can't enable bit-bang mode\n";
        return false;
    } else if(rc == -2) {
        *message = "Infinite Noise Multiplier unavailable\n";
        return false;
    }

    // Just test to see that we can write and read.
    uint8_t buf[64u] = {0u,};
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


uint64_t readRawData(struct ftdi_context *ftdic, uint8_t *result, char **message) {
    return readData_private(ftdic, NULL, result, message, false, true, 0, false);
}

uint64_t readData(struct ftdi_context *ftdic, uint8_t *keccakState, uint8_t *result, char **message, uint32_t outputMultiplier) {
    return readData_private(ftdic, keccakState, result, message, false, false, outputMultiplier, false);
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

uint64_t readData_private(struct ftdi_context *ftdic, uint8_t *keccakState, uint8_t *result, char **message, bool noOutput, bool raw, uint32_t outputMultiplier, bool devRandom) {
    uint8_t inBuf[BUFLEN];
    uint64_t totalBytesWritten = 0u;
    struct timespec start;
    clock_gettime(CLOCK_REALTIME, &start);

    // write clock signal
    if(ftdi_write_data(ftdic, outBuf, BUFLEN) != BUFLEN) {
        *message = "USB write failed";
        return -1;
    }
    // and read 512 byte from the internal buffer (in synchronous bitbang mode)
    if(ftdi_read_data(ftdic, inBuf, BUFLEN) != BUFLEN) {
        *message = "USB read failed";
        return -1;
    }

    struct timespec end;
    clock_gettime(CLOCK_REALTIME, &end);
    uint32_t us = diffTime(&start, &end);
    if(us <= MAX_MICROSEC_FOR_SAMPLES) {
        uint8_t bytes[BUFLEN/8u];
        uint32_t entropy = extractBytes(bytes, inBuf);

	// call health check and process bytes if OK
        if(!noOutput && inmHealthCheckOkToUseData() && inmEntropyOnTarget(entropy, BUFLEN)) {
            totalBytesWritten += processBytes(keccakState, bytes, result, entropy, raw, devRandom, outputMultiplier, noOutput);
        }
    }
    return totalBytesWritten;
}

bool initInfnoise(struct ftdi_context *ftdic,char *serial, char **message, bool debug) {
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
    return true;
}

#ifdef LIB_EXAMPLE_PROGRAM
// example use of libinfnoise - with keccak
int main() {
    char *serial=NULL; // use any device, can be set to a specific serial

    // initialize USB
    struct ftdi_context ftdic;
    initInfnoise(&ftdic, serial);

    // initialize keccak
    KeccakInitialize();
    uint8_t keccakState[KeccakPermutationSizeInBytes];
    KeccakInitializeState(keccakState);

    // parameters for readData(..):
    bool rawOutput = true;
    uint32_t multiplier = 10u;
    bool debug = false;

    // calculate output size based on the parameters:
    // when using the multiplier, we need a result array of max 1024 bytes - otherwise 64(BUFLEN/8) bytes
    uint32_t resultSize;
    if (multiplier == 0 || rawOutput == true) {
        resultSize = BUFLEN/8u;
    } else {
        resultSize = 1024; // optimize?
    }
    fprintf(stderr, "%d\n", resultSize);

    uint64_t totalBytesWritten = 0u;

    // read and print in a loop
    while (totalBytesWritten < 100000) {
        uint8_t result[resultSize];
        uint64_t bytesWritten = 0u;
        bytesWritten = readData(&ftdic, keccakState, result, multiplier);

	// check for -1!
        totalBytesWritten += bytesWritten;

	// make sure to only read as many bytes as readData returned. Only those have passed the health check in this round (usually all but..)
        fwrite(result, 1, bytesWritten, stdout);
    }
}
#endif
