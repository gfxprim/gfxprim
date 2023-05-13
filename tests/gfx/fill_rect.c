// SPDX-License-Identifier: GPL-2.1-or-later
/*
 * Copyright (C) 2009-2013 Cyril Hrubis <metan@ucw.cz>
 */

#include <string.h>
#include <errno.h>
#include <sys/stat.h>

#include <core/gp_pixmap.h>
#include <gfx/gp_rect.h>

#include "tst_test.h"

#include "common.h"

struct testcase {
	/* rect description */
	gp_coord x1;
	gp_coord y1;
	gp_coord x2;
	gp_coord y2;

	/* expected result */
	gp_size w, h;
	const char pixmap[];
};

static int test_rect(const struct testcase *t)
{
	gp_pixmap *c;
	int err;

	c = pixmap_alloc_canary(t->w, t->h, GP_PIXEL_G8);

	if (!c) {
		tst_err("Failed to allocate pixmap");
		return TST_UNTESTED;
	}

	gp_fill_rect(c, t->x1, t->y1, t->x2, t->y2, 1);

	err = compare_buffers(t->pixmap, c) || check_canary(c);

	if (err)
		return TST_FAILED;

	return TST_PASSED;
}

struct testcase testcase_rect_1 = {
	.x1 = 2,
	.y1 = 2,
	.x2 = 2,
	.y2 = 2,

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

struct testcase testcase_rect_9a = {
	.x1 = 1,
	.y1 = 1,
	.x2 = 3,
	.y2 = 3,

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

struct testcase testcase_rect_9b = {
	.x1 = 3,
	.y1 = 3,
	.x2 = 1,
	.y2 = 1,

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

struct testcase testcase_rect_9lu = {
	.x1 = -2147483648,
	.y1 = -2147483648,
	.x2 = 2,
	.y2 = 2,

	.w = 5,
	.h = 5,

	.pixmap = {
		1, 1, 1, 0, 0,
		1, 1, 1, 0, 0,
		1, 1, 1, 0, 0,
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
	}
};

struct testcase testcase_rect_9ld = {
	.x1 = -2147483648,
	.y1 = 2,
	.x2 = 2,
	.y2 = 2147483647,

	.w = 5,
	.h = 5,

	.pixmap = {
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
		1, 1, 1, 0, 0,
		1, 1, 1, 0, 0,
		1, 1, 1, 0, 0,
	}
};

struct testcase testcase_rect_9ru = {
	.x1 = 2,
	.y1 = -2147483648,
	.x2 = 2147483647,
	.y2 = 2,

	.w = 5,
	.h = 5,

	.pixmap = {
		0, 0, 1, 1, 1,
		0, 0, 1, 1, 1,
		0, 0, 1, 1, 1,
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
	}
};

struct testcase testcase_rect_9rd = {
	.x1 = 2,
	.y1 = 2,
	.x2 = 2147483647,
	.y2 = 2147483647,

	.w = 5,
	.h = 5,

	.pixmap = {
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
		0, 0, 1, 1, 1,
		0, 0, 1, 1, 1,
		0, 0, 1, 1, 1,
	}
};

struct testcase testcase_rect_9r = {
	.x1 = -2147483648,
	.y1 = 3,
	.x2 = 2,
	.y2 = 1,

	.w = 5,
	.h = 5,

	.pixmap = {
		0, 0, 0, 0, 0,
		1, 1, 1, 0, 0,
		1, 1, 1, 0, 0,
		1, 1, 1, 0, 0,
		0, 0, 0, 0, 0,
	}
};

struct testcase testcase_rect_9l = {
	.x1 = 2,
	.y1 = 3,
	.x2 = 2147483647,
	.y2 = 1,

	.w = 5,
	.h = 5,

	.pixmap = {
		0, 0, 0, 0, 0,
		0, 0, 1, 1, 1,
		0, 0, 1, 1, 1,
		0, 0, 1, 1, 1,
		0, 0, 0, 0, 0,
	}
};

struct testcase testcase_rect_9u = {
	.x1 = 1,
	.y1 = -2147483648,
	.x2 = 3,
	.y2 = 2,

	.w = 5,
	.h = 5,

	.pixmap = {
		0, 1, 1, 1, 0,
		0, 1, 1, 1, 0,
		0, 1, 1, 1, 0,
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
	}
};

struct testcase testcase_rect_9d = {
	.x1 = 1,
	.y1 = 2,
	.x2 = 3,
	.y2 = 2147483647,

	.w = 5,
	.h = 5,

	.pixmap = {
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
		0, 1, 1, 1, 0,
		0, 1, 1, 1, 0,
		0, 1, 1, 1, 0,
	}
};

struct testcase testcase_rect_0a = {
	.x1 = 2147483647,
	.y1 = -2147483648,
	.x2 = 2147483647,
	.y2 = -2147483648,

	.w = 5,
	.h = 5,

	.pixmap = {
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
	}
};

struct testcase testcase_rect_0b = {
	.x1 = -2147483648,
	.y1 = 2147483647,
	.x2 = -2147483648,
	.y2 = 2147483647,

	.w = 5,
	.h = 5,

	.pixmap = {
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
	}
};

struct testcase testcase_rect_25 = {
	.x1 = -2147483648,
	.y1 = 2147483647,
	.x2 = 2147483647,
	.y2 = -2147483648,

	.w = 5,
	.h = 5,

	.pixmap = {
		1, 1, 1, 1, 1,
		1, 1, 1, 1, 1,
		1, 1, 1, 1, 1,
		1, 1, 1, 1, 1,
		1, 1, 1, 1, 1,
	}
};

const struct tst_suite tst_suite = {
	.suite_name = "FillRect Testsuite",
	.tests = {
		{.name = "FillRect x1=x2 y1=y2",
		 .tst_fn = test_rect,
		 .data = &testcase_rect_1},

		{.name = "FillRect x1<x2 y1<y2",
		 .tst_fn = test_rect,
		 .data = &testcase_rect_9a},

		{.name = "FillRect x1>x2 y1>y2",
		 .tst_fn = test_rect,
		 .data = &testcase_rect_9b},

		{.name = "FillRect x1,y1 out of pixmap",
		 .tst_fn = test_rect,
		 .data = &testcase_rect_9lu,
		 .timeout = 2},

		{.name = "FillRect x1,y2 out of pixmap",
		 .tst_fn = test_rect,
		 .data = &testcase_rect_9ld,
		 .timeout = 2},

		{.name = "FillRect x2,y1 out of pixmap",
		 .tst_fn = test_rect,
		 .data = &testcase_rect_9ru,
		 .timeout = 2},

		{.name = "FillRect x2,y2 out of pixmap",
		 .tst_fn = test_rect,
		 .data = &testcase_rect_9rd,
		 .timeout = 2},

		{.name = "FillRect x1 out of pixmap",
		 .tst_fn = test_rect,
		 .data = &testcase_rect_9r,
		 .timeout = 2},

		{.name = "FillRect x2 out of pixmap",
		 .tst_fn = test_rect,
		 .data = &testcase_rect_9l,
		 .timeout = 2},

		{.name = "FillRect y1 out of pixmap",
		 .tst_fn = test_rect,
		 .data = &testcase_rect_9u,
		 .timeout = 2},

		{.name = "FillRect y2 out of pixmap",
		 .tst_fn = test_rect,
		 .data = &testcase_rect_9d,
		 .timeout = 2},

		{.name = "FillRect rect out of pixmap 1",
		 .tst_fn = test_rect,
		 .data = &testcase_rect_0a,
		 .timeout = 2},

		{.name = "FillRect rect out of pixmap 2",
		 .tst_fn = test_rect,
		 .data = &testcase_rect_0b,
		 .timeout = 2},

		{.name = "FillRect full rect",
		 .tst_fn = test_rect,
		 .data = &testcase_rect_25,
		 .timeout = 2},

		{.name = NULL}
	}
};
