#include <stdbool.h>
#include <stdint.h>
#include <sys/types.h>
#include <linux/limits.h>
#include <ftdi.h>
#include <time.h>

// The FT240X has a 512 byte buffer.  Must be multiple of 64
// We also write this in one go to the Keccak sponge, which is at most 1600 bits
#define BUFLEN 512u

bool listUSBDevices(struct ftdi_context *ftdic, char **message);

bool initInfnoise(struct ftdi_context *ftdic, char *serial, char **message, bool keccak, bool debug);

uint32_t readRawData(struct ftdi_context *ftdic, uint8_t *result, char **message, bool *errorFlag);

uint32_t readData(struct ftdi_context *ftdic, uint8_t *result, char **message, bool *errorFlag, uint32_t outputMultiplier);
