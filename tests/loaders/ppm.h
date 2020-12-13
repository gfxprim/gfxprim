// SPDX-License-Identifier: GPL-2.1-or-later
/*
 * Copyright (C) 2009-2014 Cyril Hrubis <metan@ucw.cz>
 */

#include "loader.h"

static struct testcase PPM_ascii_1x1_4bpp_black = {
	.w = 1,
	.h = 1,
	.pix = 0,
	.path = "P3\n"
	        "# 1x1 black PPM ascii 4bpp image\n"
	        "1 1\n"
		"15\n"
		"0 0 0"
};

static struct testcase PPM_ascii_1x1_8bpp_black = {
	.w = 1,
	.h = 1,
	.pix = 0,
	.path = "P3\n"
	        "# 1x1 black PPM ascii 8bpp image\n"
	        "1 1\n"
		"255\n"
		"0 0 0"
};

static struct testcase PPM_ascii_1x1_8bpp_white = {
	.w = 1,
	.h = 1,
	.pix = 0xffffff,
	.path = "P3\n"
	        "# 1x1 white PPM ascii 8bpp image\n"
	        "1 1\n"
		"255\n"
		"255 255 255"
};

static struct testcase PPM_bin_1x1_8bpp_white = {
	.w = 1,
	.h = 1,
	.pix = 0xffffff,
	.path = "P6\n"
	        "# 1x1 white PPM binary 8bpp image\n"
	        "1 1\n"
		"255\n"
		"\xff\xff\xff"
};
