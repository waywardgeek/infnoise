// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Infinite Noise TRNG kernel driver
 *
 * Copyright (C) 2024 Manuel Domke
 *
 * Based on the userspace driver by Bill Cox
 * Hardware design by Bill Cox
 *
 * This driver interfaces with the Infinite Noise TRNG USB device,
 * implements health monitoring, Keccak-1600 whitening, and registers
 * as an hwrng device to feed entropy to the kernel's random subsystem.
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/usb.h>
#include <linux/slab.h>
#include <linux/mutex.h>
#include <linux/hw_random.h>
#include <linux/workqueue.h>
#include <linux/uaccess.h>
#include <linux/delay.h>
#include "infnoise.h"

#define DRIVER_NAME	"infnoise"
#define DRIVER_VERSION	"1.0"
#define DRIVER_DESC	"Infinite Noise TRNG driver"

/* Module parameters */
bool infnoise_debug;
module_param_named(debug, infnoise_debug, bool, 0644);
MODULE_PARM_DESC(debug, "Enable debug output");

bool infnoise_raw_mode;
module_param_named(raw_mode, infnoise_raw_mode, bool, 0644);
MODULE_PARM_DESC(raw_mode, "Output raw (unwhitened) data");

unsigned int infnoise_multiplier = 1;
module_param_named(multiplier, infnoise_multiplier, uint, 0644);
MODULE_PARM_DESC(multiplier, "Output multiplier (0=entropy only, 1-10=multiply output)");

/* USB device table - only matches reprogrammed devices */
static const struct usb_device_id infnoise_table[] = {
	{ USB_DEVICE(INFNOISE_VENDOR_ID, INFNOISE_PRODUCT_ID) },
	{ }
};
MODULE_DEVICE_TABLE(usb, infnoise_table);

/* Forward declarations */
static struct usb_driver infnoise_driver;
static void infnoise_warmup_work(struct work_struct *work);
static int infnoise_read_data(struct infnoise_device *dev, u8 *result,
			      size_t max_len, bool raw);
static int infnoise_configure_ftdi(struct infnoise_device *dev);

/*
 * FTDI USB control transfer helper
 */
static int ftdi_control(struct infnoise_device *dev, u8 request,
			u16 value, u16 index)
{
	return usb_control_msg(dev->udev,
			       usb_sndctrlpipe(dev->udev, 0),
			       request,
			       USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_DIR_OUT,
			       value, index, NULL, 0,
			       INFNOISE_USB_TIMEOUT);
}

/*
 * Configure FTDI device for synchronous bit-bang mode
 */
static int infnoise_configure_ftdi(struct infnoise_device *dev)
{
	int ret;

	/* Reset device */
	ret = ftdi_control(dev, FTDI_SIO_RESET, FTDI_SIO_RESET_SIO,
			   FTDI_INDEX_INTERFACE_A);
	if (ret < 0) {
		dev_err(&dev->intf->dev, "Failed to reset device: %d\n", ret);
		return ret;
	}

	/* Set latency timer to 1ms for faster response */
	ret = ftdi_control(dev, FTDI_SIO_SET_LATENCY_TIMER, 1,
			   FTDI_INDEX_INTERFACE_A);
	if (ret < 0) {
		dev_err(&dev->intf->dev, "Failed to set latency timer: %d\n", ret);
		return ret;
	}

	/*
	 * Set baud rate to 30000
	 * FTDI baud rate encoding: value = 3000000 / baud
	 * For 30000 baud: value = 100 = 0x0064
	 */
	ret = ftdi_control(dev, FTDI_SIO_SET_BAUDRATE, 0x0064,
			   FTDI_INDEX_INTERFACE_A);
	if (ret < 0) {
		dev_err(&dev->intf->dev, "Failed to set baud rate: %d\n", ret);
		return ret;
	}

	/*
	 * Enable synchronous bit-bang mode
	 * Mode byte in high byte of value, mask in low byte
	 * Mode 0x04 = SYNCBB, Mask 0xEB = all outputs except COMP1/COMP2
	 */
	ret = ftdi_control(dev, FTDI_SIO_SET_BITMODE,
			   (FTDI_BITMODE_SYNCBB << 8) | INFNOISE_MASK,
			   FTDI_INDEX_INTERFACE_A);
	if (ret < 0) {
		dev_err(&dev->intf->dev, "Failed to set bit-bang mode: %d\n", ret);
		return ret;
	}

	/* Purge RX buffer */
	ret = ftdi_control(dev, FTDI_SIO_RESET, FTDI_SIO_RESET_PURGE_RX,
			   FTDI_INDEX_INTERFACE_A);
	if (ret < 0) {
		dev_err(&dev->intf->dev, "Failed to purge RX: %d\n", ret);
		return ret;
	}

	/* Purge TX buffer */
	ret = ftdi_control(dev, FTDI_SIO_RESET, FTDI_SIO_RESET_PURGE_TX,
			   FTDI_INDEX_INTERFACE_A);
	if (ret < 0) {
		dev_err(&dev->intf->dev, "Failed to purge TX: %d\n", ret);
		return ret;
	}

	/* Give the device time to settle after configuration */
	msleep(50);

	return 0;
}

/*
 * Test USB transfer to verify device is working
 * Similar to what libftdi does during initialization
 */
static int infnoise_test_transfer(struct infnoise_device *dev)
{
	u8 *test_buf;
	int ret, actual;

	/* Allocate DMA-capable buffer */
	test_buf = kmalloc(64, GFP_KERNEL);
	if (!test_buf)
		return -ENOMEM;

	memset(test_buf, 0, 64);

	/* Clear any halt condition on endpoints */
	usb_clear_halt(dev->udev, usb_sndbulkpipe(dev->udev, dev->bulk_out_ep));
	usb_clear_halt(dev->udev, usb_rcvbulkpipe(dev->udev, dev->bulk_in_ep));

	/* Test write */
	ret = usb_bulk_msg(dev->udev,
			   usb_sndbulkpipe(dev->udev, dev->bulk_out_ep),
			   test_buf, 64,
			   &actual, INFNOISE_USB_TIMEOUT);
	if (ret < 0) {
		dev_err(&dev->intf->dev, "Test write failed: %d\n", ret);
		kfree(test_buf);
		return ret;
	}

	/* Test read */
	ret = usb_bulk_msg(dev->udev,
			   usb_rcvbulkpipe(dev->udev, dev->bulk_in_ep),
			   test_buf, 64,
			   &actual, INFNOISE_USB_TIMEOUT);
	if (ret < 0) {
		dev_err(&dev->intf->dev, "Test read failed: %d\n", ret);
		kfree(test_buf);
		return ret;
	}

	dev_info(&dev->intf->dev, "Test transfer OK (%d bytes)\n", actual);
	kfree(test_buf);
	return 0;
}

/*
 * Attempt to recover from USB errors
 *
 * This function tries to restore communication with the device after
 * consecutive USB errors. It clears endpoint halts and reconfigures
 * the FTDI chip. If that fails, it attempts a full USB device reset.
 *
 * Returns 0 on successful recovery, negative error code on failure.
 */
static int infnoise_try_recovery(struct infnoise_device *dev)
{
	int ret;

	if (!test_bit(INFNOISE_PRESENT, &dev->flags))
		return -ENODEV;

	dev_warn(&dev->intf->dev, "Attempting error recovery...\n");

	/* Clear any halt conditions on endpoints */
	usb_clear_halt(dev->udev, usb_sndbulkpipe(dev->udev, dev->bulk_out_ep));
	usb_clear_halt(dev->udev, usb_rcvbulkpipe(dev->udev, dev->bulk_in_ep));

	/* Small delay to let things settle */
	msleep(INFNOISE_RETRY_DELAY_MS);

	/* Try to reconfigure the FTDI device */
	ret = infnoise_configure_ftdi(dev);
	if (ret < 0) {
		dev_warn(&dev->intf->dev,
			 "FTDI reconfiguration failed (%d), trying USB reset\n", ret);

		/* Last resort: reset the USB device */
		ret = usb_reset_device(dev->udev);
		if (ret < 0) {
			dev_err(&dev->intf->dev,
				"USB reset failed: %d, device may be unusable\n", ret);
			return ret;
		}

		/* Reconfigure after reset */
		msleep(50);
		ret = infnoise_configure_ftdi(dev);
		if (ret < 0) {
			dev_err(&dev->intf->dev,
				"FTDI configuration after reset failed: %d\n", ret);
			return ret;
		}
	}

	dev->recoveries++;
	dev->consecutive_errors = 0;
	dev_info(&dev->intf->dev, "Recovery successful (total recoveries: %u)\n",
		 dev->recoveries);

	return 0;
}

/*
 * Prepare the clock buffer with alternating SWEN1/SWEN2 patterns
 */
static void infnoise_prepare_clock_buffer(struct infnoise_device *dev)
{
	unsigned int i;

	for (i = 0; i < INFNOISE_BUFLEN; i += 2) {
		dev->clock_buf[i] = (1 << INFNOISE_SWEN1);
		dev->clock_buf[i + 1] = (1 << INFNOISE_SWEN2);
	}
}

/*
 * Extract entropy bits from raw USB data
 *
 * In synchronous bit-bang mode, each byte read back contains the state
 * of all pins. We extract one bit per byte from COMP1/COMP2 alternating.
 */
static int infnoise_extract_bytes(struct infnoise_device *dev, u8 *bytes,
				  size_t length, const u8 *in_buf)
{
	unsigned int i, j;

	infnoise_health_clear_entropy(&dev->health);

	for (i = 0; i < length; i++) {
		u8 byte = 0;

		for (j = 0; j < 8; j++) {
			u8 val = in_buf[i * 8 + j];
			bool even_bit = (val >> INFNOISE_COMP2) & 1;
			bool odd_bit = (val >> INFNOISE_COMP1) & 1;
			bool even = j & 1; /* Use even bit if j is odd */
			u8 bit = even ? even_bit : odd_bit;

			byte = (byte << 1) | bit;

			/* Feed bit to health checker */
			if (!infnoise_health_add_bit(&dev->health, even_bit,
						     odd_bit, even)) {
				dev_err(&dev->intf->dev,
					"Health check failed!\n");
				set_bit(INFNOISE_HEALTH_FAIL, &dev->flags);
				return -EIO;
			}
		}
		bytes[i] = byte;
	}

	return infnoise_health_get_entropy(&dev->health);
}

/*
 * Strip FTDI modem status bytes from USB data
 *
 * FTDI sends data in 64-byte packets: 2 status bytes + 62 data bytes.
 * This function extracts just the data bytes.
 */
static int infnoise_strip_ftdi_status(struct infnoise_device *dev,
				      int usb_bytes)
{
	int packets = usb_bytes / FTDI_PACKET_SIZE;
	int leftover = usb_bytes % FTDI_PACKET_SIZE;
	int data_bytes = 0;
	int i;

	for (i = 0; i < packets; i++) {
		u8 *src = dev->usb_buf + i * FTDI_PACKET_SIZE + FTDI_STATUS_SIZE;
		int copy_len = min(FTDI_DATA_PER_PACKET,
				   INFNOISE_BUFLEN - data_bytes);

		if (copy_len > 0) {
			memcpy(dev->read_buf + data_bytes, src, copy_len);
			data_bytes += copy_len;
		}
	}

	/* Handle partial packet at end if any */
	if (leftover > FTDI_STATUS_SIZE && data_bytes < INFNOISE_BUFLEN) {
		int copy_len = min(leftover - FTDI_STATUS_SIZE,
				   INFNOISE_BUFLEN - data_bytes);
		u8 *src = dev->usb_buf + packets * FTDI_PACKET_SIZE + FTDI_STATUS_SIZE;

		memcpy(dev->read_buf + data_bytes, src, copy_len);
		data_bytes += copy_len;
	}

	return data_bytes;
}

/*
 * Perform a single USB transfer cycle (internal, no retry)
 *
 * Writes clock pattern, reads back samples, extracts bits.
 * Returns number of entropy bits, or negative error.
 */
static int infnoise_usb_transfer_once(struct infnoise_device *dev)
{
	int ret;
	int actual;
	int total_read = 0;
	int data_bytes;

	/* Write clock pattern */
	ret = usb_bulk_msg(dev->udev,
			   usb_sndbulkpipe(dev->udev, dev->bulk_out_ep),
			   dev->clock_buf, INFNOISE_BUFLEN,
			   &actual, INFNOISE_USB_TIMEOUT);
	if (ret < 0) {
		if (infnoise_debug)
			dev_dbg(&dev->intf->dev, "USB write failed: %d\n", ret);
		return ret;
	}

	if (actual != INFNOISE_BUFLEN) {
		dev_err(&dev->intf->dev, "Short write: %d/%d\n",
			actual, INFNOISE_BUFLEN);
		return -EIO;
	}

	/*
	 * Read samples - FTDI returns data in 64-byte packets with 2 status
	 * bytes per packet. We need to read until we have enough data bytes.
	 */
	while (total_read < INFNOISE_USB_READ_SIZE) {
		int remaining = INFNOISE_USB_READ_SIZE - total_read;

		ret = usb_bulk_msg(dev->udev,
				   usb_rcvbulkpipe(dev->udev, dev->bulk_in_ep),
				   dev->usb_buf + total_read, remaining,
				   &actual, INFNOISE_USB_TIMEOUT);
		if (ret < 0) {
			if (infnoise_debug)
				dev_dbg(&dev->intf->dev, "USB read failed: %d\n", ret);
			return ret;
		}

		if (actual == 0) {
			if (infnoise_debug)
				dev_dbg(&dev->intf->dev, "USB read timeout, got %d/%d\n",
					total_read, INFNOISE_USB_READ_SIZE);
			return -ETIMEDOUT;
		}

		total_read += actual;

		if (infnoise_debug && total_read < INFNOISE_USB_READ_SIZE)
			dev_dbg(&dev->intf->dev, "Partial read: %d/%d, continuing\n",
				total_read, INFNOISE_USB_READ_SIZE);
	}

	/* Strip FTDI status bytes to get actual data */
	data_bytes = infnoise_strip_ftdi_status(dev, total_read);

	if (data_bytes < INFNOISE_BUFLEN) {
		dev_err(&dev->intf->dev, "Short data: %d/%d bytes\n",
			data_bytes, INFNOISE_BUFLEN);
		return -EIO;
	}

	/* Extract entropy bits */
	return infnoise_extract_bytes(dev, dev->out_buf, INFNOISE_BYTES_OUT,
				      dev->read_buf);
}

/*
 * Check if an error is potentially recoverable
 */
static bool infnoise_is_recoverable_error(int err)
{
	switch (err) {
	case -EPIPE:		/* Endpoint stalled */
	case -ETIMEDOUT:	/* Timeout */
	case -EPROTO:		/* Protocol error */
	case -EILSEQ:		/* CRC/bit stuff error */
	case -EOVERFLOW:	/* Babble/overflow */
	case -EIO:		/* I/O error */
		return true;
	case -ENODEV:		/* Device disconnected */
	case -ESHUTDOWN:	/* Device shutting down */
	case -ENOENT:		/* URB cancelled */
		return false;
	default:
		return false;
	}
}

/*
 * Perform USB transfer with retry and recovery
 *
 * Attempts the transfer up to INFNOISE_MAX_RETRIES times. If consecutive
 * errors exceed INFNOISE_RECOVERY_THRESHOLD, attempts device recovery.
 * Returns number of entropy bits, or negative error.
 */
static int infnoise_usb_transfer(struct infnoise_device *dev)
{
	int ret;
	int retry;

	for (retry = 0; retry < INFNOISE_MAX_RETRIES; retry++) {
		ret = infnoise_usb_transfer_once(dev);

		if (ret >= 0) {
			/* Success - reset consecutive error counter */
			dev->consecutive_errors = 0;
			return ret;
		}

		/* Track errors */
		dev->usb_errors++;
		dev->consecutive_errors++;

		/* Check if error is recoverable */
		if (!infnoise_is_recoverable_error(ret)) {
			if (infnoise_debug)
				dev_dbg(&dev->intf->dev,
					"Non-recoverable error: %d\n", ret);
			return ret;
		}

		/* Attempt recovery if we've hit the threshold */
		if (dev->consecutive_errors >= INFNOISE_RECOVERY_THRESHOLD) {
			int recovery_ret = infnoise_try_recovery(dev);

			if (recovery_ret < 0) {
				dev_err(&dev->intf->dev,
					"Recovery failed: %d\n", recovery_ret);
				return recovery_ret;
			}
			/* Recovery succeeded, retry counter continues */
		}

		/* Brief delay before retry */
		if (retry < INFNOISE_MAX_RETRIES - 1) {
			if (infnoise_debug)
				dev_dbg(&dev->intf->dev,
					"Retry %d/%d after error %d\n",
					retry + 1, INFNOISE_MAX_RETRIES, ret);
			msleep(INFNOISE_RETRY_DELAY_MS);
		}
	}

	dev_err(&dev->intf->dev, "Transfer failed after %d retries, last error: %d\n",
		INFNOISE_MAX_RETRIES, ret);
	return ret;
}

/*
 * Squeeze whitened data from Keccak sponge
 *
 * Extracts up to 128 bytes per call. If more data is needed,
 * performs a permutation and can be called again.
 * Returns number of bytes extracted.
 */
static int infnoise_keccak_squeeze(struct infnoise_device *dev, u8 *result,
				   size_t max_len)
{
	size_t out_bytes;

	/* Extract at most 128 bytes (16 lanes) for security margin */
	out_bytes = min_t(size_t, max_len, 128);
	infnoise_keccak_extract(&dev->keccak, result, (out_bytes + 7) / 8);

	/* Permute state for next extraction */
	infnoise_keccak_permutation(&dev->keccak);

	return out_bytes;
}

/*
 * Read random data from the device
 *
 * This is the main data path used by both hwrng and character device.
 * Supports multiplier for increased output rate from Keccak sponge.
 */
static int infnoise_read_data(struct infnoise_device *dev, u8 *result,
			      size_t max_len, bool raw)
{
	int entropy;
	size_t out_bytes;
	unsigned int mult;

	/* Check device is still present */
	if (!test_bit(INFNOISE_PRESENT, &dev->flags))
		return -ENODEV;

	/*
	 * If we have bytes left from a previous absorption (multiplier > 1),
	 * squeeze more data without doing a new USB transfer.
	 */
	if (dev->keccak_bytes_left > 0 && !raw) {
		out_bytes = min_t(size_t, max_len, dev->keccak_bytes_left);
		out_bytes = infnoise_keccak_squeeze(dev, result, out_bytes);
		dev->keccak_bytes_left -= out_bytes;
		dev->bytes_generated += out_bytes;
		return out_bytes;
	}

	/* Perform USB transfer to get new entropy */
	entropy = infnoise_usb_transfer(dev);
	if (entropy < 0)
		return entropy;

	/* Check health status */
	if (!infnoise_health_ok(&dev->health)) {
		if (infnoise_debug)
			dev_dbg(&dev->intf->dev, "Health check not OK yet\n");
		return 0; /* Discard data but don't error */
	}

	/* Check entropy meets target */
	if (!infnoise_health_entropy_on_target(&dev->health, entropy,
					       INFNOISE_BUFLEN)) {
		if (infnoise_debug)
			dev_dbg(&dev->intf->dev, "Entropy below target\n");
		return 0; /* Discard data */
	}

	/* Return raw data if requested */
	if (raw) {
		out_bytes = min_t(size_t, max_len, INFNOISE_BYTES_OUT);
		memcpy(result, dev->out_buf, out_bytes);
		dev->bytes_generated += out_bytes;
		return out_bytes;
	}

	/* Absorb raw data into Keccak sponge (64 bytes = 8 lanes) */
	infnoise_keccak_absorb(&dev->keccak, dev->out_buf, INFNOISE_BYTES_OUT / 8);

	/* Calculate total bytes to output based on multiplier */
	mult = infnoise_multiplier;
	if (mult > 10)
		mult = 10;  /* Cap at 10x for sanity */

	if (mult == 0) {
		/* Output only entropy bits worth of data */
		out_bytes = entropy / 8;
	} else {
		/* Output multiplier * 32 bytes (256 bits per multiplier unit) */
		out_bytes = mult * 32;
	}

	/* Set up remaining bytes for subsequent reads */
	dev->keccak_bytes_left = out_bytes;

	/* Squeeze first chunk */
	out_bytes = min_t(size_t, max_len, dev->keccak_bytes_left);
	out_bytes = infnoise_keccak_squeeze(dev, result, out_bytes);
	dev->keccak_bytes_left -= out_bytes;
	dev->bytes_generated += out_bytes;

	return out_bytes;
}

/*
 * Warmup work function - runs in workqueue context
 */
static void infnoise_warmup_work(struct work_struct *work)
{
	struct infnoise_device *dev = container_of(work, struct infnoise_device,
						   warmup_work);
	unsigned int rounds = 0;
	u8 discard[64];

	dev_info(&dev->intf->dev, "Starting warmup...\n");

	mutex_lock(&dev->lock);

	while (!infnoise_health_ok(&dev->health) &&
	       rounds < INFNOISE_WARMUP_ROUNDS &&
	       test_bit(INFNOISE_PRESENT, &dev->flags)) {
		infnoise_read_data(dev, discard, sizeof(discard), true);
		rounds++;
	}

	mutex_unlock(&dev->lock);

	if (!test_bit(INFNOISE_PRESENT, &dev->flags)) {
		dev_info(&dev->intf->dev, "Device disconnected during warmup\n");
		return;
	}

	if (rounds >= INFNOISE_WARMUP_ROUNDS) {
		dev_err(&dev->intf->dev,
			"Warmup failed after %u rounds\n", rounds);
		set_bit(INFNOISE_HEALTH_FAIL, &dev->flags);
	} else {
		dev_info(&dev->intf->dev,
			 "Warmup complete after %u rounds\n", rounds);
		set_bit(INFNOISE_WARMUP_DONE, &dev->flags);
	}

	complete(&dev->warmup_done);
}

/* ============== hwrng interface ============== */

static int infnoise_hwrng_read(struct hwrng *rng, void *data, size_t max,
			       bool wait)
{
	struct infnoise_device *dev = (struct infnoise_device *)rng->priv;
	bool raw = infnoise_raw_mode || dev->raw_mode;
	int ret;

	if (!test_bit(INFNOISE_PRESENT, &dev->flags))
		return -ENODEV;

	/*
	 * Update hwrng quality based on current raw mode setting.
	 * Raw output has ~0.88 bits entropy per bit.
	 * Whitened (Keccak) output is cryptographically uniform (~1.0 bits/bit).
	 */
	dev->hwrng.quality = raw ? INFNOISE_QUALITY_RAW : INFNOISE_QUALITY_WHITENED;

	/* Wait for warmup if requested */
	if (wait && !test_bit(INFNOISE_WARMUP_DONE, &dev->flags)) {
		ret = wait_for_completion_interruptible(&dev->warmup_done);
		if (ret)
			return ret;
	}

	if (!test_bit(INFNOISE_WARMUP_DONE, &dev->flags))
		return 0;

	if (test_bit(INFNOISE_HEALTH_FAIL, &dev->flags))
		return -EIO;

	mutex_lock(&dev->lock);
	ret = infnoise_read_data(dev, data, max, raw);
	mutex_unlock(&dev->lock);

	return ret;
}

static int infnoise_hwrng_register(struct infnoise_device *dev)
{
	int ret;
	bool raw = infnoise_raw_mode || dev->raw_mode;

	snprintf(dev->hwrng_name, sizeof(dev->hwrng_name),
		 "infnoise-%s", dev_name(&dev->intf->dev));

	dev->hwrng.name = dev->hwrng_name;
	dev->hwrng.read = infnoise_hwrng_read;
	dev->hwrng.priv = (unsigned long)dev;
	dev->hwrng.quality = raw ? INFNOISE_QUALITY_RAW : INFNOISE_QUALITY_WHITENED;

	ret = hwrng_register(&dev->hwrng);
	if (ret) {
		dev_err(&dev->intf->dev, "Failed to register hwrng: %d\n", ret);
		return ret;
	}

	set_bit(INFNOISE_HWRNG_REG, &dev->flags);
	dev_info(&dev->intf->dev, "Registered hwrng: %s (quality=%u)\n",
		 dev->hwrng_name, dev->hwrng.quality);

	return 0;
}

static void infnoise_hwrng_unregister(struct infnoise_device *dev)
{
	if (test_and_clear_bit(INFNOISE_HWRNG_REG, &dev->flags)) {
		hwrng_unregister(&dev->hwrng);
		dev_info(&dev->intf->dev, "Unregistered hwrng\n");
	}
}

/* ============== Character device interface ============== */

static int infnoise_open(struct inode *inode, struct file *file)
{
	struct infnoise_device *dev;
	struct usb_interface *intf;

	intf = usb_find_interface(&infnoise_driver, iminor(inode));
	if (!intf)
		return -ENODEV;

	dev = usb_get_intfdata(intf);
	if (!dev)
		return -ENODEV;

	if (!test_bit(INFNOISE_PRESENT, &dev->flags))
		return -ENODEV;

	file->private_data = dev;

	return 0;
}

static int infnoise_release(struct inode *inode, struct file *file)
{
	return 0;
}

static ssize_t infnoise_char_read(struct file *file, char __user *buf,
				  size_t count, loff_t *ppos)
{
	struct infnoise_device *dev = file->private_data;
	u8 data[128];
	size_t chunk;
	ssize_t total = 0;
	int ret;

	if (!dev)
		return -ENODEV;

	/* Wait for warmup */
	if (!test_bit(INFNOISE_WARMUP_DONE, &dev->flags)) {
		if (file->f_flags & O_NONBLOCK)
			return -EAGAIN;

		ret = wait_for_completion_interruptible(&dev->warmup_done);
		if (ret)
			return ret;
	}

	if (test_bit(INFNOISE_HEALTH_FAIL, &dev->flags))
		return -EIO;

	while (total < count) {
		chunk = min(count - total, sizeof(data));

		mutex_lock(&dev->lock);
		ret = infnoise_read_data(dev, data, chunk,
					 infnoise_raw_mode || dev->raw_mode);
		mutex_unlock(&dev->lock);

		if (ret < 0)
			return total ? total : ret;

		if (ret == 0) {
			/* No data available, try again */
			if (total > 0)
				break;
			if (file->f_flags & O_NONBLOCK)
				return -EAGAIN;
			continue;
		}

		if (copy_to_user(buf + total, data, ret))
			return -EFAULT;

		total += ret;
	}

	return total;
}

static long infnoise_ioctl(struct file *file, unsigned int cmd,
			   unsigned long arg)
{
	struct infnoise_device *dev = file->private_data;
	struct infnoise_stats stats;
	int raw;
	u32 entropy;

	if (!dev)
		return -ENODEV;

	switch (cmd) {
	case INFNOISE_GET_STATS:
		mutex_lock(&dev->lock);
		infnoise_health_get_stats(&dev->health, &stats);
		mutex_unlock(&dev->lock);
		if (copy_to_user((void __user *)arg, &stats, sizeof(stats)))
			return -EFAULT;
		return 0;

	case INFNOISE_SET_RAW:
		if (copy_from_user(&raw, (void __user *)arg, sizeof(raw)))
			return -EFAULT;
		dev->raw_mode = !!raw;
		return 0;

	case INFNOISE_GET_ENTROPY:
		mutex_lock(&dev->lock);
		entropy = infnoise_health_get_entropy(&dev->health);
		mutex_unlock(&dev->lock);
		if (copy_to_user((void __user *)arg, &entropy, sizeof(entropy)))
			return -EFAULT;
		return 0;

	default:
		return -ENOTTY;
	}
}

static const struct file_operations infnoise_fops = {
	.owner		= THIS_MODULE,
	.read		= infnoise_char_read,
	.open		= infnoise_open,
	.release	= infnoise_release,
	.llseek		= noop_llseek,
	.unlocked_ioctl	= infnoise_ioctl,
};

static struct usb_class_driver infnoise_class = {
	.name		= "infnoise%d",
	.fops		= &infnoise_fops,
	.minor_base	= 192,
};

/* ============== USB probe/disconnect ============== */

static int infnoise_probe(struct usb_interface *intf,
			  const struct usb_device_id *id)
{
	struct usb_device *udev = interface_to_usbdev(intf);
	struct usb_endpoint_descriptor *bulk_in, *bulk_out;
	struct infnoise_device *dev;
	int ret;

	dev_info(&intf->dev, "Infinite Noise TRNG detected\n");

	/* Reset USB device to clear any stale state */
	ret = usb_reset_device(udev);
	if (ret) {
		dev_warn(&intf->dev, "USB reset failed: %d (continuing)\n", ret);
		/* Non-fatal, continue anyway */
	}

	/* Find bulk endpoints */
	ret = usb_find_common_endpoints(intf->cur_altsetting,
					&bulk_in, &bulk_out, NULL, NULL);
	if (ret) {
		dev_err(&intf->dev, "Could not find bulk endpoints\n");
		return ret;
	}

	/* Allocate device structure */
	dev = kzalloc(sizeof(*dev), GFP_KERNEL);
	if (!dev)
		return -ENOMEM;

	dev->udev = usb_get_dev(udev);
	dev->intf = intf;
	dev->bulk_in_ep = bulk_in->bEndpointAddress;
	dev->bulk_out_ep = bulk_out->bEndpointAddress;

	mutex_init(&dev->lock);
	init_completion(&dev->warmup_done);
	INIT_WORK(&dev->warmup_work, infnoise_warmup_work);

	/* Allocate buffers */
	dev->clock_buf = kmalloc(INFNOISE_BUFLEN, GFP_KERNEL);
	dev->usb_buf = kmalloc(INFNOISE_USB_READ_SIZE, GFP_KERNEL);
	dev->read_buf = kmalloc(INFNOISE_BUFLEN, GFP_KERNEL);
	dev->out_buf = kmalloc(INFNOISE_BYTES_OUT, GFP_KERNEL);

	if (!dev->clock_buf || !dev->usb_buf || !dev->read_buf || !dev->out_buf) {
		ret = -ENOMEM;
		goto err_buffers;
	}

	/* Initialize health check */
	ret = infnoise_health_init(&dev->health);
	if (ret)
		goto err_buffers;

	/* Initialize Keccak */
	infnoise_keccak_init(&dev->keccak);

	/* Prepare clock buffer */
	infnoise_prepare_clock_buffer(dev);

	/* Configure FTDI */
	ret = infnoise_configure_ftdi(dev);
	if (ret)
		goto err_health;

	/* Test transfer to prime the device */
	ret = infnoise_test_transfer(dev);
	if (ret)
		goto err_health;

	/* Mark device as present */
	set_bit(INFNOISE_PRESENT, &dev->flags);

	usb_set_intfdata(intf, dev);

	/* Register character device */
	ret = usb_register_dev(intf, &infnoise_class);
	if (ret) {
		dev_err(&intf->dev, "Could not register char device: %d\n", ret);
		goto err_health;
	}
	dev->minor = intf->minor;
	dev_info(&intf->dev, "Registered /dev/infnoise%d\n",
		 intf->minor - infnoise_class.minor_base);

	/* Register hwrng */
	ret = infnoise_hwrng_register(dev);
	if (ret)
		dev_warn(&intf->dev, "hwrng registration failed, continuing\n");

	/* Start warmup in workqueue */
	schedule_work(&dev->warmup_work);

	dev_info(&intf->dev, "Infinite Noise TRNG initialized\n");

	return 0;

err_health:
	infnoise_health_free(&dev->health);
err_buffers:
	kfree(dev->clock_buf);
	kfree(dev->usb_buf);
	kfree(dev->read_buf);
	kfree(dev->out_buf);
	usb_put_dev(dev->udev);
	kfree(dev);
	return ret;
}

static void infnoise_disconnect(struct usb_interface *intf)
{
	struct infnoise_device *dev = usb_get_intfdata(intf);

	if (!dev)
		return;

	dev_info(&intf->dev, "Infinite Noise TRNG disconnected\n");

	/* Mark device as absent */
	clear_bit(INFNOISE_PRESENT, &dev->flags);

	/* Complete any waiting warmup */
	complete_all(&dev->warmup_done);

	/* Cancel warmup work */
	cancel_work_sync(&dev->warmup_work);

	/* Unregister hwrng */
	infnoise_hwrng_unregister(dev);

	/* Unregister character device */
	usb_deregister_dev(intf, &infnoise_class);

	usb_set_intfdata(intf, NULL);

	/* Free resources */
	infnoise_health_free(&dev->health);
	kfree(dev->clock_buf);
	kfree(dev->usb_buf);
	kfree(dev->read_buf);
	kfree(dev->out_buf);
	usb_put_dev(dev->udev);
	kfree(dev);
}

static struct usb_driver infnoise_driver = {
	.name		= DRIVER_NAME,
	.probe		= infnoise_probe,
	.disconnect	= infnoise_disconnect,
	.id_table	= infnoise_table,
	.supports_autosuspend = 0,  /* No suspend - device feeds entropy continuously */
};

module_usb_driver(infnoise_driver);

MODULE_AUTHOR("Manuel Domke");
MODULE_DESCRIPTION(DRIVER_DESC);
MODULE_VERSION(DRIVER_VERSION);
MODULE_LICENSE("GPL");
