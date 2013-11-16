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

struct testcase {
	/* line description */
	GP_Coord x0;
	GP_Coord y0;
	GP_Coord x1;
	GP_Coord y1;

	/* expected result */
	GP_Size w, h;
	const char pixmap[];
};

static int test_line(const struct testcase *t)
{
	GP_Context *c;
	int err;

	c = GP_ContextAlloc(t->w, t->h, GP_PIXEL_G8);

	if (c == NULL) {
		tst_err("Failed to allocate context");
		return TST_UNTESTED;
	}

	/* zero the pixels buffer */
	memset(c->pixels, 0, c->w * c->h);

	GP_Line(c, t->x0, t->y0, t->x1, t->y1, 1);

	err = compare_buffers(t->pixmap, c);

	if (err)
		return TST_FAILED;

	return TST_SUCCESS;
}

static struct testcase testcase_line_1px = {
	.x0 = 1,
	.y0 = 1,
	.x1 = 1,
	.y1 = 1,

	.w = 3,
	.h = 3,

	.pixmap = {
		0, 0, 0,
		0, 1, 0,
		0, 0, 0,
	}
};

static struct testcase testcase_line_horiz = {
	.x0 = 2,
	.y0 = 2,
	.x1 = 8,
	.y1 = 2,

	.w = 11,
	.h = 5,

	.pixmap = {
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	}
};

static struct testcase testcase_line_vert = {
	.x0 = 2,
	.y0 = 2,
	.x1 = 2,
	.y1 = 8,

	.w = 5,
	.h = 11,

	.pixmap = {
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
		0, 0, 1, 0, 0,
		0, 0, 1, 0, 0,
		0, 0, 1, 0, 0,
		0, 0, 1, 0, 0,
		0, 0, 1, 0, 0,
		0, 0, 1, 0, 0,
		0, 0, 1, 0, 0,
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
	}
};

static struct testcase testcase_line_45 = {
	.x0 = 0,
	.y0 = 0,
	.x1 = 10,
	.y1 = 10,

	.w = 11,
	.h = 11,

	.pixmap = {
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
	}
};

static struct testcase testcase_line_15 = {
	.x0 = 0,
	.y0 = 1,
	.x1 = 11,
	.y1 = 6,

	.w = 11,
	.h = 8,

	.pixmap = {
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	}
};

static struct testcase testcase_line_clip = {
	.x0 = -1000,
	.y0 = -1000,
	.x1 = 1000,
	.y1 = 1000,

	.w = 4,
	.h = 4,

	.pixmap = {
		1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 1, 0,
		0, 0, 0, 1,
	}
};

static struct testcase testcase_line_large_xy = {
	.x0 = -1000000000,
	.y0 = -1000000000,
	.x1 = 1000000000,
	.y1 = 1000000000,

	.w = 4,
	.h = 4,

	.pixmap = {
		1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 1, 0,
		0, 0, 0, 1,
	}
};

const struct tst_suite tst_suite = {
	.suite_name = "Line Testsuite",
	.tests = {
		{.name = "Line 1px",
		 .tst_fn = test_line,
		 .data = &testcase_line_1px},

		{.name = "Line Horizontal",
		 .tst_fn = test_line,
		 .data = &testcase_line_horiz},

		{.name = "Line Vertical",
		 .tst_fn = test_line,
		 .data = &testcase_line_vert},

		{.name = "Line 45 degrees",
		 .tst_fn = test_line,
		 .data = &testcase_line_45},

		{.name = "Line 15 degrees",
		 .tst_fn = test_line,
		 .data = &testcase_line_15},

		{.name = "Line clipping",
		 .tst_fn = test_line,
		 .data = &testcase_line_clip},

		{.name = "Line large coordinates",
		 .tst_fn = test_line,
		 .data = &testcase_line_large_xy,
		 .timeout = 1},

		{.name = NULL}
	}
};
