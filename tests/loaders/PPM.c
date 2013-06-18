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

#define LOAD GP_LoadPPM
#define SAVE GP_SavePPM
#include "Loader.h"

struct testcase black_1x1 = {
	.w = 1,
	.h = 1,
	.pix = 0,
	.path = "black_1x1.ppm",
};

struct testcase_save_load save_load = {
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
		 .data = &black_1x1,
		 .flags = TST_TMPDIR | TST_CHECK_MALLOC},
		
		{.name = "PPM Save Load",
		 .tst_fn = test_save_load,
		 .data = &save_load,
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
