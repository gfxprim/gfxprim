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

	/* Outer coordinates */
	GP_Coord out_x0 = GP_FP_FLOOR_TO_INT(x0 + GP_FP_1_2);
	GP_Coord out_y0 = GP_FP_FLOOR_TO_INT(y0 + GP_FP_1_2);
	GP_Coord out_x1 = GP_FP_CEIL_TO_INT(x1 - GP_FP_1_2);
	GP_Coord out_y1 = GP_FP_CEIL_TO_INT(y1 - GP_FP_1_2);
	
	/* Size */
	GP_Size w = x1 - x0;
	GP_Size h = y1 - y0;

	/* Special case, vertical 1px line */
	if (out_x0 == out_x1) {
		uint8_t mix = GP_GammaToLinear(w);
		GP_Coord i;

		/* Special case 1px 100% width line */
		if (w == GP_FP_1)
			mix = 255;

		for (i = out_y0; i <= out_y1; i++) {
			GP_Pixel p = GP_GetPixel_Raw_Clipped(context, out_x0, i);
			p = GP_MixPixels(pixel, p, mix, context->pixel_type);
			GP_PutPixel_Raw_Clipped(context, out_x0, i, p);
		}

		return;
	}

	/* Special case, horizontal 1px line */
	if (out_y0 == out_y1) {
		uint8_t mix = GP_GammaToLinear(h);
		GP_Coord i;
		
		/* Special case 1px 100% height line */
		if (h == GP_FP_1)
			mix = 255;

		for (i = out_x0; i <= out_x1; i++) {
			GP_Pixel p = GP_GetPixel_Raw_Clipped(context, i, out_y0);
			p = GP_MixPixels(pixel, p, mix, context->pixel_type);
			GP_PutPixel_Raw_Clipped(context, i, out_y0, p);
		}

		return;
	}

	/* This are integer coordinates of the "inner" rectangle */
	GP_Coord in_x0 = GP_FP_CEIL_TO_INT(x0 + GP_FP_1_2);
	GP_Coord in_y0 = GP_FP_CEIL_TO_INT(y0 + GP_FP_1_2);
	GP_Coord in_x1 = GP_FP_FLOOR_TO_INT(x1 - GP_FP_1_2);
	GP_Coord in_y1 = GP_FP_FLOOR_TO_INT(y1 - GP_FP_1_2);
	
	/* 
	 * Draw the inner rectanle in 100% intensity.
	 *
	 * Note that if out_x0 == in_x1 is 2px wide and both lines has less than
	 * 100% intensity. The same goes for out_y0 == in_y1.
	 */
	if (in_x1 >= in_x0 && (out_x0 != in_x1 || out_x1 != in_x0) 
	    && in_y1 >= in_y0 && (out_y0 != in_y1 || out_y1 != in_y0))
		GP_FillRectXYXY_Raw(context, in_x0, in_y0, in_x1, in_y1, pixel);

	/* if the outer and innter coordinates doesn't match, draw blurred edge */
	if (in_y0 != out_y0) {
		uint8_t mix = GP_GammaToLinear(GP_FP_FROM_INT(in_y0) + GP_FP_1_2 - y0);
		GP_Coord i;
	
		for (i = out_x0; i <= out_x1; i++) {
			GP_Pixel p = GP_GetPixel_Raw_Clipped(context, i, out_y0);
			p = GP_MixPixels(pixel, p, mix, context->pixel_type);
			GP_PutPixel_Raw_Clipped(context, i, out_y0, p);
		}
	}
	
	if (in_y1 != out_y1) {
		uint8_t mix = GP_GammaToLinear(y1 - GP_FP_FROM_INT(in_y0) - GP_FP_1_2);
		GP_Coord i;
	
		for (i = out_x0; i <= out_x1; i++) {
			GP_Pixel p = GP_GetPixel_Raw_Clipped(context, i, out_y1);
			p = GP_MixPixels(pixel, p, mix, context->pixel_type);
			GP_PutPixel_Raw_Clipped(context, i, out_y1, p);
		}
	}

	if (in_x0 != out_x0) {
		uint8_t mix = GP_GammaToLinear(GP_FP_FROM_INT(in_x0) + GP_FP_1_2 - x0);
		GP_Coord i;
	
		for (i = out_y0; i <= out_y1; i++) {
			GP_Pixel p = GP_GetPixel_Raw_Clipped(context, out_x0, i);
			p = GP_MixPixels(pixel, p, mix, context->pixel_type);
			GP_PutPixel_Raw_Clipped(context, out_x0, i, p);
		}
	}

	if (in_x1 != out_x1) {
		uint8_t mix = GP_GammaToLinear(x1 - GP_FP_FROM_INT(in_x1) - GP_FP_1_2);
		GP_Coord i;
	
		for (i = out_y0; i <= out_y1; i++) {
			GP_Pixel p = GP_GetPixel_Raw_Clipped(context, out_x1, i);
			p = GP_MixPixels(pixel, p, mix, context->pixel_type);
			GP_PutPixel_Raw_Clipped(context, out_x1, i, p);
		}
	}

	//TODO four corner pixels!!!
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

	GP_FillRectXYXY_AA_Raw(context, x0, y0, x1, y1, pixel);
}

void GP_FillRectXYWH_AA(GP_Context *context, GP_Coord x, GP_Coord y,
                        GP_Size w, GP_Size h, GP_Pixel pixel)
{
	if (w == 0 || h == 0)
		return;

	GP_FillRectXYXY_AA(context, x, y,
	                   x + w - GP_FP_1, y + h - GP_FP_1, pixel);
}
