GIT_VERSION := $(shell git --no-pager describe --tags --always)
GIT_COMMIT  := $(shell git rev-parse --verify HEAD)
GIT_DATE    := $(firstword $(shell git --no-pager show --date=iso-strict --format="%ad" --name-only))

PREFIX = $(DESTDIR)/usr/local

CFLAGS = -Wall -Wextra -Werror -std=c99 -O3 -I Keccak -I /usr/include/libftdi1 \
 -DGIT_VERSION=\"$(GIT_VERSION)\"\
 -DGIT_COMMIT=\"$(GIT_COMMIT)\"\
 -DGIT_DATE=\"$(GIT_DATE)\"\

FOUND = $(shell ldconfig -p | grep --silent libftdi.so && echo found)
ifeq ($(FOUND), found)
	FTDI=	-lftdi
else
	FTDI=	-lftdi1
endif

all: libinfnoise.a infnoise

infnoise: infnoise.c infnoise.h libinfnoise.a infnoise.o daemon.o
	$(CC) $(CFLAGS) -o infnoise infnoise.o daemon.o libinfnoise.a $(FTDI) -lm -lrt -L. -linfnoise

infnoise.o: infnoise.c infnoise.h
	$(CC) $(CFLAGS) -c infnoise.c

daemon.o: daemon.c
	$(CC) $(CFLAGS) -c daemon.c

writeentropy.o: writeentropy.c
	$(CC) $(CFLAGS) -c writeentropy.c

libinfnoise.o: libinfnoise.c libinfnoise.h libinfnoise_private.h healthcheck.c healthcheck.h Keccak/brg_endian.h Keccak/KeccakF-1600-reference.c
	$(CC) $(CFLAGS) -c libinfnoise.c healthcheck.c Keccak/KeccakF-1600-reference.c

libinfnoise.a: libinfnoise.o healthcheck.o KeccakF-1600-reference.o writeentropy.o
	ar rcs libinfnoise.a libinfnoise.o healthcheck.o KeccakF-1600-reference.o writeentropy.o
	ranlib libinfnoise.a

KeccakF-1600-reference.o: Keccak/brg_endian.h Keccak/KeccakF-1600-reference.c
	$(CC) $(CFLAGS) -c Keccak/KeccakF-1600-reference.c -lm -lrt
 
libs: libinfnoise.a

libinfnoise-example: libinfnoise.c libinfnoise.h
	$(CC) $(CFLAGS) -D LIB_EXAMPLE_PROGRAM -o libinfnoise-example libinfnoise.c libinfnoise.h healthcheck.c writeentropy.c daemon.c Keccak/KeccakF-1600-reference.c $(FTDI) -lm -lrt

clean:
	$(RM) infnoise *.o *.a *.gch libinfnoise-example

install:
	install -d $(PREFIX)/sbin
	install -m 0755 infnoise $(PREFIX)/sbin/
	install -d $(PREFIX)/lib/udev/rules.d/
	install -m 0644 75-infnoise.rules $(PREFIX)/lib/udev/rules.d/
	install -d $(PREFIX)/lib/systemd/system
	install -m 0644 infnoise.service $(PREFIX)/lib/systemd/system

postinstall:
	systemctl restart systemd-udevd
	systemctl enable infnoise
