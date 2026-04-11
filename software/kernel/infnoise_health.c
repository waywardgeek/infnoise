// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Health monitoring for Infinite Noise TRNG kernel driver
 *
 * Copyright (C) 2024 Manuel Domke
 *
 * Based on healthcheck.c from the userspace driver by Bill Cox
 *
 * This implements entropy measurement using prediction-based analysis.
 * The algorithm tracks how predictable the next bit is based on the
 * previous N bits, and uses this to estimate actual entropy.
 *
 * All arithmetic uses fixed-point (16.16 format) to avoid floating
 * point operations in the kernel.
 */

#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/vmalloc.h>
#include <linux/log2.h>
#include "infnoise.h"

/* Module parameter reference */
extern bool infnoise_debug;

/*
 * Fixed-point multiplication: (a * b) >> FP_SHIFT
 * Uses 64-bit intermediate to avoid overflow
 */
static inline u32 fp_mul(u32 a, u32 b)
{
	return (u32)(((u64)a * b) >> FP_SHIFT);
}

/*
 * Fixed-point division: (a << FP_SHIFT) / b
 * Uses 64-bit intermediate to avoid overflow
 */
static inline u32 fp_div(u32 a, u32 b)
{
	if (b == 0)
		return FP_ONE;
	return (u32)(((u64)a << FP_SHIFT) / b);
}

/**
 * infnoise_health_init - Initialize health check state
 * @health: Health check state structure
 *
 * Allocates prediction tables and initializes all state variables.
 * Returns 0 on success, negative error code on failure.
 */
int infnoise_health_init(struct infnoise_health *health)
{
	size_t table_size = INM_TABLE_SIZE * sizeof(u32);

	memset(health, 0, sizeof(*health));

	/* Allocate prediction tables using vmalloc (4 x 64KB = 256KB) */
	health->ones_even = vzalloc(table_size);
	health->zeros_even = vzalloc(table_size);
	health->ones_odd = vzalloc(table_size);
	health->zeros_odd = vzalloc(table_size);

	if (!health->ones_even || !health->zeros_even ||
	    !health->ones_odd || !health->zeros_odd) {
		infnoise_health_free(health);
		return -ENOMEM;
	}

	/* Initialize state */
	health->current_prob = FP_ONE;
	health->ok_to_use = false;

	return 0;
}

/**
 * infnoise_health_free - Free health check resources
 * @health: Health check state structure
 *
 * Frees all allocated prediction tables.
 */
void infnoise_health_free(struct infnoise_health *health)
{
	vfree(health->ones_even);
	vfree(health->zeros_even);
	vfree(health->ones_odd);
	vfree(health->zeros_odd);

	health->ones_even = NULL;
	health->zeros_even = NULL;
	health->ones_odd = NULL;
	health->zeros_odd = NULL;
}

/**
 * infnoise_health_reset - Reset entropy counters
 * @health: Health check state structure
 *
 * Resets the per-sample entropy counters without clearing
 * the accumulated statistics tables.
 */
void infnoise_health_reset(struct infnoise_health *health)
{
	health->num_bits_sampled = 0;
	health->current_prob = FP_ONE;
	health->num_bits_of_entropy = 0;
	health->entropy_level = 0;
	health->total_ones = 0;
	health->total_zeros = 0;
	health->even_misfires = 0;
	health->odd_misfires = 0;
}

/*
 * Scale down statistics tables to prevent overflow.
 * Called when any counter reaches INM_MAX_COUNT.
 */
static void scale_stats(struct infnoise_health *health)
{
	unsigned int i;

	for (i = 0; i < INM_TABLE_SIZE; i++) {
		health->zeros_even[i] >>= 1;
		health->ones_even[i] >>= 1;
		health->zeros_odd[i] >>= 1;
		health->ones_odd[i] >>= 1;
	}
}

/*
 * Scale down entropy counters to prevent overflow.
 */
static void scale_entropy(struct infnoise_health *health)
{
	if (health->num_bits_sampled >= INM_MIN_DATA) {
		health->num_bits_of_entropy >>= 1;
		health->num_bits_sampled >>= 1;
		health->even_misfires >>= 1;
		health->odd_misfires >>= 1;
	}
}

/*
 * Scale down zero/one totals to prevent overflow.
 */
static void scale_zero_one_counts(struct infnoise_health *health)
{
	u32 max_val = max(health->total_zeros, health->total_ones);

	if (max_val >= INM_MIN_DATA) {
		health->total_zeros >>= 1;
		health->total_ones >>= 1;
	}
}

/**
 * infnoise_health_add_bit - Process a new bit from the INM
 * @health: Health check state structure
 * @even_bit: Value from COMP2 (even clock phase)
 * @odd_bit: Value from COMP1 (odd clock phase)
 * @even: True if this is an even-phase sample
 *
 * Updates prediction statistics and entropy estimate based on the new bit.
 * Returns true if health check is still OK, false if a fatal error occurred.
 */
bool infnoise_health_add_bit(struct infnoise_health *health, bool even_bit,
			     bool odd_bit, bool even)
{
	bool bit;
	u32 zeros, ones, total;
	u32 mask = INM_TABLE_SIZE - 1;

	/* Select the appropriate bit and track misfires */
	if (even) {
		bit = even_bit;
		if (even_bit != health->prev_even)
			health->even_misfires++;
	} else {
		bit = odd_bit;
		if (odd_bit != health->prev_odd)
			health->odd_misfires++;
	}

	health->prev_even = even_bit;
	health->prev_odd = odd_bit;
	health->total_bits++;

	/* Update previous bits for next prediction */
	health->prev_bits = ((health->prev_bits << 1) | health->prev_bit) & mask;
	health->prev_bit = bit;

	/* Check for stuck-at faults after warmup */
	if (health->num_bits_sampled > INM_MIN_SAMPLE_SIZE) {
		if (bit) {
			health->total_ones++;
			health->seq_ones++;
			health->seq_zeros = 0;

			if (health->seq_ones > INM_MAX_SEQUENCE) {
				pr_crit("infnoise: stuck-at-1 fault detected (%u consecutive 1s)\n",
					health->seq_ones);
				return false;
			}
		} else {
			health->total_zeros++;
			health->seq_zeros++;
			health->seq_ones = 0;

			if (health->seq_zeros > INM_MAX_SEQUENCE) {
				pr_crit("infnoise: stuck-at-0 fault detected (%u consecutive 0s)\n",
					health->seq_zeros);
				return false;
			}
		}
	}

	/* Get prediction statistics for current context */
	if (even) {
		zeros = health->zeros_even[health->prev_bits];
		ones = health->ones_even[health->prev_bits];
	} else {
		zeros = health->zeros_odd[health->prev_bits];
		ones = health->ones_odd[health->prev_bits];
	}

	total = zeros + ones;

	/*
	 * Update probability estimate.
	 * If we have statistics, multiply probability by the frequency
	 * of the observed outcome given the context.
	 */
	if (total > 0) {
		u32 freq;

		if (bit) {
			if (ones > 0) {
				freq = fp_div(ones, total);
				health->current_prob = fp_mul(health->current_prob, freq);
			}
		} else {
			if (zeros > 0) {
				freq = fp_div(zeros, total);
				health->current_prob = fp_mul(health->current_prob, freq);
			}
		}
	}

	/*
	 * When probability drops below 0.5, we've accumulated at least
	 * 1 bit of entropy. Double the probability and count the bit.
	 */
	while (health->current_prob <= FP_HALF) {
		health->current_prob <<= 1;
		health->num_bits_of_entropy++;

		if (health->ok_to_use)
			health->entropy_level++;
	}

	health->num_bits_sampled++;

	/* Update prediction tables */
	if (bit) {
		if (even) {
			health->ones_even[health->prev_bits]++;
			if (health->ones_even[health->prev_bits] >= INM_MAX_COUNT)
				scale_stats(health);
		} else {
			health->ones_odd[health->prev_bits]++;
			if (health->ones_odd[health->prev_bits] >= INM_MAX_COUNT)
				scale_stats(health);
		}
	} else {
		if (even) {
			health->zeros_even[health->prev_bits]++;
			if (health->zeros_even[health->prev_bits] >= INM_MAX_COUNT)
				scale_stats(health);
		} else {
			health->zeros_odd[health->prev_bits]++;
			if (health->zeros_odd[health->prev_bits] >= INM_MAX_COUNT)
				scale_stats(health);
		}
	}

	scale_entropy(health);
	scale_zero_one_counts(health);

	/* Update health status */
	health->ok_to_use = infnoise_health_ok(health);

	return true;
}

/**
 * infnoise_health_estimate_entropy_per_bit - Get entropy estimate
 * @health: Health check state structure
 *
 * Returns the estimated entropy per bit in fixed-point format (16.16).
 */
u32 infnoise_health_estimate_entropy_per_bit(struct infnoise_health *health)
{
	if (health->num_bits_sampled == 0)
		return 0;

	return fp_div(health->num_bits_of_entropy, health->num_bits_sampled);
}

/**
 * infnoise_health_estimate_k - Estimate amplifier gain K
 * @health: Health check state structure
 *
 * Returns the estimated K value in fixed-point format.
 * K = 2^(entropy_per_bit)
 *
 * Since we don't have pow() in kernel, we use a lookup table
 * for common entropy values around 0.88 bits/bit.
 */
u32 infnoise_health_estimate_k(struct infnoise_health *health)
{
	u32 entropy = infnoise_health_estimate_entropy_per_bit(health);

	/*
	 * K = 2^entropy
	 * For entropy around 0.88 (57671 in FP), K should be about 1.84
	 *
	 * Approximate 2^x for x in [0.7, 1.0] range:
	 * 2^0.70 = 1.625  -> 106496 FP
	 * 2^0.75 = 1.682  -> 110230 FP
	 * 2^0.80 = 1.741  -> 114098 FP
	 * 2^0.85 = 1.802  -> 118097 FP
	 * 2^0.88 = 1.840  -> 120586 FP (design target)
	 * 2^0.90 = 1.866  -> 122291 FP
	 * 2^0.95 = 1.932  -> 126617 FP
	 * 2^1.00 = 2.000  -> 131072 FP
	 *
	 * Linear interpolation: K ≈ 1 + 1.0 * entropy (rough)
	 * Better: use piecewise linear approximation
	 */

	/* Simple linear approximation for the expected range */
	/* 2^x ≈ 1 + 0.693*x + 0.240*x^2 for small x (Taylor) */
	/* For x ≈ 0.88, better to use direct calculation */

	/* Use lookup: entropy 57671 -> K 120586 */
	/* Scale: K = FP_ONE + entropy * 0.95 approximately */
	/* More accurate: K = 65536 + (entropy * 62806) >> 16 */

	return FP_ONE + fp_mul(entropy, 62806); /* 0.958 scaling factor */
}

/**
 * infnoise_health_ok - Check if health status is acceptable
 * @health: Health check state structure
 *
 * Returns true if enough data has been collected and the measured
 * entropy is within acceptable bounds of the expected entropy.
 */
bool infnoise_health_ok(struct infnoise_health *health)
{
	u32 entropy;
	u32 lower_bound, upper_bound;

	/* Need minimum amount of data */
	if (health->total_bits < INM_MIN_DATA)
		return false;

	entropy = infnoise_health_estimate_entropy_per_bit(health);

	/* Check entropy is within INM_ACCURACY of expected */
	/* expected * accuracy >= measured */
	lower_bound = fp_div(INM_EXPECTED_ENTROPY_FP, INM_ACCURACY_FP);
	/* measured <= expected * accuracy */
	upper_bound = fp_mul(INM_EXPECTED_ENTROPY_FP, INM_ACCURACY_FP);

	return (entropy >= lower_bound) && (entropy <= upper_bound);
}

/**
 * infnoise_health_get_entropy - Get accumulated entropy level
 * @health: Health check state structure
 *
 * Returns the number of entropy bits accumulated since last clear.
 */
u32 infnoise_health_get_entropy(struct infnoise_health *health)
{
	return health->entropy_level;
}

/**
 * infnoise_health_clear_entropy - Clear accumulated entropy counter
 * @health: Health check state structure
 */
void infnoise_health_clear_entropy(struct infnoise_health *health)
{
	health->entropy_level = 0;
}

/**
 * infnoise_health_entropy_on_target - Check if entropy meets target
 * @health: Health check state structure
 * @entropy: Measured entropy bits
 * @num_bits: Number of input bits
 *
 * Returns true if the measured entropy is at least what we expect
 * for the given number of input bits.
 */
bool infnoise_health_entropy_on_target(struct infnoise_health *health,
				       u32 entropy, u32 num_bits)
{
	u32 expected;

	/* expected = num_bits * expected_entropy_per_bit */
	expected = fp_mul(num_bits << FP_SHIFT, INM_EXPECTED_ENTROPY_FP) >> FP_SHIFT;

	/* Allow INM_ACCURACY tolerance */
	return (entropy * INM_ACCURACY_FP) >= (expected << FP_SHIFT);
}

/**
 * infnoise_health_get_stats - Get health statistics
 * @health: Health check state structure
 * @stats: Output statistics structure
 *
 * Fills in the statistics structure with current health check data.
 */
void infnoise_health_get_stats(struct infnoise_health *health,
			       struct infnoise_stats *stats)
{
	memset(stats, 0, sizeof(*stats));
	stats->total_bits = health->total_bits;
	stats->total_ones = health->total_ones;
	stats->total_zeros = health->total_zeros;
	stats->even_misfires = health->even_misfires;
	stats->odd_misfires = health->odd_misfires;
	stats->entropy_estimate = infnoise_health_estimate_entropy_per_bit(health);
	stats->k_estimate = infnoise_health_estimate_k(health);
	stats->warmup_complete = health->total_bits >= INM_MIN_DATA;
	stats->health_ok = health->ok_to_use;
}
