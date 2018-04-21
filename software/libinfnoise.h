#include <stdbool.h>
#include <stdint.h>
#include <sys/types.h>
#include <ftdi.h>
#include <time.h>

// The FT240X has a 512 byte buffer.  Must be multiple of 64
// We also write this in one go to the Keccak sponge, which is at most 1600 bits
#define BUFLEN 512u

bool listUSBDevices(struct ftdi_context *ftdic, char **message);

bool initInfnoise(struct ftdi_context *ftdic, char *serial, char **message, bool debug);

bool initKeccak(struct ftdi_context *ftdic, char *serial);

uint64_t readRawData(struct ftdi_context *ftdic, uint8_t *result, char **message);

uint64_t readData(struct ftdi_context *ftdic, uint8_t *keccakState, uint8_t *result, char **message, uint32_t outputMultiplier);
