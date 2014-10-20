// Find the longest repeated binary sequence in a file.
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_STRING_SIZE 35

static bool *readDataFromFile(char *fileName, uint64_t *length) {
    printf("Reading data\n");
    FILE *file = fopen(fileName, "r");
    if(file == NULL) {
        fprintf(stderr, "Unable to open file %s\n", fileName);
        exit(1);
    }
    uint64_t pos = 0;
    uint64_t size = 16384;
    bool *data = calloc(size, sizeof(bool));
    int c = getc(file);
    while(c != EOF) {
        if(pos*8 + 8 >= size) {
            size <<= 1;
            data = realloc(data, size);
        }
        uint8_t v = c;
        uint32_t i;
        for(i = 0; i < 8; i++) {
            if(v & 1){
                data[8*pos + 7 - i] = true;
            }
            v >>= 1;
        }
        pos++;
        c = getc(file);
    }
    fclose(file);
    *length = pos*8;
    printf("Read %lu bits\n", *length);
    return data;
}

// Find out if the string has a repeated sub-string of the given length.
static bool hasSubstringOfLength(uint32_t substringLen, bool *data, uint32_t dataLen, uint8_t *stringsSeen) {
    memset(stringsSeen, 0, 1u << (substringLen-3));
    uint64_t value = 0;
    uint32_t i;
    for(i = 0; i < substringLen; i++) {
        value = (value << 1) | data[i];
    }
    for(i = substringLen; i < dataLen; i++) {
        if(stringsSeen[value >> 3] & (1 << (value & 0x7))) {
            printf("Found duplicate substring of length %u at %u(%x.%u): %lx\n", substringLen,
                i, i >> 3, i & 0x7, value);
            return true;
        }
        stringsSeen[value >> 3] |= 1 << (value & 0x7);
        value = ((value << 1) | data[i]) & (((uint64_t)1 << substringLen)-1);
    }
    return false;
}

int main(int argc, char **argv) {
    if(argc != 2) {
        fprintf(stderr, "Usage: findlongest file\n");
        return 1;
    }
    uint64_t dataLen;
    bool *data = readDataFromFile(argv[1], &dataLen);
    uint8_t *stringsSeen = calloc((uint64_t)1 << (MAX_STRING_SIZE-3), sizeof(uint8_t));
    uint32_t len;
    for(len = 4; len <= MAX_STRING_SIZE; len++) {
        if(!hasSubstringOfLength(len, data, dataLen, stringsSeen)) {
            printf("Maximum substring is %u long\n", len-1);
            return 0;
        }
    }
    printf("Maximum substring is at least %u long\n", MAX_STRING_SIZE);
    return 0;
}
