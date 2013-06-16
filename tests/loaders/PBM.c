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

struct testcase {
	GP_Size w;
	GP_Size h;
	GP_Pixel pix;
	char *path;
};

static int test_load_PBM(struct testcase *test)
{
	GP_Context *img;
	unsigned int x, y, err = 0;

	errno = 0;

	img = GP_LoadPBM(test->path, NULL);

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
		tst_msg("Invalid image size have %ux%u expected %ux%u",
		        img->w, img->h, test->w, test->h);
		GP_ContextFree(img);
		return TST_FAILED;
	}

	for (x = 0; x < img->w; x++) {
		for (y = 0; y < img->h; y++) {

			GP_Pixel pix = GP_GetPixel(img, x, y);
			
			if (pix != test->pix) {
				if (err < 5)
					tst_msg("%08x instead of %08x (%ux%u)",
					        pix, test->pix, x, y);
				err++;
			}
		}
	}

	GP_ContextFree(img);

	if (err)
		return TST_FAILED;

	return TST_SUCCESS;
}

static int test_load_fail_PBM(const char *path)
{
	GP_Context *img;

	errno = 0;

	img = GP_LoadPBM(path, NULL);

	if (img != NULL) {
		tst_msg("Succeeded unexpectedly");
		GP_ContextFree(img);
		return TST_FAILED;
	}

	switch (errno) {
	case ENOSYS:
		tst_msg("Not Implemented");
		return TST_SKIPPED;
	default:
		tst_msg("Got %s", strerror(errno));
		return TST_SUCCESS;
	}
}

struct testcase black_1x1_1 = {
	.w = 1,
	.h = 1,
	.pix = 0,
	.path = "black_1x1_1.pbm",
};

struct testcase black_1x1_2 = {
	.w = 1,
	.h = 1,
	.pix = 0,
	.path = "black_1x1_2.pbm",
};

struct testcase black_1x1_3 = {
	.w = 1,
	.h = 1,
	.pix = 0,
	.path = "black_1x1_3.pbm",
};

struct testcase black_1x1_4 = {
	.w = 1,
	.h = 1,
	.pix = 0,
	.path = "black_1x1_4.pbm",
};

struct testcase white_1x1 = {
	.w = 1,
	.h = 1,
	.pix = 1,
	.path = "white_1x1.pbm",
};

const struct tst_suite tst_suite = {
	.suite_name = "PBM",
	.tests = {
		{.name = "PBM Load 1x1 (black)",
		 .tst_fn = test_load_PBM,
		//TODO: Add copy to to res path
		 .res_path = "data/pbm/valid/black_1x1_1.pbm",
		 .data = &black_1x1_1,
		 .flags = TST_TMPDIR | TST_CHECK_MALLOC},

		{.name = "PBM Load 1x1 (white)",
		 .tst_fn = test_load_PBM,
		 .res_path = "data/pbm/valid/white_1x1.pbm",
		 .data = &white_1x1,
		 .flags = TST_TMPDIR | TST_CHECK_MALLOC},

		{.name = "PBM Load 1x1 +comments",
		 .tst_fn = test_load_PBM,
		 .res_path = "data/pbm/valid/black_1x1_2.pbm",
		 .data = &black_1x1_2,
		 .flags = TST_TMPDIR | TST_CHECK_MALLOC},

		{.name = "PBM Load 1x1 +comments +whitespaces",
		 .tst_fn = test_load_PBM,
		 .res_path = "data/pbm/valid/black_1x1_3.pbm",
		 .data = &black_1x1_3,
		 .flags = TST_TMPDIR | TST_CHECK_MALLOC},

		{.name = "PBM Load 1x1 (invalid loadable)",
		 .tst_fn = test_load_PBM,
		 .res_path = "data/pbm/valid/black_1x1_4.pbm",
		 .data = &black_1x1_4,
		 .flags = TST_TMPDIR | TST_CHECK_MALLOC},
		
		{.name = "PBM Load corrupt",
		 .tst_fn = test_load_fail_PBM,
		 .res_path = "data/pbm/corrupt/short.pbm",
		 .data = "short.pbm",
		 .flags = TST_TMPDIR | TST_CHECK_MALLOC},
		
		{.name = "PBM Load empty",
		 .tst_fn = test_load_fail_PBM,
		 .res_path = "data/pbm/corrupt/empty.pbm",
		 .data = "empty.pbm",
		 .flags = TST_TMPDIR | TST_CHECK_MALLOC},

		{.name = NULL},
	}
};
