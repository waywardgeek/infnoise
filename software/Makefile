all: infnoise hello-ftdi

infnoise: infnoise.c
	gcc -Wall -std=c11 -O3 -m64 -march=native -o infnoise infnoise.c -lm

hello-ftdi: hello-ftdi.c
	gcc -Wall -std=c11 -O3 -m64 -march=native -o hello-ftdi hello-ftdi.c -lftdi

clean:
	rm -f infnoise
