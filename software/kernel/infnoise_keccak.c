// SPDX-License-Identifier: CC0-1.0
/*
 * Keccak-1600 implementation for Infinite Noise TRNG kernel driver
 *
 * Based on the reference implementation by Guido Bertoni, Joan Daemen,
 * Michael Peeters and Gilles Van Assche.
 *
 * To the extent possible under law, the implementer has waived all copyright
 * and related or neighboring rights to the source code in this file.
 * http://creativecommons.org/publicdomain/zero/1.0/
 *
 * Adapted for Linux kernel by Manuel Domke
 */

#include <linux/types.h>
#include <linux/string.h>
#include <asm/byteorder.h>
#include "infnoise.h"

/* Pre-computed round constants for Keccak-1600 */
static const u64 keccak_round_constants[KECCAK_ROUNDS] = {
	0x0000000000000001ULL, 0x0000000000008082ULL,
	0x800000000000808aULL, 0x8000000080008000ULL,
	0x000000000000808bULL, 0x0000000080000001ULL,
	0x8000000080008081ULL, 0x8000000000008009ULL,
	0x000000000000008aULL, 0x0000000000000088ULL,
	0x0000000080008009ULL, 0x000000008000000aULL,
	0x000000008000808bULL, 0x800000000000008bULL,
	0x8000000000008089ULL, 0x8000000000008003ULL,
	0x8000000000008002ULL, 0x8000000000000080ULL,
	0x000000000000800aULL, 0x800000008000000aULL,
	0x8000000080008081ULL, 0x8000000000008080ULL,
	0x0000000080000001ULL, 0x8000000080008008ULL
};

/* Pre-computed rho offsets */
static const unsigned int keccak_rho_offsets[KECCAK_LANES] = {
	 0,  1, 62, 28, 27,
	36, 44,  6, 55, 20,
	 3, 10, 43, 25, 39,
	41, 45, 15, 21,  8,
	18,  2, 61, 56, 14
};

/* Lane index macro: x + 5*y */
#define KECCAK_INDEX(x, y) (((x) % 5) + 5 * ((y) % 5))

/* 64-bit rotate left - handle edge cases to avoid undefined behavior */
static inline u64 keccak_rol64(u64 x, unsigned int n)
{
	n &= 63;  /* Normalize to 0-63 range */
	if (n == 0)
		return x;
	return (x << n) | (x >> (64 - n));
}

/* Theta step */
static void keccak_theta(u64 *state)
{
	u64 c[5], d[5];
	unsigned int x, y;

	/* Compute column parities */
	for (x = 0; x < 5; x++) {
		c[x] = 0;
		for (y = 0; y < 5; y++)
			c[x] ^= state[KECCAK_INDEX(x, y)];
	}

	/* Compute D values */
	for (x = 0; x < 5; x++)
		d[x] = keccak_rol64(c[(x + 1) % 5], 1) ^ c[(x + 4) % 5];

	/* XOR D into state */
	for (x = 0; x < 5; x++)
		for (y = 0; y < 5; y++)
			state[KECCAK_INDEX(x, y)] ^= d[x];
}

/* Rho step */
static void keccak_rho(u64 *state)
{
	unsigned int i;

	for (i = 0; i < KECCAK_LANES; i++)
		state[i] = keccak_rol64(state[i], keccak_rho_offsets[i]);
}

/* Pi step */
static void keccak_pi(u64 *state)
{
	u64 temp[KECCAK_LANES];
	unsigned int x, y;

	memcpy(temp, state, sizeof(temp));

	for (x = 0; x < 5; x++)
		for (y = 0; y < 5; y++)
			state[KECCAK_INDEX(y, 2 * x + 3 * y)] =
				temp[KECCAK_INDEX(x, y)];
}

/* Chi step */
static void keccak_chi(u64 *state)
{
	u64 c[5];
	unsigned int x, y;

	for (y = 0; y < 5; y++) {
		for (x = 0; x < 5; x++)
			c[x] = state[KECCAK_INDEX(x, y)] ^
			       ((~state[KECCAK_INDEX(x + 1, y)]) &
				state[KECCAK_INDEX(x + 2, y)]);
		for (x = 0; x < 5; x++)
			state[KECCAK_INDEX(x, y)] = c[x];
	}
}

/* Iota step */
static void keccak_iota(u64 *state, unsigned int round)
{
	state[0] ^= keccak_round_constants[round];
}

/* Full Keccak-f[1600] permutation on 64-bit words */
static void keccak_permutation_on_words(u64 *state)
{
	unsigned int i;

	for (i = 0; i < KECCAK_ROUNDS; i++) {
		keccak_theta(state);
		keccak_rho(state);
		keccak_pi(state);
		keccak_chi(state);
		keccak_iota(state, i);
	}
}

/* Convert bytes to words (for big-endian systems) */
#if defined(__BIG_ENDIAN)
static void keccak_bytes_to_words(u64 *words, const u8 *bytes)
{
	unsigned int i, j;

	for (i = 0; i < KECCAK_LANES; i++) {
		words[i] = 0;
		for (j = 0; j < 8; j++)
			words[i] |= (u64)bytes[i * 8 + j] << (8 * j);
	}
}

static void keccak_words_to_bytes(u8 *bytes, const u64 *words)
{
	unsigned int i, j;

	for (i = 0; i < KECCAK_LANES; i++)
		for (j = 0; j < 8; j++)
			bytes[i * 8 + j] = (words[i] >> (8 * j)) & 0xFF;
}
#endif

/**
 * infnoise_keccak_permutation - Apply Keccak-f[1600] permutation
 * @keccak: Keccak state structure
 *
 * Applies the full 24-round Keccak-f[1600] permutation to the state.
 */
void infnoise_keccak_permutation(struct infnoise_keccak *keccak)
{
#if defined(__BIG_ENDIAN)
	u64 state_words[KECCAK_LANES];

	keccak_bytes_to_words(state_words, keccak->state);
	keccak_permutation_on_words(state_words);
	keccak_words_to_bytes(keccak->state, state_words);
#else
	/* On little-endian, we can work directly on the byte array */
	keccak_permutation_on_words((u64 *)keccak->state);
#endif
}

/**
 * infnoise_keccak_init - Initialize Keccak state
 * @keccak: Keccak state structure
 *
 * Zeros the Keccak state to prepare for absorbing data.
 */
void infnoise_keccak_init(struct infnoise_keccak *keccak)
{
	memset(keccak->state, 0, KECCAK_STATE_SIZE);
}

/**
 * infnoise_keccak_absorb - Absorb data into Keccak sponge
 * @keccak: Keccak state structure
 * @data: Data to absorb
 * @lanes: Number of 64-bit lanes (bytes / 8)
 *
 * XORs data into the state and applies the permutation.
 * This implements the absorb phase of the sponge construction.
 */
void infnoise_keccak_absorb(struct infnoise_keccak *keccak, const u8 *data,
			    unsigned int lanes)
{
	unsigned int i;
	unsigned int bytes = lanes * 8;

	/* XOR data into state */
	for (i = 0; i < bytes && i < KECCAK_STATE_SIZE; i++)
		keccak->state[i] ^= data[i];

	/* Apply permutation */
	infnoise_keccak_permutation(keccak);
}

/**
 * infnoise_keccak_extract - Extract data from Keccak sponge
 * @keccak: Keccak state structure
 * @data: Buffer to store extracted data
 * @lanes: Number of 64-bit lanes to extract (bytes / 8)
 *
 * Copies bytes from the state. Does NOT apply permutation;
 * caller should call keccak_permutation() between extracts
 * if extracting more data than the rate allows.
 */
void infnoise_keccak_extract(struct infnoise_keccak *keccak, u8 *data,
			     unsigned int lanes)
{
	unsigned int bytes = lanes * 8;

	if (bytes > KECCAK_STATE_SIZE)
		bytes = KECCAK_STATE_SIZE;

	memcpy(data, keccak->state, bytes);
}
