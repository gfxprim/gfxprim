// SPDX-License-Identifier: GPL-2.1-or-later
/*
 * Copyright (C) 2023 Cyril Hrubis <metan@ucw.cz>
 */

#include <string.h>
#include <core/gp_core.h>
#include <filters/gp_filters.h>
#include "tst_test.h"

static int dither_bench_RGB888_to_G1(void)
{
	gp_pixmap *buf = gp_pixmap_alloc(1000, 1000, GP_PIXEL_RGB888);
	gp_pixmap *res = gp_pixmap_alloc(1000, 1000, GP_PIXEL_G1);
	unsigned int x, y;

	if (!buf || !res)
		return TST_UNTESTED;

	for (x = 0; x < 1000; x++) {
		for (y = 0; y < 1000; y++) {
			uint8_t *pix = (void*)GP_PIXEL_ADDR_RGB888(buf, x, y);

			pix[0] = 0x01;
			pix[1] = 0xaa;
			pix[2] = 0xff;
		}
	}

	gp_filter_floyd_steinberg(buf, res, NULL);

	gp_pixmap_free(buf);
	gp_pixmap_free(res);

	return TST_PASSED;
}

static int dither_bench_G8_to_G1(void)
{
	gp_pixmap *buf = gp_pixmap_alloc(1000, 1000, GP_PIXEL_G8);
	gp_pixmap *res = gp_pixmap_alloc(1000, 1000, GP_PIXEL_G1);
	unsigned int x, y;

	if (!buf || !res)
		return TST_UNTESTED;

	for (x = 0; x < 1000; x++) {
		for (y = 0; y < 1000; y++) {
			uint8_t *pix = (void*)GP_PIXEL_ADDR_G8(buf, x, y);

			pix[1] = 0xaa;
		}
	}

	gp_filter_floyd_steinberg(buf, res, NULL);

	gp_pixmap_free(buf);
	gp_pixmap_free(res);

	return TST_PASSED;
}

const struct tst_suite tst_suite = {
	.suite_name = "Dithering benchmark",
	.tests = {
		{.name = "Floyd Steinberg RGB88 -> G1",
		 .tst_fn = dither_bench_RGB888_to_G1,
		 .bench_iter = 100},

		{.name = "Floyd Steinberg G8 -> G1",
		 .tst_fn = dither_bench_G8_to_G1,
		 .bench_iter = 100},

		{},
	}
};
