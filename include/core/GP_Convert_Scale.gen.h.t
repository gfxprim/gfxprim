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
 * Copyright (C) 2011      Tomas Gavenciak <gavento@ucw.cz>                  *
 * Copyright (C) 2013      Cyril Hrubis <metan@ucw.cz>                       *
 *                                                                           *
 *****************************************************************************/

%% extends "base.h.t"

{% block descr %}Fast value scaling macros{% endblock %}

{% macro multcoef(s1, s2) -%}
(0{% for i in range((s2 + s1 - 1) // s1) %}+{{ hex(2 ** (i * s1)) }}{% endfor %})
{%- endmacro %}

%% block body
/*
 * Helper macros to transfer s1-bit value to s2-bit value.
 * Efficient and accurate for both up- and downscaling.
 * WARNING: GP_SCALE_VAL requires constants numbers as first two parameters
 */
#define GP_SCALE_VAL(s1, s2, val) ( GP_SCALE_VAL_##s1##_##s2(val) )

%% for s1 in range(1,33)
%%  for s2 in range(1,17)
%%   if s2 > s1
#define GP_SCALE_VAL_{{s1}}_{{s2}}(val) (((val) * {{ multcoef(s1, s2) }}) >> {{ (-s2) % s1 }})
%%   else
#define GP_SCALE_VAL_{{s1}}_{{s2}}(val) ((val) >> {{ s1 - s2 }})
%%   endif
%%  endfor
%% endfor

%% endblock body
