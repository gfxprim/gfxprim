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
