#include <stdbool.h>
#include <stdint.h>

// Required accuracy of estimated vs measured entropy in health monitor
#define INM_ACCURACY 1.02

bool inmHealthCheckStart(uint8_t N, double K, bool debug);
void inmHealthCheckStop(void);
bool inmHealthCheckAddBit(bool evenBit, bool oddBit, bool even);
bool inmHealthCheckOkToUseData(void);
double inmHealthCheckEstimateK(void);
double inmHealthCheckEstimateEntropyPerBit(void);
uint32_t inmGetEntropyLevel(void);
void inmClearEntropyLevel(void);
bool inmEntropyOnTarget(uint32_t entropy, uint32_t bits);
void inmWriteEntropyStart(uint32_t bufLen, bool debug);
void inmWriteEntropyToPool(uint8_t *bytes, uint32_t length, uint32_t entropy);
void inmWaitForPoolToHaveRoom(void);

extern double inmK, inmExpectedEntropyPerBit;
