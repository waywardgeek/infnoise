/*
This is a very simple example to use libinfnoise in raw output mode.
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
    bool debug=true;		// debug mode (health monitor writes to stderr)

    char *message;
    bool errorFlag = false;

    // initialize hardware and health monitor
    struct ftdi_context ftdic;
    if (!initInfnoise(&ftdic, serial, &message, false, debug)) {
        fputs(message, stderr);
        return 1; // ERROR
    }

    // read and print in a loop (until 1M)
    uint32_t totalBytesWritten = 0;
    while (totalBytesWritten < 1000000) {
        uint8_t result[64];// result is always 64 bytes in raw mode

	// read data returns the number of bytes written to result array
        uint64_t bytesWritten = readRawData(&ftdic, result, &message, &errorFlag);
	totalBytesWritten += bytesWritten;

	// check for errors
	// note: bytesWritten is also 0 in this case, but an errorFlag is needed as
        // bytesWritten can also be 0 when data hasn't passed the health monitor.
        if (errorFlag) {
            fprintf(stderr, "Error: %s\n", message);
            return 1;
        }

        // print bytes to stdout
        fwrite(result, 1, bytesWritten, stdout);
    }
    return 0;
}
