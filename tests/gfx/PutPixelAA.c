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
#include <gfx/GP_PutPixelAA.h>

#include "tst_test.h"

#include "common.h"

struct testcase {
	/* pixel description */
	gp_coord x;
	gp_coord y;

	/* expected result */
	gp_size w, h;
	const char pixmap[];
};

static int test_pixel(const struct testcase *t)
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

	gp_putpixel_aa(c, t->x, t->y, 0xff);

	err = compare_buffers(t->pixmap, c);

	if (err)
		return TST_FAILED;

	return TST_SUCCESS;
}

static struct testcase testcase_pixel_center = {
	.x = (1<<8),
	.y = (1<<8),

	.w = 3,
	.h = 3,

	.pixmap = {
		0x00, 0x00, 0x00,
		0x00, 0xff, 0x00,
		0x00, 0x00, 0x00,
	}
};

static struct testcase testcase_pixel_hcenter = {
	.x = (1<<8),
	.y = (1<<8) + (1<<7),

	.w = 3,
	.h = 4,

	.pixmap = {
		0x00, 0x00, 0x00,
		0x00, 0x80, 0x00,
		0x00, 0x80, 0x00,
		0x00, 0x00, 0x00,
	}
};

static struct testcase testcase_pixel_vcenter = {
	.x = (1<<8) + (1<<7),
	.y = (1<<8),

	.w = 4,
	.h = 3,

	.pixmap = {
		0x00, 0x00, 0x00, 0x00,
		0x00, 0x80, 0x80, 0x00,
		0x00, 0x00, 0x00, 0x00,
	}
};

static struct testcase testcase_pixel = {
	.x = (1<<8) + (1<<7),
	.y = (1<<8) + (1<<7),

	.w = 4,
	.h = 4,

	.pixmap = {
		0x00, 0x00, 0x00, 0x00,
		0x00, 0x40, 0x40, 0x00,
		0x00, 0x40, 0x40, 0x00,
		0x00, 0x00, 0x00, 0x00,
	}
};

const struct tst_suite tst_suite = {
	.suite_name = "PutPixelAA Testsuite",
	.tests = {
		{.name = "PutPixelAA center",
		 .tst_fn = test_pixel,
		 .data = &testcase_pixel_center},

		{.name = "PutPixelAA hcenter",
		 .tst_fn = test_pixel,
		 .data = &testcase_pixel_hcenter},

		{.name = "PutPixelAA vcenter",
		 .tst_fn = test_pixel,
		 .data = &testcase_pixel_vcenter},

		{.name = "PutPixelAA",
		 .tst_fn = test_pixel,
		 .data = &testcase_pixel},

		{.name = NULL}
	}
};
