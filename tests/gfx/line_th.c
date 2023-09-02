// SPDX-License-Identifier: GPL-2.1-or-later
/*
 * Copyright (C) 2009-2012 Cyril Hrubis <metan@ucw.cz>
 */

#include <string.h>
#include <errno.h>
#include <sys/stat.h>

#include <core/gp_pixmap.h>
#include <gfx/gp_line.h>

#include "tst_test.h"

#include "common.h"

struct testcase {
	/* line description */
	gp_coord x0;
	gp_coord y0;
	gp_coord x1;
	gp_coord y1;

	/* line thickness */
	gp_size r;

	/* expected result */
	gp_size w, h;
	const char pixmap[];
};

static int test_line(const struct testcase *t)
{
	gp_pixmap *c;
	int err;

	c = pixmap_alloc_canary(t->w, t->h, GP_PIXEL_G8);

	if (!c) {
		tst_err("Failed to allocate pixmap");
		return TST_UNTESTED;
	}

	gp_line_th(c, t->x0, t->y0, t->x1, t->y1, t->r, 1);

	err = compare_buffers(t->pixmap, c) || check_canary(c);

	if (err)
		return TST_FAILED;

	return TST_PASSED;
}

static struct testcase testcase_line_1px_r0 = {
	.x0 = 1,
	.y0 = 1,
	.x1 = 1,
	.y1 = 1,

	.r = 0,

	.w = 3,
	.h = 3,

	.pixmap = {
		0, 0, 0,
		0, 1, 0,
		0, 0, 0,
	}
};

static struct testcase testcase_line_1px_r1 = {
	.x0 = 2,
	.y0 = 2,
	.x1 = 2,
	.y1 = 2,

	.r = 1,

	.w = 5,
	.h = 5,

	.pixmap = {
		0, 0, 0, 0, 0,
		0, 1, 1, 1, 0,
		0, 1, 1, 1, 0,
		0, 1, 1, 1, 0,
		0, 0, 0, 0, 0,
	}
};

static struct testcase testcase_line_1px_r2 = {
	.x0 = 3,
	.y0 = 3,
	.x1 = 3,
	.y1 = 3,

	.r = 2,

	.w = 7,
	.h = 7,

	.pixmap = {
		0, 0, 0, 0, 0, 0, 0,
		0, 1, 1, 1, 1, 1, 0,
		0, 1, 1, 1, 1, 1, 0,
		0, 1, 1, 1, 1, 1, 0,
		0, 1, 1, 1, 1, 1, 0,
		0, 1, 1, 1, 1, 1, 0,
		0, 0, 0, 0, 0, 0, 0,
	}
};

static struct testcase testcase_line_2px_h_r0 = {
	.x0 = 1,
	.y0 = 1,
	.x1 = 2,
	.y1 = 1,

	.r = 0,

	.w = 4,
	.h = 3,

	.pixmap = {
		0, 0, 0, 0,
		0, 1, 1, 0,
		0, 0, 0, 0,
	}
};

static struct testcase testcase_line_2px_v_r0 = {
	.x0 = 1,
	.y0 = 1,
	.x1 = 1,
	.y1 = 2,

	.r = 0,

	.w = 3,
	.h = 4,

	.pixmap = {
		0, 0, 0,
		0, 1, 0,
		0, 1, 0,
		0, 0, 0,
	}
};

static struct testcase testcase_line_2px_h_r1 = {
	.x0 = 1,
	.y0 = 2,
	.x1 = 2,
	.y1 = 2,

	.r = 1,

	.w = 4,
	.h = 5,

	.pixmap = {
		0, 0, 0, 0,
		0, 1, 1, 0,
		0, 1, 1, 0,
		0, 1, 1, 0,
		0, 0, 0, 0,
	}
};

static struct testcase testcase_line_2px_v_r1 = {
	.x0 = 2,
	.y0 = 1,
	.x1 = 2,
	.y1 = 2,

	.r = 1,

	.w = 5,
	.h = 4,

	.pixmap = {
		0, 0, 0, 0, 0,
		0, 1, 1, 1, 0,
		0, 1, 1, 1, 0,
		0, 0, 0, 0, 0,
	}
};

static struct testcase testcase_line_45st_1_r0 = {
	.x0 = 1,
	.y0 = 1,
	.x1 = 3,
	.y1 = 3,

	.r = 0,

	.w = 5,
	.h = 5,

	.pixmap = {
		0, 0, 0, 0, 0,
		0, 1, 0, 0, 0,
		0, 0, 1, 0, 0,
		0, 0, 0, 1, 0,
		0, 0, 0, 0, 0,
	}
};

static struct testcase testcase_line_45st_1_r1 = {
	.x0 = 2,
	.y0 = 2,
	.x1 = 5,
	.y1 = 5,

	.r = 1,

	.w = 8,
	.h = 8,

	.pixmap = {
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 1, 0, 0, 0, 0,
		0, 0, 1, 1, 1, 0, 0, 0,
		0, 1, 1, 1, 1, 1, 0, 0,
		0, 0, 1, 1, 1, 1, 1, 0,
		0, 0, 0, 1, 1, 1, 0, 0,
		0, 0, 0, 0, 1, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
	}
};

static struct testcase testcase_line_45st_1_r2 = {
	.x0 = 3,
	.y0 = 3,
	.x1 = 6,
	.y1 = 6,

	.r = 2,

	.w = 10,
	.h = 10,

	.pixmap = {
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 1, 1, 0, 0, 0, 0,
		0, 0, 0, 1, 1, 1, 1, 0, 0, 0,
		0, 0, 1, 1, 1, 1, 1, 1, 0, 0,
		0, 0, 1, 1, 1, 1, 1, 1, 0, 0,
		0, 0, 0, 1, 1, 1, 1, 0, 0, 0,
		0, 0, 0, 0, 1, 1, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	}
};

static struct testcase testcase_line_45st_2_r0 = {
	.x0 = 1,
	.y0 = 3,
	.x1 = 3,
	.y1 = 1,

	.r = 0,

	.w = 5,
	.h = 5,

	.pixmap = {
		0, 0, 0, 0, 0,
		0, 0, 0, 1, 0,
		0, 0, 1, 0, 0,
		0, 1, 0, 0, 0,
		0, 0, 0, 0, 0,
	}
};

static struct testcase testcase_line_26_6st_1_r1 = {
	.x0 = 2,
	.y0 = 2,
	.x1 = 6,
	.y1 = 4,

	.r = 1,

	.w = 8,
	.h = 6,

	.pixmap = {
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 1, 1, 0, 0, 0, 0,
		0, 0, 1, 1, 1, 1, 0, 0,
		0, 0, 1, 1, 1, 1, 1, 0,
		0, 0, 0, 1, 1, 1, 1, 0,
		0, 0, 0, 0, 0, 1, 1, 0,
	}
};


const struct tst_suite tst_suite = {
	.suite_name = "Thick Line Testsuite",
	.tests = {
		{.name = "Line 1px r=0",
		 .tst_fn = test_line,
		 .data = &testcase_line_1px_r0},

		{.name = "Line 1px r=1",
		 .tst_fn = test_line,
		 .data = &testcase_line_1px_r1},

		{.name = "Line 1px r=2",
		 .tst_fn = test_line,
		 .data = &testcase_line_1px_r2},

		{.name = "Line 2px horizontal r=0",
		 .tst_fn = test_line,
		 .data = &testcase_line_2px_h_r0},

		{.name = "Line 2px horizontal r=1",
		 .tst_fn = test_line,
		 .data = &testcase_line_2px_h_r1},

		{.name = "Line 2px vertical r=0",
		 .tst_fn = test_line,
		 .data = &testcase_line_2px_v_r0},

		{.name = "Line 2px horizontal r=1",
		 .tst_fn = test_line,
		 .data = &testcase_line_2px_v_r1},

		{.name = "Line 45 st 1 r=0",
		 .tst_fn = test_line,
		 .data = &testcase_line_45st_1_r0},

		{.name = "Line 45 st 1 r=1",
		 .tst_fn = test_line,
		 .data = &testcase_line_45st_1_r1},

		{.name = "Line 45 st 1 r=2",
		 .tst_fn = test_line,
		 .data = &testcase_line_45st_1_r2},

		{.name = "Line 45 st 2 r=0",
		 .tst_fn = test_line,
		 .data = &testcase_line_45st_2_r0},

		{.name = "Line 26.6 st r=1",
		 .tst_fn = test_line,
		 .data = &testcase_line_26_6st_1_r1},

		{.name = NULL}
	}
};
