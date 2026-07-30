/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Infinite Noise TRNG kernel driver
 *
 * Copyright (C) 2024 Manuel Domke
 *
 * Based on the userspace driver by Bill Cox
 * Hardware design by Bill Cox
 */

#ifndef INFNOISE_H
#define INFNOISE_H

#include <linux/types.h>
#include <linux/usb.h>
#include <linux/hw_random.h>
#include <linux/mutex.h>
#include <linux/completion.h>
#include <linux/workqueue.h>

/* USB device IDs - Infinite Noise TRNG (pid.codes registered) */
#define INFNOISE_VENDOR_ID	0x1209	/* pid.codes VID */
#define INFNOISE_PRODUCT_ID	0x3701	/* Infinite Noise PID */

/* FTDI commands (SIO requests) */
#define FTDI_SIO_RESET			0x00
#define FTDI_SIO_SET_MODEM_CTRL		0x01
#define FTDI_SIO_SET_FLOW_CTRL		0x02
#define FTDI_SIO_SET_BAUDRATE		0x03
#define FTDI_SIO_SET_DATA		0x04
#define FTDI_SIO_GET_MODEM_STATUS	0x05
#define FTDI_SIO_SET_EVENT_CHAR		0x06
#define FTDI_SIO_SET_ERROR_CHAR		0x07
#define FTDI_SIO_SET_LATENCY_TIMER	0x09
#define FTDI_SIO_GET_LATENCY_TIMER	0x0A
#define FTDI_SIO_SET_BITMODE		0x0B
#define FTDI_SIO_READ_PINS		0x0C
#define FTDI_SIO_READ_EEPROM		0x90
#define FTDI_SIO_WRITE_EEPROM		0x91

/* FTDI reset commands */
#define FTDI_SIO_RESET_SIO		0x00
#define FTDI_SIO_RESET_PURGE_RX		0x01
#define FTDI_SIO_RESET_PURGE_TX		0x02

/* FTDI bit-bang modes */
#define FTDI_BITMODE_RESET		0x00
#define FTDI_BITMODE_BITBANG		0x01
#define FTDI_BITMODE_MPSSE		0x02
#define FTDI_BITMODE_SYNCBB		0x04	/* Synchronous bit-bang */
#define FTDI_BITMODE_MCU		0x08
#define FTDI_BITMODE_OPTO		0x10
#define FTDI_BITMODE_CBUS		0x20
#define FTDI_BITMODE_SYNCFF		0x40

/* Pin definitions for FT240X */
#define INFNOISE_COMP1		1	/* Comparator 1 input */
#define INFNOISE_COMP2		4	/* Comparator 2 input */
#define INFNOISE_SWEN1		2	/* Switch enable 1 output */
#define INFNOISE_SWEN2		0	/* Switch enable 2 output */

/* Address/debug pins */
#define INFNOISE_ADDR0		3
#define INFNOISE_ADDR1		5
#define INFNOISE_ADDR2		6
#define INFNOISE_ADDR3		7

/* Output mask: all pins except COMP1 and COMP2 are outputs */
#define INFNOISE_MASK		(0xFF & ~(1 << INFNOISE_COMP1) & ~(1 << INFNOISE_COMP2))

/* FTDI interface index (1-based: interface 0 uses index 1) */
#define FTDI_INDEX_INTERFACE_A	1

/* Buffer sizes */
#define INFNOISE_BUFLEN		512	/* FT240X buffer size, must be multiple of 64 */
#define INFNOISE_BYTES_OUT	(INFNOISE_BUFLEN / 8)	/* 64 bytes extracted */

/* FTDI packet format: each 64-byte USB packet has 2 status bytes + 62 data bytes */
#define FTDI_PACKET_SIZE	64
#define FTDI_STATUS_SIZE	2
#define FTDI_DATA_PER_PACKET	(FTDI_PACKET_SIZE - FTDI_STATUS_SIZE)  /* 62 */
/* For 512 bytes of data, we need ceil(512/62) = 9 packets = 576 bytes */
#define INFNOISE_USB_READ_SIZE	(((INFNOISE_BUFLEN + FTDI_DATA_PER_PACKET - 1) / FTDI_DATA_PER_PACKET) * FTDI_PACKET_SIZE)

/* Health check constants */
#define INM_PREDICTION_BITS	14	/* Bits used for prediction */
#define INM_TABLE_SIZE		(1 << INM_PREDICTION_BITS)	/* 16384 entries */
#define INM_MIN_DATA		80000	/* Minimum bits before data is valid */
#define INM_MIN_SAMPLE_SIZE	100	/* Minimum samples */
#define INM_MAX_SEQUENCE	20	/* Max consecutive same bits */
#define INM_MAX_COUNT		(1 << 14)	/* Max counter value before scaling */

/*
 * Fixed-point arithmetic for health check (avoid floating point in kernel)
 * Using 16.16 fixed point format
 */
#define FP_SHIFT		16
#define FP_ONE			(1 << FP_SHIFT)	/* 1.0 in fixed point */
#define FP_HALF			(FP_ONE >> 1)	/* 0.5 in fixed point */

/* Design constants in fixed-point (K = 1.84) */
#define INM_DESIGN_K_FP		120586	/* 1.84 * 65536 */
/* log2(1.84) ≈ 0.88 in fixed-point */
#define INM_EXPECTED_ENTROPY_FP	57671	/* 0.88 * 65536 */
/* INM_ACCURACY = 1.03 in fixed-point */
#define INM_ACCURACY_FP		67502	/* 1.03 * 65536 */

/* Keccak-1600 constants */
#define KECCAK_STATE_SIZE	200	/* 1600 bits = 200 bytes */
#define KECCAK_LANES		25	/* 5x5 lanes */
#define KECCAK_ROUNDS		24	/* Number of rounds */

/*
 * hwrng quality settings (bits of entropy per 1024 bits of input)
 * - Raw mode: 0.88 bits/bit (unwhitened, reflects true entropy rate)
 * - Whitened mode: ~1.0 bits/bit (Keccak output is cryptographically uniform)
 */
#define INFNOISE_QUALITY_RAW		880	/* Raw: 0.88 bits/bit */
#define INFNOISE_QUALITY_WHITENED	990	/* Whitened: ~1.0 bits/bit, slight margin */

/* Timeouts */
#define INFNOISE_USB_TIMEOUT	1000	/* USB timeout in ms */
#define INFNOISE_WARMUP_ROUNDS	5000	/* Max warmup rounds */

/* Error recovery */
#define INFNOISE_MAX_RETRIES		3	/* Retries per transfer */
#define INFNOISE_RECOVERY_THRESHOLD	5	/* Consecutive errors before recovery */
#define INFNOISE_RETRY_DELAY_MS		10	/* Delay between retries */

/* Module parameters */
extern bool infnoise_debug;
extern bool infnoise_raw_mode;

/* IOCTL definitions */
#define INFNOISE_IOC_MAGIC	'N'
#define INFNOISE_GET_STATS	_IOR(INFNOISE_IOC_MAGIC, 1, struct infnoise_stats)
#define INFNOISE_SET_RAW	_IOW(INFNOISE_IOC_MAGIC, 2, int)
#define INFNOISE_GET_ENTROPY	_IOR(INFNOISE_IOC_MAGIC, 3, u32)

/*
 * Statistics structure for ioctl (UAPI)
 *
 * Layout must be identical on 32-bit and 64-bit so that a 32-bit
 * userspace process can issue INFNOISE_GET_STATS on a 64-bit kernel.
 * Explicit padding ensures sizeof() == 40 on both, and __aligned(8)
 * forces u64 alignment on 32-bit where u64 is normally 4-byte aligned.
 */
struct infnoise_stats {
	__u64 total_bits;
	__u32 total_ones;
	__u32 total_zeros;
	__u32 even_misfires;
	__u32 odd_misfires;
	__u32 entropy_estimate;	/* Fixed-point 16.16 */
	__u32 k_estimate;	/* Fixed-point 16.16 */
	__u8 warmup_complete;
	__u8 health_ok;
	__u8 __pad[6];
} __aligned(8);

/* Health check state */
struct infnoise_health {
	u32 *ones_even;		/* Prediction table for even bits = 1 */
	u32 *zeros_even;	/* Prediction table for even bits = 0 */
	u32 *ones_odd;		/* Prediction table for odd bits = 1 */
	u32 *zeros_odd;		/* Prediction table for odd bits = 0 */

	u32 prev_bits;		/* Previous N bits for prediction */
	u32 num_bits_sampled;	/* Total bits in current entropy window */
	u32 num_bits_of_entropy;/* Estimated entropy bits */
	u32 entropy_level;	/* Accumulated entropy */

	u64 total_bits;		/* Total bits processed */
	u32 total_ones;		/* Count of 1 bits */
	u32 total_zeros;	/* Count of 0 bits */

	u32 even_misfires;	/* Even bit prediction failures */
	u32 odd_misfires;	/* Odd bit prediction failures */

	u32 seq_zeros;		/* Consecutive zeros */
	u32 seq_ones;		/* Consecutive ones */

	u32 current_prob;	/* Current probability (fixed-point) */

	bool prev_bit;		/* Previous bit value */
	bool prev_even;		/* Previous even bit */
	bool prev_odd;		/* Previous odd bit */
	bool ok_to_use;		/* Health check passed */
};

/* Keccak state */
struct infnoise_keccak {
	u8 state[KECCAK_STATE_SIZE];
};

/* Device state flags */
enum infnoise_flags {
	INFNOISE_PRESENT	= BIT(0),	/* Device is connected */
	INFNOISE_WARMUP_DONE	= BIT(1),	/* Warmup complete */
	INFNOISE_HWRNG_REG	= BIT(2),	/* hwrng registered */
	INFNOISE_HEALTH_FAIL	= BIT(3),	/* Health check failed */
};

/* Main device structure */
struct infnoise_device {
	struct usb_device *udev;
	struct usb_interface *intf;

	/* USB endpoints */
	u8 bulk_in_ep;
	u8 bulk_out_ep;

	/* Buffers */
	u8 *clock_buf;		/* Clock pattern buffer (512 bytes) */
	u8 *usb_buf;		/* Raw USB read buffer (576 bytes w/ FTDI status) */
	u8 *read_buf;		/* Processed read buffer (512 bytes, status stripped) */
	u8 *out_buf;		/* Output buffer (64 bytes) */

	/* State */
	struct infnoise_health health;
	struct infnoise_keccak keccak;
	u32 keccak_bytes_left;		/* Bytes remaining from current absorption */

	/* hwrng interface */
	struct hwrng hwrng;
	char hwrng_name[32];

	/* Character device */
	struct usb_class_driver class;
	int minor;
	bool raw_mode;		/* Per-device raw mode */

	/* Synchronization */
	struct mutex lock;		/* Device access lock */
	struct completion warmup_done;	/* Warmup completion */
	unsigned long flags;

	/* Warmup work */
	struct work_struct warmup_work;

	/* Statistics */
	u64 bytes_generated;
	u32 usb_errors;
	u32 consecutive_errors;		/* Consecutive USB errors */
	u32 recoveries;			/* Successful recovery count */
};

/* Keccak functions (infnoise_keccak.c) */
void infnoise_keccak_init(struct infnoise_keccak *keccak);
void infnoise_keccak_absorb(struct infnoise_keccak *keccak, const u8 *data,
			    unsigned int lanes);
void infnoise_keccak_extract(struct infnoise_keccak *keccak, u8 *data,
			     unsigned int lanes);
void infnoise_keccak_permutation(struct infnoise_keccak *keccak);

/* Health check functions (infnoise_health.c) */
int infnoise_health_init(struct infnoise_health *health);
void infnoise_health_free(struct infnoise_health *health);
void infnoise_health_reset(struct infnoise_health *health);
bool infnoise_health_add_bit(struct infnoise_health *health, bool even_bit,
			     bool odd_bit, bool even);
bool infnoise_health_ok(struct infnoise_health *health);
u32 infnoise_health_get_entropy(struct infnoise_health *health);
void infnoise_health_clear_entropy(struct infnoise_health *health);
bool infnoise_health_entropy_on_target(struct infnoise_health *health,
				       u32 entropy, u32 num_bits);
u32 infnoise_health_estimate_k(struct infnoise_health *health);
u32 infnoise_health_estimate_entropy_per_bit(struct infnoise_health *health);
void infnoise_health_get_stats(struct infnoise_health *health,
			       struct infnoise_stats *stats);

#endif /* INFNOISE_H */
