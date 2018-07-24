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

struct infnoise_context {
        struct ftdi_context ftdic;
        uint32_t entropyThisTime;
        char *message;
        bool errorFlag;
        //uint8_t keccakState[KeccakPermutationSizeInBytes];
};

struct infnoise_devlist_node {
	uint8_t id;
	char *manufacturer;
	char *description;
	char *serial;
	struct infnoise_devlist_node *next;
};

typedef struct infnoise_devlist_node* devlist_node;

devlist_node* listUSBDevices(char** message);

bool initInfnoise(struct infnoise_context *context, char *serial, bool keccak, bool debug);

uint32_t readRawData(struct infnoise_context *context, uint8_t *result);

uint32_t readData(struct infnoise_context *context, uint8_t *result, uint32_t outputMultiplier);
