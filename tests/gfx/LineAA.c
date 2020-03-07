// SPDX-License-Identifier: GPL-2.1-or-later
/*
 * Copyright (C) 2009-2013 Cyril Hrubis <metan@ucw.cz>
 */

#include <string.h>
#include <errno.h>
#include <sys/stat.h>

#include <core/gp_pixmap.h>
#include <gfx/gp_line_aa.h>

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

	c = gp_pixmap_alloc(t->w, t->h, GP_PIXEL_G8);

	if (c == NULL) {
		tst_err("Failed to allocate pixmap");
		return TST_UNTESTED;
	}

	/* zero the pixels buffer */
	memset(c->pixels, 0, c->w * c->h);

	gp_line_aa(c, t->x0, t->y0, t->x1, t->y1, 0xff);

	err = compare_buffers(t->pixmap, c);

	if (err)
		return TST_FAILED;

	return TST_SUCCESS;
}

static struct testcase testcase_line_1px = {
	.x0 = (1<<8) + (1<<7),
	.y0 = (1<<8) + (1<<7),
	.x1 = (1<<8) + (1<<7),
	.y1 = (1<<8) + (1<<7),

	.w = 3,
	.h = 3,

	.pixmap = {
		0x00, 0x00, 0x00,
		0x00, 0xff, 0x00,
		0x00, 0x00, 0x00,
	}
};

static struct testcase testcase_line_2px = {
	.x0 = (1<<8),
	.y0 = (1<<8),
	.x1 = (2<<8),
	.y1 = (2<<8),

	.w = 4,
	.h = 4,

	.pixmap = {
		0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00,
	}
};

static struct testcase testcase_line_2px_h = {
	.x0 = (1<<8),
	.y0 = (1<<8),
	.x1 = (2<<8),
	.y1 = (1<<8),

	.w = 4,
	.h = 4,

	.pixmap = {
		0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00,
	}
};

static struct testcase testcase_line_2px_v = {
	.x0 = (1<<8),
	.y0 = (1<<8),
	.x1 = (1<<8),
	.y1 = (2<<8),

	.w = 4,
	.h = 4,

	.pixmap = {
		0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00,
	}
};


const struct tst_suite tst_suite = {
	.suite_name = "LineAA Testsuite",
	.tests = {
		{.name = "LineAA 1px",
		 .tst_fn = test_line,
		 .data = &testcase_line_1px},

		{.name = "LineAA 2px",
		 .tst_fn = test_line,
		 .data = &testcase_line_2px},

		{.name = "LineAA 2px horizontal",
		 .tst_fn = test_line,
		 .data = &testcase_line_2px_h},

		{.name = "LineAA 2px vertical",
		 .tst_fn = test_line,
		 .data = &testcase_line_2px_v},

		{.name = NULL}
	}
};
