#include <stdbool.h>
#include <stdint.h>
#include <sys/types.h>
#if defined(__OpenBSD__) || defined(__NetBSD__) || defined(__DragonFly__) || defined(__APPLE__) || defined(__FreeBSD__)
#include <limits.h>
#elif !defined(_WIN32)
#include <linux/limits.h>
#endif
#if !defined(_WIN32)
#include <ftdi.h>
#endif
#include <time.h>
#include "libinfnoise.h"

#define INFNOISE_VENDOR_ID 0x0403
#define INFNOISE_PRODUCT_ID 0x6015

// Required accuracy of estimated vs measured entropy in health monitor
#define INM_ACCURACY 1.03

// This is how many previous bits are used to predict the next bit from the INM
#define PREDICTION_BITS 14u

// This is the maximum time we allow to pass to perform the I/O operations, since long
// delays can reduce entropy from the INM.
#define MAX_MICROSEC_FOR_SAMPLES 5000u

// This is the gain of each of the two op-amp stages in the INM
#define DESIGN_K 1.84

#define BITMODE_SYNCBB 0x4

// This defines which pins on the FT240X are used
#define COMP1 1u
#define COMP2 4u
#define SWEN1 2u
#define SWEN2 0u

// The remaining 8 bits are driven with 0 .. 15 to help track the cause of misfires
#define ADDR0 3u
#define ADDR1 5u
#define ADDR2 6u
#define ADDR3 7u

// All data bus bits of the FT240X are outputs, except COMP1 and COMP2
#define MASK (0xffu & ~(1u << COMP1) & ~(1u << COMP2))

bool inmHealthCheckStart(uint8_t N, double K, bool debug);
void inmHealthCheckStop(void);
bool inmHealthCheckAddBit(bool evenBit, bool oddBit, bool even);
bool inmHealthCheckOkToUseData(void);
double inmHealthCheckEstimateK(void);
double inmHealthCheckEstimateEntropyPerBit(void);
uint32_t inmGetEntropyLevel(void);
void inmClearEntropyLevel(void);
bool inmEntropyOnTarget(uint32_t entropy, uint32_t bits);

void inmDumpStats(void);

extern double inmK, inmExpectedEntropyPerBit;

#if !defined(_WIN32)

bool initializeUSB(struct ftdi_context *ftdic, const char **message,char *serial);

struct timespec;
double diffTime(struct timespec *start, struct timespec *end);
uint32_t extractBytes(uint8_t *bytes, uint32_t length, uint8_t *inBuf, const char **message, bool *errorFlag);

bool outputBytes(uint8_t *bytes, uint32_t length, uint32_t entropy, bool writeDevRandom, const char **message);

#endif
