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

#include <core/gp_pixmap.h>
#include <core/gp_get_put_pixel.h>
#include <loaders/gp_loaders.h>

#include "tst_test.h"

#define LOAD gp_load_pgm
#define SAVE gp_save_pgm
#define READ gp_read_pgm
#include "Loader.h"
#include "PGM.h"

struct testcase black_1x1_1bpp = {
	.w = 1,
	.h = 1,
	.pix = 0,
	.path = "black_1x1_1bpp.pgm",
};

struct testcase black_1x1_2bpp = {
	.w = 1,
	.h = 1,
	.pix = 0,
	.path = "black_1x1_2bpp.pgm",
};

struct testcase black_1x1_4bpp = {
	.w = 1,
	.h = 1,
	.pix = 0,
	.path = "black_1x1_4bpp.pgm",
};

struct testcase black_1x1_8bpp = {
	.w = 1,
	.h = 1,
	.pix = 0,
	.path = "black_1x1_8bpp.pgm",
};

struct testcase_save_load save_load_1bpp = {
	.w = 100,
	.h = 100,
	.pixel_type = GP_PIXEL_G1,
};

struct testcase_save_load save_load_2bpp = {
	.w = 100,
	.h = 100,
	.pixel_type = GP_PIXEL_G2,
};

struct testcase_save_load save_load_4bpp = {
	.w = 100,
	.h = 100,
	.pixel_type = GP_PIXEL_G4,
};

struct testcase_save_load save_load_8bpp = {
	.w = 100,
	.h = 100,
	.pixel_type = GP_PIXEL_G8,
};

const struct tst_suite tst_suite = {
	.suite_name = "PGM",
	.tests = {
		{.name = "PGM Load 1x1 1bpp (black)",
		 .tst_fn = test_load,
		 .res_path = "data/pgm/valid/black_1x1_1bpp.pgm",
		 .data = &black_1x1_1bpp,
		 .flags = TST_TMPDIR | TST_CHECK_MALLOC},

		{.name = "PGM Load 1x1 2bpp (black)",
		 .tst_fn = test_load,
		 .res_path = "data/pgm/valid/black_1x1_2bpp.pgm",
		 .data = &black_1x1_2bpp,
		 .flags = TST_TMPDIR | TST_CHECK_MALLOC},

		 {.name = "PGM Load 1x1 4bpp (black)",
		  .tst_fn = test_load,
		  .res_path = "data/pgm/valid/black_1x1_4bpp.pgm",
		  .data = &black_1x1_4bpp,
		  .flags = TST_TMPDIR | TST_CHECK_MALLOC},

		 {.name = "PGM Load 1x1 8bpp (black)",
		  .tst_fn = test_load,
		  .res_path = "data/pgm/valid/black_1x1_8bpp.pgm",
		  .data = &black_1x1_8bpp,
		  .flags = TST_TMPDIR | TST_CHECK_MALLOC},

		 {.name = "PGM Read 1x1 1bpp (black)",
		  .tst_fn = test_read,
		  .data = &PGM_ascii_1x1_1bpp_black,
		  .flags = TST_CHECK_MALLOC},

		 {.name = "PGM Read 1x1 2bpp (black)",
		  .tst_fn = test_read,
		  .data = &PGM_ascii_1x1_2bpp_black,
		  .flags = TST_CHECK_MALLOC},

		 {.name = "PGM Read 1x1 4bpp (black)",
		  .tst_fn = test_read,
		  .data = &PGM_ascii_1x1_4bpp_black,
		  .flags = TST_CHECK_MALLOC},

		 {.name = "PGM Read 1x1 8bpp (black)",
		  .tst_fn = test_read,
		  .data = &PGM_ascii_1x1_8bpp_black,
		  .flags = TST_CHECK_MALLOC},

		 {.name = "PGM Read 1x1 1bpp (white)",
		  .tst_fn = test_read,
		  .data = &PGM_ascii_1x1_1bpp_white,
		  .flags = TST_CHECK_MALLOC},

		 {.name = "PGM Read 1x1 2bpp (white)",
		  .tst_fn = test_read,
		  .data = &PGM_ascii_1x1_2bpp_white,
		  .flags = TST_CHECK_MALLOC},

		 {.name = "PGM Read 1x1 4bpp (white)",
		  .tst_fn = test_read,
		  .data = &PGM_ascii_1x1_4bpp_white,
		  .flags = TST_CHECK_MALLOC},

		 {.name = "PGM Read 1x1 8bpp (white)",
		  .tst_fn = test_read,
		  .data = &PGM_ascii_1x1_8bpp_white,
		  .flags = TST_CHECK_MALLOC},

		 {.name = "PGM Read 1x1 1bpp (white) Raw",
		  .tst_fn = test_read,
		  .data = &PGM_bin_1x1_1bpp_white,
		  .flags = TST_CHECK_MALLOC},

		 {.name = "PGM Read 1x1 2bpp (white) Raw",
		  .tst_fn = test_read,
		  .data = &PGM_bin_1x1_2bpp_white,
		  .flags = TST_CHECK_MALLOC},

		 {.name = "PGM Read 1x1 4bpp (white) Raw",
		  .tst_fn = test_read,
		  .data = &PGM_bin_1x1_4bpp_white,
		  .flags = TST_CHECK_MALLOC},

		 {.name = "PGM Read 1x1 8bpp (white) Raw",
		  .tst_fn = test_read,
		  .data = &PGM_bin_1x1_8bpp_white,
		  .flags = TST_CHECK_MALLOC},

		 {.name = "PGM Save Load 1bpp",
		  .tst_fn = test_save_load,
		  .data = &save_load_1bpp,
		  .flags = TST_TMPDIR | TST_CHECK_MALLOC},

		 {.name = "PGM Save Load 2bpp",
		  .tst_fn = test_save_load,
		  .data = &save_load_2bpp,
		  .flags = TST_TMPDIR | TST_CHECK_MALLOC},

		 {.name = "PGM Save Load 4bpp",
		  .tst_fn = test_save_load,
		  .data = &save_load_4bpp,
		  .flags = TST_TMPDIR | TST_CHECK_MALLOC},

		 {.name = "PGM Save Load 8bpp",
		  .tst_fn = test_save_load,
		  .data = &save_load_8bpp,
		  .flags = TST_TMPDIR | TST_CHECK_MALLOC},

		{.name = "PGM Load wrong header",
		 .tst_fn = test_load_fail,
		 .res_path = "data/pgm/corrupt/wrong_header.pgm",
		 .data = "wrong_header.pgm",
		 .flags = TST_TMPDIR | TST_CHECK_MALLOC},

		{.name = "PGM Load incomplete",
		 .tst_fn = test_load_fail,
		 .res_path = "data/pgm/corrupt/incomplete.pgm",
		 .data = "incomplete.pgm",
		 .flags = TST_TMPDIR | TST_CHECK_MALLOC},

		{.name = NULL},
	}
};
