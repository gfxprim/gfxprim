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

/*

  Clipped Blit tests.

 */
#include <errno.h>

#include <core/GP_Pixmap.h>
#include <core/GP_Blit.h>
#include <core/GP_Fill.h>

#include "tst_test.h"
#include "tst_preload.h"

#include "../filters/common.c"

struct clipped_test {
	/* size of the contexes */
	GP_Size src_w, src_h;
	GP_Size dst_w, dst_h;

	/* Source rectangle */
	GP_Coord x0, y0;
	GP_Size w0, h0;

	/* Destination offset */
	GP_Coord x1, y1;

	/* If set, is used to check assert correct result */
	const char *dst;
};

static int clipped_test(struct clipped_test *test)
{
	int ret = TST_SUCCESS;
	GP_Pixmap *src, *dst;

	src = GP_PixmapAlloc(test->src_w, test->src_h, GP_PIXEL_G8);
	dst = GP_PixmapAlloc(test->dst_w, test->dst_h, GP_PIXEL_G8);

	GP_Fill(src, 1);
	GP_Fill(dst, 0);

	if (!src || !dst) {
		tst_msg("GP_PixmapAlloc() failed");
		return TST_UNTESTED;
	}

	GP_BlitXYWH_Clipped(src, test->x0, test->y0, test->w0, test->h0,
	                    dst, test->x1, test->y1);

	if (test->dst) {
		if (compare_buffers(test->dst, dst))
			ret = TST_FAILED;
	}

	GP_PixmapFree(src);
	GP_PixmapFree(dst);

	return ret;
}

static int clipped_test_canaries(struct clipped_test *test)
{
	int ret;

	tst_malloc_canaries_set(MALLOC_CANARY_BEGIN);
	ret = clipped_test(test);

	tst_malloc_canaries_set(MALLOC_CANARY_END);
	ret |= clipped_test(test);

	tst_malloc_canaries_set(MALLOC_CANARY_OFF);

	return ret;
}

static struct clipped_test off_by_one_1 = {
	.src_w = 100,
	.src_h = 100,

	.dst_w = 100,
	.dst_h = 100,

	.x0 = 0,
	.y0 = 0,
	.w0 = 100,
	.h0 = 100,

	.x1 = 1,
	.y1 = 1,
};

static struct clipped_test off_by_one_2 = {
	.src_w = 100,
	.src_h = 100,

	.dst_w = 100,
	.dst_h = 100,

	.x0 = 0,
	.y0 = 0,
	.w0 = 100,
	.h0 = 100,

	.x1 = 1,
	.y1 = 0,
};

static struct clipped_test off_by_one_3 = {
	.src_w = 100,
	.src_h = 100,

	.dst_w = 100,
	.dst_h = 100,

	.x0 = 0,
	.y0 = 0,
	.w0 = 100,
	.h0 = 100,

	.x1 = 0,
	.y1 = 1,
};

static const char empty[400] = {0};

static struct clipped_test empty_src = {
	.src_w = 10,
	.src_h = 10,

	.dst_w = 20,
	.dst_h = 20,

	.x0 = 0,
	.y0 = 0,
	.w0 = 0,
	.h0 = 0,

	.x1 = 0,
	.y1 = 0,

	.dst = empty,
};

static struct clipped_test out_of_dst_1 = {
	.src_w = 10,
	.src_h = 10,

	.dst_w = 20,
	.dst_h = 20,

	.x0 = 0,
	.y0 = 0,
	.w0 = 10,
	.h0 = 10,

	.x1 = 20,
	.y1 = 0,

	.dst = empty,
};

static struct clipped_test out_of_dst_2 = {
	.src_w = 10,
	.src_h = 10,

	.dst_w = 20,
	.dst_h = 20,

	.x0 = 0,
	.y0 = 0,
	.w0 = 10,
	.h0 = 10,

	.x1 = 0,
	.y1 = 20,

	.dst = empty,
};

static struct clipped_test src_rect_out_of_src = {
	.src_w = 10,
	.src_h = 10,

	.dst_w = 20,
	.dst_h = 20,

	.x0 = -10,
	.y0 = -10,
	.w0 = 10,
	.h0 = 10,

	.x1 = 0,
	.y1 = 0,

	.dst = empty,
};

static struct clipped_test dst_neg_coords1 = {
	.src_w = 5,
	.src_h = 5,

	.dst_w = 5,
	.dst_h = 5,

	.x0 = 2,
	.y0 = 2,
	.w0 = 5,
	.h0 = 5,

	.x1 = -2,
	.y1 = -2,

	.dst = (const char []) {
		1, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
	}
};

static struct clipped_test dst_big_coords = {
	.src_w = 5,
	.src_h = 5,

	.dst_w = 5,
	.dst_h = 5,

	.x0 = 2,
	.y0 = 2,
	.w0 = 5,
	.h0 = 5,

	.x1 = 4,
	.y1 = 4,

	.dst = (const char []) {
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 1,
	}
};

static struct clipped_test dst_neg_coords2 = {
	.src_w = 10,
	.src_h = 10,

	.dst_w = 3,
	.dst_h = 3,

	.x0 = 0,
	.y0 = 0,
	.w0 = 8,
	.h0 = 8,

	.x1 = -5,
	.y1 = -5,

	.dst = (const char []) {
		1, 1, 1,
		1, 1, 1,
		1, 1, 1,
	}
};

static struct clipped_test dst_neg_coords3 = {
	.src_w = 12,
	.src_h = 12,

	.dst_w = 3,
	.dst_h = 3,

	.x0 = 0,
	.y0 = 0,
	.w0 = 10,
	.h0 = 10,

	.x1 = -8,
	.y1 = -8,

	.dst = (const char []) {
		1, 1, 0,
		1, 1, 0,
		0, 0, 0,
	}
};

static struct clipped_test src_inside = {
	.src_w = 3,
	.src_h = 3,

	.dst_w = 5,
	.dst_h = 5,

	.x0 = 0,
	.y0 = 0,
	.w0 = 3,
	.h0 = 3,

	.x1 = 1,
	.y1 = 1,

	.dst = (const char []) {
		0, 0, 0, 0, 0,
		0, 1, 1, 1, 0,
		0, 1, 1, 1, 0,
		0, 1, 1, 1, 0,
		0, 0, 0, 0, 0,
	}
};

const struct tst_suite tst_suite = {
	.suite_name = "Blit Clipped Testsuite",
	.tests = {
		{.name = "Empty src",
		 .tst_fn = clipped_test_canaries,
		 .data = &empty_src,
		 .flags = TST_CHECK_MALLOC},

		{.name = "Out of dst 1",
		 .tst_fn = clipped_test_canaries,
		 .data = &out_of_dst_1,
		 .flags = TST_CHECK_MALLOC},

		{.name = "Out of dst 2",
		 .tst_fn = clipped_test_canaries,
		 .data = &out_of_dst_2,
		 .flags = TST_CHECK_MALLOC},

		{.name = "Src rect out of src",
		 .tst_fn = clipped_test_canaries,
		 .data = &src_rect_out_of_src,
		 .flags = TST_CHECK_MALLOC},

		{.name = "Dst negative coords 1",
		 .tst_fn = clipped_test_canaries,
		 .data = &dst_neg_coords1,
		 .flags = TST_CHECK_MALLOC},

		{.name = "Dst negative coords 2",
		 .tst_fn = clipped_test_canaries,
		 .data = &dst_neg_coords2,
		 .flags = TST_CHECK_MALLOC},

		{.name = "Dst negative coords 3",
		 .tst_fn = clipped_test_canaries,
		 .data = &dst_neg_coords3,
		 .flags = TST_CHECK_MALLOC},

		{.name = "Dst big coords",
		 .tst_fn = clipped_test_canaries,
		 .data = &dst_big_coords,
		 .flags = TST_CHECK_MALLOC},

		{.name = "Src inside dst",
		 .tst_fn = clipped_test_canaries,
		 .data = &src_inside,
		 .flags = TST_CHECK_MALLOC},

		{.name = "Regression off by one 1",
		 .tst_fn = clipped_test_canaries,
		 .data = &off_by_one_1,
		 .flags = TST_CHECK_MALLOC},

		{.name = "Regression off by one 2",
		 .tst_fn = clipped_test_canaries,
		 .data = &off_by_one_2,
		 .flags = TST_CHECK_MALLOC},

		{.name = "Regression off by one 3",
		 .tst_fn = clipped_test_canaries,
		 .data = &off_by_one_3,
		 .flags = TST_CHECK_MALLOC},

		{.name = NULL},
	}
};
