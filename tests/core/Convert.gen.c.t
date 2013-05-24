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

#include <core/GP_Convert.h>

#include "tst_test.h"

/*
 * Returns black color for particular pixel type.
 */
static GP_Pixel get_black(GP_PixelType pixel_type)
{
	switch (pixel_type) {
%% for pt in pixeltypes
	case {{ pt.C_enum }}:
%%  if pt.is_cmyk()
%%   set K = pt.chans['K']
		/* Black in CMYK is full K rest zero */
		return {{ K.C_mask }};
%%  elif pt.is_alpha()
%%   set A = pt.chans['A']
		/* Black with Alpha channel is full A rest zero */
		return {{ A.C_mask }};
%%  else
		return 0;
%%  endif
%% endfor
	default:
		tst_msg("Invalid pixel type %i", pixel_type);
		exit(TST_INTERR);
	}
}

/*
 * Returns white color for particular pixel type.
 */
static GP_Pixel get_white(GP_PixelType pixel_type)
{
	switch (pixel_type) {
%% for pt in pixeltypes
	case {{ pt.C_enum }}:
%%  if pt.is_cmyk()
		/* White in CMYK is zero */
		return 0x0;
%%  elif pt.is_rgb()
%%   set R = pt.chans['R']
%%   set G = pt.chans['G']
%%   set B = pt.chans['B']
%%   if pt.is_alpha()
%%    set A = pt.chans['A']
		/* White in RGBA */
		return {{ A.C_mask }} | {{ R.C_mask }} | {{ G.C_mask }} | {{ B.C_mask }};
%%   else
		/* Plain old RGB */
		return {{ R.C_mask }} | {{ G.C_mask }} | {{ B.C_mask }};
%%   endif
%%  elif pt.is_gray()
%%   set V = pt.chans['V']
%%   if pt.is_alpha()
%%    set A = pt.chans['A']
		/* Grayscale with Alpha */
		return {{ V.C_mask }} | {{ A.C_mask }};
%%   else
		/* Grayscale */
		return {{ V.C_mask }};
%%   endif
%%  else
		tst_msg("FIXME: Unsupported conversion to %s",
		        GP_PixelTypeName(pixel_type));
		exit(TST_INTERR);
%%  endif
%% endfor
	default:
		tst_msg("Invalid pixel type %i", pixel_type);
		exit(TST_INTERR);
	}
}

/*
 * Returns 50% gray color for particular pixel type.
 */
GP_Pixel get_gray(GP_PixelType pixel_type)
{
	switch (pixel_type) {
%% for pt in pixeltypes
	case {{ pt.C_enum }}:
%%  if pt.is_cmyk()
%%   set K = pt.chans['K']
		/* Gray in CMYK modifies K */
		return {{ hex(round(K.max / 2.00)) }}{{ K.C_shift }};
%%  elif pt.is_rgb()
%%   set R = pt.chans['R']
%%   set G = pt.chans['G']
%%   set B = pt.chans['B']
%%   if pt.is_alpha()
%%    set A = pt.chans['A']
		/* Gray in RGBA */
		return {{ A.C_mask }} |
		       ({{ hex(round(R.max / 2.00)) }}{{ R.C_shift }}) |
		       ({{ hex(round(G.max / 2.00)) }}{{ G.C_shift }}) |
		       ({{ hex(round(B.max / 2.00)) }}{{ B.C_shift }});
%%   else
		/* Gray Plain old RGB */
		return ({{ hex(round(R.max / 2.00)) }}{{ R.C_shift }}) |
		       ({{ hex(round(G.max / 2.00)) }}{{ G.C_shift }}) |
		       ({{ hex(round(B.max / 2.00)) }}{{ B.C_shift }});
%%   endif
%%  elif pt.is_gray()
%%   set V = pt.chans['V']
%%   if pt.is_alpha()
%%    set A = pt.chans['A']
		/* Gray in Grayscale with Alpha */
		return {{ A.C_mask }} |
		       ({{ hex(round(V.max / 2.00)) }}{{ V.C_shift }});
%%   else
		/* Grayscale */
		return {{ hex(round(V.max / 2.00)) }}{{ V.C_shift }};
%%   endif
%%  else
		tst_msg("FIXME: Unsupported conversion to %s",
		        GP_PixelTypeName(pixel_type));
		exit(TST_INTERR);
%%  endif
%% endfor
	default:
		tst_msg("Invalid pixel type %i", pixel_type);
		exit(TST_INTERR);
	}
}

/*
 * Returns red color for particular pixel type.
 */
static GP_Pixel get_red(GP_PixelType pixel_type)
{
	switch (pixel_type) {
%% for pt in pixeltypes
	case {{ pt.C_enum }}:
%%  if pt.is_cmyk()
%%   set M = pt.chans['M']
%%   set Y = pt.chans['Y']
		/* Red in CMYK is full M and Y rest zero */
		return {{ M.C_mask }} | {{ Y.C_mask }};
%%  elif pt.is_rgb()
%%   set R = pt.chans['R']
%%   if pt.is_alpha()
%%    set A = pt.chans['A']
		/* Red with Alpha channel is full Alpha and R rest zero */
		return {{ A.C_mask }} | {{ R.C_mask }};
%%   else
		/* Plain old RGB */
		return {{ R.C_mask }};
%%   endif
%%  elif pt.is_gray()
%%   set V = pt.chans['V']
%%   if pt.is_alpha()
%%    set A = pt.chans['A']
		/* Grayscale with Alpha channel is full Alpha + 1/3 Gray */
		return ({{ hex(V.max // 3)}}{{ V.C_shift }}) | {{ A.C_mask }};
%%   else
		/* Grayscale is 1/3 Gray */
		return {{ hex(V.max // 3) }}{{ V.C_shift }};
%%   endif
%%  else
		tst_msg("FIXME: Unsupported conversion to %s",
		        GP_PixelTypeName(pixel_type));
		exit(TST_INTERR);
%%  endif
%% endfor
	default:
		tst_msg("Invalid pixel type %i", pixel_type);
		exit(TST_INTERR);
	}
}

%% macro gen_convert_and_check(test_name, in_name, out_name)
static int convert_and_check_{{ test_name }}_{{ in_name }}_to_{{ out_name }}(void)
{
	GP_Pixel out = 0;
	GP_Pixel in = get_{{ test_name }}(GP_PIXEL_{{ in_name }});
	GP_Pixel out_exp = get_{{ test_name }}(GP_PIXEL_{{ out_name }});

	tst_msg("{{ in_name }} %08x -> {{ out_name }} %08x", in, out_exp);

	GP_Pixel_{{ in_name }}_TO_{{ out_name }}(in, out);

	if (out_exp != out) {
		tst_msg("Pixels are different have %08x, expected %08x",
		        out, out_exp);
		return TST_FAILED;
	}

	return TST_SUCCESS;
}
%% endmacro

%% macro gen_converts()
%%  for pt1 in pixeltypes
%%   if not pt1.is_unknown() and not pt1.is_palette()
%%    if pt1.name not in ['RGB888', 'RGBA8888']
{#- White -#}
{{ gen_convert_and_check('white', pt1.name, 'RGB888') }}
{{ gen_convert_and_check('white', pt1.name, 'RGBA8888') }}
{{ gen_convert_and_check('white', 'RGB888', pt1.name) }}
{{ gen_convert_and_check('white', 'RGBA8888', pt1.name) }}
{#- Black -#}
{{ gen_convert_and_check('black', pt1.name, 'RGB888') }}
{{ gen_convert_and_check('black', pt1.name, 'RGBA8888') }}
{{ gen_convert_and_check('black', 'RGB888', pt1.name) }}
{{ gen_convert_and_check('black', 'RGBA8888', pt1.name) }}
{#- Grayscale -#}
{#
%%    if pt1.name not in ['G1']
{{ gen_convert_and_check('gray', pt1.name, 'RGB888') }}
{{ gen_convert_and_check('gray', pt1.name, 'RGBA8888') }}
%%    endif
{{ gen_convert_and_check('gray', 'RGB888', pt1.name) }}
{{ gen_convert_and_check('gray', 'RGBA8888', pt1.name) }}
#}
{#- Red -#}
%%    if not pt1.is_gray()
{{ gen_convert_and_check('red', pt1.name, 'RGB888') }}
{{ gen_convert_and_check('red', pt1.name, 'RGBA8888') }}
%%    endif
{{ gen_convert_and_check('red', 'RGB888', pt1.name) }}
{{ gen_convert_and_check('red', 'RGBA8888', pt1.name) }}
%%    endif
%%   endif
%%  endfor
%% endmacro

{{ gen_converts() }}

%% macro gen_suite_entry(name, from, to)
		{.name = "Convert {{ name }} {{ from }} -> {{ to }}", 
		 .tst_fn = convert_and_check_{{ name }}_{{ from }}_to_{{ to }}},
%% endmacro

const struct tst_suite tst_suite = {
	.suite_name = "Pixel Conversions Testsuite",
	.tests = {
%% for pt1 in pixeltypes
%%  if not pt1.is_unknown() and not pt1.is_palette()
%%   if pt1.name not in ['RGB888', 'RGBA8888']
{#- White -#}
{{ gen_suite_entry('white', pt1.name, 'RGB888') }}
{{ gen_suite_entry('white', pt1.name, 'RGBA8888') }}
{{ gen_suite_entry('white', 'RGB888', pt1.name) }}
{{ gen_suite_entry('white', 'RGBA8888', pt1.name) }}
{#- Black -#}
{{ gen_suite_entry('black', pt1.name, 'RGB888') }}
{{ gen_suite_entry('black', pt1.name, 'RGBA8888') }}
{{ gen_suite_entry('black', 'RGB888', pt1.name) }}
{{ gen_suite_entry('black', 'RGBA8888', pt1.name) }}
{#- Gray -#}
{#
%%    if pt1.name not in ['G1']
{{ gen_suite_entry('gray', pt1.name, 'RGB888') }}
{{ gen_suite_entry('gray', pt1.name, 'RGBA8888') }}
%%    endif
{{ gen_suite_entry('gray', 'RGB888', pt1.name) }}
{{ gen_suite_entry('gray', 'RGBA8888', pt1.name) }}
#}
{#- Red -#}
%%    if not pt1.is_gray()
{{ gen_suite_entry('red', pt1.name, 'RGB888') }}
{{ gen_suite_entry('red', pt1.name, 'RGBA8888') }}
%%    endif
{{ gen_suite_entry('red', 'RGB888', pt1.name) }}
{{ gen_suite_entry('red', 'RGBA8888', pt1.name) }}
%%   endif
%%  endif
%% endfor
		{.name = NULL}
	}
};

%% endblock body
