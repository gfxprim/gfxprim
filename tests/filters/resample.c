// SPDX-License-Identifier: GPL-2.1-or-later
/*
 * Copyright (C) 2023-2026 Cyril Hrubis <metan@ucw.cz>
 */

#include <string.h>
#include <core/gp_core.h>
#include <filters/gp_filters.h>
#include "tst_test.h"

static int resample_1x1(enum gp_interpolation_type interp_type)
{
	gp_pixmap *src = gp_pixmap_alloc(1, 1, GP_PIXEL_RGB888);
	gp_pixmap *dst = gp_pixmap_alloc(10, 10, GP_PIXEL_RGB888);
	unsigned int x, y;

	if (!src || !dst)
		return TST_UNTESTED;

	gp_putpixel(src, 0, 0, 0xff0000);

	gp_filter_resize(src, dst, interp_type, NULL);

	for (y = 0; y < dst->h; y++) {
		for (x = 0; x < dst->w; x++) {
			gp_pixel pix = gp_getpixel(dst, x, y);

			if (pix != 0xff0000) {
				tst_msg("Invalid pixel value at %i %i 0x%6x", x, y, pix);
				return TST_FAILED;
			}
		}
	}

	gp_pixmap_free(src);
	gp_pixmap_free(dst);

	return TST_PASSED;
}

static int resample_10x10(enum gp_interpolation_type interp_type)
{
	gp_pixmap *src = gp_pixmap_alloc(10, 10, GP_PIXEL_RGB888);
	gp_pixmap *dst = gp_pixmap_alloc(1, 1, GP_PIXEL_RGB888);

	if (!src || !dst)
		return TST_UNTESTED;

	gp_fill(src, 0xff0000);

	gp_filter_resize(src, dst, interp_type, NULL);

	gp_pixel pix = gp_getpixel(dst, 0, 0);
	if (pix != 0xff0000) {
		tst_msg("Invalid pixel value 0x%6x", pix);
		return TST_FAILED;
	}

	gp_pixmap_free(src);
	gp_pixmap_free(dst);

	return TST_PASSED;
}

static int resample_1x1_nn(void)
{
	return resample_1x1(GP_INTERP_NN);
}

static int resample_1x1_lin_int(void)
{
	return resample_1x1(GP_INTERP_LINEAR_INT);
}

static int resample_1x1_lin_lf_int(void)
{
	return resample_1x1(GP_INTERP_LINEAR_LF_INT);
}

static int resample_1x1_cubic(void)
{
	return resample_1x1(GP_INTERP_CUBIC);
}

static int resample_1x1_cubic_int(void)
{
	return resample_1x1(GP_INTERP_CUBIC_INT);
}

static int resample_10x10_nn(void)
{
	return resample_10x10(GP_INTERP_NN);
}

static int resample_10x10_lin_int(void)
{
	return resample_10x10(GP_INTERP_LINEAR_INT);
}

static int resample_10x10_lin_lf_int(void)
{
	return resample_10x10(GP_INTERP_LINEAR_LF_INT);
}

static int resample_10x10_cubic(void)
{
	return resample_10x10(GP_INTERP_CUBIC);
}

static int resample_10x10_cubic_int(void)
{
	return resample_10x10(GP_INTERP_CUBIC_INT);
}

const struct tst_suite tst_suite = {
	.suite_name = "Resampling testsuite",
	.tests = {
		{.name = "Resize 1x1 -> 10x10 Nearest neighbour",
		 .tst_fn = resample_1x1_nn},

		{.name = "Resize 1x1 -> 10x10 Linear int",
		 .tst_fn = resample_1x1_lin_int},

		{.name = "Resize 1x1 -> 10x10 Linear LF int",
		 .tst_fn = resample_1x1_lin_lf_int},

		{.name = "Resize 1x1 -> 10x10 Cubic",
		 .tst_fn = resample_1x1_cubic},

		{.name = "Resize 1x1 -> 10x10 Cubic int",
		 .tst_fn = resample_1x1_cubic_int},

		{.name = "Resize 10x10 -> 1x1 Nearest neighbour",
		 .tst_fn = resample_10x10_nn},

		{.name = "Resize 10x10 -> 1x1 Linear int",
		 .tst_fn = resample_10x10_lin_int},

		{.name = "Resize 10x10 -> 1x1 Linear LF int",
		 .tst_fn = resample_10x10_lin_lf_int},

		{.name = "Resize 10x10 -> 1x1 Cubic",
		 .tst_fn = resample_10x10_cubic},

		{.name = "Resize 10x10 -> 1x1 Cubic int",
		 .tst_fn = resample_10x10_cubic_int},
		{},
	}
};
