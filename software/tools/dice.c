// This is a simple program to read from a binary file of random bits and generate
// dice rolls.

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

// Role a single die.
static void rollDie(uint32_t sides, FILE *file) {
  uint32_t randVal;
  do {
    uint32_t numBytes = 0;
    randVal = 0;
    while (1 << (8*numBytes) <= sides) {
      numBytes++;
      int c = getc(file);
      if (c == EOF) {
        printf("Ran out of random data\n");
        exit(1);
      }
      randVal = (randVal << 8) | getc(file);
    }
  } while(randVal >= sides);
  printf(" %u", randVal + 1);
}

int main(int argc, char **argv) {
  if (argc != 4) {
    printf("Usage: dice randFile numDice numSides\n");
    return 1;
  }
  FILE *file = fopen(argv[1], "r");
  uint32_t dice = atoi(argv[2]);
  uint32_t sides = atoi(argv[3]);
  uint32_t i;
  printf("Rolling %u %u-sided dice:", dice, sides);
  for (i = 0; i < dice; i++) {
    rollDie(sides, file);
  }
  printf("\n");
  fclose(file);
  return 0;
}
