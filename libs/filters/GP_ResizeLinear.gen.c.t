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

{% block descr %}Linear resampling{% endblock %}

%% block body

#include "core/GP_Context.h"
#include "core/GP_GetPutPixel.h"
#include "core/GP_Gamma.h"

#include "core/GP_Debug.h"

#include "GP_Resize.h"

/*
 * Sample row.
 *
 * The x and y are starting coordinates in source image.
 *
 * The xpix_dist is distance of two sampled pixels in source image coordinates.
 *
 * The xoff is offset of the first pixel.
 *
 * The r, g, b are used to store resulting values.
 */
static inline void linear_lp_sample_x(const GP_Context *src,
                                      uint32_t x, uint32_t y,
                                      uint32_t xpix_dist, uint32_t xoff,
                                      uint32_t *r, uint32_t *g, uint32_t *b)
{
	GP_Pixel pix;
	uint32_t i;

	pix = GP_GetPixel_Raw_24BPP(src, x, y);

	*r = (GP_Pixel_GET_R_RGB888(pix) * xoff) >> 9;
	*g = (GP_Pixel_GET_G_RGB888(pix) * xoff) >> 9;
	*b = (GP_Pixel_GET_B_RGB888(pix) * xoff) >> 9;

	for (i = (1<<14) - xoff; i > xpix_dist; i -= xpix_dist) {
		if (x < src->w - 1)
			x++;
		
		pix = GP_GetPixel_Raw_24BPP(src, x, y);

		*r += (GP_Pixel_GET_R_RGB888(pix) * xpix_dist) >> 9;
		*g += (GP_Pixel_GET_G_RGB888(pix) * xpix_dist) >> 9;
		*b += (GP_Pixel_GET_B_RGB888(pix) * xpix_dist) >> 9;
	}

	if (i > 0) {
		if (x < src->w - 1)
			x++;
			
		pix = GP_GetPixel_Raw_24BPP(src, x, y);

		*r += (GP_Pixel_GET_R_RGB888(pix) * i) >> 9;
		*g += (GP_Pixel_GET_G_RGB888(pix) * i) >> 9;
		*b += (GP_Pixel_GET_B_RGB888(pix) * i) >> 9;
	}
}

/*
 * Linear interpolation with low-pass filtering, used for fast downscaling
 * on both x and y.
 *
 * Basically we do weighted arithmetic mean of the pixels:
 *
 * [x, y],    [x + 1, y], [x + 2, y] ... [x + k, y]
 * [x, y + 1]
 * [x, y + 2]                            .
 * .                      .              .
 * .                          .          .
 * .                              .
 * [x, y + l]        ....                [x + k, y + l]
 *
 *
 * The parameter k respectively l is determined by the distance between
 * sampled coordinates on x respectively y.
 *
 * The pixels are weighted by how much they are 'hit' by the rectangle defined
 * by the sampled pixel.
 * 
 * The implementation is inspired by imlib2 downscaling algorithm.
 */
static int interpolate_linear_lp_xy(const GP_Context *src, GP_Context *dst,
                                    GP_ProgressCallback *callback)
{
	uint32_t xmap[dst->w + 1];
	uint32_t ymap[dst->h + 1];
	uint16_t xoff[dst->w + 1];
	uint16_t yoff[dst->h + 1];
	uint32_t x, y;
	uint32_t i, j;
	
	/* Pre-compute mapping for interpolation */
	uint32_t xstep = ((src->w - 1) << 16) / (dst->w - 1);
	uint32_t xpix_dist = ((dst->w - 1) << 14) / (src->w - 1);

	for (i = 0; i < dst->w + 1; i++) {
		uint32_t val = i * xstep;
		xmap[i] = val >> 16;
		xoff[i] = ((255 - ((val >> 8) & 0xff)) * xpix_dist)>>8;
	}

	uint32_t ystep = ((src->h - 1) << 16) / (dst->h - 1);
	uint32_t ypix_dist = ((dst->h - 1) << 14) / (src->h - 1);

	for (i = 0; i < dst->h + 1; i++) {
		uint32_t val = i * ystep;
		ymap[i] = val >> 16;
		yoff[i] = ((255 - ((val >> 8) & 0xff)) * ypix_dist)>>8;
	}

	/* Interpolate */
	for (y = 0; y < dst->h; y++) {
		for (x = 0; x < dst->w; x++) {
			uint32_t r, g, b;
			uint32_t r1, g1, b1;
			uint32_t x0, y0;

			x0 = xmap[x];
			y0 = ymap[y];

			linear_lp_sample_x(src, x0, y0,
			                   xpix_dist, xoff[x],
			                   &r, &g, &b);

			r = (r * yoff[y]) >> 14;
			g = (g * yoff[y]) >> 14;
			b = (b * yoff[y]) >> 14;
			
			for (j = (1<<14) - yoff[y]; j > ypix_dist; j -= ypix_dist) {
				
				x0 = xmap[x];
				
				if (y0 < src->h - 1)
					y0++;
			
				linear_lp_sample_x(src, x0, y0,
				                   xpix_dist, xoff[x],
			                           &r1, &g1, &b1);

				r += (r1 * ypix_dist) >> 14;
				g += (g1 * ypix_dist) >> 14;
				b += (b1 * ypix_dist) >> 14;
			}

			if (j > 0) {
				x0 = xmap[x];
				
				if (y0 < src->h - 1)
					y0++;
				
				linear_lp_sample_x(src, x0, y0,
				                   xpix_dist, xoff[x],
			                           &r1, &g1, &b1);
			
				r += (r1 * j) >> 14;
				g += (g1 * j) >> 14;
				b += (b1 * j) >> 14;
			}

			r = (r + (1<<4))>>5;
			g = (g + (1<<4))>>5;
			b = (b + (1<<4))>>5;

			GP_PutPixel_Raw_24BPP(dst, x, y,
			                      GP_Pixel_CREATE_RGB888(r, g, b));
		}
		
		if (GP_ProgressCallbackReport(callback, y, dst->h, dst->w))
			return 1;
	}

	GP_ProgressCallbackDone(callback);
	return 0;
}

%% for pt in pixeltypes
%%  if not pt.is_unknown() and not pt.is_palette()

static int GP_FilterResizeLinearInt_{{ pt.name }}_Raw(const GP_Context *src, GP_Context *dst,
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

int GP_FilterResizeLinearInt_Raw(const GP_Context *src, GP_Context *dst,
                                 GP_ProgressCallback *callback)
{
	switch (src->pixel_type) {
	%% for pt in pixeltypes
	%% if not pt.is_unknown() and not pt.is_palette()
	case GP_PIXEL_{{ pt.name }}:
		return GP_FilterResizeLinearInt_{{ pt.name }}_Raw(src, dst, callback);
	break;
	%% endif
	%% endfor
	default:
		return -1;
	}
}

int GP_FilterResizeLinearLFInt_Raw(const GP_Context *src, GP_Context *dst,
                                   GP_ProgressCallback *callback)
{
	float x_rat = 1.00 * dst->w / src->w;
	float y_rat = 1.00 * dst->h / src->h;

	printf("%f %f\n", x_rat, y_rat);

	if (x_rat < 1.00 && y_rat < 1.00) {

		/* Fix. */
		if (src->pixel_type != GP_PIXEL_RGB888)
			return -1;
		
		GP_DEBUG(1, "Downscaling image %ux%u -> %ux%u %2.2f %2.2f",
	                     src->w, src->h, dst->w, dst->h, x_rat, y_rat);
		return interpolate_linear_lp_xy(src, dst, callback);
	}

	//TODO: x_rat > 1.00 && y_rat < 1.00
	//TODO: x_rat < 1.00 && y_rat > 1.00

	return GP_FilterResizeLinearInt_Raw(src, dst, callback);
}

%% endblock body
