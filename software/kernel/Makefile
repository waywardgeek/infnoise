# SPDX-License-Identifier: GPL-2.0-or-later
#
# Makefile for Infinite Noise TRNG kernel module (out-of-tree build)
#

# Module name
MODULE_NAME := infnoise

# Kernel source directory (override with KDIR=...)
KDIR ?= /lib/modules/$(shell uname -r)/build

# Current directory
PWD := $(shell pwd)

# Module object files
obj-m := $(MODULE_NAME).o
$(MODULE_NAME)-y := infnoise_main.o infnoise_health.o infnoise_keccak.o

# Extra compiler flags
ccflags-y := -DDEBUG

# Default target: build module
all: modules

# Build kernel module
modules:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

# Clean build artifacts
clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean
	rm -f Module.symvers modules.order test_infnoise

# Install module
install: modules
	$(MAKE) -C $(KDIR) M=$(PWD) modules_install
	depmod -a

# Uninstall module
uninstall:
	rm -f /lib/modules/$(shell uname -r)/extra/$(MODULE_NAME).ko
	depmod -a

# Load module
load: modules
	@if lsmod | grep -q "^$(MODULE_NAME)"; then \
		echo "Module already loaded, unloading first..."; \
		sudo rmmod $(MODULE_NAME); \
	fi
	sudo insmod $(MODULE_NAME).ko

# Load with debug
load-debug: modules
	@if lsmod | grep -q "^$(MODULE_NAME)"; then \
		echo "Module already loaded, unloading first..."; \
		sudo rmmod $(MODULE_NAME); \
	fi
	sudo insmod $(MODULE_NAME).ko debug=1

# Unload module
unload:
	@if lsmod | grep -q "^$(MODULE_NAME)"; then \
		sudo rmmod $(MODULE_NAME); \
	else \
		echo "Module not loaded"; \
	fi

# Reload module
reload: unload load

# Show dmesg output from module
dmesg:
	dmesg | grep -i infnoise | tail -50

# Check module info
info: modules
	modinfo $(MODULE_NAME).ko

# Test the hwrng
test-hwrng:
	@echo "Testing hwrng interface..."
	@if [ -r /dev/hwrng ]; then \
		dd if=/dev/hwrng bs=64 count=10 | xxd | head -20; \
	else \
		echo "Error: /dev/hwrng not readable (need root?)"; \
	fi

# Test the character device
test-char:
	@echo "Testing character device..."
	@DEVNUM=$$(ls /dev/infnoise* 2>/dev/null | head -1); \
	if [ -n "$$DEVNUM" ]; then \
		dd if=$$DEVNUM bs=64 count=10 | xxd | head -20; \
	else \
		echo "Error: No /dev/infnoise* device found"; \
	fi

# Build userspace test harness
test_infnoise: test_infnoise.c
	$(CC) -Wall -Wextra -o $@ $<

# Run all tests: kmod + entropy quality (~45s total)
test: test_infnoise
	@if ! lsmod | grep -q "^$(MODULE_NAME)"; then \
		echo "Error: $(MODULE_NAME) module not loaded. Try: make load"; \
		exit 1; \
	fi
	sudo ./test_infnoise
	@echo ""
	@echo "--- FIPS 140-2 (rngtest) ---"
	@if command -v rngtest >/dev/null 2>&1 && [ -r /dev/hwrng ]; then \
		dd if=/dev/hwrng bs=2500 count=100 2>/dev/null | rngtest; \
	else \
		echo "  SKIP (rngtest not installed or /dev/hwrng not readable)"; \
	fi
	@echo ""
	@echo "--- Shannon entropy (ent) ---"
	@if command -v ent >/dev/null 2>&1 && [ -r /dev/hwrng ]; then \
		dd if=/dev/hwrng bs=2500 count=100 of=/tmp/infnoise-ent.bin 2>/dev/null && \
		ent /tmp/infnoise-ent.bin && rm -f /tmp/infnoise-ent.bin; \
	else \
		echo "  SKIP (ent not installed or /dev/hwrng not readable)"; \
	fi

# Show help
help:
	@echo "Infinite Noise TRNG Kernel Module Build System"
	@echo ""
	@echo "Targets:"
	@echo "  all          - Build the module (default)"
	@echo "  clean        - Remove build artifacts"
	@echo "  install      - Install module to system"
	@echo "  uninstall    - Remove module from system"
	@echo "  load         - Load module with insmod"
	@echo "  load-debug   - Load module with debug enabled"
	@echo "  unload       - Unload module with rmmod"
	@echo "  reload       - Unload and reload module"
	@echo "  dmesg        - Show recent kernel messages from module"
	@echo "  info         - Show module information"
	@echo "  test         - Run all tests: kmod + rngtest + ent (~45s)"
	@echo "  test-hwrng   - Dump hex from /dev/hwrng (visual check)"
	@echo "  test-char    - Dump hex from /dev/infnoise0 (visual check)"
	@echo ""
	@echo "Variables:"
	@echo "  KDIR         - Kernel source directory (default: /lib/modules/\$$(uname -r)/build)"
	@echo ""
	@echo "Example:"
	@echo "  make KDIR=/usr/src/linux-headers-6.1.0"

.PHONY: all modules clean install uninstall load load-debug unload reload dmesg info test-hwrng test-char test help
