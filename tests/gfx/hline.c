// SPDX-License-Identifier: GPL-2.1-or-later
/*
 * Copyright (C) 2009-2013 Cyril Hrubis <metan@ucw.cz>
 */

#include <string.h>
#include <errno.h>

#include <core/gp_pixmap.h>
#include <gfx/gp_hline.h>

#include "tst_test.h"

#include "common.h"

struct testcase {
	/* HLine description */
	gp_coord x0;
	gp_coord x1;
	gp_coord y;

	gp_coord x;
	gp_size lw;

	int flag;

	/* expected result */
	gp_size w, h;
	const char pixmap[];
};

static int test_hline(struct testcase *t)
{
	gp_pixmap *c;
	int err;

	c = pixmap_alloc_canary(t->w, t->h, GP_PIXEL_G8);

	if (!c) {
		tst_err("Failed to allocate pixmap");
		return TST_UNTESTED;
	}

	if (t->flag)
		gp_hline_xyw(c, t->x, t->y, t->lw, 1);
	else
		gp_hline(c, t->x0, t->x1, t->y, 1);

	err = compare_buffers(t->pixmap, c) || check_canary(c);

	if (err) {
		tst_msg("Patterns are different");
		return TST_FAILED;
	}

	return TST_PASSED;
}

static struct testcase testcase_1_px = {
	.x0 = 1,
	.x1 = 1,
	.y = 1,

	.w = 3,
	.h = 3,
	.pixmap = {
		0, 0, 0,
		0, 1, 0,
		0, 0, 0,
	}
};

static struct testcase testcase_3_px_1 = {
	.x0 = 1,
	.x1 = 3,
	.y = 1,

	.w = 5,
	.h = 3,
	.pixmap = {
		0, 0, 0, 0, 0,
		0, 1, 1, 1, 0,
		0, 0, 0, 0, 0,
	}
};

static struct testcase testcase_3_px_2 = {
	.x0 = 3,
	.x1 = 1,
	.y = 1,

	.w = 5,
	.h = 3,
	.pixmap = {
		0, 0, 0, 0, 0,
		0, 1, 1, 1, 0,
		0, 0, 0, 0, 0,
	}
};

static struct testcase testcase_clipping_1 = {
	.x0 = -10000,
	.x1 = 10000,
	.y = 1,

	.w = 5,
	.h = 3,
	.pixmap = {
		0, 0, 0, 0, 0,
		1, 1, 1, 1, 1,
		0, 0, 0, 0, 0,
	}
};

static struct testcase testcase_clipping_2 = {
	.x0 = -10000,
	.x1 = 10000,
	.y = 4,

	.w = 5,
	.h = 3,
	.pixmap = {
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
	}
};

static struct testcase testcase_clipping_3 = {
	.x0 = -100000,
	.x1 = 100000,
	.y = -4,

	.w = 5,
	.h = 3,
	.pixmap = {
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
	}
};

static struct testcase testcase_xyw_1 = {
	.x = 1,
	.y = 1,
	.lw = 0,

	.flag = 1,

	.w = 3,
	.h = 3,
	.pixmap = {
		0, 0, 0,
		0, 0, 0,
		0, 0, 0,
	}
};

static struct testcase testcase_xyw_2 = {
	.x = 1,
	.y = 1,
	.lw = 2,

	.flag = 1,

	.w = 4,
	.h = 3,
	.pixmap = {
		0, 0, 0, 0,
		0, 1, 1, 0,
		0, 0, 0, 0,
	}
};

static struct testcase testcase_xyw_clipp_1 = {
	.x = -10000,
	.y = 1,
	.lw = 20000,

	.flag = 1,

	.w = 3,
	.h = 3,
	.pixmap = {
		0, 0, 0,
		1, 1, 1,
		0, 0, 0,
	}
};

static struct testcase testcase_xyw_clipp_2 = {
	.x = 1,
	.y = 1,
	.lw = 200000,

	.flag = 1,

	.w = 3,
	.h = 3,
	.pixmap = {
		0, 0, 0,
		0, 1, 1,
		0, 0, 0,
	}
};

static struct testcase testcase_xyw_clipp_3 = {
	.x = -10000,
	.y = -10000,
	.lw = -1,

	.flag = 1,

	.w = 3,
	.h = 3,
	.pixmap = {
		0, 0, 0,
		0, 0, 0,
		0, 0, 0,
	}
};

const struct tst_suite tst_suite = {
	.suite_name = "HLine Testsuite",
	.tests = {
		{.name = "HLine 1px",
		 .tst_fn = test_hline,
		 .data = &testcase_1_px},

		{.name = "HLine 3px 1",
		 .tst_fn = test_hline,
		 .data = &testcase_3_px_1},

		{.name = "HLine 3px 2",
		 .tst_fn = test_hline,
		 .data = &testcase_3_px_2},

		{.name = "HLine clipping 1",
		 .tst_fn = test_hline,
		 .data = &testcase_clipping_1},

		{.name = "HLine clipping 2",
		 .tst_fn = test_hline,
		 .data = &testcase_clipping_2},

		{.name = "HLine clipping 3",
		 .tst_fn = test_hline,
		 .data = &testcase_clipping_3},

		{.name = "HLineXYW 1",
		 .tst_fn = test_hline,
		 .data = &testcase_xyw_1},

		{.name = "HLineXYW 2",
		 .tst_fn = test_hline,
		 .data = &testcase_xyw_2},

		{.name = "HLineXYW clipping 1",
		 .tst_fn = test_hline,
		 .data = &testcase_xyw_clipp_1},

		{.name = "HLineXYW clipping 2",
		 .tst_fn = test_hline,
		 .data = &testcase_xyw_clipp_2},

		{.name = "HLineXYW clipping 3",
		 .tst_fn = test_hline,
		 .data = &testcase_xyw_clipp_3},

		{.name = NULL}
	}
};
