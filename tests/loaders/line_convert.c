// SPDX-License-Identifier: GPL-2.1-or-later
/*
 * Copyright (C) 2009-2014 Cyril Hrubis <metan@ucw.cz>
 */

#include <string.h>
#include <errno.h>
#include <sys/stat.h>

#include <core/gp_pixmap.h>
#include <core/gp_convert.h>
#include <core/gp_get_put_pixel.h>
#include <loaders/gp_loaders.h>

#include "tst_test.h"

struct test_data {
	gp_pixel r, g, b;
};

struct test_data colors[] = {
	{0x00, 0x00, 0x00},
	{0xff, 0x00, 0x00},
	{0x00, 0xff, 0x00},
	{0x00, 0x00, 0xff}
};

#define MUL 40
#define THRESHOLD 5

#define SYM_DIFF(a, b) ((a) > (b) ? ((a) - (b)) : ((b) - (a)))

static int xRGB_to_BGR_jpg(void)
{
	gp_pixmap *src, *res;
	unsigned int x;
	int ret = TST_SUCCESS;

	src = gp_pixmap_alloc(GP_ARRAY_SIZE(colors)*MUL, 1, GP_PIXEL_xRGB8888);

	if (!src) {
		tst_msg("Failed to allocate pixmap");
		return TST_UNTESTED;
	}

	for (x = 0; x < src->w; x++) {
		gp_pixel p = gp_rgb_to_pixel(colors[x/MUL].r, colors[x/MUL].g,
		                             colors[x/MUL].b, GP_PIXEL_xRGB8888);
		gp_putpixel(src, x, 0, p);
	}

	if (gp_save_jpg(src, "test.jpg", NULL)) {
		tst_msg("Saving failed");
		return TST_FAILED;
	}

	gp_pixmap_free(src);

	res = gp_load_jpg("test.jpg", NULL);
	if (!res) {
		tst_msg("Failed to load");
		return TST_FAILED;
	}

	if (res->pixel_type != GP_PIXEL_BGR888) {
		tst_msg("Loaded jpg with wrong pixel type %s",
			gp_pixel_type_name(res->pixel_type));
		return TST_FAILED;
	}

	for (x = MUL/2; x < res->w; x+=MUL) {
		int fail = 0;

		gp_pixel p = gp_getpixel(res, x, 0);

		gp_pixel out_r = GP_PIXEL_GET_R_BGR888(p);
		gp_pixel out_g = GP_PIXEL_GET_G_BGR888(p);
		gp_pixel out_b = GP_PIXEL_GET_B_BGR888(p);

		gp_pixel in_r = colors[x/MUL].r;
		gp_pixel in_g = colors[x/MUL].g;
		gp_pixel in_b = colors[x/MUL].b;

		if (SYM_DIFF(in_r, out_r) > THRESHOLD) {
			tst_msg("Pixel red channel is wrong %02x expected %02x", out_r, in_r);
			fail = 1;
		}

		if (SYM_DIFF(in_g, out_g) > THRESHOLD) {
			tst_msg("Pixel green channel is wrong %02x expected %02x", out_g, in_g);
			fail = 1;
		}

		if (SYM_DIFF(in_b, out_b) > THRESHOLD) {
			tst_msg("Pixel blue channel is wrong %02x expected %02x", out_b, in_b);
			fail = 1;
		}

		if (fail) {
			tst_msg("Wrong pixel at %i %06x expected %02x%02x%02x", x, p, in_b, in_g, in_r);
			ret = TST_FAILED;
		}
	}

	gp_pixmap_free(res);

	return ret;
}

const struct tst_suite tst_suite = {
	.suite_name = "Line convert",
	.tests = {
		{.name = "JPEG Line convert xRGB to BGR",
		 .tst_fn = xRGB_to_BGR_jpg,
		 .flags = TST_TMPDIR},

		{.name = NULL},
	}
};
