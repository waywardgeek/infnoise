#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

static uint8_t availableInBits = 0;
static uint8_t availableOutBits = 0;
static uint8_t currentInByte = 0;
static uint8_t currentOutByte = 0;

// Read a width-bits value from the input.
static uint64_t readVal(uint8_t width, bool *endOfInput) {
    *endOfInput = false;
    uint64_t value = 0;
    uint8_t mask = 1 << (availableInBits-1);
    while(width--) {
        value <<= 1;
        if(availableInBits == 0) {
            int c = getchar();
            if(c == EOF) {
                *endOfInput = true;
                return value;
            }
            currentInByte = c;
            availableInBits = 8;
            mask = 1 << 7;
        }
        if(currentInByte & mask) {
            value |= 1;
        }
        mask >>= 1;
        availableInBits--;
    }
    return value;
}

// Write out the value in reverse order.
static void writeValReverse(uint64_t value, uint8_t width) {
    while(width--) {
        currentOutByte <<= 1;
        currentOutByte |= value & 1;
        value >>= 1;
        availableOutBits++;
        if(availableOutBits == 8) {
            putchar(currentOutByte);
            availableOutBits = 0;
        }
    }
}

int main(int argc, char **argv) {
    uint8_t width = 8;
    if(argc == 2) {
        width = atoi(argv[1]);
    }
    if(width == 0 || width > 64 || argc > 2) {
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
