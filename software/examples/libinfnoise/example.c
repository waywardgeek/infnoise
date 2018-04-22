/*
This is a simple example to use libinfnoise with whitened and multiplied output.
*/

// Required to include clock_gettime
#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <ftdi.h>
#include <libinfnoise.h>

int main()
{
    // parameters
    char *serial=NULL; 		// can be set to a specific serial, NULL uses the first found device
    bool initKeccak = true;	// initialize Keccak sponge (used for whitening)
    uint32_t multiplier = 10u;	// multiplier for whitening
    bool debug = false;		// debug mode (health monitor writes to stderr)

    char *message;
    bool errorFlag = false;

    // initialize hardware and health monitor
    struct ftdi_context ftdic;
    if (!initInfnoise(&ftdic, serial, &message, true, debug)) {
        fputs(message, stderr);
        return 1; // ERROR
    }

    // calculate output size based on the parameters:
    // when using the multiplier, we need a result array of 32*MULTIPLIER - otherwise 64(BUFLEN/8) bytes
    uint32_t resultSize;
    if (multiplier == 0 || rawOutput == true) {
        resultSize = BUFLEN/8u;
    } else {
        resultSize = multiplier*32u;
    }

    // read and print in a loop (until 1M is read)
    uint64_t totalBytesWritten = 0u;
    while (totalBytesWritten < 1000000) {
        uint8_t result[resultSize];

	// read data returns the number of bitrs written to result byte-array
        uint64_t bytesWritten = readData(&ftdic, result, &message, &errorFlag, multiplier);
	totalBytesWritten += bytesWritten;

	// check for errors
	// note: bytesWritten is also 0 in this case, but an errorFlag is needed as
        // bytesWritten can also be 0 when data hasn't passed the health monitor.
        if (errorFlag) {
            fprintf(stderr, "Error: %s\n", message);
            return 1;
        }

        // print as many bytes
        fwrite(result, 1, bytesWritten, stdout);
    }
    return 0;
}
