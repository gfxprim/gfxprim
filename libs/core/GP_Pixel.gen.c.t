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
 * Copyright (C) 2011-2012 Tomas Gavenciak <gavento@ucw.cz>                  *
 * Copyright (C) 2011-2013 Cyril Hrubis    <metan@ucw.cz>                    *
 *                                                                           *
 *****************************************************************************/

%% extends "base.c.t"

%% block descr
Pixel type definitions and functions
%% endblock

%% block body
#include <stdio.h>
#include "GP_Pixel.h"
#include "GP_GetSetBits.h"

%%- macro getflags(pt)
{% if pt.is_alpha() %} | GP_PIXEL_HAS_ALPHA{% endif -%}
{% if pt.is_rgb() %} | GP_PIXEL_IS_RGB{% endif -%}
{% if pt.is_palette() %} | GP_PIXEL_IS_PALETTE{% endif -%}
{% if pt.is_gray() %} | GP_PIXEL_IS_GRAYSCALE{% endif -%}
{% if pt.is_cmyk() %} | GP_PIXEL_IS_CMYK{% endif -%}
%%- endmacro

/* 
 * Description of all known pixel types 
 */
const GP_PixelTypeDescription const GP_PixelTypes [GP_PIXEL_MAX] = {
%% for pt in pixeltypes
	/* GP_PIXEL_{{ pt.name }} */ {
		.type	= GP_PIXEL_{{ pt.name }},
		.name	= "{{ pt.name }}",
		.size	= {{ pt.pixelsize.size }},
		.bit_endian  = {{ pt.pixelsize.bit_endian_const }},
		.numchannels = {{ len(pt.chanslist) }},
		.bitmap      = "{{ pt.bits|join("") }}",
		.flags       = 0{{ getflags(pt) }},
		.channels    = {
%% for c in pt.chanslist
			{ .name = "{{ c[0] }}", .offset = {{ c[1] }}, .size = {{ c[2] }} },
%% endfor
	} },
%% endfor
};

#warning FIXME: do generic get set bit for pixel printing

%% for pt in pixeltypes
%% if not pt.is_unknown()
/*
 * snprintf a human readable value of pixel type {{pt.name}} 
 */
void GP_PixelSNPrint_{{ pt.name }}(char *buf, size_t len, GP_Pixel p)
{
	snprintf(buf, len, "<{{ pt.name }} 0x%0{{ (pt.pixelsize.size+3)//4 }}x{% for c in pt.chanslist %} {{ c[0] }}=%d{% endfor %}>",
	GP_GET_BITS(0, {{ pt.pixelsize.size }}, p){% for c in pt.chanslist %}, GP_GET_BITS({{ c[1] }}, {{ c[2] }}, p){% endfor %});
}

%% endif
%% endfor
%% endblock body
