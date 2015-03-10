TARGET=$(shell awk 'NR==1{print $$1}' /etc/issue)

all: infnoise infnoise-v1

infnoise: infnoise.c infnoise.h healthcheck.c writeentropy.c Keccak/KeccakF-1600-reference.c Keccak/brg_endian.h
ifeq ($(TARGET),Fedora)
	gcc -Wall -std=c99 -O3 -I Keccak -DDIST=Fedora -o infnoise infnoise.c healthcheck.c writeentropy.c Keccak/KeccakF-1600-reference.c -lftdi1 -lm -lrt
else
	gcc -Wall -std=c99 -O3 -I Keccak -o infnoise infnoise.c healthcheck.c writeentropy.c Keccak/KeccakF-1600-reference.c -lftdi -lm -lrt
endif

infnoise-v1: infnoise.c infnoise.h healthcheck.c writeentropy.c Keccak/KeccakF-1600-reference.c Keccak/brg_endian.h
ifeq ($(TARGET),Fedora)
	gcc -Wall -std=c99 -O3 -DVERSION1 -I Keccak -DDIST=Fedora -o infnoise-v1 infnoise.c healthcheck.c writeentropy.c Keccak/KeccakF-1600-reference.c -lftdi1 -lm -lrt
else
	gcc -Wall -std=c99 -O3 -DVERSION1 -I Keccak -o infnoise-v1 infnoise.c healthcheck.c writeentropy.c Keccak/KeccakF-1600-reference.c -lftdi -lm -lrt
endif

clean:
	rm -f infnoise infnoise-v1
