/* Driver for the Infinite Noise Multiplier USB stick */

// Required to include clock_gettime
#define _POSIX_C_SOURCE 200809L

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

#include <ftdi.h>
#include "infnoise.h"
#include "libinfnoise.h"
#include "libinfnoise_private.h"
#include "KeccakF-1600-interface.h"

static void initOpts(struct opt_struct *opts) {
	opts->outputMultiplier = 0u;
	opts->daemon = false;
	opts->debug = false;
	opts->devRandom = false;
	opts->noOutput = false;
	opts->listDevices = false;
	opts->raw = false;
	opts->version = false;
	opts->help = false;
	opts->none = false;
	opts->pidFileName =
	opts->serial = NULL;
}

int main(int argc, char **argv)
{
    struct ftdi_context ftdic;
    struct opt_struct opts;
    int xArg;
    bool multiplierAssigned = false;

    initOpts(&opts);

    // Process arguments
    for(xArg = 1; xArg < argc; xArg++) {
        if(!strcmp(argv[xArg], "--raw")) {
            opts.raw = true;
        } else if(!strcmp(argv[xArg], "--debug")) {
            opts.debug = true;
        } else if(!strcmp(argv[xArg], "--dev-random")) {
            opts.devRandom = true;
        } else if(!strcmp(argv[xArg], "--no-output")) {
            opts.noOutput = true;
        } else if(!strcmp(argv[xArg], "--multiplier") && xArg+1 < argc) {
            xArg++;
            multiplierAssigned = true;
            int tmpOutputMult = atoi(argv[xArg]);
            if(tmpOutputMult < 0) {
                fputs("Multiplier must be >= 0\n", stderr);
                return 1;
            }
            opts.outputMultiplier = tmpOutputMult;
        } else if(!strcmp(argv[xArg], "--pidfile")) {
            xArg++;
            opts.pidFileName = argv[xArg];
            if(opts.pidFileName == NULL || !strcmp("", opts.pidFileName)) {
                fputs("--pidfile without file name\n", stderr);
                return 1;
            }
        } else if(!strcmp(argv[xArg], "--serial")) {
            xArg++;
            opts.serial = argv[xArg];
            if(opts.serial == NULL || !strcmp("",opts.serial)) {
                fputs("--serial without value\n", stderr);
                return 1;
            }
        } else if(!strcmp(argv[xArg], "--daemon")) {
            opts.daemon = true;
        } else if(!strcmp(argv[xArg], "--list-devices")) {
            opts.listDevices = true;
        } else if(!strcmp(argv[xArg], "--version") || !strcmp(argv[xArg], "-v")) {
            opts.version = true;
        } else if(!strcmp(argv[xArg], "--help") || !strcmp(argv[xArg], "-h")) {
            opts.help = true;
        } else {
            opts.help = true;
	    opts.none = true;
        }
    }

    if (opts.help) {
            fputs("Usage: infnoise [options]\n"
                            "Options are:\n"
                            "    --debug - turn on some debug output\n"
                            "    --dev-random - write entropy to /dev/random instead of stdout\n"
                            "    --raw - do not whiten the output\n"
                            "    --multiplier <value> - write 256 bits * value for each 512 bits written to\n"
                            "      the Keccak sponge.  Default of 0 means write all the entropy.\n"
                            "    --no-output - do not write random output data\n"
                            "    --pidfile <file> - write process ID to file\n"
                            "    --daemon - run in the background\n"
                            "    --serial <serial> - use specified device\n"
                            "    --list-devices - list available devices\n"
                            "    --version - show version information\n"
                            "    --help - this help output\n", stdout);
	if (opts.none) {
            return 1;
        } else {
            return 0;
        }
    }

    // read environment variables, not overriding command line options
    if (opts.serial == NULL) {
        if (getenv("INFNOISE_SERIAL") != NULL) {
            opts.serial = getenv("INFNOISE_SERIAL");
        }
    }

    if (opts.debug == false) {
        if (getenv("INFNOISE_DEBUG") != NULL) {
            if (!strcmp("true",getenv("INFNOISE_DEBUG"))) {
                opts.debug = true;
            }
        }
    }

    if (multiplierAssigned == false) {
        if (getenv("INFNOISE_MULTIPLIER") != NULL) {
            int tmpOutputMult = atoi(getenv("INFNOISE_MULTIPLIER"));
            if (tmpOutputMult < 0) {
                fputs("Multiplier must be >= 0\n", stderr);
                return 1;
            }
            multiplierAssigned = true;
            opts.outputMultiplier = tmpOutputMult;
        }
    }

    if(!multiplierAssigned && opts.devRandom) {
        opts.outputMultiplier = 2u; // Don't throw away entropy when writing to /dev/random unless told to do so
    }

    if (opts.version) {
	printf("GIT VERSION - %s\n", GIT_VERSION);
	printf("GIT COMMIT  - %s\n", GIT_COMMIT);
	printf("GIT DATE    - %s\n", GIT_DATE);
	return 0;
    }

    char *message;
    if (opts.listDevices) {
        struct inm_devlist *device_list;
        device_list = malloc(sizeof(struct inm_devlist));

        if(!listUSBDevices(&ftdic, device_list, &message)) {
            fputs(message, stderr);
            return 1;
        }

	// debug:
        uint8_t i=0;
        struct inm_devlist_node *tmp;
        for ( tmp = device_list->head; tmp != NULL; tmp=tmp->next) {
            if (tmp->device->serial != NULL) {
                printf("%s\n", tmp->device->serial);
            }
            printf("%d\n", i);
            i+=1;
        }
	return 0;
    }

    if (opts.devRandom) {
        inmWriteEntropyStart(BUFLEN/8u, opts.debug); // todo: create method in libinfnoise.h for this
	// also todo: check if superUser in this mode (it will fail silently if not :-/)
    }

    // Optionally run in the background and optionally write a PID-file
    startDaemon(&opts);

    // initialize USB device and health check
    if (initInfnoise(&ftdic, opts.serial, &message, opts.debug) != true) {
        fputs(message, stderr);
        return 1; // ERROR (message still goes to stderr)
    }

    // initialize keccak
    KeccakInitialize();
    uint8_t keccakState[KeccakPermutationSizeInBytes];
    KeccakInitializeState(keccakState);

    uint8_t result[1024]; // only used in noOutput mode (and libinfnoise)

    uint64_t totalBytesWritten = 0u;
    while(true) {
        uint64_t prevTotalBytesWritten = totalBytesWritten;
        uint64_t bytesWritten = readData_private(&ftdic, keccakState, result, &message, opts.noOutput, opts.raw, opts.outputMultiplier, opts.devRandom); // calling libinfnoise's private readData method

        if (totalBytesWritten == (unsigned long)-1) {
            fputs(message, stderr);
            return 1;
        }
        totalBytesWritten += bytesWritten;
        if(opts.debug && (1u << 20u)*(totalBytesWritten/(1u << 20u)) > (1u << 20u)*(prevTotalBytesWritten/(1u << 20u))) {
            fprintf(stderr, "Output %lu bytes\n", (unsigned long)totalBytesWritten);
        }
    }
    return 0;
}
