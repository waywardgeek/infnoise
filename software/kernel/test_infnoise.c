// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Userspace test harness for Infinite Noise TRNG kernel module
 *
 * Copyright (C) 2026 Avinash H. Duduskar
 *
 * Verifies driver mechanics: read interfaces, ioctl dispatch, non-blocking
 * I/O, and device lifecycle.  Entropy quality testing is handled by
 * make test (rngtest, ent) and the project's tests/runtests.sh suite.
 *
 * Requires: module loaded, device connected, root privileges.
 *
 * Build:  make test_infnoise
 * Run:    sudo ./test_infnoise   (or: make test)
 *
 * VID/PID note: the module matches 0x1209:0x3701 (pid.codes registered).
 * Stock Infinite Noise hardware ships as 0x0403:0x6015 (FTDI generic).
 * To test on unmodified hardware, temporarily change INFNOISE_VENDOR_ID
 * and INFNOISE_PRODUCT_ID in infnoise.h, rebuild, rmmod ftdi_sio, and
 * insmod the patched module.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <stdint.h>

/* Must match kernel header exactly */
struct infnoise_stats {
	uint64_t total_bits;
	uint32_t total_ones;
	uint32_t total_zeros;
	uint32_t even_misfires;
	uint32_t odd_misfires;
	uint32_t entropy_estimate;
	uint32_t k_estimate;
	uint8_t warmup_complete;
	uint8_t health_ok;
	uint8_t __pad[6];
} __attribute__((aligned(8)));

#define INFNOISE_IOC_MAGIC	'N'
#define INFNOISE_GET_STATS	_IOR(INFNOISE_IOC_MAGIC, 1, struct infnoise_stats)
#define INFNOISE_SET_RAW	_IOW(INFNOISE_IOC_MAGIC, 2, int)
#define INFNOISE_GET_ENTROPY	_IOR(INFNOISE_IOC_MAGIC, 3, uint32_t)

static int passed = 0, failed = 0, skipped = 0;

#define TEST(name) printf("  %-50s ", name)
#define PASS() do { printf("\033[32mPASS\033[0m\n"); passed++; } while(0)
#define FAIL(msg) do { printf("\033[31mFAIL\033[0m (%s)\n", msg); failed++; } while(0)
#define SKIP(msg) do { printf("\033[33mSKIP\033[0m (%s)\n", msg); skipped++; } while(0)

/* ── Read interfaces ────────────────────────────────────── */

static void test_chardev_read(int fd)
{
	uint8_t buf[256];
	ssize_t n;

	TEST("chardev read returns data");
	n = read(fd, buf, sizeof(buf));
	if (n < 0) {
		FAIL(strerror(errno));
		return;
	}
	if (n == 0) {
		FAIL("got 0 bytes");
		return;
	}
	printf("got %zd bytes ", n);
	PASS();
}

static void test_hwrng_read(void)
{
	int fd;
	uint8_t buf[64];
	ssize_t n;

	TEST("/dev/hwrng read returns data");
	fd = open("/dev/hwrng", O_RDONLY);
	if (fd < 0) {
		SKIP(strerror(errno));
		return;
	}
	n = read(fd, buf, sizeof(buf));
	close(fd);
	if (n < 0) {
		FAIL(strerror(errno));
		return;
	}
	if (n == 0) {
		FAIL("got 0 bytes");
		return;
	}
	printf("got %zd bytes ", n);
	PASS();
}

/* ── Non-blocking I/O ───────────────────────────────────── */

static void test_nonblock(void)
{
	TEST("O_NONBLOCK returns EAGAIN or data");
	int fd = open("/dev/infnoise0", O_RDONLY | O_NONBLOCK);
	if (fd < 0) {
		FAIL(strerror(errno));
		return;
	}
	uint8_t buf[64];
	ssize_t n = read(fd, buf, sizeof(buf));
	close(fd);

	if (n > 0) {
		printf("got %zd bytes ", n);
		PASS();
	} else if (n < 0 && errno == EAGAIN) {
		printf("EAGAIN (device warming up) ");
		PASS();
	} else if (n < 0) {
		FAIL(strerror(errno));
	} else {
		FAIL("unexpected 0 return");
	}
}

/* ── Ioctl interface ────────────────────────────────────── */

static void test_ioctl_get_stats(int fd)
{
	struct infnoise_stats stats;

	TEST("GET_STATS returns sane values");
	memset(&stats, 0xAA, sizeof(stats));

	if (ioctl(fd, INFNOISE_GET_STATS, &stats) < 0) {
		FAIL(strerror(errno));
		return;
	}

	if (!stats.warmup_complete) {
		FAIL("warmup_complete is 0");
		return;
	}
	if (!stats.health_ok) {
		FAIL("health_ok is 0");
		return;
	}

	printf("bits=%lu ones=%u zeros=%u ",
	       (unsigned long)stats.total_bits, stats.total_ones,
	       stats.total_zeros);
	PASS();
}

static void test_ioctl_get_entropy(int fd)
{
	uint32_t entropy = 0;

	TEST("GET_ENTROPY returns non-zero");
	if (ioctl(fd, INFNOISE_GET_ENTROPY, &entropy) < 0) {
		FAIL(strerror(errno));
		return;
	}
	if (entropy == 0) {
		FAIL("entropy is 0");
		return;
	}
	printf("entropy=%u ", entropy);
	PASS();
}

static void test_ioctl_set_raw(int fd)
{
	int raw = 0;

	TEST("SET_RAW accepts value");
	if (ioctl(fd, INFNOISE_SET_RAW, &raw) < 0) {
		FAIL(strerror(errno));
		return;
	}
	PASS();
}

static void test_ioctl_bad_cmd(int fd)
{
	TEST("invalid ioctl returns ENOTTY");
	int ret = ioctl(fd, _IO('N', 99), NULL);
	if (ret == -1 && errno == ENOTTY) {
		PASS();
	} else {
		char buf[64];
		snprintf(buf, sizeof(buf), "ret=%d errno=%d (%s)",
			 ret, errno, strerror(errno));
		FAIL(buf);
	}
}

/* ── Device lifecycle ───────────────────────────────────── */

static void test_open_close_reopen(void)
{
	TEST("open/close/reopen 5 times");

	for (int i = 0; i < 5; i++) {
		int fd = open("/dev/infnoise0", O_RDONLY);
		if (fd < 0) {
			char msg[64];
			snprintf(msg, sizeof(msg), "open %d: %s", i, strerror(errno));
			FAIL(msg);
			return;
		}
		uint8_t buf[32];
		ssize_t n = read(fd, buf, sizeof(buf));
		close(fd);
		if (n <= 0) {
			char msg[64];
			snprintf(msg, sizeof(msg), "read %d: %zd bytes", i, n);
			FAIL(msg);
			return;
		}
	}
	printf("5 cycles OK ");
	PASS();
}

/* ── Main ───────────────────────────────────────────────── */

int main(void)
{
	int fd;

	printf("\nInfinite Noise TRNG Kernel Module — Test Harness\n");
	printf("=================================================\n\n");

	fd = open("/dev/infnoise0", O_RDONLY);
	if (fd < 0) {
		fprintf(stderr, "Cannot open /dev/infnoise0: %s\n", strerror(errno));
		fprintf(stderr, "Is the module loaded? Try: sudo insmod infnoise.ko\n");
		return 1;
	}

	printf("--- Read interfaces ---\n");
	test_chardev_read(fd);
	test_hwrng_read();

	printf("\n--- Non-blocking I/O ---\n");
	test_nonblock();

	printf("\n--- Ioctl interface ---\n");
	test_ioctl_get_stats(fd);
	test_ioctl_get_entropy(fd);
	test_ioctl_set_raw(fd);
	test_ioctl_bad_cmd(fd);

	printf("\n--- Device lifecycle ---\n");
	test_open_close_reopen();

	close(fd);

	printf("\n=================================================\n");
	printf("Results: %d passed, %d failed, %d skipped\n\n",
	       passed, failed, skipped);

	return failed > 0 ? 1 : 0;
}
