bool inmHealthCheckStart(uint8_t N, double K);
void inmHealthCheckStop(void);
bool inmHealthCheckAddBit(bool bit);
bool inmHealthCheckOkToUseData(void);
double inmHealthCheckEstimateK(void);
double inmHealthCheckEstimateEntropyPerBit(void);
// Returns number of bytes of entropy added so far
uint32_t inmHealthCheckGetEntropyLevel(void);
void inmHealthCheckReduceEntropyLevel(uint32_t numBytes);
