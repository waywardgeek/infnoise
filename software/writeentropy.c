// This writes entropy to the Linux /dev/random pool using ioctl, so that entropy increases.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/poll.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <linux/random.h>
#include "infnoise.h"

#define SIZE_PROC_FILENAME "/proc/sys/kernel/random/poolsize"
#define FILL_PROC_FILENAME "/proc/sys/kernel/random/write_wakeup_threshold"

static uint32_t inmBufLen;
static bool inmDebug;
static int inmDevRandomFD;
static uint32_t inmFillWatermark;
static struct rand_pool_info *inmPoolInfo;

// Find the entropy pool size.
static uint32_t readNumberFromFile(char *fileName) {
    FILE *file = fopen(fileName, "r");
    if(file == NULL) {
        fprintf(stderr, "Unable to open %s\n", fileName);
        exit(1);
    }
    char buf[42];
    uint32_t i = 0;
    int c = getc(file);
    while(c != EOF) {
        buf[i++] = c;
        c = getc(file);
    }
    buf[i] = '\0';
    uint32_t value = atoi(buf);
    fclose(file);
    return value;
}

// Open /dev/random
void inmWriteEntropyStart(uint32_t bufLen, bool debug) {
    inmBufLen = bufLen;
    inmDebug = debug;
    //inmDevRandomFD = open("/dev/random", O_WRONLY);
    inmDevRandomFD = open("/dev/random", O_RDWR);
    if(inmDevRandomFD < 0) {
        fprintf(stderr, "Unable to open /dev/random\n");
        exit(1);
    }
    inmPoolInfo = calloc(1, sizeof(struct rand_pool_info) + bufLen);
    if(inmPoolInfo == NULL) {
        fprintf(stderr, "Unable to allocate memory\n");
        exit(1);
    }
    inmFillWatermark = readNumberFromFile(FILL_PROC_FILENAME);
    if(inmDebug) {
        printf("Entropy pool size:%u\n", readNumberFromFile(SIZE_PROC_FILENAME));
    }
}

// Block until either the entropy pool has room, or 1 second has passed.
void inmWaitForPoolToHaveRoom(void) {
    printf("starting select\n");
    int ent_count;
    struct pollfd pfd = {
        fd:   inmDevRandomFD,
        events: POLLOUT,
    };
    int64_t timeout_usec;
    if (ioctl(inmDevRandomFD, RNDGETENTCNT, &ent_count) == 0 && ent_count < inmFillWatermark) {
        printf("Not full\n");
        return;
    }
    timeout_usec = 1000; // One second
    poll(&pfd, 1, timeout_usec);
    printf("Finished select\n");
}

// Add the bytes to the entropy pool.  This can be unwhitenened, but the estimated bits of
// entropy needs to be accurate or pessimistic.  Return false if the Linux entropy pool is
// full after writing.
void inmWriteEntropyToPool(uint8_t *bytes, uint32_t length, uint32_t entropy) {
    inmPoolInfo->entropy_count = entropy;
    inmPoolInfo->buf_size = length;
    memcpy(inmPoolInfo->buf, bytes, length);
    //printf("Writing %u bytes with %u bits of entropy to /dev/random\n", length, entropy);
    ioctl(inmDevRandomFD, RNDADDENTROPY, inmPoolInfo);
}
