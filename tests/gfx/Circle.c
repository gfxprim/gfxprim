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

#include <core/GP_Pixmap.h>
#include <gfx/GP_Circle.h>

#include "tst_test.h"

#include "common.h"

struct testcase {
	/* cicle description */
	GP_Coord x;
	GP_Coord y;
	GP_Size r;

	/* expected result */
	GP_Size w, h;
	const char pixmap[];
};

static int test_circle(const struct testcase *t)
{
	GP_Pixmap *c;
	int err;

	c = GP_PixmapAlloc(t->w, t->h, GP_PIXEL_G8);

	if (c == NULL) {
		tst_err("Failed to allocate pixmap");
		return TST_UNTESTED;
	}

	/* zero the pixels buffer */
	memset(c->pixels, 0, c->w * c->h);

	GP_Circle(c, t->x, t->y, t->r, 1);

	err = compare_buffers(t->pixmap, c);

	if (err)
		return TST_FAILED;

	return TST_SUCCESS;
}

struct testcase testcase_circle_r_0 = {
	.x = 2,
	.y = 2,
	.r = 0,

	.w = 5,
	.h = 5,

	.pixmap = {
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
		0, 0, 1, 0, 0,
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
	}
};

struct testcase testcase_circle_r_1 = {
	.x = 2,
	.y = 2,
	.r = 1,

	.w = 5,
	.h = 5,

	.pixmap = {
		0, 0, 0, 0, 0,
		0, 0, 1, 0, 0,
		0, 1, 0, 1, 0,
		0, 0, 1, 0, 0,
		0, 0, 0, 0, 0,
	}
};

struct testcase testcase_circle_r_2 = {
	.x = 3,
	.y = 3,
	.r = 2,

	.w = 7,
	.h = 7,

	.pixmap = {
		0, 0, 0, 0, 0, 0, 0,
		0, 0, 1, 1, 1, 0, 0,
		0, 1, 0, 0, 0, 1, 0,
		0, 1, 0, 0, 0, 1, 0,
		0, 1, 0, 0, 0, 1, 0,
		0, 0, 1, 1, 1, 0, 0,
		0, 0, 0, 0, 0, 0, 0,
	}
};

struct testcase testcase_circle_r_3 = {
	.x = 4,
	.y = 4,
	.r = 3,

	.w = 9,
	.h = 9,

	.pixmap = {
		0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 1, 1, 1, 0, 0, 0,
		0, 0, 1, 0, 0, 0, 1, 0, 0,
		0, 1, 0, 0, 0, 0, 0, 1, 0,
		0, 1, 0, 0, 0, 0, 0, 1, 0,
		0, 1, 0, 0, 0, 0, 0, 1, 0,
		0, 0, 1, 0, 0, 0, 1, 0, 0,
		0, 0, 0, 1, 1, 1, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0,
	}
};

struct testcase testcase_circle_r_4 = {
	.x = 5,
	.y = 5,
	.r = 4,

	.w = 11,
	.h = 11,

	.pixmap = {
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0,
		0, 0, 1, 1, 0, 0, 0, 1, 1, 0, 0,
		0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0,
		0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0,
		0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0,
		0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0,
		0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0,
		0, 0, 1, 1, 0, 0, 0, 1, 1, 0, 0,
		0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	}
};

struct testcase testcase_circle_r_5_clip = {
	.x = 0,
	.y = 5,
	.r = 5,

	.w = 11,
	.h = 11,

	.pixmap = {
		1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0,
		1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0,
	}
};

struct testcase testcase_circle_r_6_clip = {
	.x = 0,
	.y = 0,
	.r = 6,

	.w = 8,
	.h = 8,

	.pixmap = {
		0, 0, 0, 0, 0, 0, 1, 0,
		0, 0, 0, 0, 0, 0, 1, 0,
		0, 0, 0, 0, 0, 0, 1, 0,
		0, 0, 0, 0, 0, 1, 0, 0,
		0, 0, 0, 0, 0, 1, 0, 0,
		0, 0, 0, 1, 1, 0, 0, 0,
		1, 1, 1, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
	}
};

const struct tst_suite tst_suite = {
	.suite_name = "Circle Testsuite",
	.tests = {
		{.name = "Circle r=0",
		 .tst_fn = test_circle,
		 .data = &testcase_circle_r_0},

		{.name = "Circle r=1",
		 .tst_fn = test_circle,
		 .data = &testcase_circle_r_1},

		{.name = "Circle r=2",
		 .tst_fn = test_circle,
		 .data = &testcase_circle_r_2},

		{.name = "Circle r=3",
		 .tst_fn = test_circle,
		 .data = &testcase_circle_r_3},

		{.name = "Circle r=4",
		 .tst_fn = test_circle,
		 .data = &testcase_circle_r_4},

		{.name = "Circle r=5 + clipping",
		 .tst_fn = test_circle,
		 .data = &testcase_circle_r_5_clip},

		{.name = "Circle r=6 + clipping",
		 .tst_fn = test_circle,
		 .data = &testcase_circle_r_6_clip},

		{.name = NULL}
	}
};
