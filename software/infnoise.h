#include <stdbool.h>
#include <stdint.h>
#include <sys/types.h>

// Required accuracy of estimated vs measured entropy in health monitor
#define INM_ACCURACY 1.03

// The FT240X has a 512 byte buffer.  Must be multiple of 64
// We also write this in one go to the Keccak sponge, which is at most 1600 bits
#define BUFLEN 512

// This is how many previous bits are used to predict the next bit from the INM
#define PREDICTION_BITS 14

// This is the maximum time we allow to pass to perform the I/O operations, since long
// delays can reduce entropy from the INM.
#define MAX_MICROSEC_FOR_SAMPLES 5000

// This is the gain of each of the two op-amp stages in the INM
#define DESIGN_K 1.84

#define BITMODE_SYNCBB 0x4

//#define VERSION1

// This defines which pins on the FT240X are used
#ifdef VERSION1

// The V1 version is the original raw board with the edge connector instead of a real USB plug
#define COMP1 2
#define COMP2 0
#define SWEN1 4
#define SWEN2 1

#else

// This is the production version with a real USB plug
#define COMP1 1
#define COMP2 4
#define SWEN1 2
#define SWEN2 0

#endif

// The remaining 8 bits are driven with 0 .. 15 to help track the cause of misfires
#define ADDR0 3
#define ADDR1 5
#define ADDR2 6
#define ADDR3 7

// All data bus bits of the FT240X are outputs, except COMP1 and COMP2
#define MASK (0xff & ~(1 << COMP1) & ~(1 << COMP2))

bool inmHealthCheckStart(uint8_t N, double K, bool debug);
void inmHealthCheckStop(void);
bool inmHealthCheckAddBit(bool evenBit, bool oddBit, bool even, uint8_t addr);
bool inmHealthCheckOkToUseData(void);
double inmHealthCheckEstimateK(void);
double inmHealthCheckEstimateEntropyPerBit(void);
uint32_t inmGetEntropyLevel(void);
void inmClearEntropyLevel(void);
bool inmEntropyOnTarget(uint32_t entropy, uint32_t bits);
void inmWriteEntropyStart(uint32_t bufLen, bool debug);
void inmWriteEntropyToPool(uint8_t *bytes, uint32_t length, uint32_t entropy);
void inmWaitForPoolToHaveRoom(void);
void inmDumpStats(void);
void startDaemon(bool daemon, bool pidFile, char *fileName);
bool isSuperUser(void);

extern double inmK, inmExpectedEntropyPerBit;
