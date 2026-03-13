/* Driver for the Infinite Noise Multiplier USB stick */

// Required to include clock_gettime
#define _POSIX_C_SOURCE 200809L

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#if defined(__OpenBSD__) || defined(__NetBSD__) || defined(__DragonFly__) || defined(__FreeBSD__)
#include <fcntl.h>
#endif

#include <string.h>
#include <signal.h>
#include <time.h>
#include <getopt.h>
#include "infnoise.h"
#include "libinfnoise.h"


volatile sig_atomic_t running = 1;

void term(int signum)
{
   (void) signum;
   running = 0;
}

static void initOpts(struct opt_struct *opts) {
    opts->outputMultiplier = 0u;
    opts->feedFreq = 30u;
    opts->forceReseed = false;
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
static struct option longopts[] = {
        {"raw",          no_argument,       NULL, 'r'},
        {"debug",        no_argument,       NULL, 'D'},
        {"dev-random",   no_argument,       NULL, 'R'},
        {"reseed-crng",  no_argument,       NULL, 'C'},
        {"no-output",    no_argument,       NULL, 'n'},
        {"feed-frequency",    required_argument, NULL, 'f'},
        {"multiplier",   required_argument, NULL, 'm'},
        {"pidfile",      required_argument, NULL, 'p'},
        {"serial",       required_argument, NULL, 's'},
        {"daemon",       no_argument,       NULL, 'd'},
        {"list-devices", no_argument,       NULL, 'l'},
        {"version",      no_argument,       NULL, 'v'},
        {"help",         no_argument,       NULL, 'h'},
        {NULL,           0,                 NULL, 0}};

// Write the bytes to either stdout, or /dev/random.
bool outputBytes(uint8_t *bytes, uint32_t length, uint32_t entropy, bool writeDevRandom, bool forceKernelReseed, uint32_t feedFrequency, const char **message) {
    if (!writeDevRandom) {
        if (fwrite(bytes, 1, length, stdout) != length) {
            *message = "Unable to write output from Infinite Noise Multiplier";
            return false;
        }
    } else {
#if defined(__OpenBSD__) || defined(__NetBSD__) || defined(__DragonFly__) || defined(__FreeBSD__)
        // quell compiler warning about unused variable.
        static int devRandomFD = -1;
        (void) entropy;

        if (devRandomFD < 0)
            devRandomFD = open("/dev/random", O_WRONLY);
        if (devRandomFD < 0) {
            *message = "Unable to open random(4)";
            return false;
        };
        // we are not trapping EINT and EAGAIN; as the random(4) driver seems
        // to not treat partial writes as not an error. So we think that comparing
        // to length is fine.
        //
        if (write(devRandomFD, bytes, length) != length) {
            *message = "Unable to write output from Infinite Noise Multiplier to random(4)";
            return false;
        }
#endif
#if defined(__APPLE__)
        *message = "macOS doesn't support writes to entropy pool";
        // suppress warning
        (void) entropy;
        (void) feedFrequency;
        return false;
#endif
#ifdef LINUX
        inmWaitForPoolToHaveRoom(feedFrequency);
        inmWriteEntropyToPool(bytes, length, entropy);
        if(forceKernelReseed) {
            inmForceKernelRngReseed();
        }
#endif
    }
    return true;
}

int main(int argc, char **argv) {
    struct infnoise_context context;
    struct opt_struct opts;
    int ch;
    bool multiplierAssigned = false;

    initOpts(&opts);

    // Process arguments
    while ((ch = getopt_long(argc, argv, "rDRCnf:m:p:s:dlvh", longopts, NULL)) !=
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
            case 'C':
                opts.forceReseed = true;
                break;
            case 'n':
                opts.noOutput = true;
                break;
            case 'f': ;
                int tmpFeedrate = atoi(optarg);
                if (tmpFeedrate < 0) {
                    fputs("feed-freq must be >= 0\n", stderr);
                    return 1;
                }
                opts.feedFreq = tmpFeedrate;
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
                if (opts.pidFileName == NULL || opts.pidFileName[0] == '\0') {
                    fputs("--pidfile without file name\n", stderr);
                    return 1;
                }
                break;
            case 's':
                opts.serial = optarg;
                if (opts.serial == NULL || opts.serial[0] == '\0') {
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
              "    -C, --reseed-crng - force reseed of /dev/random on write\n"
              "    -r, --raw - do not whiten the output\n"
              "    -m, --multiplier <value> - write 256 bits * value for each 512 bits written to\n"
              "      the Keccak sponge.  Default of 0 means write all the entropy.\n"
              "    -f, --feed-frequency - feed interval for /dev/random, in seconds (default: 30)\n"
              "    -n, --no-output - do not write random output data\n"
              "    -p, --pidfile <file> - write process ID to file\n"
              "    -d, --daemon - run in the background\n"
              "    -s, --serial <serial> - use specified device\n"
              "    -l, --list-devices - list available devices\n"
              "    -v, --version - show version information\n"
              "    -h, --help - this help output\n",
              stdout);
        return opts.none;
    }

    if (opts.version) {
        printf("GIT VERSION - %s\n", GIT_VERSION);
        printf("GIT COMMIT  - %s\n", GIT_COMMIT);
        printf("GIT DATE    - %s\n", GIT_DATE);
        return 0;
    }

    // read environment variables, not overriding command line options
    if (opts.serial == NULL) {
        opts.serial = getenv("INFNOISE_SERIAL");
    }

    if (!opts.debug) {
        char *envDbg = getenv("INFNOISE_DEBUG");
        opts.debug = (envDbg != NULL
                      && !strcmp("true", envDbg));
    }

    if (!multiplierAssigned) {
        char *envMultiplier = getenv("INFNOISE_MULTIPLIER");
        if (envMultiplier != NULL) {
            int tmpOutputMult = atoi(envMultiplier);
            if (tmpOutputMult < 0) {
                fputs("Multiplier must be >= 0\n", stderr);
                return 1;
            }
            multiplierAssigned = true;
            opts.outputMultiplier = tmpOutputMult;
        }
    }

    if (!multiplierAssigned && opts.devRandom) {
        opts.outputMultiplier = 2u; // Don't throw away entropy when writing to /dev/random unless told to do so
    }

    if (opts.listDevices) {
        infnoise_devlist_node_t* devlist = listUSBDevices(&context.message);
        if (devlist == NULL) {
            fprintf(stderr, "Error: %s\n", context.message);
            return 1;
        }
        infnoise_devlist_node_t* curdev;
        int id = 0;
        for (curdev = devlist; curdev != NULL; id++) {
            printf("ID: %i, Manufacturer: %s, Description: %s, Serial: %s\n",
                   id, curdev->manufacturer, curdev->description, curdev->serial);

            // cleanup:
            infnoise_devlist_node_t* olddev = curdev;
            curdev = curdev->next;
            free(olddev);
        }
        return 0;
    }

    if (opts.devRandom) {
#ifdef LINUX
        inmWriteEntropyStart(BUFLEN / 8u, opts.debug); // TODO: create method in libinfnoise.h for this?
        // also TODO: check superUser in this mode (it will fail silently if not :-/)
#endif
#if defined(__OpenBSD__) || defined(__NetBSD__) || defined(__DragonFly__) || defined(__FreeBSD__)
        int devRandomFD = open("/dev/random", O_WRONLY);
        if (devRandomFD < 0) {
            fprintf(stderr, "Unable to open /dev/random\n");
            exit(1);
        }
        close(devRandomFD);
#endif
#if defined(__APPLE__)
        context.message = "dev/random not supported on macOS";
        fprintf(stderr, "Error: %s\n", context.message);
        return 1;
#endif
    }

    // Optionally run in the background and optionally write a PID-file
    startDaemon(&opts);

    // initialize USB device, health check and Keccak state (see libinfnoise)
    if (!initInfnoise(&context, opts.serial, !opts.raw, opts.debug)) {
        fprintf(stderr, "Error: %s\n", context.message);
        return 1; // ERROR
    }

    // calculate output size based on the parameters:
    uint64_t resultSize;
    if (opts.outputMultiplier <= 2 || opts.raw) {
        resultSize = 64u;
    } else {
        resultSize = 128u;
    }
    //fprintf(stderr, "resultsize: %lu\n", resultSize);

    // get proper shutdown
    struct sigaction action;
    memset(&action, 0, sizeof(action));
    action.sa_handler = term;
    sigaction(SIGTERM, &action, NULL);
    sigaction(SIGINT, &action, NULL);

    // endless loop
    uint64_t totalBytesWritten = 0u;
    while (running) {
        uint8_t result[resultSize];
        uint64_t bytesWritten = readData(&context, result, opts.raw, opts.outputMultiplier);
        totalBytesWritten += bytesWritten;

        if (context.errorFlag) {
            fprintf(stderr, "Error: %s\n", context.message);
            return 1;
        }

        if (!opts.noOutput
            && !outputBytes(result, bytesWritten, context.entropyThisTime, opts.devRandom, opts.forceReseed, opts.feedFreq, &context.message)) {
            fprintf(stderr, "Error: %s\n", context.message);
            return 1;
        }

        if (opts.debug &&
            (1u << 20u) * (totalBytesWritten / (1u << 20u)) > (1u << 20u) * (totalBytesWritten + bytesWritten / (1u << 20u))) {
            fprintf(stderr, "Output %lu bytes\n", (unsigned long) totalBytesWritten);
        }
    }
    deinitInfnoise(&context);
#ifdef LINUX
    inmWriteEntropyEnd();
#endif
    return 0;
}
