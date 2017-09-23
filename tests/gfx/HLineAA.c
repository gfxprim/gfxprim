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
#include <sys/stat.h>

#include <core/GP_Pixmap.h>
#include <gfx/GP_HLineAA.h>

#include "tst_test.h"

#include "common.h"

struct testcase {
	/* line description */
	GP_Coord x0;
	GP_Coord x1;
	GP_Coord y;

	/* expected result */
	GP_Size w, h;
	const char pixmap[];
};

static int test_line(const struct testcase *t)
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
	GP_HLineAA(c, t->x0, t->x1, t->y, 0xff);

	err = compare_buffers(t->pixmap, c);

	if (err)
		return TST_FAILED;

	return TST_SUCCESS;
}

static struct testcase testcase_line_len_0_1 = {
	.x0 = (1<<8),
	.x1 = (1<<8),
	.y  = (1<<8),

	.w = 3,
	.h = 3,

	.pixmap = {
		0x00, 0x00, 0x00,
		0x00, 0x00, 0x00,
		0x00, 0x00, 0x00,
	}
};

static struct testcase testcase_line_len_0_2 = {
	.x0 = (1<<8) + (1<<7),
	.x1 = (1<<8) + (1<<7),
	.y  = (1<<8),

	.w = 3,
	.h = 3,

	.pixmap = {
		0x00, 0x00, 0x00,
		0x00, 0x00, 0x00,
		0x00, 0x00, 0x00,
	}
};

static struct testcase testcase_line_len_0_3 = {
	.x0 = (1<<8),
	.x1 = (1<<8),
	.y  = (1<<8) + (1<<7),

	.w = 3,
	.h = 3,

	.pixmap = {
		0x00, 0x00, 0x00,
		0x00, 0x00, 0x00,
		0x00, 0x00, 0x00,
	}
};

static struct testcase testcase_line_len_0_4 = {
	.x0 = (1<<8) + (1<<7),
	.x1 = (1<<8) + (1<<7),
	.y  = (1<<8) + (1<<7),

	.w = 3,
	.h = 3,

	.pixmap = {
		0x00, 0x00, 0x00,
		0x00, 0x00, 0x00,
		0x00, 0x00, 0x00,
	}
};

static struct testcase testcase_line_len_0_5 = {
	.x0 = (1<<8) + (1<<3) + 1,
	.x1 = (1<<8) + (1<<3),
	.y  = (1<<8) + (1<<2),

	.w = 3,
	.h = 3,

	.pixmap = {
		0x00, 0x00, 0x00,
		0x00, 0x00, 0x00,
		0x00, 0x00, 0x00,
	}
};

static struct testcase testcase_line_len_05_1 = {
	.x0 = (1<<8),
	.x1 = (1<<8) + (1<<7),
	.y  = (1<<8),

	.w = 3,
	.h = 3,

	.pixmap = {
		0x00, 0x00, 0x00,
		0x00, 0x80, 0x00,
		0x00, 0x00, 0x00,
	}
};

static struct testcase testcase_line_len_05_2 = {
	.x0 = (1<<8),
	.x1 = (1<<8) + (1<<7),
	.y  = (1<<8) + (1<<7),

	.w = 3,
	.h = 4,

	.pixmap = {
		0x00, 0x00, 0x00,
		0x00, 0x40, 0x00,
		0x00, 0x40, 0x00,
		0x00, 0x00, 0x00,
	}
};

static struct testcase testcase_line_len_1_1 = {
	.x0 = (1<<8),
	.x1 = (2<<8),
	.y  = (1<<8),

	.w = 4,
	.h = 3,

	.pixmap = {
		0x00, 0x00, 0x00, 0x00,
		0x00, 0x80, 0x80, 0x00,
		0x00, 0x00, 0x00, 0x00,
	}
};

static struct testcase testcase_line_len_1_2 = {
	.x0 = (1<<8) + (1<<7),
	.x1 = (2<<8) + (1<<7),
	.y  = (1<<8),

	.w = 4,
	.h = 3,

	.pixmap = {
		0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0xff, 0x00,
		0x00, 0x00, 0x00, 0x00,
	}
};

static struct testcase testcase_line_len_1_3 = {
	.x0 = (1<<8),
	.x1 = (2<<8),
	.y  = (1<<8) + (1<<7),

	.w = 4,
	.h = 4,

	.pixmap = {
		0x00, 0x00, 0x00, 0x00,
		0x00, 0x40, 0x40, 0x00,
		0x00, 0x40, 0x40, 0x00,
		0x00, 0x00, 0x00, 0x00,
	}
};

static struct testcase testcase_line_len_1_4 = {
	.x0 = (1<<8) + (1<<7),
	.x1 = (2<<8) + (1<<7),
	.y  = (1<<8) + (1<<7),

	.w = 4,
	.h = 4,

	.pixmap = {
		0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x80, 0x00,
		0x00, 0x00, 0x80, 0x00,
		0x00, 0x00, 0x00, 0x00,
	}
};

static struct testcase testcase_line_len_4_1 = {
	.x0 = (1<<8),
	.x1 = (4<<8),
	.y  = (1<<8),

	.w = 6,
	.h = 3,

	.pixmap = {
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x80, 0xff, 0xff, 0x80, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	}
};

static struct testcase testcase_line_len_4_2 = {
	.x0 = (1<<8),
	.x1 = (4<<8),
	.y  = (1<<8) + (1<<7),

	.w = 6,
	.h = 4,

	.pixmap = {
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x40, 0x80, 0x80, 0x40, 0x00,
		0x00, 0x40, 0x80, 0x80, 0x40, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	}
};

const struct tst_suite tst_suite = {
	.suite_name = "HLineAA Testsuite",
	.tests = {
		{.name = "HLineAA len=0 1",
		 .tst_fn = test_line,
		 .data = &testcase_line_len_0_1},

		{.name = "HLineAA len=0 2",
		 .tst_fn = test_line,
		 .data = &testcase_line_len_0_2},

		{.name = "HLineAA len=0 3",
		 .tst_fn = test_line,
		 .data = &testcase_line_len_0_3},

		{.name = "HLineAA len=0 4",
		 .tst_fn = test_line,
		 .data = &testcase_line_len_0_4},

		{.name = "HLineAA len=0 5",
		 .tst_fn = test_line,
		 .data = &testcase_line_len_0_5},

		{.name = "HLineAA len=0.5 1",
		 .tst_fn = test_line,
		 .data = &testcase_line_len_05_1},

		{.name = "HLineAA len=0.5 2",
		 .tst_fn = test_line,
		 .data = &testcase_line_len_05_2},

		{.name = "LineAA len=1 1",
		 .tst_fn = test_line,
		 .data = &testcase_line_len_1_1},

		{.name = "LineAA len=1 2",
		 .tst_fn = test_line,
		 .data = &testcase_line_len_1_2},

		{.name = "LineAA len=1 3",
		 .tst_fn = test_line,
		 .data = &testcase_line_len_1_3},

		{.name = "LineAA len=1 4",
		 .tst_fn = test_line,
		 .data = &testcase_line_len_1_4},

		{.name = "LineAA len=4 1",
		 .tst_fn = test_line,
		 .data = &testcase_line_len_4_1},

		{.name = "LineAA len=4 2",
		 .tst_fn = test_line,
		 .data = &testcase_line_len_4_2},

		{.name = NULL}
	}
};
