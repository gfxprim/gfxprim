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

{% block descr %}Blit conversions tests.{% endblock %}

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

static GP_Pixel rgb_to_pixel(int r, int g, int b, GP_Context *c)
{
	if (GP_PixelHasFlags(c->pixel_type, GP_PIXEL_HAS_ALPHA))
		return GP_RGBAToContextPixel(r, g, b, 0xff, c);

	return GP_RGBToContextPixel(r, g, b, c);
}

%% macro gen_blit(name, r, g, b, pt1, pt2)
static int blit_{{ name }}_{{ pt1.name }}_to_{{ pt2.name }}(void)
{
	GP_Context *src = GP_ContextAlloc(100, 100, GP_PIXEL_{{ pt1.name }});
	GP_Context *dst = GP_ContextAlloc(100, 100, GP_PIXEL_{{ pt2.name }});

	if (src == NULL || dst == NULL) {
		GP_ContextFree(src);
		GP_ContextFree(dst);
		tst_msg("Malloc failed :(");
		return TST_UNTESTED;
	}

	/* Fill source with color, destination with pseudo random mess */
	GP_Pixel pix_src = rgb_to_pixel({{ r }}, {{ g }}, {{ b }}, src);
	GP_Pixel pix_dst = rgb_to_pixel({{ r }}, {{ g }}, {{ b }}, dst);

        tst_msg("pixel_src=%08x pixel_dst=%08x", pix_src, pix_dst);

	fill_context(src, pix_src);
	mess_context(dst);

	GP_Blit(src, 0, 0, src->w, src->h, dst, 0, 0);

	if (check_filled(dst, pix_dst))
		return TST_FAILED;

	return TST_SUCCESS;
}
%% endmacro

%% macro blit_color(name, r, g, b)
%%  for pt1 in pixeltypes
%%   if not pt1.is_unknown() and not pt1.is_palette()
%%    for pt2 in pixeltypes
%%     if not pt2.is_unknown() and not pt2.is_palette()
{{ gen_blit(name, r, g, b, pt1, pt2) }}
%%     endif
%%    endfor
%%   endif
%%  endfor
%% endmacro

{{ blit_color('black', '0x00', '0x00', '0x00') }}
{{ blit_color('white', '0xff', '0xff', '0xff') }}

%% macro blit_equal_pixel(name, r, g, b)
%%  for pt1 in pixeltypes
%%   if not pt1.is_unknown() and not pt1.is_palette()
{{ gen_blit(name, r, g, b, pt1, pt1) }}
%%   endif
%%  endfor
%% endmacro

{{ blit_equal_pixel('equal_pixel', '0x0f', '0xff', '0x00') }}

%% macro gen_blit2(name, r, g, b, pname1, pname2)
{{ gen_blit(name, r, g, b, pixeltypes_dict[pname1], pixeltypes_dict[pname2]) }}
%% endmacro

{{ gen_blit2('red', '0xff', '0x00', '0x00', 'RGB888', 'CMYK8888') }}
{{ gen_blit2('green', '0x00', '0xff', '0x00', 'RGB888', 'CMYK8888') }}
{{ gen_blit2('blue', '0x00', '0x00', '0xff', 'RGB888', 'CMYK8888') }}
{{ gen_blit2('gray', '0xef', '0xef', '0xef', 'RGB888', 'CMYK8888') }}

{{ gen_blit2('red', '0xff', '0x00', '0x00', 'CMYK8888', 'RGB888') }}
{{ gen_blit2('green', '0x00', '0xff', '0x00', 'CMYK8888', 'RGB888') }}
{{ gen_blit2('blue', '0x00', '0x00', '0xff', 'CMYK8888', 'RGB888') }}
{{ gen_blit2('gray', '0xef', '0xef', '0xef', 'CMYK8888', 'RGB888') }}


%% macro gen_suite_entry(name, from, to)
		{.name = "Blit {{ from }} to {{ to }}", 
		 .tst_fn = blit_{{ name }}_{{ from }}_to_{{ to }}},
%% endmacro

const struct tst_suite tst_suite = {
	.suite_name = "Blit Conversions Testsuite",
	.tests = {
%% for pt1 in pixeltypes
%%  if not pt1.is_unknown() and not pt1.is_palette()
%%   for pt2 in pixeltypes
%%    if not pt2.is_unknown() and not pt2.is_palette()
		{.name = "Blit black {{ pt1.name }} to {{ pt2.name }}", 
		 .tst_fn = blit_black_{{ pt1.name }}_to_{{ pt2.name }}},
		{.name = "Blit white {{ pt1.name }} to {{ pt2.name }}", 
		 .tst_fn = blit_white_{{ pt1.name }}_to_{{ pt2.name }}},
%%    endif
%%   endfor
%%  endif
%% endfor
%% for pt1 in pixeltypes
%%  if not pt1.is_unknown() and not pt1.is_palette()
		{.name = "Blit {{ pt1.name }} to {{ pt1.name }}", 
		 .tst_fn = blit_equal_pixel_{{ pt1.name }}_to_{{ pt1.name }}},
%%  endif
%% endfor

{{ gen_suite_entry('red', 'RGB888', 'CMYK8888') }}
{{ gen_suite_entry('green', 'RGB888', 'CMYK8888') }}
{{ gen_suite_entry('blue', 'RGB888', 'CMYK8888') }}
{{ gen_suite_entry('gray', 'RGB888', 'CMYK8888') }}

{{ gen_suite_entry('red', 'CMYK8888', 'RGB888') }}
{{ gen_suite_entry('green', 'CMYK8888', 'RGB888') }}
{{ gen_suite_entry('blue', 'CMYK8888', 'RGB888') }}
{{ gen_suite_entry('gray', 'CMYK8888', 'RGB888') }}

		{.name = NULL}
	}
};

%% endblock body
