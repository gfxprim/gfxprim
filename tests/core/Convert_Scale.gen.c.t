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
#include <math.h>

#include <core/GP_Convert_Scale.gen.h>

#include "tst_test.h"

%% set max = 16

%% for i in range(1, max)
%%  for j in range(1, max)
static int check_convert_{{ i }}_{{ j }}(void)
{
	unsigned int v, res, exp_res, fail = 0;
	float fres;

	for (v = 0; v < {{ 2 ** i - 1 }}; v++) {
		res = GP_SCALE_VAL_{{ i }}_{{ j }}(v);
%%   if j > i
		/*
		 * We have {{ 2**i }} values and we need to map them to
		 * subset of {{ 2**j }} values while making sure 0 -> 0
		 * and {{ 2**i - 1 }} -> {{ 2**j - 1 }} and that the
		 * mapping is as evenly distributed as possible.
		 *
		 * So we map the input to 0-1 interval by dividing it by
		 * maximal input value {{ 2**i - 1 }} and then multiply
		 * it by output maximal value {{ 2**j - 1}}.
		 */
		fres = (v / {{ (2.00 ** i - 1) }}) * {{ (2.00 ** j - 1) }};
		exp_res = round(fres);
%%   else
		/*
		 * We have {{ 2**i }} values that must be mapped to {{ 2**j }}
		 * so we do simple division and floor() which maps the values
		 * evenly, 0 -> 0 and {{ 2**i - 1 }} -> {{ 2**j - 1 }}.
		 *
		 * In terms for implementation this is just bitshift.
		 */
		fres = v * {{ (2.00 ** j) / (2.00 ** i) }};
		exp_res = floor(fres);
%%   endif

		if (res != exp_res) {
			if (fail < 5)
				tst_msg("GP_SCALE_{{ i }}_{{ j }}(%i) = %i, "
				        "expected %i %f", v, res, exp_res, fres);
			fail++;
		}
	}

	if (fail) {
		if (fail > 5)
			tst_msg("+ next %u failures", fail - 5);
		return TST_FAILED;
	}

	return TST_SUCCESS;
}

%%  endfor
%% endfor

const struct tst_suite tst_suite = {
	.suite_name = "Pixel Conversions Testsuite",
	.tests = {
%% for i in range(1, max)
%%  for j in range(1, max)
		{.name = "SCALE_{{ i }}_{{ j }}()",
		 .tst_fn = check_convert_{{ i }}_{{ j }}},
%%  endfor
%% endfor
		{.name = NULL}
	}
};

%% endblock body
