CFLAGS = -Wall -Wextra -Werror -std=c99 -O3 -I Keccak -I /usr/include/libftdi1

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
	install -m 0755 infnoise $(DESTDIR)/sbin/
	install -d $(DESTDIR)/lib/udev/rules.d/
	install -m 0644 75-infnoise.rules $(DESTDIR)/lib/udev/rules.d/
	install -d $(DESTDIR)/lib/systemd/system
	install -m 0644 infnoise.service $(DESTDIR)/lib/systemd/system

postinstall:
	systemctl restart systemd-udevd
	systemctl enable infnoise
