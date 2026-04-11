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
        fprintf(stdout, "Error: %s\n", context.message);
        return 1; // ERROR
    }

    uint32_t resultSize;
    if (multiplier <= 2 || initKeccak == false) {
        resultSize = 64u;
    } else {
        resultSize = 128u;
    }

    // read and print in a loop (until 1M is read)
    uint64_t totalBytesWritten = 0u;
    while (totalBytesWritten < 1000000) {
        uint8_t result[resultSize];

	// readData returns bytes written (>0), transient (0), or error (<0)
        int32_t rc = readData(&context, result, !initKeccak, multiplier);
        if (rc < 0) {
            fprintf(stderr, "Error: %s\n", context.message);
            return -1;
        }
        uint32_t bytesWritten = (uint32_t)rc;
	totalBytesWritten += bytesWritten;
        fprintf(stderr, "infnoise bytes read: %lu\n", (unsigned long) totalBytesWritten);

        // print as many bytes as readData told us
        fwrite(result, 1, bytesWritten, stdout);
    }
    return 0;
}
