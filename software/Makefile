all: infnoise healthcheck findlongest

infnoise: infnoise.c healthcheck.c Keccak/KeccakF-1600-reference.c Keccak/brg_endian.h
	gcc -Wall -std=c11 -O3 -m64 -march=native -I Keccak -o infnoise infnoise.c healthcheck.c Keccak/KeccakF-1600-reference.c -lftdi -lm

healthcheck: healthcheck.c
	gcc -Wall -std=c11 -O3 -m64 -march=native -D TEST_HEALTHCHECK -o healthcheck healthcheck.c -lm

findlongest: findlongest.c
	gcc -Wall -std=c11 -O3 -m64 -march=native -o findlongest findlongest.c

clean:
	rm -f healthcheck infnoise findlongest
