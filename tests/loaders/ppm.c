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

#define LOAD gp_load_ppm
#define SAVE gp_save_ppm
#define READ gp_read_ppm
#include "loader.h"
#include "ppm.h"

static struct testcase PPM_black_1x1 = {
	.w = 1,
	.h = 1,
	.pix = 0,
	.path = "black_1x1.ppm",
};

static struct testcase_save_load PPM_save_load = {
	.w = 100,
	.h = 100,
	.pixel_type = GP_PIXEL_RGB888,
};

const struct tst_suite tst_suite = {
	.suite_name = "PPM",
	.tests = {
		{.name = "PPM Load 1x1 (black)",
		 .tst_fn = test_load,
		 .res_path = "data/ppm/valid/black_1x1.ppm",
		 .data = &PPM_black_1x1,
		 .flags = TST_TMPDIR | TST_CHECK_MALLOC},

		{.name = "PPM Read 1x1 4bpp (black)",
		 .tst_fn = test_read,
		 .data = &PPM_ascii_1x1_4bpp_black,
		 .flags = TST_CHECK_MALLOC},

		{.name = "PPM Read 1x1 8bpp (black)",
		 .tst_fn = test_read,
		 .data = &PPM_ascii_1x1_8bpp_black,
		 .flags = TST_CHECK_MALLOC},

		{.name = "PPM Read 1x1 8bpp (white)",
		 .tst_fn = test_read,
		 .data = &PPM_ascii_1x1_8bpp_white,
		 .flags = TST_CHECK_MALLOC},

		{.name = "PPM Read 1x1 8bpp (white) Raw",
		 .tst_fn = test_read,
		 .data = &PPM_bin_1x1_8bpp_white,
		 .flags = TST_CHECK_MALLOC},

		{.name = "PPM Save Load",
		 .tst_fn = test_save_load,
		 .data = &PPM_save_load,
		 .flags = TST_TMPDIR | TST_CHECK_MALLOC},

		{.name = "PPM Load wrong header",
		 .tst_fn = test_load_fail,
		 .res_path = "data/ppm/corrupt/wrong_header.ppm",
		 .data = "wrong_header.ppm",
		 .flags = TST_TMPDIR | TST_CHECK_MALLOC},

		{.name = "PPM Load incomplete",
		 .tst_fn = test_load_fail,
		 .res_path = "data/ppm/corrupt/incomplete.ppm",
		 .data = "incomplete.ppm",
		 .flags = TST_TMPDIR | TST_CHECK_MALLOC},

		{.name = NULL},
	}
};
