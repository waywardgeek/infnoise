#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

static uint8_t availableInBits = 0u;
static uint8_t availableOutBits = 0u;
static uint8_t currentInByte = 0u;
static uint8_t currentOutByte = 0u;

// Read a width-bits value from the input.
static uint64_t readVal(uint8_t width, bool *endOfInput) {
    *endOfInput = false;
    uint64_t value = 0u;
    uint8_t mask = 1u << (availableInBits-1u);
    while(width--) {
        value <<= 1u;
        if(availableInBits == 0u) {
            int c = getchar();
            if(c == EOF) {
                *endOfInput = true;
                return value;
            }
            currentInByte = c;
            availableInBits = 8u;
            mask = 1u << 7u;
        }
        if(currentInByte & mask) {
            value |= 1u;
        }
        mask >>= 1u;
        availableInBits--;
    }
    return value;
}

// Write out the value in reverse order.
static void writeValReverse(uint64_t value, uint8_t width) {
    while(width--) {
        currentOutByte <<= 1u;
        currentOutByte |= value & 1u;
        value >>= 1u;
        availableOutBits++;
        if(availableOutBits == 8u) {
            putchar(currentOutByte);
            availableOutBits = 0u;
        }
    }
}

int main(int argc, char **argv) {
    uint8_t width = 8u;
    if(argc == 2) {
        width = atoi(argv[1]);
    }
    if(width == 0u || width > 64u || argc > 2) {
        fprintf(stderr, "Usage: flipbits [width]\n");
        return 1;
    }
    bool endOfInput;
    uint64_t value = readVal(width, &endOfInput);
    while(!endOfInput) {
        writeValReverse(value, width);
        value = readVal(width, &endOfInput);
    }
    return 0;
}
