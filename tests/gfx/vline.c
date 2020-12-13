// SPDX-License-Identifier: GPL-2.1-or-later
/*
 * Copyright (C) 2009-2013 Cyril Hrubis <metan@ucw.cz>
 */

#include <string.h>
#include <errno.h>

#include <core/gp_pixmap.h>
#include <gfx/gp_vline.h>

#include "tst_test.h"

#include "common.h"

struct testcase {
	/* VLine description */
	gp_coord x;
	gp_coord y0;
	gp_coord y1;

	gp_coord y;
	gp_size lh;

	int flag;

	/* expected result */
	gp_size w, h;
	const char pixmap[];
};

static int test_vline(struct testcase *t)
{
	gp_pixmap *c;
	int err;

	c = pixmap_alloc_canary(t->w, t->h, GP_PIXEL_G8);

	if (!c) {
		tst_err("Failed to allocate pixmap");
		return TST_UNTESTED;
	}

	/* zero the pixels buffer */
	memset(c->pixels, 0, c->w * c->h);

	if (t->flag)
		gp_vline_xyh(c, t->x, t->y, t->lh, 1);
	else
		gp_vline(c, t->x, t->y0, t->y1, 1);

	err = compare_buffers(t->pixmap, c) || check_canary(c);

	if (err) {
		tst_msg("Patterns are different");
		return TST_FAILED;
	}

	return TST_SUCCESS;
}

static struct testcase testcase_1_px = {
	.x  = 1,
	.y0 = 1,
	.y1 = 1,

	.w = 3,
	.h = 3,
	.pixmap = {
		0, 0, 0,
		0, 1, 0,
		0, 0, 0,
	}
};

static struct testcase testcase_3_px_1 = {
	.x = 1,
	.y0 = 1,
	.y1 = 3,

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

static struct testcase testcase_3_px_2 = {
	.x  = 1,
	.y0 = 3,
	.y1 = 1,

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

static struct testcase testcase_clipping_1 = {
	.x  = 1,
	.y0 = -10000,
	.y1 = 10000,

	.w = 3,
	.h = 5,
	.pixmap = {
		0, 1, 0,
		0, 1, 0,
		0, 1, 0,
		0, 1, 0,
		0, 1, 0,
	}
};

static struct testcase testcase_clipping_2 = {
	.x  = 4,
	.y0 = -10000,
	.y1 = 10000,

	.w = 3,
	.h = 5,
	.pixmap = {
		0, 0, 0,
		0, 0, 0,
		0, 0, 0,
		0, 0, 0,
		0, 0, 0,
	}
};

static struct testcase testcase_clipping_3 = {
	.x  = -4,
	.y0 = -100000,
	.y1 = 100000,

	.w = 3,
	.h = 5,
	.pixmap = {
		0, 0, 0,
		0, 0, 0,
		0, 0, 0,
		0, 0, 0,
		0, 0, 0,
	}
};

static struct testcase testcase_xyh_1 = {
	.x = 1,
	.y = 1,
	.lh = 0,

	.flag = 1,

	.w = 3,
	.h = 3,
	.pixmap = {
		0, 0, 0,
		0, 0, 0,
		0, 0, 0,
	}
};

static struct testcase testcase_xyh_2 = {
	.x = 1,
	.y = 1,
	.lh = 2,

	.flag = 1,

	.w = 4,
	.h = 4,
	.pixmap = {
		0, 0, 0, 0,
		0, 1, 0, 0,
		0, 1, 0, 0,
		0, 0, 0, 0,
	}
};

static struct testcase testcase_xyh_clipp_1 = {
	.x = 1,
	.y = -10000,
	.lh = 20000,

	.flag = 1,

	.w = 3,
	.h = 3,
	.pixmap = {
		0, 1, 0,
		0, 1, 0,
		0, 1, 0,
	}
};

static struct testcase testcase_xyh_clipp_2 = {
	.x = 1,
	.y = 1,
	.lh = 200000,

	.flag = 1,

	.w = 3,
	.h = 3,
	.pixmap = {
		0, 0, 0,
		0, 1, 0,
		0, 1, 0,
	}
};

static struct testcase testcase_xyh_clipp_3 = {
	.x = -10000,
	.y = -10000,
	.lh = -1,

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
	.suite_name = "VLine Testsuite",
	.tests = {
		{.name = "VLine 1px",
		 .tst_fn = test_vline,
		 .data = &testcase_1_px},

		{.name = "VLine 3px 1",
		 .tst_fn = test_vline,
		 .data = &testcase_3_px_1},

		{.name = "VLine 3px 2",
		 .tst_fn = test_vline,
		 .data = &testcase_3_px_2},

		{.name = "VLine clipping 1",
		 .tst_fn = test_vline,
		 .data = &testcase_clipping_1},

		{.name = "VLine clipping 2",
		 .tst_fn = test_vline,
		 .data = &testcase_clipping_2},

		{.name = "VLine clipping 3",
		 .tst_fn = test_vline,
		 .data = &testcase_clipping_3},

		{.name = "VLineXYH 1",
		 .tst_fn = test_vline,
		 .data = &testcase_xyh_1},

		{.name = "VLineXYH 2",
		 .tst_fn = test_vline,
		 .data = &testcase_xyh_2},

		{.name = "VLineXYH clipping 1",
		 .tst_fn = test_vline,
		 .data = &testcase_xyh_clipp_1},

		{.name = "VLineXYH clipping 2",
		 .tst_fn = test_vline,
		 .data = &testcase_xyh_clipp_2},

		{.name = "VLineXYH clipping 3",
		 .tst_fn = test_vline,
		 .data = &testcase_xyh_clipp_3},

		{.name = NULL}
	}
};
