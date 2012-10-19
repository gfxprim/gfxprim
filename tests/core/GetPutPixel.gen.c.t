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

%% extends "base.test.c.t"

%% block body

#include <stdio.h>

#include <core/GP_Context.h>
#include <core/GP_GetPutPixel.h>

#include "tst_test.h"

static void fill_context(GP_Context *c, GP_Pixel p)
{
	GP_Coord x, y;

	for (x = 0; x < c->w; x++)
		for (y = 0; y < c->h; y++)
			GP_PutPixel(c, x, y, p);
}

static int check_filled(GP_Context *c)
{
	GP_Coord x, y;
	GP_Pixel p;

	p = GP_GetPixel(c, 0, 0);

	for (x = 0; x < c->w; x++)
		for (y = 0; y < c->h; y++)
			if (p != GP_GetPixel(c, x, y)) {
				tst_report(0, "Pixels different %i %i", x, y);
				return 1;
			}
	
	return 0;
}

static int try_pattern(GP_Context *c, GP_Pixel p)
{
	fill_context(c, p);

	tst_report(0, "Filling pattern 0x%x", p);

	if (check_filled(c))
		return 1;

	return 0;
}

%% for pt in pixeltypes
%% if not pt.is_unknown()
static int GetPutPixel_{{ pt.name }}(void)
{
	GP_Context *c;
	int err = 0;

	c = GP_ContextAlloc(100, 100, GP_PIXEL_{{ pt.name }});

	if (c == NULL) {
		tst_report(0, "GP_ContextAlloc() failed");
		return TST_FAILED;
	}

	if (try_pattern(c, 0x55555555 & {{ 2 ** pt.pixelsize.size - 1}}))
		err++;
	
	if (try_pattern(c, 0xaaaaaaaa & {{ 2 ** pt.pixelsize.size - 1}}))
		err++;
	
	if (try_pattern(c, 0x0f0f0f0f & {{ 2 ** pt.pixelsize.size - 1}}))
		err++;
	
	if (try_pattern(c, 0xf0f0f0f0 & {{ 2 ** pt.pixelsize.size - 1}}))
		err++;

	GP_ContextFree(c);

	if (err)
		return TST_FAILED;

	return TST_SUCCESS;
}
%% endif
%% endfor

const struct tst_suite tst_suite = {
	.suite_name = "GetPutPixel Testsuite",
	.tests = {
%% for pt in pixeltypes
%% if not pt.is_unknown()
		{.name = "GetPutPixel {{ pt.name }}", 
		 .tst_fn = GetPutPixel_{{ pt.name }}},
%% endif
%% endfor
		
		{.name = NULL}
	}
};

%% endblock body
