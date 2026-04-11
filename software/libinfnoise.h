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

// The FT240X has a 512 byte buffer.  Must be multiple of 64
// We also write this in one go to the Keccak sponge, which is at most 1600 bits
#define BUFLEN 512u

// KeccakPermutationSizeInBytes = 1600/8 = 200; avoid Keccak header dependency
#define INFNOISE_KECCAK_STATE_SIZE 200

#ifdef __cplusplus
extern "C" {
#endif

// Machine-readable error codes for readData and initInfnoise.
// Negative values are fatal errors; zero is transient (retry); positive is success.
typedef enum {
    INFNOISE_OK              =  0,
    INFNOISE_ERR_USB_WRITE   = -1,
    INFNOISE_ERR_USB_READ    = -2,
    INFNOISE_ERR_HEALTH      = -3,
    INFNOISE_ERR_TIMING      = -4,
    INFNOISE_ERR_ENTROPY     = -5,
    INFNOISE_ERR_NOT_FOUND   = -6,
    INFNOISE_ERR_INIT        = -7,
    INFNOISE_ERR_USB_BAUD    = -8,
    INFNOISE_ERR_USB_BITMODE = -9,
} infnoise_error_t;

#if !defined(_WIN32)

// Health checker state — previously file-scope globals in healthcheck.c
struct infnoise_health_state {
    // Configuration (set once in inmHealthCheckStart)
    uint8_t  N;
    double   K;
    double   expectedEntropyPerBit;
    bool     debug;

    // Dynamically allocated prediction tables
    uint32_t *onesEven;
    uint32_t *zerosEven;
    uint32_t *onesOdd;
    uint32_t *zerosOdd;

    // Running state
    uint32_t prevBits;
    uint32_t numBitsSampled;
    uint32_t numBitsOfEntropy;
    double   currentProbability;
    uint64_t totalBits;
    bool     prevBit;
    uint32_t entropyLevel;
    uint32_t numSequentialZeros;
    uint32_t numSequentialOnes;
    uint32_t totalOnes;
    uint32_t totalZeros;
    uint32_t evenMisfires;
    uint32_t oddMisfires;
    bool     prevEven;
    bool     prevOdd;
};

struct infnoise_context {
    struct ftdi_context ftdic;
    uint32_t entropyThisTime;
    const char *message;

    // used in multiplier mode to keep track of bytes to be put out
    uint32_t keccakBytesGiven;

    // Keccak sponge state — previously global in libinfnoise.c
    uint8_t keccakState[INFNOISE_KECCAK_STATE_SIZE];

    // USB clock signal buffer — previously global in libinfnoise.c
    uint8_t outBuf[BUFLEN];

    // Health checker — previously static globals in healthcheck.c
    struct infnoise_health_state health;
};

typedef struct _infnoise_devlist_node_t infnoise_devlist_node_t;
struct _infnoise_devlist_node_t {
    char manufacturer[128];
    char description[128];
    char serial[128];
    infnoise_devlist_node_t *next;
};

/*
 * returns a struct of infnoise_devlist_node listing all connected FTDI FT240 devices by their USB descriptors
 *
 * parameters:
 *  - message: pointer for error message
 *
 *  returns: NULL when none found or infnoise_devlist_node
 */
infnoise_devlist_node_t* listUSBDevices(const char **message);

/*
 * initialize the Infinite Noise TRNG - must be called once before readData() works
 *
 * parameters:
 *  - context: pointer to infnoise_context struct
 *  - serial: optional serial number of the device (NULL)
 *  - keccak: initialize Keccak sponge (required to use readData with raw=false)
 *  - debug: debug flag
 * returns: boolean success indicator (0=success)
*/
bool initInfnoise(struct infnoise_context *context, char *serial, bool keccak, bool debug);


/*
 * deinitialize the Infinite Noise TRNG
 *
 * parameters:
 *  - context: pointer to infnoise_context struct
*/
void deinitInfnoise(struct infnoise_context *context);

/*
 * Reads some bytes from the TRNG and stores them in the "result" byte array.
 * The array has to be of sufficient size. Please refer to the example programs.
 * (64 byte for normal operation or 128byte for multiplier mode)
 *
 * Return value:
 *   > 0: number of bytes written to result
 *     0: transient condition (timing exceeded, entropy off-target) — retry
 *   < 0: fatal error (infnoise_error_t code) — check context->message
 *
 * context->message is set with a human-readable diagnostic on error.
 *
 * parameters:
 *  - context: infnoise_context struct with device pointer and state variables
 *  - result: pointer to byte array to store the result
 *  - raw: boolean flag for raw or whitened output
 *  - outputMultiplier: only used for whitened output
*/
int32_t readData(struct infnoise_context *context, uint8_t *result, bool raw, uint32_t outputMultiplier);

#ifdef __cplusplus
}
#endif

#endif
