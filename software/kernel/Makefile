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
	rm -f Module.symvers modules.order

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

# Run rngtest on hwrng output
rngtest:
	@echo "Running rngtest (1000 blocks)..."
	@if [ -r /dev/hwrng ]; then \
		cat /dev/hwrng | rngtest -c 1000 || true; \
	else \
		echo "Error: /dev/hwrng not readable"; \
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
	@echo "  test-hwrng   - Test hwrng interface"
	@echo "  test-char    - Test character device"
	@echo "  rngtest      - Run rngtest on hwrng output"
	@echo ""
	@echo "Variables:"
	@echo "  KDIR         - Kernel source directory (default: /lib/modules/\$$(uname -r)/build)"
	@echo ""
	@echo "Example:"
	@echo "  make KDIR=/usr/src/linux-headers-6.1.0"

.PHONY: all modules clean install uninstall load load-debug unload reload dmesg info test-hwrng test-char rngtest help
