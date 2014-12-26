all: healthcheck findlongest entcheck hex2bin bin2hex flipbits genbits

healthcheck: ../healthcheck.c
	gcc -Wall -std=c99 -O3 -D TEST_HEALTHCHECK -o healthcheck ../healthcheck.c -lm -lrt

entcheck: entcheck.c
	gcc -Wall -std=c99 -O3 -o entcheck entcheck.c -lm -lrt

findlongest: findlongest.c
	gcc -Wall -std=c99 -O3 -o findlongest findlongest.c

hex2bin: hex2bin.c
	gcc -Wall -std=c99 -O3 -o hex2bin hex2bin.c

bin2hex: bin2hex.c
	gcc -Wall -std=c99 -O3 -o bin2hex bin2hex.c

flipbits: flipbits.c
	gcc -Wall -std=c99 -O3 -o flipbits flipbits.c

genbits: genbits.c
	gcc -Wall -std=c99 -O3 -o genbits genbits.c

clean:
	rm -f healthcheck findlongest entcheck hex2bin bin2hex flipbits
