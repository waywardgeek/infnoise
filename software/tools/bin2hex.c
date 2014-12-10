#include <stdio.h>

int main() {
    int value = getchar();
    int count = 0;
    while(value != EOF) {
        printf("%02x", value);
        value = getchar();
        if(++count == 64) {
            putchar('\n');
        }
    }
    putchar('\n');
    return 0;
}
