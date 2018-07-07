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
#include <sys/types.h>
#include <ftdi.h> // requires <sys/types.h>
#include <getopt.h>
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

// getopt_long(3) options descriptor

static struct option longopts[] = {{"raw", no_argument, NULL, 'r'},
                                   {"debug", no_argument, NULL, 'D'},
                                   {"dev-random", no_argument, NULL, 'R'},
                                   {"no-output", no_argument, NULL, 'n'},
                                   {"multiplier", required_argument, NULL, 'm'},
                                   {"pidfile", required_argument, NULL, 'p'},
                                   {"serial", required_argument, NULL, 's'},
                                   {"daemon", no_argument, NULL, 'd'},
                                   {"list-devices", no_argument, NULL, 'l'},
                                   {"version", no_argument, NULL, 'v'},
                                   {"help", no_argument, NULL, 'h'},
                                   {NULL, 0, NULL, 0}};

int main(int argc, char **argv) {
    struct ftdi_context ftdic;
    struct opt_struct opts;
    int ch;
    bool multiplierAssigned = false;

    initOpts(&opts);

    // Process arguments
    while ((ch = getopt_long(argc, argv, "rDRnm:p:s:dlvh", longopts, NULL)) !=
           -1) {
        switch (ch) {
        case 'r':
            opts.raw = true;
            break;
        case 'D':
            opts.debug = true;
            break;
        case 'R':
            opts.devRandom = true;
            break;
        case 'n':
            opts.noOutput = true;
            break;
        case 'm':
            multiplierAssigned = true;
            int tmpOutputMult = atoi(optarg);
            if (tmpOutputMult < 0) {
                fputs("Multiplier must be >= 0\n", stderr);
                return 1;
            }
            opts.outputMultiplier = tmpOutputMult;
            break;
        case 'p':
            opts.pidFileName = optarg;
            if (opts.pidFileName == NULL || !strcmp("", opts.pidFileName)) {
                fputs("--pidfile without file name\n", stderr);
                return 1;
            }
            break;
        case 's':
            opts.serial = optarg;
            if (opts.serial == NULL || !strcmp("", opts.serial)) {
                fputs("--serial without value\n", stderr);
                return 1;
            }
            break;
        case 'd':
            opts.daemon = true;
            break;
        case 'l':
            opts.listDevices = true;
            break;
        case 'v':
            opts.version = true;
            break;
        case 'h':
            opts.help = true;
            break;
		default:
            opts.help = true;
            opts.none = true;
        }
    }

    if (opts.help) {
        fputs("Usage: infnoise [options]\n"
              "Options are:\n"
              "    -D, --debug - turn on some debug output\n"
              "    -R, --dev-random - write entropy to /dev/random instead of "
              "stdout\n"
              "    -r, --raw - do not whiten the output\n"
              "    -m, --multiplier <value> - write 256 bits * value for each 512 bits written to\n"
              "      the Keccak sponge.  Default of 0 means write all the entropy.\n"
              "    -n, --no-output - do not write random output data\n"
              "    -p, --pidfile <file> - write process ID to file\n"
              "    -d, --daemon - run in the background\n"
              "    -s, --serial <serial> - use specified device\n"
              "    -l, --list-devices - list available devices\n"
              "    -v, --version - show version information\n"
              "    -h, --help - this help output\n",
              stdout);
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

    char *message = "no data?";
    bool errorFlag = false;
    if (opts.listDevices) {
        if(!listUSBDevices(&ftdic, &message)) {
            fputs(message, stderr);
            return 1;
        }
        //fputs(message, stdout); // todo: put list of devices to &message and print here, not in libinfnoise
        return 0;
    }

    if (opts.devRandom) {
#ifdef LINUX
        inmWriteEntropyStart(BUFLEN/8u, opts.debug); // todo: create method in libinfnoise.h for this?
        // also todo: check superUser in this mode (it will fail silently if not :-/)
#endif
#ifdef MACOS
        message = "dev/random not supported on macOS";
        return 0;
#endif
    }

    // Optionally run in the background and optionally write a PID-file
    startDaemon(&opts);

    // initialize USB device, health check and Keccak state (see libinfnoise)
    if (!initInfnoise(&ftdic, opts.serial, &message, !opts.raw, opts.debug)) {
        fputs(message, stderr);
        return 1; // ERROR
    }

    // endless loop
    uint64_t totalBytesWritten = 0u;
    while(true) {
        uint64_t prevTotalBytesWritten = totalBytesWritten;
        totalBytesWritten += readData_private(&ftdic, NULL, &message, &errorFlag, opts.noOutput, opts.raw, opts.outputMultiplier, opts.devRandom); // calling libinfnoise's private readData method

        if (errorFlag) {
            fprintf(stderr, "Error: %s\n", message);
            return 1;
        }

        if(opts.debug && (1u << 20u)*(totalBytesWritten/(1u << 20u)) > (1u << 20u)*(prevTotalBytesWritten/(1u << 20u))) {
            fprintf(stderr, "Output %lu bytes\n", (unsigned long)totalBytesWritten);
        }
    }
    return 0;
}
