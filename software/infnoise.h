#include <stdbool.h>
#include <stdint.h>
#include <sys/types.h>

// Structure for parsed command line options
struct opt_struct {
	uint32_t outputMultiplier; // We output all the entropy when outputMultiplier == 0
	bool daemon;		// Run as daemon?
	bool debug;		// Print debugging info?
	bool devRandom;		// Feed /dev/random?
	bool noOutput;		// Supress output?
	bool listDevices;	// List possible USB-devices?
	bool help;		// Show help
	bool none;		// set to true when no valid arguments where detected
	bool raw;		// No whitening?
	bool version;		// Show version
	char *pidFileName;	// Name of optional PID-file
	char *serial;		// Name of selected device
};

//bool inmHealthCheckStart(uint8_t N, double K, struct opt_struct *opts);
//void inmHealthCheckStop(void);
/*bool inmHealthCheckAddBit(bool evenBit, bool oddBit, bool even);
bool inmHealthCheckOkToUseData(void);
double inmHealthCheckEstimateK(void);
double inmHealthCheckEstimateEntropyPerBit(void);
uint32_t inmGetEntropyLevel(void);
void inmClearEntropyLevel(void);
bool inmEntropyOnTarget(uint32_t entropy, uint32_t bits);
void inmWriteEntropyStart(uint32_t bufLen, struct opt_struct *opts);
void inmWriteEntropyToPool(uint8_t *bytes, uint32_t length, uint32_t entropy);
void inmWaitForPoolToHaveRoom(void);
void inmDumpStats(void);
*/
void startDaemon(struct opt_struct *opts);
bool isSuperUser(void);


//extern double inmK, inmExpectedEntropyPerBit;

