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
 * Copyright (C) 2009-2012 Cyril Hrubis <metan@ucw.cz>                       *
 *                                                                           *
 *****************************************************************************/

#include <string.h>
#include <errno.h>
#include <sys/stat.h>

#include <core/GP_Context.h>
#include <loaders/GP_Loaders.h>

#include "tst_test.h"

static int test_load_PNG(const char *path)
{
	GP_Context *img;

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

const struct tst_suite tst_suite = {
	.suite_name = "PNG",
	.tests = {
		/* PNG loader tests */
		{.name = "PNG 100x100 RGB",
		 .tst_fn = test_load_PNG,
		 .res_path = "data/png/valid/100x100-red.png",
		 .data = "100x100-red.png",
		 .flags = TST_TMPDIR | TST_CHECK_MALLOC},
		
		{.name = "PNG 100x100 RGB 50\% alpha",
		 .tst_fn = test_load_PNG,
		 .res_path = "data/png/valid/100x100-red-alpha.png",
		 .data = "100x100-red-alpha.png",
		 .flags = TST_TMPDIR | TST_CHECK_MALLOC},
		
		{.name = "PNG 100x100 8 bit Grayscale",
		 .tst_fn = test_load_PNG,
		 .res_path = "data/png/valid/100x100-black-grayscale.png",
		 .data = "100x100-black-grayscale.png",
		 .flags = TST_TMPDIR | TST_CHECK_MALLOC},
		
		{.name = "PNG 100x100 8 bit Grayscale + alpha",
		 .tst_fn = test_load_PNG,
		 .res_path = "data/png/valid/100x100-black-grayscale-alpha.png",
		 .data = "100x100-black-grayscale-alpha.png",
		 .flags = TST_TMPDIR | TST_CHECK_MALLOC},
		
		{.name = "PNG 100x100 Palette + alpha",
		 .tst_fn = test_load_PNG,
		 .res_path = "data/png/valid/100x100-palette-alpha.png",
		 .data = "100x100-palette-alpha.png",
		 .flags = TST_TMPDIR | TST_CHECK_MALLOC},
		
		{.name = "PNG 100x100 Palette",
		 .tst_fn = test_load_PNG,
		 .res_path = "data/png/valid/100x100-red-palette.png",
		 .data = "100x100-red-palette.png",
		 .flags = TST_TMPDIR | TST_CHECK_MALLOC},

		{.name = NULL},
	}
};
