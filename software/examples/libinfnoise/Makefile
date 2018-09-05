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

all: libinfnoise-example-simple libinfnoise-example-complex

libinfnoise-example-simple: example-simple.c
	$(CC) $(CFLAGS) -o libinfnoise-example-simple example-simple.c $(FTDI) -lm -lrt -linfnoise

libinfnoise-example-complex: example-complex.c
	$(CC) $(CFLAGS) -o libinfnoise-example-complex example-complex.c $(FTDI) -lm -lrt -linfnoise

clean:
	$(RM) libinfnoise-example* *.o

