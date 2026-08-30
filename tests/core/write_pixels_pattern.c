// SPDX-License-Identifier: GPL-2.1-or-later
/*
 * Copyright (C) 2026 Cyril Hrubis <metan@ucw.cz>
 */

/*
 * gp_write_pixels_pattern_*() write the middle of a run byte by byte and each
 * of the two ends, where the run does not start or stop on a byte boundary,
 * with a single read-modify-write under the bits that end covers.
 *
 * The ends are where a pixel size, a bit order and a start offset all meet, so
 * they are checked against the obvious per pixel implementation, for every
 * combination of them: 1, 2 and 4 bits per pixel, both bit orders, every start
 * offset, lengths that stop inside the first byte and lengths that run past it,
 * and every mask the pattern can hand down -- a pixel is either covered or not,
 * so a field of the mask is all ones or all zeroes.
 *
 * At 4 bits per pixel the row is two different bytes that alternate, starting
 * with the upper one, the mask is checked over both of them. The middle of an
 * opaque run is written as 16 bit pixels there, which is why the runs are
 * long enough to contain a few of them.
 */

#include <string.h>
#include <core/gp_pixel.h>
#include <core/gp_get_set_bits.h>
#include <core/gp_write_pixels.gen.h>

#include "tst_test.h"

#define BUF_SIZE 16
#define FILL 0x39
#define PIXELS 0xc65a

static void write_pixels(uint8_t *buf, uint8_t off, size_t cnt, uint16_t mask,
                         uint8_t bpp, int up_bit)
{
	uint8_t ppb = 8 / bpp;
	size_t i;

	for (i = 0; i < cnt; i++) {
		size_t pos = off + i;
		uint8_t *dst = buf + pos / ppb;
		uint8_t idx = pos % ppb;
		unsigned int shift = up_bit ? 8 - bpp * (idx + 1) : bpp * idx;
		/* The upper byte at even bytes, 1 and 2 bpp use the same one */
		unsigned int half = bpp == 4 && (pos / ppb) % 2 ? 0 : 8;
		uint8_t byte = bpp == 4 ? PIXELS >> half : PIXELS >> 8;
		uint8_t m = bpp == 4 ? mask >> half : mask;

		if (GP_GET_BITS(shift, bpp, m))
			GP_SET_BITS1(shift, bpp, dst, GP_GET_BITS(shift, bpp, byte));
	}
}

static void write_pattern(uint8_t *buf, uint8_t off, size_t cnt, uint16_t mask,
                          uint8_t bpp, int up_bit)
{
	/* Up to two bits per pixel both bytes of the row are the same */
	gp_pixel_pattern_row row8 = {
		.pixels = (PIXELS >> 8) * 0x0101,
		.mask = (mask & 0xff) * 0x0101,
	};
	gp_pixel_pattern_row row16 = {.pixels = PIXELS, .mask = mask};

	switch (bpp) {
	case 1:
		if (up_bit)
			gp_write_pixels_pattern_1BPP_UB(buf, off, cnt, &row8);
		else
			gp_write_pixels_pattern_1BPP_DB(buf, off, cnt, &row8);
	break;
	case 2:
		if (up_bit)
			gp_write_pixels_pattern_2BPP_UB(buf, off, cnt, &row8);
		else
			gp_write_pixels_pattern_2BPP_DB(buf, off, cnt, &row8);
	break;
	case 4:
		if (up_bit)
			gp_write_pixels_pattern_4BPP_UB(buf, off, cnt, &row16);
		else
			gp_write_pixels_pattern_4BPP_DB(buf, off, cnt, &row16);
	break;
	}
}

/*
 * The run is written at an even and at an odd address as well, the 4 bpp
 * middle is written as 16 bit pixels, which have to start at an even one.
 */
static int pattern_run(uint8_t bpp, int up_bit, uint16_t mask,
                       uint8_t off, size_t cnt)
{
	uint16_t res_buf[BUF_SIZE/2 + 1], exp_buf[BUF_SIZE/2 + 1];
	unsigned int align;
	int err = 0;

	for (align = 0; align < 2; align++) {
		uint8_t *res = (uint8_t *)res_buf + align;
		uint8_t *exp = (uint8_t *)exp_buf + align;

		memset(res_buf, FILL, sizeof(res_buf));
		memset(exp_buf, FILL, sizeof(exp_buf));

		write_pattern(res, off, cnt, mask, bpp, up_bit);
		write_pixels(exp, off, cnt, mask, bpp, up_bit);

		if (!memcmp(res_buf, exp_buf, sizeof(res_buf)))
			continue;

		tst_msg("bpp=%u up_bit=%i mask=%04x off=%u cnt=%zu align=%u",
		        bpp, up_bit, mask, off, cnt, align);
		err++;
	}

	return err;
}

static int pattern_ends(unsigned int bpp)
{
	uint8_t ppb = 8 / bpp;
	/* Pixels the mask covers, two bytes of them at 4 bpp */
	uint8_t ppm = bpp == 4 ? 2 * ppb : ppb;
	unsigned int m, off, up_bit;
	size_t cnt;
	int err = 0;

	for (up_bit = 0; up_bit < 2; up_bit++) {
		for (m = 0; m < 1u << ppm; m++) {
			uint16_t mask = 0;
			uint8_t i;

			for (i = 0; i < ppm; i++) {
				if (m & (1u << i))
					mask |= ((1u << bpp) - 1) << (bpp * i);
			}

			for (off = 0; off < ppb; off++) {
				for (cnt = 1; cnt <= 5u * ppm; cnt++)
					err += pattern_run(bpp, up_bit, mask, off, cnt);
			}
		}
	}

	if (err)
		tst_msg("%i runs of %u bpp differ", err, bpp);

	return err ? TST_FAILED : TST_PASSED;
}

static int pattern_ends_1bpp(void)
{
	return pattern_ends(1);
}

static int pattern_ends_2bpp(void)
{
	return pattern_ends(2);
}

static int pattern_ends_4bpp(void)
{
	return pattern_ends(4);
}

const struct tst_suite tst_suite = {
	.suite_name = "Pattern Write Pixels Testsuite",
	.tests = {
		{.name = "Pattern run ends 1bpp", .tst_fn = pattern_ends_1bpp},
		{.name = "Pattern run ends 2bpp", .tst_fn = pattern_ends_2bpp},
		{.name = "Pattern run ends 4bpp", .tst_fn = pattern_ends_4bpp},
		{.name = NULL}
	}
};
