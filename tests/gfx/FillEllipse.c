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

#include <core/GP_Pixmap.h>
#include <gfx/GP_Ellipse.h>

#include "tst_test.h"

#include "common.h"

struct testcase {
	/* cicle description */
	GP_Coord x;
	GP_Coord y;
	GP_Size a;
	GP_Size b;

	/* expected result */
	GP_Size w, h;
	const char pixmap[];
};

static int test_ellipse(const struct testcase *t)
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

	GP_FillEllipse(c, t->x, t->y, t->a, t->b, 1);

	err = compare_buffers(t->pixmap, c);

	if (err)
		return TST_FAILED;

	return TST_SUCCESS;
}

static struct testcase testcase_ellipse_a0_b0 = {
	.x = 2,
	.y = 2,
	.a = 0,
	.b = 0,

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

static struct testcase testcase_ellipse_a1_b0 = {
	.x = 2,
	.y = 2,
	.a = 1,
	.b = 0,

	.w = 5,
	.h = 5,

	.pixmap = {
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
		0, 1, 1, 1, 0,
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
	}
};

static struct testcase testcase_ellipse_a0_b1 = {
	.x = 2,
	.y = 2,
	.a = 0,
	.b = 1,

	.w = 5,
	.h = 5,

	.pixmap = {
		0, 0, 0, 0, 0,
		0, 0, 1, 0, 0,
		0, 0, 1, 0, 0,
		0, 0, 1, 0, 0,
		0, 0, 0, 0, 0,
	}
};

static struct testcase testcase_ellipse_a1_b1 = {
	.x = 2,
	.y = 2,
	.a = 1,
	.b = 1,

	.w = 5,
	.h = 5,

	.pixmap = {
		0, 0, 0, 0, 0,
		0, 0, 1, 0, 0,
		0, 1, 1, 1, 0,
		0, 0, 1, 0, 0,
		0, 0, 0, 0, 0,
	}
};

static struct testcase testcase_ellipse_a2_b1 = {
	.x = 3,
	.y = 3,
	.a = 2,
	.b = 1,

	.w = 7,
	.h = 7,

	.pixmap = {
		0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0,
		0, 0, 1, 1, 1, 0, 0,
		0, 1, 1, 1, 1, 1, 0,
		0, 0, 1, 1, 1, 0, 0,
		0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0,
	}
};

static struct testcase testcase_ellipse_a1_b2 = {
	.x = 3,
	.y = 3,
	.a = 1,
	.b = 2,

	.w = 7,
	.h = 7,

	.pixmap = {
		0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 1, 0, 0, 0,
		0, 0, 1, 1, 1, 0, 0,
		0, 0, 1, 1, 1, 0, 0,
		0, 0, 1, 1, 1, 0, 0,
		0, 0, 0, 1, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0,
	}
};

static struct testcase testcase_ellipse_a2_b2 = {
	.x = 3,
	.y = 3,
	.a = 2,
	.b = 2,

	.w = 7,
	.h = 7,

	.pixmap = {
		0, 0, 0, 0, 0, 0, 0,
		0, 0, 1, 1, 1, 0, 0,
		0, 1, 1, 1, 1, 1, 0,
		0, 1, 1, 1, 1, 1, 0,
		0, 1, 1, 1, 1, 1, 0,
		0, 0, 1, 1, 1, 0, 0,
		0, 0, 0, 0, 0, 0, 0,
	}
};

static struct testcase testcase_ellipse_a1_b3 = {
	.x = 4,
	.y = 4,
	.a = 1,
	.b = 3,

	.w = 9,
	.h = 9,

	.pixmap = {
		0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 1, 0, 0, 0, 0,
		0, 0, 0, 1, 1, 1, 0, 0, 0,
		0, 0, 0, 1, 1, 1, 0, 0, 0,
		0, 0, 0, 1, 1, 1, 0, 0, 0,
		0, 0, 0, 1, 1, 1, 0, 0, 0,
		0, 0, 0, 1, 1, 1, 0, 0, 0,
		0, 0, 0, 0, 1, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0,
	}
};

static struct testcase testcase_ellipse_a3_b2 = {
	.x = 4,
	.y = 4,
	.a = 3,
	.b = 2,

	.w = 9,
	.h = 9,

	.pixmap = {
		0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 1, 1, 1, 0, 0, 0,
		0, 1, 1, 1, 1, 1, 1, 1, 0,
		0, 1, 1, 1, 1, 1, 1, 1, 0,
		0, 1, 1, 1, 1, 1, 1, 1, 0,
		0, 0, 0, 1, 1, 1, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0,
	}
};

static struct testcase testcase_ellipse_a3_b3 = {
	.x = 4,
	.y = 4,
	.a = 3,
	.b = 3,

	.w = 9,
	.h = 9,

	.pixmap = {
		0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 1, 1, 1, 0, 0, 0,
		0, 0, 1, 1, 1, 1, 1, 0, 0,
		0, 1, 1, 1, 1, 1, 1, 1, 0,
		0, 1, 1, 1, 1, 1, 1, 1, 0,
		0, 1, 1, 1, 1, 1, 1, 1, 0,
		0, 0, 1, 1, 1, 1, 1, 0, 0,
		0, 0, 0, 1, 1, 1, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0,
	}
};

static struct testcase testcase_ellipse_a6_b6_clip = {
	.x = 0,
	.y = 0,
	.a = 6,
	.b = 6,

	.w = 8,
	.h = 8,

	.pixmap = {
		1, 1, 1, 1, 1, 1, 1, 0,
		1, 1, 1, 1, 1, 1, 1, 0,
		1, 1, 1, 1, 1, 1, 1, 0,
		1, 1, 1, 1, 1, 1, 0, 0,
		1, 1, 1, 1, 1, 1, 0, 0,
		1, 1, 1, 1, 1, 0, 0, 0,
		1, 1, 1, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
	}
};

static struct testcase testcase_ellipse_a5_b5_clip = {
	.x = 0,
	.y = 5,
	.a = 5,
	.b = 5,

	.w = 11,
	.h = 11,

	.pixmap = {
		1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0,
		1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0,
		1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0,
		1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0,
		1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0,
		1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0,
		1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0,
		1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0,
		1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0,
		1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0,
		1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0,
	}
};

const struct tst_suite tst_suite = {
	.suite_name = "FillEllipse Testsuite",
	.tests = {
		{.name = "FillEllipse a=0 b=0",
		 .tst_fn = test_ellipse,
		 .data = &testcase_ellipse_a0_b0},

		{.name = "FillEllipse a=1 b=0",
		 .tst_fn = test_ellipse,
		 .data = &testcase_ellipse_a1_b0},

		{.name = "FillEllipse a=0 b=1",
		 .tst_fn = test_ellipse,
		 .data = &testcase_ellipse_a0_b1},

		{.name = "FillEllipse a=1 b=1",
		 .tst_fn = test_ellipse,
		 .data = &testcase_ellipse_a1_b1},

		{.name = "FillEllipse a=2 b=1",
		 .tst_fn = test_ellipse,
		 .data = &testcase_ellipse_a2_b1},

		{.name = "FillEllipse a=1 b=2",
		 .tst_fn = test_ellipse,
		 .data = &testcase_ellipse_a1_b2},

		{.name = "FillEllipse a=2 b=2",
		 .tst_fn = test_ellipse,
		 .data = &testcase_ellipse_a2_b2},

		{.name = "FillEllipse a=1 b=3",
		 .tst_fn = test_ellipse,
		 .data = &testcase_ellipse_a1_b3},

		{.name = "FillEllipse a=3 b=2",
		 .tst_fn = test_ellipse,
		 .data = &testcase_ellipse_a3_b2},

		{.name = "FillEllipse a=3 b=3",
		 .tst_fn = test_ellipse,
		 .data = &testcase_ellipse_a3_b3},

		{.name = "FillEllipse a=5 b=5 clipped",
		 .tst_fn = test_ellipse,
		 .data = &testcase_ellipse_a5_b5_clip},

		{.name = "FillEllipse a=6 b=6 clipped",
		 .tst_fn = test_ellipse,
		 .data = &testcase_ellipse_a6_b6_clip},

		{.name = NULL}
	}
};
