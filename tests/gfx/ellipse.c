// SPDX-License-Identifier: GPL-2.1-or-later
/*
 * Copyright (C) 2009-2012 Cyril Hrubis <metan@ucw.cz>
 */

#include <string.h>
#include <errno.h>
#include <sys/stat.h>

#include <core/gp_pixmap.h>
#include <gfx/gp_ellipse.h>

#include "tst_test.h"

#include "common.h"

struct testcase {
	/* cicle description */
	gp_coord x;
	gp_coord y;
	gp_size a;
	gp_size b;

	/* expected result */
	gp_size w, h;
	const char pixmap[];
};

static int test_ellipse(const struct testcase *t)
{
	gp_pixmap *c;
	int err;

	c = pixmap_alloc_canary(t->w, t->h, GP_PIXEL_G8);

	if (!c) {
		tst_err("Failed to allocate pixmap");
		return TST_UNTESTED;
	}

	gp_ellipse(c, t->x, t->y, t->a, t->b, 1);

	err = compare_buffers(t->pixmap, c) || check_canary(c);

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
		0, 1, 0, 1, 0,
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
		0, 1, 0, 0, 0, 1, 0,
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
		0, 0, 1, 0, 1, 0, 0,
		0, 0, 1, 0, 1, 0, 0,
		0, 0, 1, 0, 1, 0, 0,
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
		0, 1, 0, 0, 0, 1, 0,
		0, 1, 0, 0, 0, 1, 0,
		0, 1, 0, 0, 0, 1, 0,
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
		0, 0, 0, 1, 0, 1, 0, 0, 0,
		0, 0, 0, 1, 0, 1, 0, 0, 0,
		0, 0, 0, 1, 0, 1, 0, 0, 0,
		0, 0, 0, 1, 0, 1, 0, 0, 0,
		0, 0, 0, 1, 0, 1, 0, 0, 0,
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
		0, 1, 1, 0, 0, 0, 1, 1, 0,
		0, 1, 0, 0, 0, 0, 0, 1, 0,
		0, 1, 1, 0, 0, 0, 1, 1, 0,
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
		0, 0, 1, 0, 0, 0, 1, 0, 0,
		0, 1, 0, 0, 0, 0, 0, 1, 0,
		0, 1, 0, 0, 0, 0, 0, 1, 0,
		0, 1, 0, 0, 0, 0, 0, 1, 0,
		0, 0, 1, 0, 0, 0, 1, 0, 0,
		0, 0, 0, 1, 1, 1, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0,
	}
};

static struct testcase testcase_ellipse_a0_b0_clip = {
	.x = 2,
	.y = 2,
	.a = 0,
	.b = 0,

	.w = 1,
	.h = 1,

	.pixmap = { 0 }
};

static struct testcase testcase_ellipse_a6_b6_clip = {
	.x = 0,
	.y = 0,
	.a = 6,
	.b = 6,

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

static struct testcase testcase_ellipse_a5_b5_clip = {
	.x = 0,
	.y = 5,
	.a = 5,
	.b = 5,

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

const struct tst_suite tst_suite = {
	.suite_name = "Ellipse Testsuite",
	.tests = {
		{.name = "Ellipse a=0 b=0",
		 .tst_fn = test_ellipse,
		 .data = &testcase_ellipse_a0_b0},

		{.name = "Ellipse a=1 b=0",
		 .tst_fn = test_ellipse,
		 .data = &testcase_ellipse_a1_b0},

		{.name = "Ellipse a=0 b=1",
		 .tst_fn = test_ellipse,
		 .data = &testcase_ellipse_a0_b1},

		{.name = "Ellipse a=1 b=1",
		 .tst_fn = test_ellipse,
		 .data = &testcase_ellipse_a1_b1},

		{.name = "Ellipse a=2 b=1",
		 .tst_fn = test_ellipse,
		 .data = &testcase_ellipse_a2_b1},

		{.name = "Ellipse a=1 b=2",
		 .tst_fn = test_ellipse,
		 .data = &testcase_ellipse_a1_b2},

		{.name = "Ellipse a=2 b=2",
		 .tst_fn = test_ellipse,
		 .data = &testcase_ellipse_a2_b2},

		{.name = "Ellipse a=1 b=3",
		 .tst_fn = test_ellipse,
		 .data = &testcase_ellipse_a1_b3},

		{.name = "Ellipse a=3 b=2",
		 .tst_fn = test_ellipse,
		 .data = &testcase_ellipse_a3_b2},

		{.name = "Ellipse a=3 b=3",
		 .tst_fn = test_ellipse,
		 .data = &testcase_ellipse_a3_b3},

		{.name = "Ellipse a=0 b=0 clipped",
		 .tst_fn = test_ellipse,
		 .data = &testcase_ellipse_a0_b0_clip},

		{.name = "Ellipse a=5 b=5 clipped",
		 .tst_fn = test_ellipse,
		 .data = &testcase_ellipse_a5_b5_clip},

		{.name = "Ellipse a=6 b=6 clipped",
		 .tst_fn = test_ellipse,
		 .data = &testcase_ellipse_a6_b6_clip},

		{.name = NULL}
	}
};
