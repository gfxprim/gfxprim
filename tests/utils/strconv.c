// SPDX-License-Identifier: GPL-2.1-or-later
/*

   Copyright (C) 2025 Cyril Hrubis <metan@ucw.cz>

 */

#include <utils/gp_strconv.h>

#include "tst_test.h"

static int uint32_len_conversions(void)
{
	uint32_t i = 0;
	char buf[64];
	uint32_t inc = 1;
	uint32_t tick = 0;

	for (;;) {
		size_t len;
		ssize_t ret;
		uint32_t res;

		len = gp_u32toa(buf, i, false);
		ret = gp_atou32(buf, len, &res);

		if ((ssize_t)len != ret) {
			tst_msg("Converted len=%zu consumed len %zi (val %u)",
			        len, ret, (unsigned int) i);
			return TST_FAILED;
		}

		if (i != res) {
			tst_msg("Original value %u converted value %u",
			        (unsigned int) i, (unsigned int) res);
			return TST_FAILED;
		}

		if (i >= UINT32_MAX - inc)
			break;

		tick++;
		if (!(tick % 100000))
			inc *= 2;

		i += inc;
	}

	return TST_PASSED;
}

static int uint64_len_conversions(void)
{
	uint64_t i = 0;
	char buf[64];
	uint64_t inc = 1;
	uint64_t tick = 0;

	for (;;) {
		size_t len;
		ssize_t ret;
		uint64_t res;

		len = gp_u64toa(buf, i, false);
		ret = gp_atou64(buf, len, &res);

		if ((ssize_t)len != ret) {
			tst_msg("Converted len=%zu consumed len %zi (val %u)",
			        len, ret, (unsigned int) i);
			return TST_FAILED;
		}

		if (i != res) {
			tst_msg("Original value %llu converted value %llu",
			        (unsigned long long) i, (unsigned long long) res);
			return TST_FAILED;
		}

		if (i >= UINT64_MAX - inc)
			break;

		tick++;
		if (!(tick % 10000))
			inc *= 2;

		i += inc;
	}

	return TST_PASSED;
}

static int uint32_nul_conversions(void)
{
	uint32_t i = 0;
	char buf[64];
	uint32_t inc = 1;
	uint32_t tick = 0;

	for (;;) {
		size_t len;
		ssize_t ret;
		uint32_t res;

		len = gp_u32toa(buf, i, true);
		ret = gp_atou32(buf, 0, &res);

		if ((ssize_t)len != ret+1) {
			tst_msg("Converted len=%zu consumed len %zi (val %u)",
			        len, ret, (unsigned int) i);
			return TST_FAILED;
		}

		if (i != res) {
			tst_msg("Original value %u converted value %u",
			        (unsigned int) i, (unsigned int) res);
			return TST_FAILED;
		}

		if (i >= UINT32_MAX/2 - inc)
			break;

		tick++;
		if (!(tick % 100000))
			inc *= 2;

		i += inc;
	}

	return TST_PASSED;
}

static int uint64_nul_conversions(void)
{
	uint64_t i = 0;
	char buf[64];
	uint64_t inc = 1;
	uint64_t tick = 0;

	for (;;) {
		size_t len;
		ssize_t ret;
		uint64_t res;

		len = gp_u64toa(buf, i, true);
		ret = gp_atou64(buf, 0, &res);

		if ((ssize_t)len != ret+1) {
			tst_msg("Converted len=%zu consumed len %zi (val %u)",
			        len, ret, (unsigned int) i);
			return TST_FAILED;
		}

		if (i != res) {
			tst_msg("Original value %llu converted value %llu",
			        (unsigned long long) i, (unsigned long long) res);
			return TST_FAILED;
		}

		if (i >= UINT64_MAX - inc)
			break;

		tick++;
		if (!(tick % 10000))
			inc *= 2;

		i += inc;
	}

	return TST_PASSED;
}

static int uint32_limits(void)
{
	const char *uint32_max = "4294967295";
	const char *uint32_over_max = "4294967296";
	const char *uint32_too_large = "11111111111";
	uint32_t res;
	ssize_t ret;

	ret = gp_atou32(uint32_max, 0, &res);
	if (ret != 10) {
		tst_msg("Wrong return %zi expected 10", ret);
		return TST_FAILED;
	}

	if (res != UINT32_MAX) {
		tst_msg("Wrong value converted %u", (unsigned int)res);
		return TST_FAILED;
	}

	ret = gp_atou32(uint32_over_max, 0, &res);
	if (ret != -1) {
		tst_msg("Overflow not detected!");
		return TST_FAILED;
	}

	ret = gp_atou32(uint32_too_large, 0, &res);
	if (ret != -1) {
		tst_msg("Overflow not detected!");
		return TST_FAILED;
	}

	return TST_PASSED;
}

static int uint64_limits(void)
{
	const char *uint64_max = "18446744073709551615";
	const char *uint64_over_max = "18446744073709551616";
	const char *uint64_too_large = "111111111111111111111";
	uint64_t res;
	ssize_t ret;

	ret = gp_atou64(uint64_max, 0, &res);
	if (ret != 20) {
		tst_msg("Wrong return %zi expected 20", ret);
		return TST_FAILED;
	}

	if (res != UINT64_MAX) {
		tst_msg("Wrong value converted %llu",
		        (unsigned long long)res);
		return TST_FAILED;
	}

	ret = gp_atou64(uint64_over_max, 0, &res);
	if (ret != -1) {
		tst_msg("Overflow not detected!");
		return TST_FAILED;
	}

	ret = gp_atou64(uint64_too_large, 0, &res);
	if (ret != -1) {
		tst_msg("Overflow not detected!");
		return TST_FAILED;
	}

	return TST_PASSED;
}

const struct tst_suite tst_suite = {
	.suite_name = "ASCII integer conversions",
	.tests = {
		{.name = "uint32_t len conversions",
		 .tst_fn = uint32_len_conversions},

		{.name = "uint64_t len conversions",
		 .tst_fn = uint64_len_conversions},

		{.name = "uint32_t nul conversions",
		 .tst_fn = uint32_nul_conversions},

		{.name = "uint64_t nul conversions",
		 .tst_fn = uint64_nul_conversions},

		{.name = "uint32_t limits",
		 .tst_fn = uint32_limits},

		{.name = "uint64_t limits",
		 .tst_fn = uint64_limits},

		{}
	}
};
