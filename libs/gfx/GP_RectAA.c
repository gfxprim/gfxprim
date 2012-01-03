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

	/* Draw the full part of the rect */
	GP_FillRect(context, x0>>8, y0>>8, x1>>8, y1>>8, pixel);
	
	/* Draw the "frame" around */
	GP_Coord i;

	uint8_t u_perc;
	uint8_t d_perc;
	
	d_perc = GP_GammaToLinear(x0%8);
	u_perc = GP_GammaToLinear(x1%8);

	for (i = x0>>8; i <= x1>>8; i++) {
		GP_Pixel u = GP_GetPixel(context, i, (y0>>8) - 1);
		GP_Pixel d = GP_GetPixel(context, i, (y1>>8) + 1);
		
		u = GP_MixPixels(pixel, u, u_perc, context->pixel_type);
		d = GP_MixPixels(pixel, d, d_perc, context->pixel_type);
		
		GP_PutPixel(context, i, (y0>>8) - 1, u);
		GP_PutPixel(context, i, (y1>>8) + 1, d);
	}
	
	d_perc = GP_GammaToLinear(y0%8);
	u_perc = GP_GammaToLinear(y1%8);
	
	for (i = y0>>8; i <= y1>>8; i++) {
		GP_Pixel u = GP_GetPixel(context, (x0>>8) - 1, i);
		GP_Pixel d = GP_GetPixel(context, (x1>>8) + 1, i);
		
		u = GP_MixPixels(pixel, u, u_perc, context->pixel_type);
		d = GP_MixPixels(pixel, d, d_perc, context->pixel_type);
		
		GP_PutPixel(context, (x0>>8) - 1, i, u);
		GP_PutPixel(context, (x1>>8) + 1, i, d);
	}

	uint8_t perc;
	GP_Pixel p;

	perc = GP_GammaToLinear((x0%8+y0%8)/2);
	p = GP_GetPixel(context, (x0>>8) - 1, (y0>>8) - 1);
	p = GP_MixPixels(pixel, p, perc, context->pixel_type);
	GP_PutPixel(context, (x0>>8) - 1, (y0>>8) - 1, p);

	perc = GP_GammaToLinear((x1%8+y0%8)/2);
	p = GP_GetPixel(context, (x1>>8) + 1, (y0>>8) - 1);
	p = GP_MixPixels(pixel, p, perc, context->pixel_type);
	GP_PutPixel(context, (x1>>8) + 1, (y0>>8) - 1, p);

	perc = GP_GammaToLinear((x0%8+y1%8)/2);
	p = GP_GetPixel(context, (x0>>8) - 1, (y1>>8) + 1);
	p = GP_MixPixels(pixel, p, perc, context->pixel_type);
	GP_PutPixel(context, (x0>>8) - 1, (y1>>8) + 1, p);
	
	perc = GP_GammaToLinear((x1%8+y1%8)/2);
	p = GP_GetPixel(context, (x1>>8) + 1, (y1>>8) + 1);
	p = GP_MixPixels(pixel, p, perc, context->pixel_type);
	GP_PutPixel(context, (x1>>8) + 1, (y1>>8) + 1, p);
}

void GP_FillRectXYWH_AA_Raw(GP_Context *context, GP_Coord x, GP_Coord y,
                            GP_Size w, GP_Size h, GP_Pixel pixel)
{
	if (w == 0 || h == 0)
		return;

	GP_FillRectXYXY_AA_Raw(context, x, y, x + w - 1, y + h - 1, pixel);
}

void GP_FillRectXYXY_AA(GP_Context *context, GP_Coord x0, GP_Coord y0,
                        GP_Coord x1, GP_Coord y1, GP_Pixel pixel)
{
	GP_CHECK_CONTEXT(context);
	
	GP_TRANSFORM_POINT(context, x0, y0);
	GP_TRANSFORM_POINT(context, x1, y1);

	GP_FillRect_AA_Raw(context, x0, y0, x1, y1, pixel);
}

void GP_FillRectXYWH_AA(GP_Context *context, GP_Coord x, GP_Coord y,
                        GP_Size w, GP_Size h, GP_Pixel pixel)
{
	if (w == 0 || h == 0)
		return;

	GP_FillRectXYXY_AA(context, x, y, x + w - 1, y + h - 1, pixel);
}
