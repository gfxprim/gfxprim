// SPDX-License-Identifier: GPL-2.1-or-later
/*
 * Copyright (C) 2009-2014 Cyril Hrubis <metan@ucw.cz>
 */

#include "loader.h"

static struct testcase PBM_ascii_1x1_black = {
	.w = 1,
	.h = 1,
	.pix = 0,
	.path = "P1\n"
	        "# 1x1 black PBM ascii image\n"
	        "1 1\n"
		"1"
};

static struct testcase PBM_ascii_1x1_white = {
	.w = 1,
	.h = 1,
	.pix = 1,
	.path = "P1\n"
	        "# 1x1 black PBM ascii image\n"
	        "1 1\n"
		"0"
};

static struct testcase PBM_bin_1x1_black = {
	.w = 1,
	.h = 1,
	.pix = 0,
	.path = "P4\n"
	        "# 1x1 black PBM binary image\n"
	        "1 1\n"
		"\x80"
};

static struct testcase PBM_bin_1x1_white = {
	.w = 1,
	.h = 1,
	.pix = 1,
	.path = "P4\n"
	        "# 1x1 black PBM binary image\n"
	        "1 1\n"
		"\x01"
};
