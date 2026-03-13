# Switching to the Infinite Noise TRNG Kernel Module

This guide assumes you are currently using the userspace `infnoise` binary.

---

## Step 1: Stop the userspace daemon

```bash
sudo systemctl stop infnoise 2>/dev/null || true
sudo systemctl disable infnoise 2>/dev/null || true
sudo pkill rngd 2>/dev/null || true
sudo pkill infnoise 2>/dev/null || true
```

---

## Step 2: Check device VID/PID (one-time setup)

```bash
lsusb | grep -E "0403:6015|1209:3701"
```

- If you see **`1209:3701`** → skip to Step 4.
- If you see **`0403:6015`** → continue with Step 3.

---

## Step 3: Reprogram device EEPROM (only if VID/PID is 0403:6015)

**This permanently modifies the device. Only needed once per device.**

```bash
# Install ftdi_eeprom
sudo apt install ftdi-eeprom          # Debian/Ubuntu
# sudo dnf install ftdi-eeprom        # Fedora

# Unbind the FTDI serial driver
sudo modprobe -r ftdi_sio

# Flash the Infinite Noise VID/PID (infnoise-eeprom.conf is in this repo)
sudo ftdi_eeprom --flash-eeprom infnoise-eeprom.conf
```

Unplug and replug the device, then verify:

```bash
lsusb | grep 1209:3701
```

---

## Step 4: Install build prerequisites

```bash
sudo apt install linux-headers-$(uname -r) build-essential  # Debian/Ubuntu
# sudo dnf install kernel-devel gcc make                     # Fedora/RHEL
# sudo pacman -S linux-headers base-devel                    # Arch
```

---

## Step 5: Build and test the module

```bash
cd /path/to/infnoise/software/kernel
make
sudo make load
```

Check that the device was detected:

```bash
dmesg | grep infnoise
```

Read some random data to confirm it works:

```bash
sudo dd if=/dev/hwrng bs=64 count=4 | xxd
```

---

## Step 6: Install for automatic loading at boot

```bash
sudo cp infnoise.ko /lib/modules/$(uname -r)/extra/
sudo depmod -a
echo "infnoise" | sudo tee /etc/modules-load.d/infnoise.conf
```

Reboot and verify the module loaded automatically:

```bash
lsmod | grep infnoise
cat /sys/class/misc/hw_random/rng_current
```

---

## Done

The kernel module is now active. It automatically feeds entropy to the kernel — no `rngd` or any other daemon is needed.

To verify entropy is being generated:

```bash
# Check hwrng thread is running
ps aux | grep hwrng

# Quick FIPS test (requires rng-tools)
sudo dd if=/dev/hwrng bs=2500 count=100 | rngtest -c 100
```
