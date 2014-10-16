all: infnoise hello-ftdi healthcheck

infnoise: infnoise.c
	gcc -Wall -std=c11 -O3 -m64 -march=native -o infnoise infnoise.c -lm

hello-ftdi: hello-ftdi.c
	gcc -Wall -std=c11 -O3 -m64 -march=native -o hello-ftdi hello-ftdi.c -lftdi

healthcheck: healthcheck.c
	gcc -Wall -std=c11 -g -m64 -march=native -o healthcheck healthcheck.c -lm

clean:
	rm -f infnoise
