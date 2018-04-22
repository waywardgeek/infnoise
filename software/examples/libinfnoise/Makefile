GIT_VERSION := $(shell git --no-pager describe --tags --always)
GIT_COMMIT  := $(shell git rev-parse --verify HEAD)
GIT_DATE    := $(firstword $(shell git --no-pager show --date=iso-strict --format="%ad" --name-only))

PREFIX = $(DESTDIR)/usr/local

CFLAGS = -Wall -Wextra -Werror -std=c99 -O3 -fPIC -I Keccak -I /usr/include/libftdi1 \
 -DGIT_VERSION=\"$(GIT_VERSION)\"\
 -DGIT_COMMIT=\"$(GIT_COMMIT)\"\
 -DGIT_DATE=\"$(GIT_DATE)\"\

LDFLAGS= -shared
ODIR=obj

FTDI := $(shell libftdi-config --libs)

all: libinfnoise-example libinfnoise-example-raw

libinfnoise-example: example.c
	$(CC) $(CFLAGS) -o libinfnoise-example example.c $(FTDI) -lm -lrt -L.. -linfnoise

libinfnoise-example-raw: example.c
	$(CC) $(CFLAGS) -o libinfnoise-example-raw example-raw.c $(FTDI) -lm -lrt -L.. -linfnoise

clean:
	$(RM) libinfnoise-example* *.o

