/*****************************************************************************
 * This file is part of gfxprim library.                                     *
 *                                                                           *
 * Gfxprim is free software; you can redistribute it and/or                  *
 * modify it under the terms of the GNU Lesser General Public                *
 * License as published by the Free Software Foundation; either              *
 * version 2.1 of the License, or (at your option) any later version.        *
 *                                                                           *
 * Gfxprim is distributed in the hope that it will be useful,                *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU         *
 * Lesser General Public License for more details.                           *
 *                                                                           *
 * You should have received a copy of the GNU Lesser General Public          *
 * License along with gfxprim; if not, write to the Free Software            *
 * Foundation, Inc., 51 Franklin Street, Fifth Floor,                        *
 * Boston, MA  02110-1301  USA                                               *
 *                                                                           *
 * Copyright (C) 2009-2013 Cyril Hrubis <metan@ucw.cz>                       *
 *                                                                           *
 *****************************************************************************/

#include <string.h>
#include <errno.h>
#include <sys/stat.h>

#include <core/GP_Context.h>
#include <core/GP_GetPutPixel.h>
#include <loaders/GP_Loaders.h>

#include "tst_test.h"

static int test_load_PNG(const char *path)
{
	GP_Context *img;

	errno = 0;

	img = GP_LoadPNG(path, NULL);

	if (img == NULL) {
		switch (errno) {
		case ENOSYS:
			tst_msg("Not Implemented");
			return TST_SKIPPED;
		default:
			tst_msg("Got %s", strerror(errno));
			return TST_FAILED;
		}
	}

	/*
	 * TODO: check correct data.
	 */

	GP_ContextFree(img);

	return TST_SUCCESS;
}

struct check_color_test {
	const char *path;
	GP_Pixel pixel;
};

static int test_load_PNG_check_color(struct check_color_test *test)
{
	GP_Context *img;

	errno = 0;

	img = GP_LoadPNG(test->path, NULL);

	if (img == NULL) {
		switch (errno) {
		case ENOSYS:
			tst_msg("Not Implemented");
			return TST_SKIPPED;
		default:
			tst_msg("Got %s", strerror(errno));
			return TST_FAILED;
		}
	}

	unsigned int x, y, fail = 0;

	for (x = 0; x < img->w; x++) {
		for (y = 0; y < img->w; y++) {
			GP_Pixel p = GP_GetPixel(img, x, y);

			if (p != test->pixel) {
				if (!fail)
					tst_msg("First failed at %u,%u %x %x",
					        x, y, p, test->pixel);
				fail = 1;
			}
		}
	}

	if (!fail)
		tst_msg("Context pixels are correct");

	GP_ContextFree(img);

	if (fail)
		return TST_FAILED;

	return TST_SUCCESS;
}

static struct check_color_test white_adam7 = {
	.path = "100x100-white-adam7.png",
	.pixel = 0xffffff,
};

static struct check_color_test black_grayscale = {
	.path = "100x100-black-grayscale.png",
	.pixel = 0x000000,
};

static struct check_color_test red = {
	.path = "100x100-red.png",
	.pixel = 0xff0000,
};

static int test_save_PNG(GP_PixelType pixel_type)
{
	GP_Context *ctx;
	int ret;

	ctx = GP_ContextAlloc(100, 100, pixel_type);

	if (ctx == NULL) {
		tst_msg("Failed to allocate context");
		return TST_UNTESTED;
	}

	errno = 0;

	ret = GP_SavePNG(ctx, "/dev/null", NULL);

	if (ret == 0) {
		tst_msg("Saved successfully");
		GP_ContextFree(ctx);
		return TST_SUCCESS;
	}

	switch (errno) {
	case ENOSYS:
		tst_msg("Not Implemented");
		GP_ContextFree(ctx);
		return TST_SKIPPED;
	default:
		tst_msg("Failed and errno is not ENOSYS (%i)", errno);
		GP_ContextFree(ctx);
		return TST_FAILED;
	}
}

const struct tst_suite tst_suite = {
	.suite_name = "PNG",
	.tests = {
		/* PNG loader tests */
		{.name = "PNG Load 100x100 RGB",
		 .tst_fn = test_load_PNG_check_color,
		 .res_path = "data/png/valid/100x100-red.png",
		 .data = &red,
		 .flags = TST_TMPDIR | TST_CHECK_MALLOC},

		{.name = "PNG Load 100x100 RGB 50\% alpha",
		 .tst_fn = test_load_PNG,
		 .res_path = "data/png/valid/100x100-red-alpha.png",
		 .data = "100x100-red-alpha.png",
		 .flags = TST_TMPDIR | TST_CHECK_MALLOC},

		{.name = "PNG Load 100x100 8 bit Grayscale",
		 .tst_fn = test_load_PNG_check_color,
		 .res_path = "data/png/valid/100x100-black-grayscale.png",
		 .data = &black_grayscale,
		 .flags = TST_TMPDIR | TST_CHECK_MALLOC},

		{.name = "PNG Load 100x100 8 bit Grayscale + alpha",
		 .tst_fn = test_load_PNG,
		 .res_path = "data/png/valid/100x100-black-grayscale-alpha.png",
		 .data = "100x100-black-grayscale-alpha.png",
		 .flags = TST_TMPDIR | TST_CHECK_MALLOC},

		{.name = "PNG Load 100x100 Palette + alpha",
		 .tst_fn = test_load_PNG,
		 .res_path = "data/png/valid/100x100-palette-alpha.png",
		 .data = "100x100-palette-alpha.png",
		 .flags = TST_TMPDIR | TST_CHECK_MALLOC},

		{.name = "PNG Load 100x100 Palette",
		 .tst_fn = test_load_PNG,
		 .res_path = "data/png/valid/100x100-red-palette.png",
		 .data = "100x100-red-palette.png",
		 .flags = TST_TMPDIR | TST_CHECK_MALLOC},

		{.name = "PNG Load 100x100 RGB Adam7",
		 .tst_fn = test_load_PNG_check_color,
		 .res_path = "data/png/valid/100x100-white-adam7.png",
		 .data = &white_adam7,
		 .flags = TST_TMPDIR | TST_CHECK_MALLOC},

		{.name = "PNG Save 100x100 G1",
		 .tst_fn = test_save_PNG,
		 .data = (void*)GP_PIXEL_G1,
		 .flags = TST_CHECK_MALLOC},

		{.name = "PNG Save 100x100 G2",
		 .tst_fn = test_save_PNG,
		 .data = (void*)GP_PIXEL_G2,
		 .flags = TST_CHECK_MALLOC},

		{.name = "PNG Save 100x100 G4",
		 .tst_fn = test_save_PNG,
		 .data = (void*)GP_PIXEL_G4,
		 .flags = TST_CHECK_MALLOC},

		{.name = "PNG Save 100x100 G8",
		 .tst_fn = test_save_PNG,
		 .data = (void*)GP_PIXEL_G8,
		 .flags = TST_CHECK_MALLOC},

		{.name = "PNG Save 100x100 RGB888",
		 .tst_fn = test_save_PNG,
		 .data = (void*)GP_PIXEL_RGB888,
		 .flags = TST_CHECK_MALLOC},

		{.name = "PNG Save 100x100 RGBA8888",
		 .tst_fn = test_save_PNG,
		 .data = (void*)GP_PIXEL_RGBA8888,
		 .flags = TST_CHECK_MALLOC},

		{.name = "PNG Save 100x100 BGR888",
		 .tst_fn = test_save_PNG,
		 .data = (void*)GP_PIXEL_BGR888,
		 .flags = TST_CHECK_MALLOC},

		{.name = "PNG Save 100x100 G16",
		 .tst_fn = test_save_PNG,
		 .data = (void*)GP_PIXEL_G16,
		 .flags = TST_CHECK_MALLOC},
		{.name = NULL},
	}
};
