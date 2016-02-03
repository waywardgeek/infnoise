CFLAGS = -Wall -std=c99 -O3 -I Keccak -I /usr/include/libftdi1

@_=	$(shell ld -lftdi 2> /dev/null)
ifeq ($$?, 0)
	FTDI=	-lftdi
else
	FTDI=	-lftdi1
endif

all: infnoise

infnoise: infnoise.c infnoise.h healthcheck.c writeentropy.c daemon.c Keccak/KeccakF-1600-reference.c Keccak/brg_endian.h
	$(CC) $(CFLAGS) -o infnoise infnoise.c healthcheck.c writeentropy.c daemon.c Keccak/KeccakF-1600-reference.c $(FTDI) -lm -lrt

clean:
	$(RM) infnoise
