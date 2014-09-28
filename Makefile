all: infnoise

infnoise: infnoise.c
	gcc -Wall -std=c99 -O3 -m64 -march=native -o infnoise infnoise.c -lm
