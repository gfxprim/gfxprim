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

{% block descr %}GP_GET_BITS() and GP_SET_BITS() tests.{% endblock %}

%% block body

#include <stdlib.h>
#include <stdint.h>
#include <core/GP_GetSetBits.h>
#include <core/GP_Common.h>
#include "tst_test.h"

static const uint32_t patterns[] = {
	0x00000000,
	0xffffffff,
	0x55555555,
	0xaaaaaaaa,
	0x43f8af32,
};

%%  for len in range(1, 33)
%%   for off in range(0, 33 - len)
static int getbits_{{ off }}_{{ len }}(void)
{
	uint32_t p_exp, p_get;
	unsigned int i, fail = 0;

	for (i = 0; i < GP_ARRAY_SIZE(patterns); i++) {
		p_get = GP_GET_BITS({{ off }}, {{ len }}, patterns[i]);
		p_exp = (patterns[i] >> {{ off }}) & {{ hex((2 ** len) - 1) }};

		if (p_get != p_exp) {
			tst_msg("Pattern 0x%08x differs get 0x%08x, expected 0x%08x",
			        patterns[i], p_get, p_exp);
			fail++;
		} else {
			tst_msg("Pattern 0x%08x have 0x%08x", patterns[i], p_get);
		}
	}

	if (fail)
		return TST_FAILED;

	return TST_SUCCESS;
}
%%   endfor
%%  endfor

%%  macro mask(off, len)
~{{ hex((2 ** len - 1) * (2 ** off)) }}
%%-  endmacro

%%  for len in range(1, 33)
%%   for off in range(0, 33 - len)
static int setbits_{{ off }}_{{ len }}(void)
{
	uint32_t p_exp, canary1, p_get, canary2, val;
	unsigned int i, j, fail = 0;

	for (i = 0; i < GP_ARRAY_SIZE(patterns); i++) {
		for (j = 0; j < GP_ARRAY_SIZE(patterns); j++) {
			{# GP_SET_BITS() needs value clamped to the len #}
			val = patterns[j] & {{ hex(2 ** len - 1) }};

			canary1 = 0;
			canary2 = 0;
			p_get = patterns[i];
			GP_SET_BITS({{ off }}, {{ len }}, p_get, val);
			
			p_exp = patterns[i] & {{ mask(off, len) }};
			p_exp |= val<<{{ off }};

			if (p_get != p_exp || canary1 != 0 || canary2 != 0) {
				tst_msg("BG 0x%08x FG 0x%08x differs get 0x%08x, expected 0x%08x",
				        patterns[i], val, p_get, p_exp);
				fail++;
			}
		}
	}

	if (fail)
		return TST_FAILED;

	return TST_SUCCESS;
}
%%   endfor
%%  endfor

const struct tst_suite tst_suite = {
	.suite_name = "GetSetBits testsuite",
	.tests = {
%%  for len in range(1, 33)
%%   for off in range(0, 33 - len)
		{.name = "GP_GET_BITS off={{ off }} len={{ len }}",
		 .tst_fn = getbits_{{ off }}_{{ len }}},
%%   endfor
%%  endfor
%%  for len in range(1, 33)
%%   for off in range(0, 33 - len)
		{.name = "GP_SET_BITS off={{ off }} len={{ len }}",
		 .tst_fn = setbits_{{ off }}_{{ len }}},
%%   endfor
%%  endfor
		{.name = NULL}
	}
};

%% endblock body
