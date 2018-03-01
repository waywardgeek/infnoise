GIT_VERSION := $(shell git --no-pager describe --tags --always)
GIT_COMMIT  := $(shell git rev-parse --verify HEAD)
GIT_DATE    := $(firstword $(shell git --no-pager show --date=iso-strict --format="%ad" --name-only))
BUILD_DATE  := $(shell date --iso=seconds)

PREFIX = $(DESTDIR)/usr/local

CFLAGS = -Wall -Wextra -Werror -std=c99 -O3 -I Keccak -I /usr/include/libftdi1 \
 -DGIT_VERSION=\"$(GIT_VERSION)\"\
 -DGIT_COMMIT=\"$(GIT_COMMIT)\"\
 -DGIT_DATE=\"$(GIT_DATE)\"\
 -DBUILD_DATE=\"$(BUILD_DATE)\"

FOUND = $(shell ldconfig -p | grep --silent libftdi.so && echo found)
ifeq ($(FOUND), found)
	FTDI=	-lftdi
else
	FTDI=	-lftdi1
endif

all: infnoise

infnoise: infnoise.c infnoise.h healthcheck.c writeentropy.c daemon.c Keccak/KeccakF-1600-reference.c Keccak/brg_endian.h
	$(CC) $(CFLAGS) -o infnoise infnoise.c healthcheck.c writeentropy.c daemon.c Keccak/KeccakF-1600-reference.c $(FTDI) -lm -lrt

clean:
	$(RM) infnoise

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
