#include <stdio.h>
#include <ctype.h>

int isHexDigit(int c) {
    return isdigit(c) || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');
}

int readChar(void) {
    int c = getchar();
    while(c != EOF && !isHexDigit(c)) {
        c = getchar();
    }
    return c;
}

int getDigit(char c) {
    c = tolower(c);
    if(c >= 'a' && c <= 'f') {
        return 10 + c - 'a';
    }
    return c - '0';
}

int main() {
    int upper = readChar();
    int lower = readChar();
    while(upper != EOF && lower != EOF) {
        int upperDigit = getDigit(upper);
        int lowerDigit = getDigit(lower);
        putchar((upperDigit << 4) | lowerDigit);
        upper = readChar();
        lower = readChar();
    }
    return 0;
}
