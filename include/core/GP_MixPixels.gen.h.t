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
 * Copyright (C) 2011-2012 Cyril Hrubis <metan@ucw.cz>                       *
 *                                                                           *
 *****************************************************************************/

%% extends "base.h.t"

%% block descr
Macros to mix two pixels accordingly to percentage.
%% endblock

%% block body

#include "core/GP_Context.h"
#include "core/GP_Pixel.h"
#include "core/GP_GetPutPixel.h"
#include "core/GP_GammaCorrection.h"

%% for pt in pixeltypes
%%  if not pt.is_unknown()

/*
 * Mixes two {{ pt.name }} pixels.
 *
 * The percentage is expected as 8 bit unsigned integer [0 .. 255]
 */
#define GP_MIX_PIXELS_LINEAR_{{ pt.name }}(pix1, pix2, perc) ({ \
%%   for c in pt.chanslist
	GP_Pixel {{ c[0] }}; \
\
	{{ c[0] }}  = GP_Pixel_GET_{{ c[0] }}_{{ pt.name }}(pix1) * (perc); \
	{{ c[0] }} += GP_Pixel_GET_{{ c[0] }}_{{ pt.name }}(pix2) * (255 - (perc)); \
	{{ c[0] }} = ({{ c[0] }} + 128) / 255; \
\
%%   endfor
\
	GP_Pixel_CREATE_{{ pt.name }}({{ pt.chanslist[0][0] }}{% for c in pt.chanslist[1:] %}, {{ c[0] }}{% endfor %}); \
})

/*
 * Mixes two {{ pt.name }} pixels.
 *
 * The percentage is expected as 8 bit unsigned integer [0 .. 255]
 */
#define GP_MIX_PIXELS_GAMMA_{{ pt.name }}(pix1, pix2, perc) ({ \
%%   for c in pt.chanslist
	GP_Pixel {{ c[0] }}; \
\
	{{ c[0] }}  = GP_Gamma{{ c[2] }}ToLinear10(GP_Pixel_GET_{{ c[0] }}_{{ pt.name }}(pix1)) * (perc); \
	{{ c[0] }} += GP_Gamma{{ c[2] }}ToLinear10(GP_Pixel_GET_{{ c[0] }}_{{ pt.name }}(pix2)) * (255 - (perc)); \
	{{ c[0] }} = ({{ c[0] }} + 128) / 255; \
	{{ c[0] }} = GP_Linear10ToGamma{{ c[2] }}({{ c[0] }}); \
\
%%   endfor
\
	GP_Pixel_CREATE_{{ pt.name }}({{ pt.chanslist[0][0] }}{% for c in pt.chanslist[1:] %}, {{ c[0] }}{% endfor %}); \
})

#define GP_MIX_PIXELS_{{ pt.name }}(pix1, pix2, perc) \
%%   if pt.is_rgb()
	GP_MIX_PIXELS_GAMMA_{{ pt.name }}(pix1, pix2, perc)
%%   else
	GP_MIX_PIXELS_LINEAR_{{ pt.name }}(pix1, pix2, perc)
%%   endif

%%  endif
%% endfor

static inline GP_Pixel GP_MixPixels(GP_Pixel pix1, GP_Pixel pix2,
                                    uint8_t perc, GP_PixelType pixel_type)
{
	switch (pixel_type) {
%% for pt in pixeltypes
%%  if not pt.is_unknown()
	case GP_PIXEL_{{ pt.name }}:
		return GP_MIX_PIXELS_LINEAR_{{ pt.name }}(pix1, pix2, perc);
%%  endif
%% endfor
	default:
		GP_ABORT("Unknown pixeltype");
	}
}


%% for pt in pixeltypes
%%  if not pt.is_unknown()
static inline void GP_MixPixel_Raw_{{ pt.name }}(GP_Context *context,
			GP_Coord x, GP_Coord y, GP_Pixel pixel, uint8_t perc)
{
	GP_Pixel pix = GP_GetPixel_Raw_{{ pt.pixelsize.suffix }}(context, x, y);
	pix = GP_MIX_PIXELS_{{ pt.name }}(pixel, pix, perc);
	GP_PutPixel_Raw_{{ pt.pixelsize.suffix }}(context, x, y, pix);
}

%%  endif
%% endfor

%% for pt in pixeltypes
%%  if not pt.is_unknown()
static inline void GP_MixPixel_Raw_Clipped_{{ pt.name }}(GP_Context *context,
			GP_Coord x, GP_Coord y, GP_Pixel pixel, uint8_t perc)
{
	if (GP_PIXEL_IS_CLIPPED(context, x, y))
		return;

	GP_MixPixel_Raw_{{ pt.name }}(context, x, y, pixel, perc);
}

%%  endif
%% endfor

static inline void GP_MixPixel_Raw(GP_Context *context, GP_Coord x, GP_Coord y,
                                   GP_Pixel pixel, uint8_t perc)
{
	switch (context->pixel_type) {
%% for pt in pixeltypes
%%  if not pt.is_unknown()
	case GP_PIXEL_{{ pt.name }}:
				GP_MixPixel_Raw_{{ pt.name }}(context, x, y, pixel, perc);
	break;
%%  endif
%% endfor
	default:
		GP_ABORT("Unknown pixeltype");
	}
}

static inline void GP_MixPixel_Raw_Clipped(GP_Context *context,
                                           GP_Coord x, GP_Coord y,
                                           GP_Pixel pixel, uint8_t perc)
{
	switch (context->pixel_type) {
%% for pt in pixeltypes
%%  if not pt.is_unknown()
	case GP_PIXEL_{{ pt.name }}:
		GP_MixPixel_Raw_Clipped_{{ pt.name }}(context, x, y, pixel, perc);
	break;
%%  endif
%% endfor
	default:
		GP_ABORT("Unknown pixeltype");
	}
}

%% endblock body
