// SPDX-License-Identifier: GPL-2.1-or-later
/*
 * Copyright (C) 2009-2018 Cyril Hrubis <metan@ucw.cz>
 */

#include <string.h>
#include <errno.h>
#include <sys/stat.h>

#include <core/gp_pixmap.h>
#include <gfx/gp_triangle.h>

#include <gfx/gp_line.h>

#include "tst_test.h"

#include "common.h"

struct testcase {
	/* line description */
	gp_coord x0;
	gp_coord y0;
	gp_coord x1;
	gp_coord y1;
	gp_coord x2;
	gp_coord y2;

	/* expected result */
	gp_size w, h;
	const char pixmap[];
};

static int test_fill_triangle(const struct testcase *t)
{
	gp_pixmap *c;
	int err;

	c = pixmap_alloc_canary(t->w, t->h, GP_PIXEL_G8);

	if (!c) {
		tst_err("Failed to allocate pixmap");
		return TST_UNTESTED;
	}

/*
	gp_line(c, t->x0, t->y0, t->x1, t->y1, 1);
	gp_line(c, t->x0, t->y0, t->x2, t->y2, 1);
	gp_line(c, t->x2, t->y2, t->x1, t->y1, 1);
*/
	gp_fill_triangle(c, t->x0, t->y0, t->x1, t->y1, t->x2, t->y2, 1);

	err = compare_buffers(t->pixmap, c) || check_canary(c);

	if (err)
		return TST_FAILED;

	return TST_PASSED;
}

static struct testcase triangle_point = {
	.x0 = 1,
	.y0 = 1,
	.x1 = 1,
	.y1 = 1,
	.x2 = 1,
	.y2 = 1,

	.w = 3,
	.h = 3,

	.pixmap = {
		0, 0, 0,
		0, 1, 0,
		0, 0, 0,
	}
};

static struct testcase triangle_hline = {
	.x0 = 1,
	.y0 = 1,
	.x1 = 2,
	.y1 = 1,
	.x2 = 3,
	.y2 = 1,

	.w = 5,
	.h = 3,

	.pixmap = {
		0, 0, 0, 0, 0,
		0, 1, 1, 1, 0,
		0, 0, 0, 0, 0,
	}
};

static struct testcase triangle_vline = {
	.x0 = 1,
	.y0 = 1,
	.x1 = 1,
	.y1 = 2,
	.x2 = 1,
	.y2 = 3,

	.w = 3,
	.h = 5,

	.pixmap = {
		0, 0, 0,
		0, 1, 0,
		0, 1, 0,
		0, 1, 0,
		0, 0, 0,
	}
};

static struct testcase triangle1 = {
	.x0 = 1,
	.y0 = 2,
	.x1 = 3,
	.y1 = 2,
	.x2 = 2,
	.y2 = 1,

	.w = 5,
	.h = 4,

	.pixmap = {
		0, 0, 0, 0, 0,
		0, 0, 1, 0, 0,
		0, 1, 1, 1, 0,
		0, 0, 0, 0, 0,
	}
};

static struct testcase triangle2 = {
	.x0 = 1,
	.y0 = 1,
	.x1 = 3,
	.y1 = 1,
	.x2 = 2,
	.y2 = 2,

	.w = 5,
	.h = 4,

	.pixmap = {
		0, 0, 0, 0, 0,
		0, 1, 1, 1, 0,
		0, 0, 1, 0, 0,
		0, 0, 0, 0, 0,
	}
};

static struct testcase triangle3 = {
	.x0 = 2,
	.y0 = 2,
	.x1 = 1,
	.y1 = 1,
	.x2 = 2,
	.y2 = 1,

	.w = 4,
	.h = 4,

	.pixmap = {
		0, 0, 0, 0,
		0, 1, 1, 0,
		0, 0, 1, 0,
		0, 0, 0, 0,
	}
};

static struct testcase triangle4 = {
	.x0 = 2,
	.y0 = 2,
	.x1 = 1,
	.y1 = 1,
	.x2 = 1,
	.y2 = 2,

	.w = 4,
	.h = 4,

	.pixmap = {
		0, 0, 0, 0,
		0, 1, 0, 0,
		0, 1, 1, 0,
		0, 0, 0, 0,
	}
};

const struct tst_suite tst_suite = {
	.suite_name = "Fill Triangle Testsuite",
	.tests = {
		{.name = "Fill Triangle point",
		 .tst_fn = test_fill_triangle,
		 .data = &triangle_point},

		{.name = "Fill Triangle hline",
		 .tst_fn = test_fill_triangle,
		 .data = &triangle_hline},

		{.name = "Fill Triangle vline",
		 .tst_fn = test_fill_triangle,
		 .data = &triangle_vline},

		{.name = "Fill Triangle 1",
		 .tst_fn = test_fill_triangle,
		 .data = &triangle1},

		{.name = "Fill Triangle 2",
		 .tst_fn = test_fill_triangle,
		 .data = &triangle2},

		{.name = "Fill Triangle 3",
		 .tst_fn = test_fill_triangle,
		 .data = &triangle3},

		{.name = "Fill Triangle 4",
		 .tst_fn = test_fill_triangle,
		 .data = &triangle4},

		{.name = NULL}
	}
};
