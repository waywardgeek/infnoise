#include <stdbool.h>
#include <stdint.h>
#include <sys/types.h>
#if defined(__OpenBSD__) || defined(__NetBSD__) || defined(__DragonFly__) || defined(__APPLE__) || defined(__FreeBSD__)
#include <limits.h>
#elif !defined(_WIN32)
#include <linux/limits.h>
#endif

// Structure for parsed command line options
struct opt_struct {
        uint32_t outputMultiplier; // We output all the entropy when outputMultiplier == 0
        bool daemon;            // Run as daemon?
        bool debug;             // Print debugging info?
        bool devRandom;         // Feed /dev/random?
        bool noOutput;          // Supress output?
        bool listDevices;       // List possible USB-devices?
        bool help;              // Show help
        bool none;              // set to true when no valid arguments where detected
        bool raw;               // No whitening?
        bool version;           // Show version
        char *pidFileName;      // Name of optional PID-file
        char *serial;           // Name of selected device
};


void inmWriteEntropyStart(uint32_t bufLen, bool debug);
void inmWriteEntropyEnd();
void inmWriteEntropyToPool(uint8_t *bytes, uint32_t length, uint32_t entropy);
void inmWaitForPoolToHaveRoom(void);

void startDaemon(struct opt_struct *opts);
