/*
This is a simple example to use libinfnoise with whitened and multiplied output.
*/

#include <stdio.h>
#include <ftdi.h>
#include <libinfnoise.h>

int main()
{
    // parameters
    char *serial=NULL; 		// can be set to a specific serial, NULL uses the first found device
    bool initKeccak = true;	// initialize Keccak sponge (used for whitening)
    uint32_t multiplier = 1u;	// multiplier for whitening
    bool debug = false;		// debug mode (health monitor writes to stderr)

    // initialize hardware and health monitor
    struct infnoise_context context;
    if (!initInfnoise(&context, serial, initKeccak, debug)) {
        fprintf(stderr, "Error: %s\n", context.message);
        return 1; // ERROR
    }

    // fixed result size of 512 bit (64byte)
    uint8_t resultSize = 64u;

    // read and print in a loop (until 1MB is read)
    uint64_t totalBytesWritten = 0u;
    while (totalBytesWritten < 1000000) {
        uint8_t result[resultSize];

	context.errorFlag = false;
	// readRawData returns the number of bytes written to result array
        uint64_t bytesWritten = readData(&context, result, !initKeccak, multiplier);

	// check for errors
	// note: bytesWritten is also 0 in this case, but an errorFlag is needed as
        // bytesWritten can also be 0 when data hasn't passed the health monitor.
	// (which happens sometimes in normal operation - and is expected behaviour)
        if (context.errorFlag) {
            fprintf(stderr, "Error: %s\n", context.message);
            return -1;
        }

        // print as many bytes as readData told us
        fwrite(result, 1, bytesWritten, stdout);

	// sum up
	totalBytesWritten += bytesWritten;
        fprintf(stderr, "bytes read: %lu\n", (unsigned long) totalBytesWritten);

    }
    return 0;
}
