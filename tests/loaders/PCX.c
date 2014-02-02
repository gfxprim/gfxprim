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
 * Copyright (C) 2009-2014 Cyril Hrubis <metan@ucw.cz>                       *
 *                                                                           *
 *****************************************************************************/

#include <string.h>
#include <errno.h>
#include <sys/stat.h>

#include <core/GP_Context.h>
#include <core/GP_GetPutPixel.h>
#include <loaders/GP_PCX.h>

#include "tst_test.h"

struct testcase {
	const char *path;
	GP_Size w, h;
	GP_PixelType pixel_type;
	GP_Pixel pixel;
};

static int test_load_PCX(struct testcase *test)
{
	GP_Context *img;

	errno = 0;

	img = GP_LoadPCX(test->path, NULL);

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

	if (img->w != test->w || img->h != test->h) {
		tst_msg("Wrong size have %ux%u expected %ux%u",
		        img->w, img->h, test->w, test->h);
		GP_ContextFree(img);
		return TST_FAILED;
	}

	if (img->pixel_type != test->pixel_type) {
		tst_msg("Wrong pixel type have %s expected %s",
		        GP_PixelTypeName(img->pixel_type),
		        GP_PixelTypeName(test->pixel_type));
		GP_ContextFree(img);
		return TST_FAILED;
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

static struct testcase v3_0_1bpp_10x10_white = {
	.path = "ver3_0_palette_1bpp_10x10_white.pcx",
	.w = 10,
	.h = 10,
	.pixel_type = GP_PIXEL_G1,
	.pixel = 0x000001,
};

static struct testcase v3_0_2bpp_10x10_white = {
	.path = "ver3_0_palette_2bpp_10x10_white.pcx",
	.w = 10,
	.h = 10,
	.pixel_type = GP_PIXEL_RGB888,
	.pixel = 0xffffff,
};

static struct testcase v3_0_4bpp_10x10_white = {
	.path = "ver3_0_palette_4bpp_10x10_white.pcx",
	.w = 10,
	.h = 10,
	.pixel_type = GP_PIXEL_RGB888,
	.pixel = 0xffffff,
};

static struct testcase v2_8_4bpp_10x10_white = {
	.path = "ver2_8_palette_4bpp_10x10_white.pcx",
	.w = 10,
	.h = 10,
	.pixel_type = GP_PIXEL_RGB888,
	.pixel = 0xffffff,
};

static struct testcase v3_0_8bpp_10x10_white = {
	.path = "ver3_0_palette_8bpp_10x10_white.pcx",
	.w = 10,
	.h = 10,
	.pixel_type = GP_PIXEL_RGB888,
	.pixel = 0xffffff,
};

static struct testcase v3_0_24bpp_10x10_white = {
	.path = "ver3_0_palette_24bpp_10x10_white.pcx",
	.w = 10,
	.h = 10,
	.pixel_type = GP_PIXEL_RGB888,
	.pixel = 0xffffff,
};

const struct tst_suite tst_suite = {
	.suite_name = "PCX",
	.tests = {
		{.name = "PCX Load ver3.0 1bpp 10x10 white",
		 .tst_fn = test_load_PCX,
		 .res_path = "data/pcx/valid/ver3_0_palette_1bpp_10x10_white.pcx",
		 .data = &v3_0_1bpp_10x10_white,
		 .flags = TST_TMPDIR | TST_CHECK_MALLOC},

		{.name = "PCX Load ver3.0 2bpp 10x10 white",
		 .tst_fn = test_load_PCX,
		 .res_path = "data/pcx/valid/ver3_0_palette_2bpp_10x10_white.pcx",
		 .data = &v3_0_2bpp_10x10_white,
		 .flags = TST_TMPDIR | TST_CHECK_MALLOC},

		{.name = "PCX Load ver3.0 4bpp 10x10 white",
		 .tst_fn = test_load_PCX,
		 .res_path = "data/pcx/valid/ver3_0_palette_4bpp_10x10_white.pcx",
		 .data = &v3_0_4bpp_10x10_white,
		 .flags = TST_TMPDIR | TST_CHECK_MALLOC},

		{.name = "PCX Load ver2.8 4bpp 10x10 white",
		 .tst_fn = test_load_PCX,
		 .res_path = "data/pcx/valid/ver2_8_palette_4bpp_10x10_white.pcx",
		 .data = &v2_8_4bpp_10x10_white,
		 .flags = TST_TMPDIR | TST_CHECK_MALLOC},

		{.name = "PCX Load ver3.0 8bpp 10x10 white",
		 .tst_fn = test_load_PCX,
		 .res_path = "data/pcx/valid/ver3_0_palette_8bpp_10x10_white.pcx",
		 .data = &v3_0_8bpp_10x10_white,
		 .flags = TST_TMPDIR | TST_CHECK_MALLOC},

		{.name = "PCX Load ver3.0 24bpp 10x10 white",
		 .tst_fn = test_load_PCX,
		 .res_path = "data/pcx/valid/ver3_0_palette_24bpp_10x10_white.pcx",
		 .data = &v3_0_24bpp_10x10_white,
		 .flags = TST_TMPDIR | TST_CHECK_MALLOC},
		{.name = NULL},
	}
};
