all: infnoise infnoise-v1 healthcheck findlongest entcheck hex2bin

infnoise: infnoise.c infnoise.h healthcheck.c writeentropy.c Keccak/KeccakF-1600-reference.c Keccak/brg_endian.h
	gcc -Wall -std=c99 -O3 -I Keccak -o infnoise infnoise.c healthcheck.c writeentropy.c Keccak/KeccakF-1600-reference.c -lftdi -lm -lrt

infnoise-v1: infnoise.c infnoise.h healthcheck.c writeentropy.c Keccak/KeccakF-1600-reference.c Keccak/brg_endian.h
	gcc -Wall -std=c99 -O3 -DVERSION1 -I Keccak -o infnoise-v1 infnoise.c healthcheck.c writeentropy.c Keccak/KeccakF-1600-reference.c -lftdi -lm -lrt

healthcheck: healthcheck.c
	gcc -Wall -std=c99 -O3 -D TEST_HEALTHCHECK -o healthcheck healthcheck.c -lm -lrt

entcheck: entcheck.c
	gcc -Wall -std=c99 -O3 -o entcheck entcheck.c -lm -lrt

findlongest: findlongest.c
	gcc -Wall -std=c99 -O3 -o findlongest findlongest.c

hex2bin: hex2bin.c
	gcc -Wall -std=c99 -O3 -o hex2bin hex2bin.c

clean:
	rm -f healthcheck infnoise infnoise-v1 findlongest hex2bin
