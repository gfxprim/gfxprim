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
 * Copyright (C) 2009-2011 Jiri "BlueBear" Dluhos                            *
 *                         <jiri.bluebear.dluhos@gmail.com>                  *
 *                                                                           *
 * Copyright (C) 2009-2012 Cyril Hrubis <metan@ucw.cz>                       *
 *                                                                           *
 *****************************************************************************/

%% extends "base.c.t"

{% block descr %}Horizontal line drawing{% endblock %}

%% block body

#include "core/GP_FnPerBpp.h"
#include "core/GP_WritePixel.h"
#include "core/GP_Transform.h"

/* #include "algo/HLine.algo.h" */

#include "gfx/GP_HLine.h"
#include "gfx/GP_VLine.h"

%% for ps in pixelsizes

void GP_HLine_Raw_{{ ps.suffix }}(GP_Context *context, int x0, int x1, int y,
			       GP_Pixel pixel)
{
	/* draw always from left to right, swap coords if necessary */
	if (x0 > x1)
		GP_SWAP(x0, x1);

	/* return immediately if the line is completely out of surface */
	if (y < 0 || y >= (int) context->h ||
	    x1 < 0 || x0 >= (int) context->w)
		return;

	/* clip the line against surface boundaries */
	x0 = GP_MAX(x0, 0);
	x1 = GP_MIN(x1, (int) context->w - 1);

	size_t length = 1 + x1 - x0;
	void *start = GP_PIXEL_ADDR(context, x0, y);

%% if ps.needs_bit_endian()
	GP_WritePixels_{{ ps.suffix }}(start, x0 % (8 / context->bpp), length, pixel);
%% else
	GP_WritePixels_{{ ps.suffix }}(start, length, pixel);
%% endif

}

%% endfor

/* Generate drawing functions for various bit depths. */

//TODO: BIT ENDIANESS
/*
DEF_HLINE_BU_FN(GP_HLine_Raw_1BPP_LE, GP_Context*, GP_Pixel, GP_PIXEL_ADDR, GP_WritePixels_1BPP_LE)
DEF_HLINE_BU_FN(GP_HLine_Raw_1BPP_BE, GP_Context*, GP_Pixel, GP_PIXEL_ADDR, GP_WritePixels1bpp)
DEF_HLINE_BU_FN(GP_HLine_Raw_2BPP_LE, GP_Context*, GP_Pixel, GP_PIXEL_ADDR, GP_WritePixels_2BPP_LE)
DEF_HLINE_BU_FN(GP_HLine_Raw_2BPP_BE, GP_Context*, GP_Pixel, GP_PIXEL_ADDR, GP_WritePixels2bpp)
DEF_HLINE_BU_FN(GP_HLine_Raw_4BPP_LE, GP_Context*, GP_Pixel, GP_PIXEL_ADDR, GP_WritePixels_4BPP_LE)
DEF_HLINE_BU_FN(GP_HLine_Raw_4BPP_BE, GP_Context*, GP_Pixel, GP_PIXEL_ADDR, GP_WritePixels4bpp)
DEF_HLINE_BU_FN(GP_HLine_Raw_18BPP_LE, GP_Context *, GP_Pixel, GP_PIXEL_ADDR, GP_WritePixels18bpp)

DEF_HLINE_FN(GP_HLine_Raw_8BPP,  GP_Context *, GP_Pixel, GP_PIXEL_ADDR, GP_WritePixels8bpp)
DEF_HLINE_FN(GP_HLine_Raw_16BPP, GP_Context *, GP_Pixel, GP_PIXEL_ADDR, GP_WritePixels16bpp)
DEF_HLINE_FN(GP_HLine_Raw_24BPP, GP_Context *, GP_Pixel, GP_PIXEL_ADDR, GP_WritePixels24bpp)
DEF_HLINE_FN(GP_HLine_Raw_32BPP, GP_Context *, GP_Pixel, GP_PIXEL_ADDR, GP_WritePixels32bpp)
*/

void GP_HLineXXY_Raw(GP_Context *context, GP_Coord x0, GP_Coord x1,
                     GP_Coord y, GP_Pixel pixel)
{
	GP_CHECK_CONTEXT(context);

	GP_FN_PER_BPP_CONTEXT(GP_HLine_Raw, context, context, x0, x1, y,
	                      pixel);
}

void GP_HLineXYW_Raw(GP_Context *context, GP_Coord x, GP_Coord y, GP_Size w,
                     GP_Pixel pixel)
{
	if (w == 0)
		return;

	GP_HLineXXY_Raw(context, x, x + w - 1, y, pixel);
}

void GP_HLineXXY(GP_Context *context, GP_Coord x0, GP_Coord x1, GP_Coord y,
                     GP_Pixel pixel)
{
	GP_CHECK_CONTEXT(context);

	if (context->axes_swap) {
		GP_TRANSFORM_Y(context, x0);
		GP_TRANSFORM_Y(context, x1);
		GP_TRANSFORM_X(context, y);
		GP_VLine_Raw(context, y, x0, x1, pixel);
	} else {
		GP_TRANSFORM_X(context, x0);
		GP_TRANSFORM_X(context, x1);
		GP_TRANSFORM_Y(context, y);
		GP_HLine_Raw(context, x0, x1, y, pixel);
	}
}

void GP_HLineXYW(GP_Context *context, GP_Coord x, GP_Coord y, GP_Size w,
                 GP_Pixel pixel)
{
	if (w == 0)
		return;

	GP_HLineXXY(context, x, x + w - 1, y, pixel);
}

%% endblock body
