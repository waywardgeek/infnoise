all: infnoise infnoise-v1 passgen

infnoise: infnoise.c infnoise.h healthcheck.c writeentropy.c Keccak/KeccakF-1600-reference.c Keccak/brg_endian.h
	gcc -Wall -std=c99 -O3 -I Keccak -o infnoise infnoise.c healthcheck.c writeentropy.c Keccak/KeccakF-1600-reference.c -lftdi -lm -lrt

infnoise-v1: infnoise.c infnoise.h healthcheck.c writeentropy.c Keccak/KeccakF-1600-reference.c Keccak/brg_endian.h
	gcc -Wall -std=c99 -O3 -DVERSION1 -I Keccak -o infnoise-v1 infnoise.c healthcheck.c writeentropy.c Keccak/KeccakF-1600-reference.c -lftdi -lm -lrt

passgen: passgen.c
	gcc -Wall -std=c99 -O3 -o passgen passgen.c -lm

clean:
	rm -f infnoise infnoise-v1
