// SPDX-License-Identifier: GPL-2.1-or-later
/*
 * Copyright (C) 2009-2014 Cyril Hrubis <metan@ucw.cz>
 */

#include <string.h>
#include <errno.h>
#include <sys/stat.h>

#include <core/gp_pixmap.h>
#include <core/gp_get_put_pixel.h>
#include <loaders/gp_loaders.h>

#include "tst_test.h"

struct testcase {
	const char *path;
	gp_size w, h;
	gp_pixel_type pixel_type;
};

static int test_load_ico(struct testcase *test)
{
	gp_pixmap *img;

	errno = 0;

	img = gp_load_ico(test->path, NULL);
	if (!img) {
		switch (errno) {
		case ENOSYS:
			tst_msg("Not Implemented");
			return TST_SKIPPED;
		default:
			tst_msg("Got %s", strerror(errno));
			return TST_FAILED;
		}
	}

	if (img->w != test->w || img->h != test->h) {
		tst_msg("Wrong size have %ux%u expected %ux%u",
		        img->w, img->h, test->w, test->h);
		gp_pixmap_free(img);
		return TST_FAILED;
	}

	if (img->pixel_type != test->pixel_type) {
		tst_msg("Wrong pixel type have %s expected %s",
		        gp_pixel_type_name(img->pixel_type),
		        gp_pixel_type_name(test->pixel_type));
		gp_pixmap_free(img);
		return TST_FAILED;
	}

	unsigned int x, y, fail = 0;

	gp_pixel val_mask = gp_pixel_chan_mask(test->pixel_type, "R") |
	                    gp_pixel_chan_mask(test->pixel_type, "G") |
	                    gp_pixel_chan_mask(test->pixel_type, "B");
	gp_pixel alpha_mask = gp_pixel_chan_mask(test->pixel_type, "A");

	for (x = 0; x < img->w; x++) {
		for (y = 0; y < img->w; y++) {
			gp_pixel p = gp_getpixel(img, x, y);
			int do_fail = 0;

			//Test images have black diagonal cross and white background
			if (x == y || x == (img->h - y-1)) {
				if ((p & val_mask) != 0)
					do_fail = 1;
			} else {
				if ((p & val_mask) != val_mask)
					do_fail = 1;
			}

			//And alpha is set to 100% if present
			if ((p & alpha_mask) != alpha_mask)
				do_fail = 1;

			if (do_fail) {
				if (!fail)
					tst_msg("First failed at %u,%u %x",
					        x, y, p);
				fail = 1;
			}
		}
	}

	if (!fail)
		tst_msg("Pixmap pixels are correct");

	gp_pixmap_free(img);

	if (fail)
		return TST_FAILED;

	return TST_PASSED;
}

static struct testcase ico_png_8x8 = {
	.path = "png_8x8.ico",
	.w = 8,
	.h = 8,
	.pixel_type = GP_PIXEL_RGBA8888,
};

static struct testcase ico_1bpp_8x8 = {
	.path = "1bpp_8x8.ico",
	.w = 8,
	.h = 8,
	.pixel_type = GP_PIXEL_RGB888,
};

static struct testcase ico_4bpp_8x8 = {
	.path = "4bpp_8x8.ico",
	.w = 8,
	.h = 8,
	.pixel_type = GP_PIXEL_RGB888,
};

static struct testcase ico_8bpp_8x8 = {
	.path = "8bpp_8x8.ico",
	.w = 8,
	.h = 8,
	.pixel_type = GP_PIXEL_RGB888,
};

static struct testcase ico_24bpp_8x8 = {
	.path = "24bpp_8x8.ico",
	.w = 8,
	.h = 8,
	.pixel_type = GP_PIXEL_RGB888,
};

static struct testcase ico_32bpp_8x8 = {
	.path = "32bpp_8x8.ico",
	.w = 8,
	.h = 8,
	.pixel_type = GP_PIXEL_xRGB8888,
};

const struct tst_suite tst_suite = {
	.suite_name = "ico",
	.tests = {
		{.name = "ico 8x8 1bpp",
		 .tst_fn = test_load_ico,
		 .res_path = "data/ico/valid/1bpp_8x8.ico",
		 .data = &ico_1bpp_8x8,
		 .flags = TST_TMPDIR | TST_CHECK_MALLOC},

		{.name = "ico 8x8 4bpp",
		 .tst_fn = test_load_ico,
		 .res_path = "data/ico/valid/4bpp_8x8.ico",
		 .data = &ico_4bpp_8x8,
		 .flags = TST_TMPDIR | TST_CHECK_MALLOC},

		{.name = "ico 8x8 8bpp",
		 .tst_fn = test_load_ico,
		 .res_path = "data/ico/valid/8bpp_8x8.ico",
		 .data = &ico_8bpp_8x8,
		 .flags = TST_TMPDIR | TST_CHECK_MALLOC},

		{.name = "ico 8x8 24bpp",
		 .tst_fn = test_load_ico,
		 .res_path = "data/ico/valid/24bpp_8x8.ico",
		 .data = &ico_24bpp_8x8,
		 .flags = TST_TMPDIR | TST_CHECK_MALLOC},

		{.name = "ico 8x8 32bpp",
		 .tst_fn = test_load_ico,
		 .res_path = "data/ico/valid/32bpp_8x8.ico",
		 .data = &ico_32bpp_8x8,
		 .flags = TST_TMPDIR | TST_CHECK_MALLOC},

		{.name = "ico with embedded png",
		 .tst_fn = test_load_ico,
		 .res_path = "data/ico/valid/png_8x8.ico",
		 .data = &ico_png_8x8,
		 .flags = TST_TMPDIR | TST_CHECK_MALLOC},

		{.name = NULL},
	}
};
