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
 * Copyright (C) 2009-2010 Jiri "BlueBear" Dluhos                            *
 *                         <jiri.bluebear.dluhos@gmail.com>                  *
 *                                                                           *
 * Copyright (C) 2009-2010 Cyril Hrubis <metan@ucw.cz>                       *
 *                                                                           *
 *****************************************************************************/

#include "GP.h"
#include "algo/HLine.algo.h"
#include "GP_FnPerBpp.h"

/* Generate drawing functions for various bit depths. */
DEF_HLINE_BU_FN(GP_HLine1bpp, GP_Context *, GP_Pixel, GP_PIXEL_ADDR, GP_WritePixels1bpp)
DEF_HLINE_BU_FN(GP_HLine2bpp, GP_Context *, GP_Pixel, GP_PIXEL_ADDR, GP_WritePixels2bpp)
DEF_HLINE_BU_FN(GP_HLine4bpp, GP_Context *, GP_Pixel, GP_PIXEL_ADDR, GP_WritePixels4bpp)
DEF_HLINE_FN(GP_HLine8bpp,  GP_Context *, GP_Pixel, GP_PIXEL_ADDR, GP_WritePixels8bpp)
DEF_HLINE_FN(GP_HLine16bpp, GP_Context *, GP_Pixel, GP_PIXEL_ADDR, GP_WritePixels16bpp)
DEF_HLINE_FN(GP_HLine24bpp, GP_Context *, GP_Pixel, GP_PIXEL_ADDR, GP_WritePixels24bpp)
DEF_HLINE_FN(GP_HLine32bpp, GP_Context *, GP_Pixel, GP_PIXEL_ADDR, GP_WritePixels32bpp)

void GP_HLineXXY(GP_Context *context, int x0, int x1, int y, GP_Pixel pixel)
{
	GP_CHECK_CONTEXT(context);
	
	GP_FN_PER_BPP(GP_HLine, context, x0, x1, y, pixel);
}

void GP_HLineXYW(GP_Context *context, int x, int y, unsigned int w,
                 GP_Pixel pixel)
{
	/* zero width: do not draw anything */
	if (w == 0)
		return;

	GP_HLineXXY(context, x, x + w - 1, y, pixel);
}

void GP_THLineXXY(GP_Context *context, int x0, int x1, int y, GP_Pixel pixel)
{
	GP_CHECK_CONTEXT(context);
	
	if (context->axes_swap) {
		GP_TRANSFORM_Y(context, x0);
		GP_TRANSFORM_Y(context, x1);
		GP_TRANSFORM_X(context, y);
		GP_VLine(context, y, x0, x1, pixel);
	} else {
		GP_TRANSFORM_X(context, x0);
		GP_TRANSFORM_X(context, x1);
		GP_TRANSFORM_Y(context, y);
		GP_HLine(context, x0, x1, y, pixel);
	}
}

void GP_THLineXYW(GP_Context *context, int x, int y, unsigned int w,
                  GP_Pixel pixel)
{
	/* zero width: do not draw anything */
	if (w == 0)
		return;

	GP_THLineXXY(context, x, x + w - 1, y, pixel);
}
