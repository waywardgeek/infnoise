#include <stdbool.h>
#include <stdint.h>
#include <sys/types.h>
#include <ftdi.h>
#include <time.h>

// The FT240X has a 512 byte buffer.  Must be multiple of 64
// We also write this in one go to the Keccak sponge, which is at most 1600 bits
#define BUFLEN 512u


// struct for ftdi_device_descriptor
struct infnoise_device {
	uint8_t index;
	char *manufacturer;
	char *product;
	char *serial;
};


struct inm_devlist_node
{
    struct infnoise_device *device;
    struct inm_devlist_node *next;
};

struct inm_devlist
{
    struct inm_devlist_node *head;
};

// struct for list of devices
struct infnoise_device_list {
	struct infnoise_device device;
	struct infnoise_device_list * next;
};

bool listUSBDevices(struct ftdi_context *ftdic, struct inm_devlist *result, char **message);

bool initInfnoise(struct ftdi_context *ftdic, char *serial, char **message, bool debug);

bool initKeccak(struct ftdi_context *ftdic, char *serial);

uint64_t readRawData(struct ftdi_context *ftdic, uint8_t *result, char **message);

uint64_t readData(struct ftdi_context *ftdic, uint8_t *keccakState, uint8_t *result, char **message, uint32_t outputMultiplier);
