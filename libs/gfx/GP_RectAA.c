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
 * Copyright (C) 2009-2012 Cyril Hrubis <metan@ucw.cz>                       *
 *                                                                           *
 *****************************************************************************/

#include "core/GP_Transform.h"
#include "core/GP_GetPutPixel.h"
#include "core/GP_MixPixels.h"
#include "core/GP_FixedPoint.h"
#include "core/GP_GammaCorrection.h"
#include "GP_Rect.h"
#include "GP_RectAA.h"

void GP_FillRectXYXY_AA_Raw(GP_Context *context, GP_Coord x0, GP_Coord y0,
                            GP_Coord x1, GP_Coord y1, GP_Pixel pixel)
{
	GP_CHECK_CONTEXT(context);

	if (x0 > x1)
		GP_SWAP(x0, x1);

	if (y0 > y1)
		GP_SWAP(y0, y1);

	GP_Size w = x1 - x0;
	GP_Size h = y1 - y0;

	printf("W = %f, H = %f, X = %f, Y = %f\n", 
	       GP_FP_TO_FLOAT(w), GP_FP_TO_FLOAT(h),
	       GP_FP_TO_FLOAT(x0), GP_FP_TO_FLOAT(y0));

	/* This are integer coordinates of the "inner" rectangle */
	GP_Coord xi0 = GP_FP_CEIL(x0);
	GP_Coord yi0 = GP_FP_CEIL(y0);
	GP_Coord xi1 = GP_FP_FLOOR(x1);
	GP_Coord yi1 = GP_FP_FLOOR(y1);

	if (xi1 >= xi0 && yi1 >= yi0)
		GP_FillRect(context, xi0, yi0, xi1, yi1, pixel);
	
	printf("%i %i %i %i\n", xi0, yi0, yi1, yi1);

	/* Draw the "frame" around */
	GP_Coord i;

	uint8_t u_perc;
	uint8_t d_perc;
	
	u_perc = GP_GammaToLinear(GP_FP_1 - GP_FP_FRAC(y0));
	d_perc = GP_GammaToLinear(GP_FP_FRAC(y1));

	for (i = GP_FP_CEIL(x0); i <= GP_FP_FLOOR(x1); i++) {
		GP_Pixel u = GP_GetPixel(context, i, GP_FP_FLOOR(y0));
		GP_Pixel d = GP_GetPixel(context, i, GP_FP_CEIL(y1));
		
		u = GP_MixPixels(pixel, u, u_perc, context->pixel_type);
		d = GP_MixPixels(pixel, d, d_perc, context->pixel_type);
		
		GP_PutPixel(context, i, GP_FP_FLOOR(y0), u);
		GP_PutPixel(context, i, GP_FP_CEIL(y1), d);
	}

	u_perc = GP_GammaToLinear(GP_FP_1 - GP_FP_FRAC(x0));
	d_perc = GP_GammaToLinear(GP_FP_FRAC(x1));
	
	for (i = GP_FP_CEIL(y0); i <= GP_FP_FLOOR(y1); i++) {
		GP_Pixel u = GP_GetPixel(context, GP_FP_FLOOR(x0), i);
		GP_Pixel d = GP_GetPixel(context, GP_FP_CEIL(x1), i);
		
		u = GP_MixPixels(pixel, u, u_perc, context->pixel_type);
		d = GP_MixPixels(pixel, d, d_perc, context->pixel_type);
		
		GP_PutPixel(context, GP_FP_FLOOR(x0), i, u);
		GP_PutPixel(context, GP_FP_CEIL(x1), i, d);
	}

	return;

	uint8_t perc;
	GP_Pixel p;

	perc = GP_GammaToLinear((GP_FP_FRAC(x0) + GP_FP_FRAC(y0) + 1)/2);
	p = GP_GetPixel(context, GP_FP_FLOOR(x0), GP_FP_FLOOR(y0));
	p = GP_MixPixels(pixel, p, perc, context->pixel_type);
	GP_PutPixel(context, GP_FP_FLOOR(x0), GP_FP_FLOOR(y0), p);

	perc = GP_GammaToLinear((GP_FP_FRAC(x1) + GP_FP_FRAC(y0) + 1)/2);
	p = GP_GetPixel(context, (x1>>8) + 1, (y0>>8) - 1);
	p = GP_MixPixels(pixel, p, perc, context->pixel_type);
	GP_PutPixel(context, (x1>>8) + 1, (y0>>8) - 1, p);

	perc = GP_GammaToLinear((GP_FP_FRAC(x0) + GP_FP_FRAC(y1) + 1)/2);
	p = GP_GetPixel(context, (x0>>8) - 1, (y1>>8) + 1);
	p = GP_MixPixels(pixel, p, perc, context->pixel_type);
	GP_PutPixel(context, (x0>>8) - 1, (y1>>8) + 1, p);
	
	perc = GP_GammaToLinear((GP_FP_FRAC(x1) + GP_FP_FRAC(y1) + 1)/2);
	p = GP_GetPixel(context, (x1>>8) + 1, (y1>>8) + 1);
	p = GP_MixPixels(pixel, p, perc, context->pixel_type);
	GP_PutPixel(context, (x1>>8) + 1, (y1>>8) + 1, p);
}

void GP_FillRectXYWH_AA_Raw(GP_Context *context, GP_Coord x, GP_Coord y,
                            GP_Size w, GP_Size h, GP_Pixel pixel)
{
	if (w == 0 || h == 0)
		return;

	GP_FillRectXYXY_AA_Raw(context, x, y,
	                       x + w - GP_FP_1, y + h - GP_FP_1, pixel);
}

void GP_FillRectXYXY_AA(GP_Context *context, GP_Coord x0, GP_Coord y0,
                        GP_Coord x1, GP_Coord y1, GP_Pixel pixel)
{
	GP_CHECK_CONTEXT(context);
	
	GP_TRANSFORM_POINT_FP(context, x0, y0);
	GP_TRANSFORM_POINT_FP(context, x1, y1);

	GP_FillRect_AA_Raw(context, x0, y0, x1, y1, pixel);
}

void GP_FillRectXYWH_AA(GP_Context *context, GP_Coord x, GP_Coord y,
                        GP_Size w, GP_Size h, GP_Pixel pixel)
{
	if (w == 0 || h == 0)
		return;

	GP_FillRectXYXY_AA(context, x, y,
	                   x + w - GP_FP_1, y + h - GP_FP_1, pixel);
}
