CFLAGS=-Wall -Wextra -Werror -std=c99 -O3

all: passgen healthcheck findlongest entcheck hex2bin bin2hex flipbits dice

passgen: passgen.c
	$(CC) $(CFLAGS) -o passgen passgen.c -lm

healthcheck: ../healthcheck.c
	$(CC) $(CFLAGS) -D TEST_HEALTHCHECK -o healthcheck ../healthcheck.c -lm -lrt

entcheck: entcheck.c
	$(CC) $(CFLAGS) -o entcheck entcheck.c -lm -lrt

findlongest: findlongest.c
	$(CC) $(CFLAGS) -o findlongest findlongest.c

hex2bin: hex2bin.c
	$(CC) $(CFLAGS) -o hex2bin hex2bin.c

bin2hex: bin2hex.c
	$(CC) $(CFLAGS) -o bin2hex bin2hex.c

flipbits: flipbits.c
	$(CC) $(CFLAGS) -o flipbits flipbits.c

dice: dice.c
	$(CC) $(CFLAGS) -o dice dice.c

clean:
	$(RM) passgen healthcheck findlongest entcheck hex2bin bin2hex flipbits dice
