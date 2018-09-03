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
    bool initKeccak = false;	// initialize Keccak sponge (used for whitening)
    uint32_t multiplier = 1u;	// multiplier for whitening
    bool debug = true;		// debug mode (health monitor writes to stderr)

    // initialize hardware and health monitor
    struct infnoise_context context;
    fprintf(stdout, "pre-initi: %s\n", "");

    if (!initInfnoise(&context, serial, initKeccak, debug)) {
        fprintf(stdout, "erri: %s\n", "");
        fputs(context.message, stderr);
        return 1; // ERROR
    }
    fprintf(stdout, "initi: %s\n", "");

    uint32_t resultSize;
    if (multiplier == 0 || initKeccak == false) {
        resultSize = 512u;
    } else {
        resultSize = 1024u;
    }

    // read and print in a loop (until 1M is read)
    uint64_t totalBytesWritten = 0u;
    while (totalBytesWritten < 1000000) {
        uint8_t result[resultSize];

	// readRawData returns the number of bytes written to result array
        uint64_t bytesWritten = readData(&context, result, !initKeccak, multiplier);
        fprintf(stderr, "infnoise bytes read: %lu\n", bytesWritten);

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
