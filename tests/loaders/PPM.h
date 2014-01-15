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
