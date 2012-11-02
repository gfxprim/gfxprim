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
 * Copyright (C) 2009-2012 Cyril Hrubis <metan@ucw.cz>                       *
 *                                                                           *
 *****************************************************************************/

#include <string.h>
#include <errno.h>
#include <sys/stat.h>

#include <core/GP_Context.h>
#include <gfx/GP_Line.h>

#include "tst_test.h"

#include "common.h"

static const char line_5_5_5_5_11x11[] = {
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};

static const char line_1_5_9_5_11x11[] = {
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};

static const char line_0_0_10_10_11x11[] = {
	1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
};

static const char line_0_0_11_5_11x11[] = {
	1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};

static int test_line(const char *pattern, GP_Size w, GP_Size h,
                     GP_Coord x0, GP_Coord y0, GP_Coord x1, GP_Coord y1)
{
	GP_Context *c;
	int err;

	c = GP_ContextAlloc(w, h, GP_PIXEL_G8);

	if (c == NULL) {
		tst_err("Failed to allocate context");
		return TST_UNTESTED;
	}

	/* zero the pixels buffer */
	memset(c->pixels, 0, c->w * c->h);

	GP_Line(c, x0, y0, x1, y1, 1);

	err = compare_buffers(pattern, c);

	if (err) {
		tst_msg("Patterns are different");
		return TST_FAILED;
	}

	return TST_SUCCESS;
}

static int test_line_5_5_5_5(void)
{
	return test_line(line_5_5_5_5_11x11, 11, 11, 5, 5, 5, 5);
}

static int test_line_1_5_9_5(void)
{
	return test_line(line_1_5_9_5_11x11, 11, 11, 1, 5, 9, 5);
}

static int test_line_0_0_10_10(void)
{
	return test_line(line_0_0_10_10_11x11, 11, 11, 0, 0, 10, 10);
}

static int test_line__1__1_11_11(void)
{
	return test_line(line_0_0_10_10_11x11, 11, 11, -1, -1, 11, 11);
}

static int test_line_0_0_11_5(void)
{
	return test_line(line_0_0_11_5_11x11, 11, 11, 0, 0, 11, 5);
}

const struct tst_suite tst_suite = {
	.suite_name = "GFX Line Testsuite",
	.tests = {
		{.name = "Line 5x5 - 5x5", .tst_fn = test_line_5_5_5_5},
		{.name = "Line 1x5 - 9x5", .tst_fn = test_line_1_5_9_5},
		{.name = "Line 0x0 - 10x10", .tst_fn = test_line_0_0_10_10},
		{.name = "Line 0x0 - 11x5", .tst_fn = test_line_0_0_11_5},
		{.name = "Line -1x-1 - 11x11", .tst_fn = test_line__1__1_11_11},
		{.name = NULL}
	}
};
