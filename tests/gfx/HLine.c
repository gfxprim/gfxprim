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

#include <core/GP_Pixmap.h>
#include <gfx/GP_HLine.h>

#include "tst_test.h"

#include "common.h"

struct testcase {
	/* HLine description */
	GP_Coord x0;
	GP_Coord x1;
	GP_Coord y;

	GP_Coord x;
	GP_Size lw;

	int flag;

	/* expected result */
	GP_Size w, h;
	const char pixmap[];
};

static int test_hline(struct testcase *t)
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

	if (t->flag)
		GP_HLineXYW(c, t->x, t->y, t->lw, 1);
	else
		GP_HLine(c, t->x0, t->x1, t->y, 1);

	err = compare_buffers(t->pixmap, c);

	if (err) {
		tst_msg("Patterns are different");
		return TST_FAILED;
	}

	return TST_SUCCESS;
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
