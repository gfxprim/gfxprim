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
 * Copyright (C) 2009-2012 Cyril Hrubis <metan@ucw.cz>                       *
 *                                                                           *
 *****************************************************************************/

#include <string.h>
#include <errno.h>
#include <sys/stat.h>

#include <core/gp_pixmap.h>
#include <gfx/GP_CircleSeg.h>

#include "tst_test.h"

#include "common.h"

static const char circle_empty_11x11[] = {
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};

static const char circle_r_0_11x11[] = {
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};

static const char circle_r_1_s_1_11x11[] = {
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};

static const char circle_r_1_s_2_11x11[] = {
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};

static const char circle_r_1_s_3_11x11[] = {
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};

static const char circle_r_1_s_4_11x11[] = {
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};

static const char circle_r_2_s_1_2_11x11[] = {
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0,
	0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0,
	0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};

static const char circle_r_2_s_1_3_11x11[] = {
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0,
	0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0,
	0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};

static const char circle_r_2_s_1_4_11x11[] = {
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0,
	0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};

static int test_circle(const char *pattern, gp_size w, gp_size h,
                       gp_coord x, gp_coord y, const int r, uint8_t seg_flag)
{
	gp_pixmap *c;
	int err;

	c = gp_pixmap_alloc(w, h, GP_PIXEL_G8);

	if (c == NULL) {
		tst_err("Failed to allocate pixmap");
		return TST_UNTESTED;
	}

	/* zero the pixels buffer */
	memset(c->pixels, 0, c->w * c->h);

	gp_circle_seg(c, x, y, r, seg_flag, 1);

	err = compare_buffers(pattern, c);

	if (err) {
		tst_msg("Patterns are different");
		return TST_FAILED;
	}

	return TST_SUCCESS;
}

static int test_circle_r_0_s_1(void)
{
	return test_circle(circle_r_0_11x11, 11, 11, 5, 5, 0, GP_CIRCLE_SEG1);
}

static int test_circle_r_0_s_0(void)
{
	return test_circle(circle_empty_11x11, 11, 11, 5, 5, 0, 0);
}

static int test_circle_r_1_s_1(void)
{
	return test_circle(circle_r_1_s_1_11x11, 11, 11, 5, 5, 1, GP_CIRCLE_SEG1);
}

static int test_circle_r_1_s_2(void)
{
	return test_circle(circle_r_1_s_2_11x11, 11, 11, 5, 5, 1, GP_CIRCLE_SEG2);
}

static int test_circle_r_1_s_3(void)
{
	return test_circle(circle_r_1_s_3_11x11, 11, 11, 5, 5, 1, GP_CIRCLE_SEG3);
}

static int test_circle_r_1_s_4(void)
{
	return test_circle(circle_r_1_s_4_11x11, 11, 11, 5, 5, 1, GP_CIRCLE_SEG4);
}

static int test_circle_r_2_s_1_2(void)
{
	return test_circle(circle_r_2_s_1_2_11x11, 11, 11, 5, 5, 2,
	                   GP_CIRCLE_SEG1|GP_CIRCLE_SEG2);
}

static int test_circle_r_2_s_1_3(void)
{
	return test_circle(circle_r_2_s_1_3_11x11, 11, 11, 5, 5, 2,
	                   GP_CIRCLE_SEG1|GP_CIRCLE_SEG3);
}

static int test_circle_r_2_s_1_4(void)
{
	return test_circle(circle_r_2_s_1_4_11x11, 11, 11, 5, 5, 2,
	                   GP_CIRCLE_SEG1|GP_CIRCLE_SEG4);
}

static int test_circle_r__2_s_1_3(void)
{
	return test_circle(circle_r_2_s_1_3_11x11, 11, 11, 5, 5, -2,
	                   GP_CIRCLE_SEG1|GP_CIRCLE_SEG3);
}

const struct tst_suite tst_suite = {
	.suite_name = "GFX CircleSeg Testsuite",
	.tests = {
		{.name = "CircleSeg Seg0 r=0", .tst_fn = test_circle_r_0_s_0},
		{.name = "CircleSeg Seg1 r=0", .tst_fn = test_circle_r_0_s_1},
		{.name = "CircleSeg Seg1 r=1", .tst_fn = test_circle_r_1_s_1},
		{.name = "CircleSeg Seg2 r=1", .tst_fn = test_circle_r_1_s_2},
		{.name = "CircleSeg Seg3 r=1", .tst_fn = test_circle_r_1_s_3},
		{.name = "CircleSeg Seg4 r=1", .tst_fn = test_circle_r_1_s_4},
		{.name = "CircleSeg Seg1|2 r=2", .tst_fn = test_circle_r_2_s_1_2},
		{.name = "CircleSeg Seg1|3 r=2", .tst_fn = test_circle_r_2_s_1_3},
		{.name = "CircleSeg Seg1|4 r=2", .tst_fn = test_circle_r_2_s_1_4},
		{.name = "CircleSeg Seg1|3 r=-2", .tst_fn = test_circle_r__2_s_1_3},
		{.name = NULL}
	}
};
