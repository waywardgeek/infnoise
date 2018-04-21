GIT_VERSION := $(shell git --no-pager describe --tags --always)
GIT_COMMIT  := $(shell git rev-parse --verify HEAD)
GIT_DATE    := $(firstword $(shell git --no-pager show --date=iso-strict --format="%ad" --name-only))

PREFIX = $(DESTDIR)/usr/local

CFLAGS = -Wall -Wextra -Werror -std=c99 -O3 -fPIC -I Keccak -I /usr/include/libftdi1 \
 -DGIT_VERSION=\"$(GIT_VERSION)\"\
 -DGIT_COMMIT=\"$(GIT_COMMIT)\"\
 -DGIT_DATE=\"$(GIT_DATE)\"\

LDFLAGS= -shared

FTDI := $(shell libftdi-config --libs)

all: libinfnoise.a infnoise

infnoise: infnoise.c infnoise.h libinfnoise.a infnoise.o daemon.o
	$(CC) $(CFLAGS) -o infnoise infnoise.o daemon.o libinfnoise.a $(FTDI) -lm -lrt -L. -linfnoise

infnoise.o: infnoise.c infnoise.h
	$(CC) $(CFLAGS) -c infnoise.c

daemon.o: daemon.c
	$(CC) $(CFLAGS) -c daemon.c

writeentropy.o: writeentropy.c
	$(CC) $(CFLAGS) -c writeentropy.c

# static lib compiled into infnoise binary
libinfnoise.o: libinfnoise.c libinfnoise.h libinfnoise_private.h healthcheck.c healthcheck.h Keccak/brg_endian.h Keccak/KeccakF-1600-reference.c
	$(CC) $(CFLAGS) -c libinfnoise.c healthcheck.c

libinfnoise.a: libinfnoise.o healthcheck.o KeccakF-1600-reference.o writeentropy.o
	ar rcs libinfnoise.a libinfnoise.o healthcheck.o KeccakF-1600-reference.o writeentropy.o
	ranlib libinfnoise.a

# shared lib
libinfnoise.so: libinfnoise.o healthcheck.o KeccakF-1600-reference.o writeentropy.o
	$(CC) $(CFLAGS) -fvisibility=hidden -o libinfnoise.so libinfnoise.o healthcheck.o KeccakF-1600-reference.o writeentropy.o -Wl,--version-script=libinfnoise.version $(FTDI) -lm -shared 

KeccakF-1600-reference.o: Keccak/brg_endian.h Keccak/KeccakF-1600-reference.c
	$(CC) $(CFLAGS) -c Keccak/KeccakF-1600-reference.c -lm -lrt

libs: libinfnoise.a

clean:
	$(RM) infnoise *.o *.a *.gch *.so libinfnoise-example

install:
	install -d $(PREFIX)/sbin
	install -m 0755 infnoise $(PREFIX)/sbin/
	install -d $(PREFIX)/lib/udev/rules.d/
	install -m 0644 init_scripts/75-infnoise.rules $(PREFIX)/lib/udev/rules.d/
	install -d $(PREFIX)/lib/systemd/system
	install -m 0644 init_scripts/infnoise.service $(PREFIX)/lib/systemd/system

postinstall:
	systemctl restart systemd-udevd
	systemctl enable infnoise
