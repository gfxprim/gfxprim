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
