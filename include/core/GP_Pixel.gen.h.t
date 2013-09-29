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
 * Copyright (C) 2011 Tomas Gavenciak <gavento@ucw.cz>                       *
 * Copyright (C) 2013 Cyril Hrubis <metan@ucw.cz>                            *
 *                                                                           *
 *****************************************************************************/

%% extends "base.h.t"

%% block descr
Pixel type definitions and functions.
Do not include directly, use GP_Pixel.h
%% endblock

%% block body

/*
 * List of all known pixel types
 */
typedef enum GP_PixelType {
%% for pt in pixeltypes
	GP_PIXEL_{{ pt.name }},
%% endfor
	GP_PIXEL_MAX,
} GP_PixelType;

%% for pt in pixeltypes
#define GP_PIXEL_{{ pt.name }} GP_PIXEL_{{ pt.name }}
%% endfor

%% for pt in pixeltypes
%%  if not pt.is_unknown()
/* Automatically generated code for pixel type {{ pt.name }}
 *
 * Size (bpp): {{ pt.pixelsize.size }} ({{ pt.pixelsize.suffix }})
 * Bit endian: {{ pt.pixelsize.bit_endian_const }}
 * Pixel structure: {{ pt.bits|join("") }}
 * Channels:
%%   for c in pt.chanslist
 *   {{ c[0] }}  offset:{{ c[1] }} size:{{ c[2] }}
%%   endfor
 */

/*
 * macros to get channels of pixel type {{ pt.name }}
 */
%%   for c in pt.chanslist
#define GP_Pixel_GET_{{ c[0] }}_{{ pt.name }}(p) (GP_GET_BITS({{ c[1] }}, {{ c[2] }}, (p)))
%%   endfor

/*
 * macros to get address and bit-offset of a pixel {{ pt.name }} in a context
 */
#define GP_PIXEL_ADDR_{{ pt.name }}(context, x, y) GP_PIXEL_ADDR_{{ pt.pixelsize.suffix }}(context, x, y)
#define GP_PIXEL_ADDR_OFFSET_{{ pt.name }}(x) GP_PIXEL_ADDR_OFFSET_{{ pt.pixelsize.suffix }}(x)

/*
 * macros to create GP_Pixel of pixel type {{ pt.name }} directly from given values.
 * The values MUST be already clipped/converted to relevant value ranges.
 */
#define GP_Pixel_CREATE_{{ pt.name }}({{ pt.chanslist[0][0] }}{% for c in pt.chanslist[1:] %}, {{ c[0] }}{% endfor %}) (0\
%%   for c in pt.chanslist
	+ (({{ c[0] }}) << {{ c[1] }}) \
%%   endfor
	)

%%  endif
%% endfor

/*
 * macros for branching on PixelType (similar to GP_FnPerBpp macros)
 */

%% for r in ['', 'return ']
#define GP_FN_{% if r %}RET_{% endif %}PER_PIXELTYPE(FN_NAME, type, ...)\
	switch (type) { \
%%  for pt in pixeltypes
%%   if not pt.is_unknown()
		case GP_PIXEL_{{ pt.name }}:\
			{{ r }}FN_NAME{{'##'}}_{{ pt.name }}(__VA_ARGS__);\
			break;\
%%   endif
%%  endfor
		default: GP_ABORT("Invalid PixelType %d", type);\
	}

%% endfor

%% endblock body
