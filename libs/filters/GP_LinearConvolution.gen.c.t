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

{% block descr %}Linear Convolution{% endblock %}

%% block body

#include <errno.h>

#include "core/GP_Context.h"
#include "core/GP_GetPutPixel.h"
#include "core/GP_TempAlloc.h"
#include "core/GP_Clamp.h"
#include "core/GP_Debug.h"

#include "GP_Linear.h"

#define MUL 1024

%% for pt in pixeltypes
%%  if not pt.is_unknown() and not pt.is_palette()

static int h_lin_conv_{{ pt.name }}(const GP_Context *src,
                                    GP_Coord x_src, GP_Coord y_src,
                                    GP_Size w_src, GP_Size h_src,
                                    GP_Context *dst,
                                    GP_Coord x_dst, GP_Coord y_dst,
				    float kernel[], uint32_t kw, float kern_div,
				    GP_ProgressCallback *callback)
{
	GP_Coord x, y;
	uint32_t i;
	int ikernel[kw], ikern_div;
	uint32_t size = w_src + kw - 1;

	GP_DEBUG(1, "Horizontal linear convolution kernel width %u "
	            "offset %ix%i rectangle %ux%u",
		    kw, x_src, y_src, w_src, h_src);

	for (i = 0; i < kw; i++)
		ikernel[i] = kernel[i] * MUL + 0.5;

	ikern_div = kern_div * MUL + 0.5;

	/* Create temporary buffers */
	GP_TempAllocCreate(temp, {{ len(pt.chanslist) }} * size * sizeof(int));

	%% for c in pt.chanslist
	int *{{ c.name }} = GP_TempAllocGet(temp, size * sizeof(int));
	%% endfor

	/* Do horizontal linear convolution */
	for (y = 0; y < (GP_Coord)h_src; y++) {
		int yi = GP_MIN(y_src + y, (int)src->h - 1);

		/* Fetch the whole row */
		GP_Pixel pix = GP_GetPixel_Raw_{{ pt.pixelsize.suffix }}(src, 0, yi);

		int xi = x_src - kw/2;
		i = 0;

		/* Copy border pixel until the source image starts */
		while (xi <= 0 && i < size) {
			%% for c in pt.chanslist
			{{ c.name }}[i] = GP_Pixel_GET_{{ c.name }}_{{ pt.name }}(pix);
			%% endfor
			i++;
			xi++;
		}

		/* Use as much source image pixels as possible */
		while (xi < (int)src->w && i < size) {
			pix = GP_GetPixel_Raw_{{ pt.pixelsize.suffix }}(src, xi, yi);

			%% for c in pt.chanslist
			{{ c.name }}[i] = GP_Pixel_GET_{{ c.name }}_{{ pt.name }}(pix);
			%% endfor

			i++;
			xi++;
		}

		/* Copy the rest the border pixel when we are out again */
		while (i < size) {
			%% for c in pt.chanslist
			{{ c.name }}[i] = GP_Pixel_GET_{{ c.name }}_{{ pt.name }}(pix);
			%% endfor

			i++;
		}

		for (x = 0; x < (GP_Coord)w_src; x++) {
			%% for c in pt.chanslist
			int32_t {{ c.name }}_sum = MUL/2;
			int *p{{ c.name }} = {{ c.name }} + x;
			%% endfor

			/* count the pixel value from neighbours weighted by kernel */
			for (i = 0; i < kw; i++) {
				%% for c in pt.chanslist
				{{ c.name }}_sum += (*p{{ c.name }}++) * ikernel[i];
				%% endfor
			}

			/* divide the result */
			%% for c in pt.chanslist
			{{ c.name }}_sum /= ikern_div;
			%% endfor

			/* and clamp just to be extra sure */
			%% for c in pt.chanslist
			{{ c.name }}_sum = GP_CLAMP({{ c.name }}_sum, 0, {{ c.max }});
			%% endfor

			GP_PutPixel_Raw_{{ pt.pixelsize.suffix }}(dst, x_dst + x, y_dst + y,
			                      GP_Pixel_CREATE_{{ pt.name }}(
					      {{ expand_chanslist(pt, "", "_sum") }}
					      ));
		}

		if (GP_ProgressCallbackReport(callback, y, h_src, w_src)) {
			GP_TempAllocFree(temp);
			return 1;
		}
	}

	GP_TempAllocFree(temp);

	GP_ProgressCallbackDone(callback);
	return 0;
}

%%  endif
%% endfor


int GP_FilterHLinearConvolution_Raw(const GP_Context *src,
                                    GP_Coord x_src, GP_Coord y_src,
                                    GP_Size w_src, GP_Size h_src,
                                    GP_Context *dst,
                                    GP_Coord x_dst, GP_Coord y_dst,
				    float kernel[], uint32_t kw, float kern_div,
				    GP_ProgressCallback *callback)
{
	switch (src->pixel_type) {
	%% for pt in pixeltypes
	%%  if not pt.is_unknown() and not pt.is_palette()
	case GP_PIXEL_{{ pt.name }}:
		return h_lin_conv_{{ pt.name }}(src, x_src, y_src, w_src, h_src,
                                                dst, x_dst, y_dst,
		                                kernel, kw, kern_div, callback);
	break;
	%%  endif
	%% endfor
	default:
		errno = EINVAL;
		return -1;
	}
}

%% for pt in pixeltypes
%%  if not pt.is_unknown() and not pt.is_palette()

static int v_lin_conv_{{ pt.name }}(const GP_Context *src,
                                    GP_Coord x_src, GP_Coord y_src,
                                    GP_Size w_src, GP_Size h_src,
                                    GP_Context *dst,
                                    GP_Coord x_dst, GP_Coord y_dst,
                                    float kernel[], uint32_t kh, float kern_div,
                                    GP_ProgressCallback *callback)
{
	GP_Coord x, y;
	uint32_t i;
	int ikernel[kh], ikern_div;
	uint32_t size = h_src + kh - 1;

	for (i = 0; i < kh; i++)
		ikernel[i] = kernel[i] * MUL + 0.5;

	ikern_div = kern_div * MUL + 0.5;

	/* Create temporary buffers */
	GP_TempAllocCreate(temp, {{ len(pt.chanslist) }} * size * sizeof(int));

	%% for c in pt.chanslist
	int *{{ c.name }} = GP_TempAllocGet(temp, size * sizeof(int));
	%% endfor

	/* Do vertical linear convolution */
	for (x = 0; x < (GP_Coord)w_src; x++) {
		int xi = GP_MIN(x_src + x, (int)src->w - 1);

		/* Fetch the whole row */
		GP_Pixel pix = GP_GetPixel_Raw_{{ pt.pixelsize.suffix }}(src, xi, 0);

		int yi = y_src - kh/2;
		i = 0;

		/* Copy border pixel until the source image starts */
		while (yi <= 0 && i < size) {
			%% for c in pt.chanslist
			{{ c.name }}[i] = GP_Pixel_GET_{{ c.name }}_{{ pt.name }}(pix);
			%% endfor

			i++;
			yi++;
		}

		/* Use as much source image pixels as possible */
		while (yi < (int)src->h && i < size) {
			pix = GP_GetPixel_Raw_{{ pt.pixelsize.suffix }}(src, xi, yi);

			%% for c in pt.chanslist
			{{ c.name }}[i] = GP_Pixel_GET_{{ c.name }}_{{ pt.name }}(pix);
			%% endfor

			i++;
			yi++;
		}

		/* Copy the rest the border pixel when we are out again */
		while (i < size) {
			%% for c in pt.chanslist
			{{ c.name }}[i] = GP_Pixel_GET_{{ c.name }}_{{ pt.name }}(pix);
			%% endfor

			i++;
		}

		for (y = 0; y < (GP_Coord)h_src; y++) {
			%% for c in pt.chanslist
			int32_t {{ c.name }}_sum = MUL/2;
			int *p{{ c.name }} = {{ c.name }} + y;
			%% endfor

			/* count the pixel value from neighbours weighted by kernel */
			for (i = 0; i < kh; i++) {
				%% for c in pt.chanslist
				{{ c.name }}_sum += (*p{{ c.name }}++) * ikernel[i];
				%% endfor
			}

			/* divide the result */
			%% for c in pt.chanslist
			{{ c.name }}_sum /= ikern_div;
			%% endfor

			/* and clamp just to be extra sure */
			%% for c in pt.chanslist
			{{ c.name }}_sum = GP_CLAMP({{ c.name }}_sum, 0, {{ c.max }});
			%% endfor

			GP_PutPixel_Raw_{{ pt.pixelsize.suffix }}(dst, x_dst + x, y_dst + y,
			                      GP_Pixel_CREATE_{{ pt.name }}(
					      {{ expand_chanslist(pt, "", "_sum") }}
					      ));
		}

		if (GP_ProgressCallbackReport(callback, x, w_src, h_src)) {
			GP_TempAllocFree(temp);
			return 1;
		}
	}

	GP_TempAllocFree(temp);

	GP_ProgressCallbackDone(callback);
	return 0;
}

%%  endif
%% endfor

int GP_FilterVLinearConvolution_Raw(const GP_Context *src,
                                    GP_Coord x_src, GP_Coord y_src,
                                    GP_Size w_src, GP_Size h_src,
                                    GP_Context *dst,
                                    GP_Coord x_dst, GP_Coord y_dst,
                                    float kernel[], uint32_t kh, float kern_div,
                                    GP_ProgressCallback *callback)
{
	GP_DEBUG(1, "Vertical linear convolution kernel width %u "
	            "offset %ix%i rectangle %ux%u",
		    kh, x_src, y_src, w_src, h_src);

	switch (src->pixel_type) {
	%% for pt in pixeltypes
	%%  if not pt.is_unknown() and not pt.is_palette()
	case GP_PIXEL_{{ pt.name }}:
		return v_lin_conv_{{ pt.name }}(src, x_src, y_src, w_src, h_src,
                                                dst, x_dst, y_dst,
		                                kernel, kh, kern_div, callback);
	break;
	%%  endif
	%% endfor
	default:
		errno = EINVAL;
		return -1;
	}
}

%% for pt in pixeltypes
%%  if not pt.is_unknown() and not pt.is_palette()

static int lin_conv_{{ pt.name }}(const GP_Context *src,
                                  GP_Coord x_src, GP_Coord y_src,
                                  GP_Size w_src, GP_Size h_src,
                                  GP_Context *dst,
                                  GP_Coord x_dst, GP_Coord y_dst,
                                  float kernel[], uint32_t kw, uint32_t kh,
                                  float kern_div, GP_ProgressCallback *callback)
{
	GP_Coord x, y;
	unsigned int i, j;

	/* Do linear convolution */
	for (y = 0; y < (GP_Coord)h_src; y++) {
		%% for c in pt.chanslist
		uint32_t {{ c.name }}[kw][kh];
		%% endfor
		GP_Pixel pix;

		/* Prefill the buffer on the start */
		for (j = 0; j < kh; j++) {
			for (i = 0; i < kw - 1; i++) {
				int xi = x_src + i - kw/2;
				int yi = y_src + y + j - kh/2;

				xi = GP_CLAMP(xi, 0, (int)src->w - 1);
				yi = GP_CLAMP(yi, 0, (int)src->h - 1);

				pix = GP_GetPixel_Raw_{{ pt.pixelsize.suffix }}(src, xi, yi);

				%% for c in pt.chanslist
				{{ c.name }}[i][j] = GP_Pixel_GET_{{ c.name }}_{{ pt.name }}(pix);
				%% endfor
			}
		}

		int idx = kw - 1;

		for (x = 0; x < (GP_Coord)w_src; x++) {
			%% for c in pt.chanslist
			float {{ c.name }}_sum = 0;
			%% endfor

			for (j = 0; j < kh; j++) {
				int xi = x_src + x + kw/2;
				int yi = y_src + y + j - kh/2;

				xi = GP_CLAMP(xi, 0, (int)src->w - 1);
				yi = GP_CLAMP(yi, 0, (int)src->h - 1);

				pix = GP_GetPixel_Raw_{{ pt.pixelsize.suffix }}(src, xi, yi);

				%% for c in pt.chanslist
				{{ c.name }}[idx][j] = GP_Pixel_GET_{{ c.name }}_{{ pt.name }}(pix);
				%% endfor
			}

			/* Count the pixel value from neighbours weighted by kernel */
			for (i = 0; i < kw; i++) {
				int k;

				if ((int)i < idx + 1)
					k = kw - idx - 1 + i;
				else
					k = i - idx - 1;

				for (j = 0; j < kh; j++) {
					%% for c in pt.chanslist
					{{ c.name }}_sum += {{ c.name }}[i][j] * kernel[k + j * kw];
					%% endfor
				}
			}

			/* divide the result */
			%% for c in pt.chanslist
			{{ c.name }}_sum /= kern_div;
			%% endfor

			/* and clamp just to be extra sure */
			%% for c in pt.chanslist
			int {{ c.name }}_res = GP_CLAMP((int){{ c.name }}_sum, 0, {{ c.max }});
			%% endfor

			pix = GP_Pixel_CREATE_{{ pt.name }}({{ expand_chanslist(pt, "", "_res") }});

			GP_PutPixel_Raw_{{ pt.pixelsize.suffix }}(dst, x_dst + x, y_dst + y, pix);

			idx++;

			if (idx >= (int)kw)
				idx = 0;
		}

		if (GP_ProgressCallbackReport(callback, y, h_src, w_src))
			return 1;
	}

	GP_ProgressCallbackDone(callback);
	return 0;
}

%%  endif
%% endfor

int GP_FilterLinearConvolution_Raw(const GP_Context *src,
                                   GP_Coord x_src, GP_Coord y_src,
                                   GP_Size w_src, GP_Size h_src,
                                   GP_Context *dst,
                                   GP_Coord x_dst, GP_Coord y_dst,
                                   float kernel[], uint32_t kw, uint32_t kh,
                                   float kern_div, GP_ProgressCallback *callback)
{
	GP_DEBUG(1, "Linear convolution kernel %ix%i rectangle %ux%u",
	            kw, kh, w_src, h_src);

	switch (src->pixel_type) {
	%% for pt in pixeltypes
	%%  if not pt.is_unknown() and not pt.is_palette()
	case GP_PIXEL_{{ pt.name }}:
		return lin_conv_{{ pt.name }}(src, x_src, y_src, w_src, h_src,
                                              dst, x_dst, y_dst,
		                              kernel, kw, kh, kern_div, callback);
	break;
	%%  endif
	%% endfor
	default:
		errno = EINVAL;
		return -1;
	}
}

%% endblock body
