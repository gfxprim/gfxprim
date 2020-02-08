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

#define LOAD gp_load_pbm
#define SAVE gp_save_pbm
#define READ gp_read_pbm
#include "Loader.h"
#include "PBM.h"

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

struct testcase black_1x1_bin = {
	.w = 1,
	.h = 1,
	.pix = 0,
	.path = "black_1x1_bin.pbm",
};

struct testcase black_2x2_bin = {
	.w = 2,
	.h = 2,
	.pix = 0,
	.path = "black_2x2_bin.pbm",
};

struct testcase black_3x9_bin = {
	.w = 3,
	.h = 9,
	.pix = 0,
	.path = "black_3x9_bin.pbm",
};

struct testcase_save_load save_load = {
	.w = 100,
	.h = 100,
	.pixel_type = GP_PIXEL_G1,
};

const struct tst_suite tst_suite = {
	.suite_name = "PBM",
	.tests = {
		{.name = "PBM Load 1x1 (black)",
		 .tst_fn = test_load,
		//TODO: Add copy to to res path
		 .res_path = "data/pbm/valid/black_1x1_1.pbm",
		 .data = &black_1x1_1,
		 .flags = TST_TMPDIR | TST_CHECK_MALLOC},

		{.name = "PBM Load 1x1 (white)",
		 .tst_fn = test_load,
		 .res_path = "data/pbm/valid/white_1x1.pbm",
		 .data = &white_1x1,
		 .flags = TST_TMPDIR | TST_CHECK_MALLOC},

		{.name = "PBM Load 1x1 +comments",
		 .tst_fn = test_load,
		 .res_path = "data/pbm/valid/black_1x1_2.pbm",
		 .data = &black_1x1_2,
		 .flags = TST_TMPDIR | TST_CHECK_MALLOC},

		{.name = "PBM Load 1x1 +comments +whitespaces",
		 .tst_fn = test_load,
		 .res_path = "data/pbm/valid/black_1x1_3.pbm",
		 .data = &black_1x1_3,
		 .flags = TST_TMPDIR | TST_CHECK_MALLOC},

		{.name = "PBM Load 1x1 (invalid loadable)",
		 .tst_fn = test_load,
		 .res_path = "data/pbm/valid/black_1x1_4.pbm",
		 .data = &black_1x1_4,
		 .flags = TST_TMPDIR | TST_CHECK_MALLOC},

		{.name = "PBM Load 1x1 (black) Raw",
		 .tst_fn = test_load,
		 .res_path = "data/pbm/valid/black_1x1_bin.pbm",
		 .data = &black_1x1_bin,
		 .flags = TST_TMPDIR | TST_CHECK_MALLOC},

		{.name = "PBM Load 2x2 (black) Raw",
		 .tst_fn = test_load,
		 .res_path = "data/pbm/valid/black_2x2_bin.pbm",
		 .data = &black_2x2_bin,
		 .flags = TST_TMPDIR | TST_CHECK_MALLOC},

		{.name = "PBM Load 3x9 (black) Raw",
		 .tst_fn = test_load,
		 .res_path = "data/pbm/valid/black_3x9_bin.pbm",
		 .data = &black_3x9_bin,
		 .flags = TST_TMPDIR | TST_CHECK_MALLOC},

		{.name = "PBM Read 1x1 (black)",
		 .tst_fn = test_read,
		 .data = &PBM_ascii_1x1_black,
		 .flags = TST_CHECK_MALLOC},

		{.name = "PBM Read 1x1 (white)",
		 .tst_fn = test_read,
		 .data = &PBM_ascii_1x1_white,
		 .flags = TST_CHECK_MALLOC},

		{.name = "PBM Read 1x1 (black) Raw",
		 .tst_fn = test_read,
		 .data = &PBM_bin_1x1_black,
		 .flags = TST_CHECK_MALLOC},

		{.name = "PBM Read 1x1 (white) Raw",
		 .tst_fn = test_read,
		 .data = &PBM_bin_1x1_white,
		 .flags = TST_CHECK_MALLOC},

		{.name = "PBM Load corrupt",
		 .tst_fn = test_load_fail,
		 .res_path = "data/pbm/corrupt/short.pbm",
		 .data = "short.pbm",
		 .flags = TST_TMPDIR | TST_CHECK_MALLOC},

		{.name = "PBM Load wrong header",
		 .tst_fn = test_load_fail,
		 .res_path = "data/pbm/corrupt/wrong_header.pbm",
		 .data = "wrong_header.pbm",
		 .flags = TST_TMPDIR | TST_CHECK_MALLOC},

		{.name = "PBM Load empty",
		 .tst_fn = test_load_fail,
		 .res_path = "data/pbm/corrupt/empty.pbm",
		 .data = "empty.pbm",
		 .flags = TST_TMPDIR | TST_CHECK_MALLOC},

		{.name = "PBM Save Load",
		 .tst_fn = test_save_load,
		 .data = &save_load,
		 .flags = TST_TMPDIR | TST_CHECK_MALLOC},

		{.name = NULL},
	}
};
