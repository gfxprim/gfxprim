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

	gp_line(c, t->x0, t->y0, t->x1, t->y1, 1);

	err = compare_buffers(t->pixmap, c) || check_canary(c);

	if (err)
		return TST_FAILED;

	return TST_PASSED;
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

static struct testcase testcase_line_horiz1 = {
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

static struct testcase testcase_line_horiz2 = {
	.x0 = 8,
	.y0 = 2,
	.x1 = 2,
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

static struct testcase testcase_line_vert1 = {
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

static struct testcase testcase_line_vert2 = {
	.x0 = 2,
	.y0 = 8,
	.x1 = 2,
	.y1 = 2,

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
	.x1 = 10,
	.y1 = 6,

	.w = 11,
	.h = 8,

	.pixmap = {
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1,
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

static struct testcase line_nearly_vertical = {
	.x0 = 0,
	.y0 = 0,
	.x1 = 1,
	.y1 = 9,

	.w = 2,
	.h = 10,

	.pixmap = {
		1, 0,
		1, 0,
		1, 0,
		1, 0,
		1, 0,
		0, 1,
		0, 1,
		0, 1,
		0, 1,
		0, 1,
	}
};

static struct testcase line_nearly_horizontal = {
	.x0 = 0,
	.y0 = 0,
	.x1 = 9,
	.y1 = 1,

	.w = 10,
	.h = 2,

	.pixmap = {
		1, 1, 1, 1, 1, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 1, 1, 1, 1, 1,
	}
};

static struct testcase line_0_0_1_2 = {
	.x0 = 0,
	.y0 = 0,
	.x1 = 1,
	.y1 = 2,

	.w = 2,
	.h = 3,

	.pixmap = {
		1, 0,
		1, 1,
		0, 1,
	}
};

static struct testcase line_0_0_1_4 = {
	.x0 = 0,
	.y0 = 0,
	.x1 = 1,
	.y1 = 4,

	.w = 2,
	.h = 5,

	.pixmap = {
		1, 0,
		1, 0,
		1, 1,
		0, 1,
		0, 1,
	}
};

static struct testcase line_0_0_2_1 = {
	.x0 = 0,
	.y0 = 0,
	.x1 = 2,
	.y1 = 1,

	.w = 3,
	.h = 2,

	.pixmap = {
		1, 1, 0,
		0, 1, 1,
	}
};

static struct testcase line_0_0_4_1 = {
	.x0 = 0,
	.y0 = 0,
	.x1 = 4,
	.y1 = 1,

	.w = 5,
	.h = 2,

	.pixmap = {
		1, 1, 1, 0, 0,
		0, 0, 1, 1, 1,
	}
};

static struct testcase line_0_0_2_4 = {
	.x0 = 0,
	.y0 = 0,
	.x1 = 2,
	.y1 = 4,

	.w = 3,
	.h = 5,

	.pixmap = {
		1, 0, 0,
		1, 0, 0,
		0, 1, 0,
		0, 0, 1,
		0, 0, 1,
	}
};

static struct testcase line_0_0_4_2 = {
	.x0 = 0,
	.y0 = 0,
	.x1 = 4,
	.y1 = 2,

	.w = 5,
	.h = 3,

	.pixmap = {
		1, 1, 0, 0, 0,
		0, 0, 1, 0, 0,
		0, 0, 0, 1, 1,
	}
};

static struct testcase line_0_0_8_4 = {
	.x0 = 0,
	.y0 = 0,
	.x1 = 8,
	.y1 = 4,

	.w = 9,
	.h = 5,

	.pixmap = {
		1, 1, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 1, 1, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 1, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 1, 1, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 1, 1,
	}
};

static struct testcase line_0_0_4_8 = {
	.x0 = 0,
	.y0 = 0,
	.x1 = 4,
	.y1 = 8,

	.w = 5,
	.h = 9,

	.pixmap = {
		1, 0, 0, 0, 0,
		1, 0, 0, 0, 0,
		0, 1, 0, 0, 0,
		0, 1, 0, 0, 0,
		0, 0, 1, 0, 0,
		0, 0, 0, 1, 0,
		0, 0, 0, 1, 0,
		0, 0, 0, 0, 1,
		0, 0, 0, 0, 1,
	}
};


const struct tst_suite tst_suite = {
	.suite_name = "Line Testsuite",
	.tests = {
		{.name = "Line 1px",
		 .tst_fn = test_line,
		 .data = &testcase_line_1px},

		{.name = "Line Horizontal 1",
		 .tst_fn = test_line,
		 .data = &testcase_line_horiz1},

		{.name = "Line Horizontal 2",
		 .tst_fn = test_line,
		 .data = &testcase_line_horiz2},

		{.name = "Line Vertical 1",
		 .tst_fn = test_line,
		 .data = &testcase_line_vert1},

		{.name = "Line Vertical 2",
		 .tst_fn = test_line,
		 .data = &testcase_line_vert2},

		{.name = "Line 45 degrees",
		 .tst_fn = test_line,
		 .data = &testcase_line_45},

		{.name = "Line 15 degrees",
		 .tst_fn = test_line,
		 .data = &testcase_line_15},

		{.name = "Line nearly vertical",
		 .tst_fn = test_line,
		 .data = &line_nearly_vertical},

		{.name = "Line nearly horizontal",
		 .tst_fn = test_line,
		 .data = &line_nearly_horizontal},

		{.name = "Line 0, 0, 1, 2",
		 .tst_fn = test_line,
		 .data = &line_0_0_1_2},

		{.name = "line 0, 0, 1, 4",
		 .tst_fn = test_line,
		 .data = &line_0_0_1_4},

		{.name = "Line 0, 0, 2, 1",
		 .tst_fn = test_line,
		 .data = &line_0_0_2_1},

		{.name = "line 0, 0, 4, 1",
		 .tst_fn = test_line,
		 .data = &line_0_0_4_1},

		{.name = "line 0, 0, 2, 4",
		 .tst_fn = test_line,
		 .data = &line_0_0_2_4},

		{.name = "line 0, 0, 4, 2",
		 .tst_fn = test_line,
		 .data = &line_0_0_4_2},

		{.name = "line 0, 0, 8, 4",
		 .tst_fn = test_line,
		 .data = &line_0_0_8_4},

		{.name = "line 0, 0, 4, 8",
		 .tst_fn = test_line,
		 .data = &line_0_0_4_8},

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
