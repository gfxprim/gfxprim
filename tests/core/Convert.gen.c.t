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
	case GP_PIXEL_{{ pt.name }}:
%%  if pt.is_cmyk()
%%   set K = pt.chans['K']
		/* Black in CMYK is full K rest zero */
		return {{ hex((2 ** K[2] - 1) * (2 ** K[1]))}};
%%  elif pt.is_alpha()
%%   set A = pt.chans['A']
		/* Black with Alpha channel is full A rest zero */
		return {{ hex((2 ** A[2] - 1) * (2 ** A[1]))}};
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
 * Returns red color for particular pixel type.
 */
static GP_Pixel get_red(GP_PixelType pixel_type)
{
	switch (pixel_type) {
%% for pt in pixeltypes
	case GP_PIXEL_{{ pt.name }}:
%%  if pt.is_cmyk()
%%   set M = pt.chans['M']
%%   set Y = pt.chans['Y']
		/* Red in CMYK is full M and Y rest zero */
		return {{ hex((2 ** M[2] - 1) * (2 ** M[1]))}}
		       | {{ hex((2 ** Y[2] - 1) * (2 ** Y[1]))}};
%%  elif pt.is_rgb()
%%   set R = pt.chans['R']
%%   if pt.is_alpha()
%%    set A = pt.chans['A']
		/* Red with Alpha channel is full Alpha and R rest zero */
		return {{ hex((2 ** A[2] - 1) * (2 ** A[1]))}}
		       | {{ hex((2 ** R[2] - 1) * (2 ** R[1]))}};
%%   else
		/* Plain old RGB */
		return {{ hex((2 ** R[2] - 1) * (2 ** R[1]))}};
%%   endif
%%  elif pt.is_gray()
%%   set V = pt.chans['V']
%%   if pt.is_alpha()
%%    set A = pt.chans['A']
		/* Grayscale with Alpha channel is full Alpha + 1/3 Gray */
		return {{ hex(((2 ** V[2] - 1) // 3) * (2 ** V[1]))}};
		       | {{ hex((2 ** R[2] - 1) * (2 ** R[1]))}};
%%   else
		/* Grayscale is 1/3 Gray */
		return {{ hex(((2 ** V[2] - 1) // 3) * (2 ** V[1]))}};
%%   endif
%%  else
		tst_msg("Unsupported conversion to %s", GP_PixelTypeName(pixel_type));
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
%%    for pt2 in pixeltypes
%%     if pt2.name in ['RGB888', 'RGBA8888'] 
{{ gen_convert_and_check('black', pt2.name, pt1.name) }}
{{ gen_convert_and_check('red', pt2.name, pt1.name) }}
%%     endif
%%    endfor
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
%%   for pt2 in pixeltypes
%%    if pt2.name in ['RGB888', 'RGBA8888'] 
{{ gen_suite_entry('black', pt2.name, pt1.name) }}
{{ gen_suite_entry('red', pt2.name, pt1.name) }}
%%    endif
%%   endfor
%%  endif
%% endfor

{{ gen_suite_entry('red', 'RGB888', 'CMYK8888') }}

		{.name = NULL}
	}
};

%% endblock body
