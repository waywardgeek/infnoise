// Find the longest repeated binary sequence in a file.
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_STRING_SIZE 34u

// Generate random data for use in testing.
static bool *generateRandomData(uint32_t size) {
    bool *data = calloc(size, sizeof(bool));
    srand(time(NULL));
    uint32_t i;
    for(i = 0u; i < size; ) {
        uint32_t randVal = rand();
        uint32_t j;
        for(j = 0u; j < 16u; j++) {
            if(randVal & 1u) {
                data[i] = true;
            }
            i++;
            randVal >>= 1u;
        }
    }
    return data;
}

static bool *readDataFromFile(char *fileName, uint64_t *length) {
    printf("Reading data\n");
    FILE *file = fopen(fileName, "r");
    if(file == NULL) {
        fprintf(stderr, "Unable to open file %s\n", fileName);
        exit(1);
    }
    uint64_t pos = 0u;
    uint64_t size = 16384u;
    bool *data = calloc(size, sizeof(bool));
    int c = getc(file);
    while(c != EOF) {
        if(pos*8u + 8u >= size) {
            size <<= 1u;
            data = realloc(data, size);
        }
        uint8_t v = c;
        uint32_t i;
        for(i = 0u; i < 8u; i++) {
            if(v & 1u){
                data[8u*pos + 7u - i] = true;
            }
            v >>= 1u;
        }
        pos++;
        c = getc(file);
    }
    fclose(file);
    *length = pos*8u;
    printf("Read %llu bits\n", (long long)(*length));
    return data;
}

// Find out if the string has a repeated sub-string of the given length.
static bool hasSubstringOfLength(uint32_t substringLen, bool *data, uint32_t dataLen, uint8_t *stringsSeen) {
    memset(stringsSeen, 0, 1u << (substringLen-3));
    uint64_t value = 0u;
    uint32_t i;
    for(i = 0u; i < substringLen; i++) {
        value = (value << 1u) | data[i];
    }
    for(i = substringLen; i < dataLen; i++) {
        if(stringsSeen[value >> 3u] & (1u << (value & 0x7u))) {
            printf("Found duplicate substring of length %u at %u(%x.%u): %llx\n", substringLen,
                i, i >> 3u, i & 0x7u, (long long)value);
            return true;
        }
        stringsSeen[value >> 3u] |= 1u << (value & 0x7u);
        value = ((value << 1u) | data[i]) & (((uint64_t)1u << substringLen)-1u);
    }
    return false;
}

int main(int argc, char **argv) {
    bool test = false;
    if(argc != 2) {
        fprintf(stderr, "Usage: findlongest file\n"
                        "       findlongest --test\n");
        return 1;
    }
    if(!strcmp(argv[1], "--test")) {
        test = true;
    }
    uint64_t dataLen;
    bool *data;
    if(test) {
        dataLen = 1u << 24u;
        data = generateRandomData(dataLen);
    } else {
        data = readDataFromFile(argv[1], &dataLen);
    }
    uint8_t *stringsSeen = calloc((uint64_t)1 << (MAX_STRING_SIZE-3u), sizeof(uint8_t));
    uint32_t len;
    for(len = 4u; len <= MAX_STRING_SIZE; len++) {
        if(!hasSubstringOfLength(len, data, dataLen, stringsSeen)) {
            printf("Maximum substring is %u long\n", len-1u);
            return 0;
        }
    }
    printf("Maximum substring is at least %u long\n", MAX_STRING_SIZE);
    return 0;
}
