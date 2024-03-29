// SPDX-License-Identifier: GPL-2.1-or-later
/*
 * Copyright (C) 2009-2012 Cyril Hrubis <metan@ucw.cz>
 */

#include <string.h>
#include <errno.h>
#include <sys/stat.h>

#include <core/gp_pixmap.h>
#include <gfx/gp_circle.h>

#include "tst_test.h"

#include "common.h"

struct testcase {
	/* cicle description */
	gp_coord x;
	gp_coord y;
	gp_size r;

	/* expected result */
	gp_size w, h;
	const char pixmap[];
};

static int test_circle(const struct testcase *t)
{
	gp_pixmap *c;
	int err;

	c = pixmap_alloc_canary(t->w, t->h, GP_PIXEL_G8);

	if (!c) {
		tst_err("Failed to allocate pixmap");
		return TST_UNTESTED;
	}

	gp_circle(c, t->x, t->y, t->r, 1);

	err = compare_buffers(t->pixmap, c) || check_canary(c);

	if (err)
		return TST_FAILED;

	return TST_PASSED;
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

struct testcase testcase_circle_r_0_clip = {
	.x = 2,
	.y = 2,
	.r = 0,

	.w = 1,
	.h = 1,

	.pixmap = { 0 }
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

		{.name = "Circle r=0 + clipping",
		 .tst_fn = test_circle,
		 .data = &testcase_circle_r_0_clip},

		{.name = "Circle r=5 + clipping",
		 .tst_fn = test_circle,
		 .data = &testcase_circle_r_5_clip},

		{.name = "Circle r=6 + clipping",
		 .tst_fn = test_circle,
		 .data = &testcase_circle_r_6_clip},

		{.name = NULL}
	}
};
