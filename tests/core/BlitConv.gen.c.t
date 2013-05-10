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

%% extends "base.test.c.t"

%% block body

#include <stdio.h>

#include <core/GP_Context.h>
#include <core/GP_Convert.h>
#include <core/GP_GetPutPixel.h>
#include <core/GP_Blit.h>

#include "tst_test.h"

static void fill_context(GP_Context *c, GP_Pixel p)
{
	GP_Coord x, y;

	for (x = 0; x < (GP_Coord)c->w; x++)
		for (y = 0; y < (GP_Coord)c->h; y++)
			GP_PutPixel(c, x, y, p);
}

static void mess_context(GP_Context *c)
{
	GP_Coord y;
	unsigned int i;

	for (y = 0; y < (GP_Coord)c->h; y++) {
		uint8_t *row = GP_PIXEL_ADDR(c, 0, y);
		for (i = 0; i < c->bytes_per_row; i++) {
			row[i] = y ^ i;
		}
	}
}

static int check_filled(GP_Context *c, GP_Pixel p)
{
	GP_Coord x, y;
	GP_Pixel pc;

	for (x = 0; x < (GP_Coord)c->w; x++) {
		for (y = 0; y < (GP_Coord)c->h; y++) {
			pc = GP_GetPixel(c, x, y);
			if (p != pc) {
				tst_msg("Pixels different %08x %08x", p, pc);
				return 1;
			}
		}
	}
	
	return 0;
}

%% for pt1 in pixeltypes
%%  if not pt1.is_unknown() and not pt1.is_palette()
%%   for pt2 in pixeltypes
%%    if not pt2.is_unknown() and not pt2.is_palette()
static int blit_black_{{ pt1.name }}_to_{{ pt2.name }}(void)
{
	GP_Context *src = GP_ContextAlloc(100, 100, GP_PIXEL_{{ pt1.name }});
	GP_Context *dst = GP_ContextAlloc(100, 100, GP_PIXEL_{{ pt2.name }});

	if (src == NULL || dst == NULL) {
		GP_ContextFree(src);
		GP_ContextFree(dst);
		tst_msg("Malloc failed :(");
		return TST_UNTESTED;
	}

	/* Fill source with black, destination with pseudo random mess */
%%     if pt1.is_alpha()
	GP_Pixel black_src = GP_RGBAToContextPixel(0, 0, 0, 0xff, src);
%%     else
	GP_Pixel black_src = GP_RGBToContextPixel(0, 0, 0, src);
%%     endif
%%     if pt2.is_alpha()
	GP_Pixel black_dst = GP_RGBAToContextPixel(0, 0, 0, 0xff, src);
%%     else
	GP_Pixel black_dst = GP_RGBToContextPixel(0, 0, 0, dst);
%%     endif

	fill_context(src, black_src);
	mess_context(dst);

	GP_Blit(src, 0, 0, src->w, src->h, dst, 0, 0);

	if (check_filled(dst, black_dst))
		return TST_FAILED;

	return TST_SUCCESS;
}

static int blit_white_{{ pt1.name }}_to_{{ pt2.name }}(void)
{
	GP_Context *src = GP_ContextAlloc(100, 100, GP_PIXEL_{{ pt1.name }});
	GP_Context *dst = GP_ContextAlloc(100, 100, GP_PIXEL_{{ pt2.name }});

	if (src == NULL || dst == NULL) {
		GP_ContextFree(src);
		GP_ContextFree(dst);
		tst_msg("Malloc failed :(");
		return TST_UNTESTED;
	}

	/* Fill source with white, destination with pseudo random mess */
%%     if pt1.is_alpha()
	GP_Pixel white_src = GP_RGBAToContextPixel(0xff, 0xff, 0xff, 0xff, src);
%%     else
	GP_Pixel white_src = GP_RGBToContextPixel(0xff, 0xff, 0xff, src);
%%     endif
%%     if pt2.is_alpha()
	GP_Pixel white_dst = GP_RGBAToContextPixel(0xff, 0xff, 0xff, 0xff, src);
%%     else
	GP_Pixel white_dst = GP_RGBToContextPixel(0xff, 0xff, 0xff, dst);
%%     endif

	fill_context(src, white_src);
	mess_context(dst);

	GP_Blit(src, 0, 0, src->w, src->h, dst, 0, 0);

	if (check_filled(dst, white_dst))
		return TST_FAILED;

	return TST_SUCCESS;
}

%%    endif
%%   endfor
%%  endif
%% endfor

const struct tst_suite tst_suite = {
	.suite_name = "Blit Conversions Testsuite",
	.tests = {
%% for pt1 in pixeltypes
%%  if not pt1.is_unknown() and not pt1.is_palette()
%%   for pt2 in pixeltypes
%%    if not pt2.is_unknown() and not pt2.is_palette()
		{.name = "blit black {{ pt1.name }} to {{ pt2.name }}", 
		 .tst_fn = blit_black_{{ pt1.name }}_to_{{ pt2.name }}},
		{.name = "blit white {{ pt1.name }} to {{ pt2.name }}", 
		 .tst_fn = blit_white_{{ pt1.name }}_to_{{ pt2.name }}},
%%    endif
%%   endfor
%%  endif
%% endfor
		{.name = NULL}
	}
};

%% endblock body
