#include <stdbool.h>
#include <stdint.h>
#include <sys/types.h>
#if defined(__OpenBSD__) || defined(__NetBSD__) || defined(__DragonFly__) || defined(__APPLE__) || defined(__FreeBSD__)
#include <limits.h>
#else
#include <linux/limits.h>
#endif
#include <ftdi.h>
#include <time.h>

// The FT240X has a 512 byte buffer.  Must be multiple of 64
// We also write this in one go to the Keccak sponge, which is at most 1600 bits
#define BUFLEN 512u

bool listUSBDevices(struct ftdi_context *ftdic, char **message);

bool initInfnoise(struct ftdi_context *ftdic, char *serial, char **message, bool keccak, bool debug);

// If |result| is NULL, then output is to stdout.
uint32_t readRawData(struct ftdi_context *ftdic, uint8_t *result, char **message, bool *errorFlag);

// If |result| is NULL, then output is to stdout.
uint32_t readData(struct ftdi_context *ftdic, uint8_t *result, char **message, bool *errorFlag, uint32_t outputMultiplier);
