#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv) {
    int value = getchar();
    while(value != EOF) {
        int i, revVal = 0;
        for(i = 0; i < 8; i++) {
            revVal <<= 1;
            revVal |= value & 1;
            value >>= 1;
        }
        putchar(revVal);
        value = getchar();
    }
    return 0;
}
