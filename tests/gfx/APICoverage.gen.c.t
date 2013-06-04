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

    The purpose of this test is to exercise as much codepaths as possible
    without checking for result corectness.

  */

%% extends "base.test.c.t"

{% block descr %}GFX API converage tests.{% endblock %}

%% block body

#include <stdio.h>

#include <core/GP_Context.h>
#include <gfx/GP_Gfx.h>

#include "tst_test.h"

%% set API_List = [
    ['Fill', 'GP_Context:in', 'int:pixel'],
    
    ['HLine', 'GP_Context:in', 'int:x0', 'int:x1', 'int:y', 'int:pixel'],
    ['VLine', 'GP_Context:in', 'int:x', 'int:y0', 'int:y1', 'int:pixel'],
    
    ['Line', 'GP_Context:in', 'int:x0', 'int:y0',
     'int:x1', 'int:y1', 'int:pixel'],
    
    ['Circle', 'GP_Context:in', 'int:xcenter', 'int:ycenter',
     'int:r', 'int:pixel'],
    ['FillCircle', 'GP_Context:in', 'int:xcenter', 'int:ycenter',
     'int:r', 'int:pixel'],

    ['Ellipse', 'GP_Context:in', 'int:xcenter', 'int:ycenter',
     'int:a', 'int:b', 'int:pixel'],
    ['FillEllipse', 'GP_Context:in', 'int:xcenter', 'int:ycenter',
     'int:a', 'int:b', 'int:pixel'],

    ['Ring', 'GP_Context:in', 'int:xc', 'int:yc',
     'int:r1', 'int:r2', 'int:pixel'],
    ['FillRing', 'GP_Context:in', 'int:xc', 'int:yc',
     'int:r1', 'int:r2', 'int:pixel'],

    ['Rect', 'GP_Context:in', 'int:x0', 'int:y0',
     'int:x1', 'int:y1', 'int:pixel'],
    ['FillRect', 'GP_Context:in', 'int:x0', 'int:y0',
     'int:x1', 'int:y1', 'int:pixel'],
    
    ['Triangle', 'GP_Context:in', 'int:x0', 'int:y0',
     'int:x1', 'int:y1', 'int:x2', 'int:y2', 'int:pixel'],
    ['FillTriangle', 'GP_Context:in', 'int:x0', 'int:y0',
     'int:x1', 'int:y1', 'int:x2', 'int:y2', 'int:pixel'],
    
    ['Tetragon', 'GP_Context:in', 'int:x0', 'int:y0', 'int:x1', 'int:y1', 
     'int:x2', 'int:y2', 'int:x3', 'int:y3', 'int:pixel'],
    ['FillTetragon', 'GP_Context:in', 'int:x0', 'int:y0', 'int:x1', 'int:y1', 
     'int:x2', 'int:y2', 'int:x3', 'int:y3', 'int:pixel'],

]

%% macro prep_context(id, pt)
	GP_Context *{{ id }} = GP_ContextAlloc(331, 331, GP_PIXEL_{{ pt.name }});
%% endmacro

%% macro prep_int(id)
	int {{ id }} = 2;
%% endmacro

%% macro prep_param(param, pt)
%%  if (param.split(':', 1)[0] == 'GP_Context')
{{ prep_context(param.split(':', 1)[1], pt) }}
%%  endif
%%  if (param.split(':', 1)[0] == 'float')
{{ prep_float(param.split(':', 1)[1]) }}
%%  endif
%%  if (param.split(':', 1)[0] == 'int')
{{ prep_int(param.split(':', 1)[1]) }}
%%  endif
%% endmacro

{% macro get_param(param) %}{% if len(param.split(':', 1)) == 1 %}NULL{% else %}{{ param.split(':', 1)[1] }}{% endif %}{% endmacro %}

%% for fn in API_List
%%  for pt in pixeltypes
%%   if not pt.is_unknown()

static int Gfx_{{ fn[0]}}_{{ pt.name }}(void)
{
%%    for param in fn[1:]
{{ prep_param(param, pt) }}
%%    endfor

	GP_{{ fn[0] }}({{ get_param(fn[1]) }}{% for param in fn[2:] %}, {{ get_param(param) }}{% endfor %});

	return TST_SUCCESS;
}

%%   endif
%%  endfor
%% endfor

const struct tst_suite tst_suite = {
	.suite_name = "Gfx API Coverage",
	.tests = {
%% for fn in API_List
%%  for pt in pixeltypes
%%   if not pt.is_unknown()
		{.name = "{{ fn[0] }} {{ pt.name }}", 
		 .tst_fn = Gfx_{{ fn[0] }}_{{ pt.name }}},
%%   endif
%%  endfor
%% endfor
		{.name = NULL}
	}
};

%% endblock body
