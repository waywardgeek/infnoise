/*
This is a more advanced example to use libinfnoise with raw, whitened and/or multiplied output.
*/

#include <stdio.h>
#include <ftdi.h>
#include <libinfnoise.h>

int main()
{
    // parameters
    char *serial=NULL; 		// can be set to a specific serial, NULL uses the first found device
    bool initKeccak = true;	// initialize Keccak sponge (used for whitening)
    uint32_t multiplier = 2u;	// multiplier for whitening
    bool debug = true;		// debug mode (health monitor writes to stderr)

    // initialize hardware and health monitor
    struct infnoise_context context;

    if (!initInfnoise(&context, serial, initKeccak, debug)) {
        fputs(context.message, stderr);
        return 1; // ERROR
    }

    uint32_t resultSize;
    if (multiplier <= 1 || initKeccak == false) {
        resultSize = 32u;
    } else if (multiplier==2) {
	resultSize=64;
    } else {
        resultSize = 128u;
    }
    fprintf(stdout, "Error: %i\n", resultSize);

    // read and print in a loop (until 1M is read)
    uint64_t totalBytesWritten = 0u;
    while (totalBytesWritten < 1000000) {
        uint8_t result[resultSize];

	// readRawData returns the number of bytes written to result array
        uint64_t bytesWritten = readData(&context, result, !initKeccak, multiplier);

	// check for errors
	// note: bytesWritten is also 0 in this case, but an errorFlag is needed as
        // bytesWritten can also be 0 when data hasn't passed the health monitor.
        if (context.errorFlag) {
            fprintf(stderr, "Error: %s\n", context.message);
            return -1;
        }
        fprintf(stderr, "infnoise bytes read: %lu\n", (unsigned long) bytesWritten);
	totalBytesWritten += bytesWritten;

        // print as many bytes as readData told us
        fwrite(result, 1, bytesWritten, stdout);
    }
    return 0;
}
