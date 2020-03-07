// SPDX-License-Identifier: GPL-2.1-or-later
/*
 * Copyright (C) 2009-2014 Cyril Hrubis <metan@ucw.cz>
 */

#include "Loader.h"

static struct testcase PGM_ascii_1x1_1bpp_black = {
	.w = 1,
	.h = 1,
	.pix = 0,
	.path = "P2\n"
	        "# 1x1 black PGM 1bpp ascii image\n"
	        "1 1\n"
		"1\n"
		"0"
};

static struct testcase PGM_ascii_1x1_2bpp_black = {
	.w = 1,
	.h = 1,
	.pix = 0,
	.path = "P2\n"
	        "# 1x1 black PGM 2bpp ascii image\n"
	        "1 1\n"
		"3\n"
		"0"
};

static struct testcase PGM_ascii_1x1_4bpp_black = {
	.w = 1,
	.h = 1,
	.pix = 0,
	.path = "P2\n"
	        "# 1x1 black PGM 4bpp ascii image\n"
	        "1 1\n"
		"15\n"
		"0"
};

static struct testcase PGM_ascii_1x1_8bpp_black = {
	.w = 1,
	.h = 1,
	.pix = 0,
	.path = "P2\n"
	        "# 1x1 black PGM 8bpp ascii image\n"
	        "1 1\n"
		"255\n"
		"0"
};

static struct testcase PGM_ascii_1x1_1bpp_white = {
	.w = 1,
	.h = 1,
	.pix = 1,
	.path = "P2\n"
	        "# 1x1 white PGM 1bpp ascii image\n"
	        "1 1\n"
		"1\n"
		"1"
};

static struct testcase PGM_ascii_1x1_2bpp_white = {
	.w = 1,
	.h = 1,
	.pix = 3,
	.path = "P2\n"
	        "# 1x1 white PGM 2bpp ascii image\n"
	        "1 1\n"
		"3\n"
		"3"
};

static struct testcase PGM_ascii_1x1_4bpp_white = {
	.w = 1,
	.h = 1,
	.pix = 15,
	.path = "P2\n"
	        "# 1x1 white PGM 4bpp ascii image\n"
	        "1 1\n"
		"15\n"
		"15"
};

static struct testcase PGM_ascii_1x1_8bpp_white = {
	.w = 1,
	.h = 1,
	.pix = 255,
	.path = "P2\n"
	        "# 1x1 white PGM 8bpp ascii image\n"
	        "1 1\n"
		"255\n"
		"255"
};

static struct testcase PGM_bin_1x1_1bpp_white = {
	.w = 1,
	.h = 1,
	.pix = 1,
	.path = "P5\n"
	        "# 1x1 white PGM 1bpp binary image\n"
	        "1 1\n"
		"1\n"
		"\x01"
};

static struct testcase PGM_bin_1x1_2bpp_white = {
	.w = 1,
	.h = 1,
	.pix = 3,
	.path = "P5\n"
	        "# 1x1 white PGM 2bpp binary image\n"
	        "1 1\n"
		"3\n"
		"\x03"
};

static struct testcase PGM_bin_1x1_4bpp_white = {
	.w = 1,
	.h = 1,
	.pix = 15,
	.path = "P5\n"
	        "# 1x1 white PGM 4bpp binary image\n"
	        "1 1\n"
		"15\n"
		"\x0f"
};

static struct testcase PGM_bin_1x1_8bpp_white = {
	.w = 1,
	.h = 1,
	.pix = 255,
	.path = "P5\n"
	        "# 1x1 white PGM 8bpp binary image\n"
	        "1 1\n"
		"255\n"
		"\xff"
};
