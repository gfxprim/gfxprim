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

#include <core/GP_Context.h>
#include <gfx/GP_Gfx.h>

#include "tst_test.h"

static int bench_line(GP_PixelType type)
{
	GP_Context *img = GP_ContextAlloc(800, 600, type);

	if (img == NULL) {
		tst_report(0, "Malloc failed");
		return TST_INTERR;
	}

	unsigned int i;

	for (i = 0; i < 100000; i++) {
		GP_Line(img, 0 + i % 100, 0 - i % 100,
		        800 - i%200, 600 + i%200, i % 0xff);
	}

	return TST_SUCCESS;
}

static int bench_line_1bpp(void)
{
	return bench_line(GP_PIXEL_G1);
}

static int bench_line_2bpp(void)
{
	return bench_line(GP_PIXEL_G2);
}

static int bench_line_4bpp(void)
{
	return bench_line(GP_PIXEL_G4);
}

static int bench_line_8bpp(void)
{
	return bench_line(GP_PIXEL_G8);
}

static int bench_line_32bpp(void)
{
	return bench_line(GP_PIXEL_RGB888);
}

static int bench_circle(GP_PixelType type)
{
	GP_Context *img = GP_ContextAlloc(800, 600, type);

	if (img == NULL) {
		tst_report(0, "Malloc failed");
		return TST_INTERR;
	}

	unsigned int i;

	for (i = 0; i < 100000; i++) {
		GP_Circle(img, img->w/2, img->h/2, i % 1000, i%0xff);
	}

	return TST_SUCCESS;
}

static int bench_circle_1bpp(void)
{
	return bench_circle(GP_PIXEL_G1);
}

static int bench_circle_2bpp(void)
{
	return bench_circle(GP_PIXEL_G2);
}

static int bench_circle_4bpp(void)
{
	return bench_circle(GP_PIXEL_G4);
}

static int bench_circle_8bpp(void)
{
	return bench_circle(GP_PIXEL_G8);
}

static int bench_circle_32bpp(void)
{
	return bench_circle(GP_PIXEL_RGB888);
}

const struct tst_suite tst_suite = {
	.suite_name = "GFX Benchmark",
	.tests = {
		{.name = "Line 1BPP", .tst_fn = bench_line_1bpp,
	         .bench_iter = 10},
		{.name = "Line 2BPP", .tst_fn = bench_line_2bpp,
	         .bench_iter = 10},
		{.name = "Line 4BPP", .tst_fn = bench_line_4bpp,
	         .bench_iter = 10},
		{.name = "Line 8BPP", .tst_fn = bench_line_8bpp,
	         .bench_iter = 10},
		{.name = "Line 32BPP", .tst_fn = bench_line_32bpp,
	         .bench_iter = 10},
		
		{.name = "Circle 1BPP", .tst_fn = bench_circle_1bpp,
	         .bench_iter = 10},
		{.name = "Circle 2BPP", .tst_fn = bench_circle_2bpp,
	         .bench_iter = 10},
		{.name = "Circle 4BPP", .tst_fn = bench_circle_4bpp,
	         .bench_iter = 10},
		{.name = "Circle 8BPP", .tst_fn = bench_circle_8bpp,
	         .bench_iter = 10},
		{.name = "Circle 32BPP", .tst_fn = bench_circle_32bpp,
	         .bench_iter = 10},
		
		{.name = NULL},
	}
};
