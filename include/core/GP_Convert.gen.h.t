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
 * Copyright (C) 2011-2013 Cyril Hrubis <metan@ucw.cz>                       *
 * Copyright (C) 2011      Tomas Gavenciak <gavento@ucw.cz>                  *
 *                                                                           *
 *****************************************************************************/

%% extends "base.h.t"

{% block descr %}Convert PixelType values macros and functions{% endblock %}

{# RGB -> CMYK requires special handling #}
%% macro rgb_to_cmyk(in, out)
%%  set R = in.chans['R']
%%  set G = in.chans['G']
%%  set B = in.chans['B']
%%  set C = out.chans['C']
%%  set M = out.chans['M']
%%  set Y = out.chans['Y']
%%  set K = out.chans['K']
%%  set max_size = max(R.size, G.size, B.size)
%%  set max_val = 2 ** max_size - 1
	GP_Pixel _R = GP_SCALE_VAL_{{ R.size }}_{{ max_size }}(GP_GET_BITS({{ R.off }}+o1, {{ R.size }}, p1)); \
	GP_Pixel _G = GP_SCALE_VAL_{{ G.size }}_{{ max_size }}(GP_GET_BITS({{ G.off }}+o1, {{ G.size }}, p1)); \
	GP_Pixel _B = GP_SCALE_VAL_{{ B.size }}_{{ max_size }}(GP_GET_BITS({{ B.off }}+o1, {{ B.size }}, p1)); \
	GP_Pixel _K = GP_MAX3(_R, _G, _B); \
	GP_SET_BITS({{ C.off }}+o2, {{ C.size }}, p2, GP_SCALE_VAL_{{ max_size }}_{{ C.size }}((_K - _R))); \
	GP_SET_BITS({{ M.off }}+o2, {{ M.size }}, p2, GP_SCALE_VAL_{{ max_size }}_{{ M.size }}((_K - _G))); \
	GP_SET_BITS({{ Y.off }}+o2, {{ Y.size }}, p2, GP_SCALE_VAL_{{ max_size }}_{{ Y.size }}((_K - _B))); \
	GP_SET_BITS({{ K.off }}+o2, {{ K.size }}, p2, GP_SCALE_VAL_{{ max_size }}_{{ K.size }}({{ max_val }} - _K)); \
%% endmacro

%% macro GP_Pixel_TYPE_TO_TYPE(pt1, pt2)
/*** {{ pt1.name }} -> {{ pt2.name }} ***
 * macro reads p1 ({{ pt1.name }} at bit-offset o1)
 * and writes to p2 ({{ pt2.name }} at bit-offset o2)
 * the relevant part of p2 is assumed to be cleared (zero) */
#define GP_Pixel_{{ pt1.name }}_TO_{{ pt2.name }}_OFFSET(p1, o1, p2, o2) do { \
{# special cases -#}
%%  if pt1.is_rgb() and pt2.is_cmyk()
{{ rgb_to_cmyk(pt1, pt2) -}}
%%  else
%%   for c2 in pt2.chanslist
{# case 1: just copy a channel -#}
%%    if c2[0] in pt1.chans.keys()
%%     set c1 = pt1.chans[c2[0]]
        /* {{ c2[0] }}:={{ c1[0] }} */ GP_SET_BITS({{ c2.off }}+o2, {{ c2.size }}, p2,\
                GP_SCALE_VAL_{{ c1.size }}_{{ c2.size }}(GP_GET_BITS({{ c1.off }}+o1, {{ c1.size }}, p1))); \
{# case 2: set A to full opacity (not present in source) -#}
%%    elif c2[0]=='A'
        /* A:={{ c2.C_max }} */GP_SET_BITS({{ c2.off }}+o2, {{ c2.size }}, p2, {{ c2.C_max }}); \
{# case 3: calculate V as average of RGB -#}
%%    elif c2[0]=='V' and pt1.is_rgb()
	/* V:=RGB_avg */ GP_SET_BITS({{ c2.off }}+o2, {{ c2.size }}, p2, ( \
%%     for c1 in [pt1.chans['R'], pt1.chans['G'], pt1.chans['B']]
                /* {{ c1.name }} */ GP_SCALE_VAL_{{ c1.size }}_{{ c2.size }}(GP_GET_BITS({{ c1.off }}+o1, {{ c1.size }}, p1)) + \
%%     endfor
        0)/3);\
{# case 4: set each RGB to V -#}
%%    elif c2[0] in 'RGB' and pt1.is_gray()
%%     set c1 = pt1.chans['V']
        /* {{ c2[0] }}:=V */ GP_SET_BITS({{ c2.off }}+o2, {{ c2.size }}, p2,\
                GP_SCALE_VAL_{{ c1.size }}_{{ c2.size }}(GP_GET_BITS({{ c1.off }}+o1, {{ c1.size }}, p1))); \
{# case 5: CMYK to RGB -#}
%%    elif c2[0] in 'RGB' and pt1.is_cmyk()
%%     set K = pt1.chans['K']
{# Get the right channel -#}
%%     if c2[0] == 'R'
%%      set V = pt1.chans['C']
%%     elif c2[0] == 'G'
%%      set V = pt1.chans['M']
%%     else
%%      set V = pt1.chans['Y']
%%     endif
	GP_SET_BITS({{ c2.off }}+o2, {{ c2.size }}, p2,\
                    (({{ c2.C_max }} * ({{ K.C_max }} - GP_GET_BITS({{ K.off }}+o1, {{ K.size }}, p1)) * \
                     ({{ V.C_max }} - GP_GET_BITS({{ V.off }}+o1, {{ V.size }}, p1)))) / ({{ K.C_max }} * {{ V.C_max }})); \
{# case 7: invalid mapping -#}
%%    else
{{ error('Channel conversion ' + pt1.name + ' to ' + pt2.name + ' not supported.') }}
%%   endif
%%  endfor
%% endif
} while (0)

/* a version without offsets */
#define GP_Pixel_{{ pt1.name }}_TO_{{ pt2.name }}(p1, p2) \
        GP_Pixel_{{ pt1.name }}_TO_{{ pt2.name }}_OFFSET(p1, 0, p2, 0)

%% endmacro


%% block body
#include "GP_GetSetBits.h"
#include "GP_Context.h"
#include "GP_Pixel.h"

{#
 # Loop around "central" pixel types
-#}
%% for pt in [pixeltypes_dict['RGB888'], pixeltypes_dict['RGBA8888']]
%%  for i in pixeltypes
%%   if not i.is_unknown() and not i.is_palette()
{{ GP_Pixel_TYPE_TO_TYPE(pt, i) }}
%%    if i.name not in ['RGB888', 'RGBA8888']
{{ GP_Pixel_TYPE_TO_TYPE(i, pt) }}
%%    endif
%%   endif
%%  endfor

/*
 * Convert {{ pt.name }} to any other PixelType
 * Does not work on palette types at all (yet)
 */
GP_Pixel GP_{{ pt.name }}ToPixel(GP_Pixel pixel, GP_PixelType type);

/*
 * Function converting to {{ pt.name }} from any other PixelType
 * Does not work on palette types at all (yet)
 */
GP_Pixel GP_PixelTo{{ pt.name }}(GP_Pixel pixel, GP_PixelType type);

%% endfor

/* Experimental macros testing generated scripts */
{{ GP_Pixel_TYPE_TO_TYPE(pixeltypes_dict['RGB565'], pixeltypes_dict['RGBA8888']) }}
{{ GP_Pixel_TYPE_TO_TYPE(pixeltypes_dict['RGBA8888'], pixeltypes_dict['G2']) }}

%% endblock body
