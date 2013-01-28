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

%% extends "filter.c.t"

{% block descr %}Nearest Neighbour resampling{% endblock %}

%% block body

#include "core/GP_Context.h"
#include "core/GP_GetPutPixel.h"

#include "core/GP_Debug.h"

#include "GP_Resize.h"

%% for pt in pixeltypes
%% if not pt.is_unknown()

static int GP_FilterResizeNN_{{ pt.name }}_Raw(const GP_Context *src,
	GP_Context *dst, GP_ProgressCallback *callback)
{
	uint32_t xmap[dst->w];
	uint32_t ymap[dst->h];
	uint32_t i;
	GP_Coord x, y;
	
	GP_DEBUG(1, "Scaling image %ux%u -> %ux%u %2.2f %2.2f",
	            src->w, src->h, dst->w, dst->h,
		    1.00 * dst->w / src->w, 1.00 * dst->h / src->h);

	/* Pre-compute mapping for interpolation */
	for (i = 0; i < dst->w; i++)
		xmap[i] = ((((i * (src->w - 1))<<8) + (dst->w - 1)/2) / (dst->w - 1) + (1<<7))>>8;

	for (i = 0; i < dst->h; i++)
		ymap[i] = ((((i * (src->h - 1))<<8) + (dst->h - 1)/2) / (dst->h - 1) + (1<<7))>>8;

	/* Interpolate */
	for (y = 0; y < (GP_Coord)dst->h; y++) {
		for (x = 0; x < (GP_Coord)dst->w; x++) {
			GP_Pixel pix = GP_GetPixel_Raw_{{ pt.pixelsize.suffix }}(src, xmap[x], ymap[y]);

			GP_PutPixel_Raw_{{ pt.pixelsize.suffix }}(dst, x, y, pix);
		}
	
		if (GP_ProgressCallbackReport(callback, y, dst->h, dst->w))
			return 1;
	}

	GP_ProgressCallbackDone(callback);
	return 0;
}

%% endif
%% endfor

int GP_FilterResizeNN_Raw(const GP_Context *src, GP_Context *dst,
                          GP_ProgressCallback *callback)
{
	switch (src->pixel_type) {
	%% for pt in pixeltypes
	%% if not pt.is_unknown()
	case GP_PIXEL_{{ pt.name }}:
		return GP_FilterResizeNN_{{ pt.name }}_Raw(src, dst, callback);
	break;
	%% endif
	%% endfor
	default:
		return -1;
	}
}

%% endblock body
