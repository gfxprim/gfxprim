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
 * Copyright (C) 2009-2014 Cyril Hrubis <metan@ucw.cz>                       *
 *                                                                           *
 *****************************************************************************/

%% extends "filter.c.t"

{% block descr %}Linear resampling{% endblock %}

%% block body

#include <string.h>
#include <errno.h>

#include "core/GP_Context.h"
#include "core/GP_GetPutPixel.h"
#include "core/GP_Gamma.h"

#include "core/GP_Debug.h"

#include "GP_Resize.h"

%%- macro fetch_row(pt, src, y, suff)
		{
			unsigned int x;

			for (x = 0; x < src->w; x++) {
				GP_Pixel pix = GP_GetPixel_Raw_{{ pt.pixelsize.suffix }}({{ src }}, x, {{ y }});
%%  for c in pt.chanslist
				{{ c.name }}{{ suff }}[x] = GP_Pixel_GET_{{ c.name }}_{{ pt.name }}(pix);
%%  endfor
			}
		}
%% endmacro

#define SUM(COL, COLD, MUL) \
			COLD[x] += (COL[xmap[x]] * (MULT - xoff[x]) * (MUL)) / MULT;\
\
			for (j = xmap[x]+1; j < xmap[x+1]; j++) { \
				COLD[x] += (COL[j] * (MUL)); \
			} \
\
			if (xoff[x+1]) \
				COLD[x] += (COL[xmap[x+1]] * xoff[x+1] * (MUL)) / MULT;

%% for pt in pixeltypes
%%  if not pt.is_unknown() and not pt.is_palette()

static int resize_lin_lf_{{ pt.name }}(const GP_Context *src, GP_Context *dst,
                                       GP_ProgressCallback *callback)
{
	uint32_t xmap[dst->w + 1];
	uint32_t ymap[dst->h + 1];
	uint16_t xoff[dst->w + 1];
	uint16_t yoff[dst->h + 1];
	uint32_t x, y;
	uint32_t i, j;
{# Reduce fixed point bits for > 8 bits per channel (fixed 16 bit Grayscale) #}
%%   if pt.chanslist[0].size > 8
	const int MULT=256;
%%   else
	const int MULT=1024;
%%   endif
	/* Pre-compute mapping for interpolation */
	for (i = 0; i <= dst->w; i++) {
		xmap[i] = (i * src->w) / dst->w;
		xoff[i] = (MULT * (i * src->w))/dst->w - MULT * xmap[i];
	}

	for (i = 0; i <= dst->h; i++) {
		ymap[i] = (i * src->h) / dst->h;
		yoff[i] = (MULT * (i * src->h))/dst->h - MULT * ymap[i];
	}

	/* Compute pixel area for the final normalization */
	uint32_t div = (xmap[1] * MULT + xoff[1]) * (ymap[1] * MULT + yoff[1]) / MULT;

	for (y = 0; y < dst->h; y++) {
%%   for c in pt.chanslist
		uint32_t {{ c.name }}[src->w];
		uint32_t {{ c.name }}_res[src->w];
%%   endfor

%%   for c in pt.chanslist
		memset({{ c.name }}_res, 0, sizeof({{ c.name }}_res));
%%   endfor

		/* Sum first row */
		{{ fetch_row(pt, 'src', 'ymap[y]', '') }}
		for (x = 0; x < dst->w; x++) {
%%   for c in pt.chanslist
			SUM({{ c.name }}, {{ c.name }}_res, MULT-yoff[y]);
%%   endfor
		}

		/* Sum middle */
		for (i = ymap[y]+1; i < ymap[y+1]; i++) {
			{{ fetch_row(pt, 'src', 'i', '') }}
			for (x = 0; x < dst->w; x++) {
%%   for c in pt.chanslist
				SUM({{ c.name }}, {{ c.name }}_res, MULT);
%%   endfor
			}
		}

		/* Sum last row */
		if (yoff[y+1]) {
			{{ fetch_row(pt, 'src', 'ymap[y+1]', '') }}
			for (x = 0; x < dst->w; x++) {
%%   for c in pt.chanslist
				SUM({{ c.name }}, {{ c.name }}_res, yoff[y+1]);
%%   endfor
			}
		}

		for (x = 0; x < dst->w; x++) {
%%   for c in pt.chanslist
			uint32_t {{ c.name }}_p = ({{ c.name }}_res[x] + div/2) / div;
%%   endfor
                        GP_PutPixel_Raw_{{ pt.pixelsize.suffix }}(dst, x, y,
				GP_Pixel_CREATE_{{ pt.name }}({{ expand_chanslist(pt, '', '_p') }}));
		}

		if (GP_ProgressCallbackReport(callback, y, dst->h, dst->w))
			return 1;
	}

	GP_ProgressCallbackDone(callback);
	return 0;
}

%%  endif
%% endfor

%% for pt in pixeltypes
%%  if not pt.is_unknown() and not pt.is_palette()

static int resize_lin{{ pt.name }}(const GP_Context *src, GP_Context *dst,
                                   GP_ProgressCallback *callback)
{
	uint32_t xmap[dst->w + 1];
	uint32_t ymap[dst->h + 1];
	uint8_t  xoff[dst->w + 1];
	uint8_t  yoff[dst->h + 1];
	uint32_t x, y, i;

	GP_DEBUG(1, "Scaling image %ux%u -> %ux%u %2.2f %2.2f",
	            src->w, src->h, dst->w, dst->h,
		    1.00 * dst->w / src->w, 1.00 * dst->h / src->h);

	/* Pre-compute mapping for interpolation */
	uint32_t xstep = ((src->w - 1) << 16) / (dst->w - 1);

	for (i = 0; i < dst->w + 1; i++) {
		uint32_t val = i * xstep;
		xmap[i] = val >> 16;
		xoff[i] = (val >> 8) & 0xff;
	}

	uint32_t ystep = ((src->h - 1) << 16) / (dst->h - 1);

	for (i = 0; i < dst->h + 1; i++) {
		uint32_t val = i * ystep;
		ymap[i] = val >> 16;
		yoff[i] = (val >> 8) & 0xff;
	}

	/* Interpolate */
	for (y = 0; y < dst->h; y++) {
		for (x = 0; x < dst->w; x++) {
			GP_Pixel pix00, pix01, pix10, pix11;
			GP_Coord x0, x1, y0, y1;
			%% for c in pt.chanslist
			uint32_t {{ c[0] }}, {{ c[0] }}0, {{ c[0] }}1;
			%% endfor

			x0 = xmap[x];
			x1 = xmap[x] + 1;

			if (x1 >= (GP_Coord)src->w)
				x1 = src->w - 1;

			y0 = ymap[y];
			y1 = ymap[y] + 1;

			if (y1 >= (GP_Coord)src->h)
				y1 = src->h - 1;

			pix00 = GP_GetPixel_Raw_{{ pt.pixelsize.suffix }}(src, x0, y0);
			pix10 = GP_GetPixel_Raw_{{ pt.pixelsize.suffix }}(src, x1, y0);
			pix01 = GP_GetPixel_Raw_{{ pt.pixelsize.suffix }}(src, x0, y1);
			pix11 = GP_GetPixel_Raw_{{ pt.pixelsize.suffix }}(src, x1, y1);

			%% for c in pt.chanslist
			{{ c[0] }}0 = GP_Pixel_GET_{{ c[0] }}_{{ pt.name }}(pix00) * (255 - xoff[x]);
			%% endfor

			%% for c in pt.chanslist
			{{ c[0] }}0 += GP_Pixel_GET_{{ c[0] }}_{{ pt.name }}(pix10) * xoff[x];
			%% endfor

			%% for c in pt.chanslist
			{{ c[0] }}1 = GP_Pixel_GET_{{ c[0] }}_{{ pt.name }}(pix01) * (255 - xoff[x]);
			%% endfor

			%% for c in pt.chanslist
			{{ c[0] }}1 += GP_Pixel_GET_{{ c[0] }}_{{ pt.name }}(pix11) * xoff[x];
			%% endfor

			%% for c in pt.chanslist
			{{ c[0] }} = ({{ c[0] }}1 * yoff[y] + {{ c[0] }}0 * (255 - yoff[y]) + (1<<15)) >> 16;
			%% endfor

			GP_PutPixel_Raw_{{ pt.pixelsize.suffix }}(dst, x, y,
			                      GP_Pixel_CREATE_{{ pt.name }}({{ expand_chanslist(pt, "") }}));
		}

		if (GP_ProgressCallbackReport(callback, y, dst->h, dst->w))
			return 1;
	}

	GP_ProgressCallbackDone(callback);
	return 0;
}

%%  endif
%% endfor

static int resize_lin(const GP_Context *src, GP_Context *dst,
                     GP_ProgressCallback *callback)
{
	switch (src->pixel_type) {
%% for pt in pixeltypes
%%  if not pt.is_unknown() and not pt.is_palette()
	case GP_PIXEL_{{ pt.name }}:
		return resize_lin{{ pt.name }}(src, dst, callback);
	break;
%%  endif
%% endfor
	default:
		GP_WARN("Invalid pixel type %s",
		        GP_PixelTypeName(src->pixel_type));
		errno = EINVAL;
		return -1;
	}
}

int GP_FilterResizeLinearInt(const GP_Context *src, GP_Context *dst,
                             GP_ProgressCallback *callback)
{
	if (src->pixel_type != dst->pixel_type) {
		GP_WARN("The src and dst pixel types must match");
		errno = EINVAL;
		return 1;
	}

	return resize_lin(src, dst, callback);
}

static int resize_lin_lf(const GP_Context *src, GP_Context *dst,
                          GP_ProgressCallback *callback)
{
	float x_rat = 1.00 * dst->w / src->w;
	float y_rat = 1.00 * dst->h / src->h;

	if (x_rat < 1.00 && y_rat < 1.00) {

		GP_DEBUG(1, "Downscaling image %ux%u -> %ux%u %2.2f %2.2f",
	                     src->w, src->h, dst->w, dst->h, x_rat, y_rat);

		switch (src->pixel_type) {
%% for pt in pixeltypes
%%  if not pt.is_unknown() and not pt.is_palette()
		case GP_PIXEL_{{ pt.name }}:
			return resize_lin_lf_{{ pt.name }}(src, dst, callback);
		break;
%%  endif
%% endfor
		default:
			GP_WARN("Invalid pixel type %s",
			        GP_PixelTypeName(src->pixel_type));
			errno = EINVAL;
			return -1;
		}
	}

	//TODO: x_rat > 1.00 && y_rat < 1.00
	//TODO: x_rat < 1.00 && y_rat > 1.00

	return resize_lin(src, dst, callback);
}

int GP_FilterResizeLinearLFInt(const GP_Context *src, GP_Context *dst,
                               GP_ProgressCallback *callback)
{
	if (src->pixel_type != dst->pixel_type) {
		GP_WARN("The src and dst pixel types must match");
		errno = EINVAL;
		return 1;
	}

	return resize_lin_lf(src, dst, callback);
}

%% endblock body
