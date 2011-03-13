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
#include "algo/VLine.algo.h"
#include "GP_FnPerBpp.h"

/* Generate drawing functions for various bit depths. */
DEF_VLINE_FN(GP_VLine1bpp,  GP_Context *, GP_Pixel, GP_PutPixel1bpp)
DEF_VLINE_FN(GP_VLine2bpp,  GP_Context *, GP_Pixel, GP_PutPixel2bpp)
DEF_VLINE_FN(GP_VLine4bpp,  GP_Context *, GP_Pixel, GP_PutPixel4bpp)
DEF_VLINE_FN(GP_VLine8bpp,  GP_Context *, GP_Pixel, GP_PutPixel8bpp)
DEF_VLINE_FN(GP_VLine16bpp, GP_Context *, GP_Pixel, GP_PutPixel16bpp)
DEF_VLINE_FN(GP_VLine24bpp, GP_Context *, GP_Pixel, GP_PutPixel24bpp)
DEF_VLINE_FN(GP_VLine32bpp, GP_Context *, GP_Pixel, GP_PutPixel32bpp)

GP_RetCode GP_VLineXYY(GP_Context *context, int x, int y0, int y1, GP_Pixel pixel)
{
	if (!context)
		return GP_ENULLPTR;
	if (!GP_IS_CONTEXT_VALID(context))
		return GP_EBADCONTEXT;

	GP_FN_PER_BPP(GP_VLine, context, x, y0, y1, pixel);
}

GP_RetCode GP_VLineXYH(GP_Context *context, int x, int y, unsigned int height,
	GP_Pixel pixel)
{
	if (height == 0)
		return GP_ESUCCESS;	/* zero height: do not draw anything */

	return GP_VLineXYY(context, x, y, y + height - 1, pixel);
}

GP_RetCode GP_TVLineXYY(GP_Context *context, int x, int y0, int y1, GP_Pixel pixel)
{
	if (!context)
		return GP_ENULLPTR;
	if (!GP_IS_CONTEXT_VALID(context))
		return GP_EBADCONTEXT;

	if (context->axes_swap) {
		GP_TRANSFORM_Y(context, x);
		GP_TRANSFORM_X(context, y0);
		GP_TRANSFORM_X(context, y1);
		return GP_HLine(context, y0, y1, x, pixel);
	}

	GP_TRANSFORM_X(context, x);
	GP_TRANSFORM_Y(context, y0);
	GP_TRANSFORM_Y(context, y1);
	return GP_VLine(context, x, y0, y1, pixel);
}

GP_RetCode GP_TVLineXYH(GP_Context *context, int x, int y, unsigned int height,
	GP_Pixel pixel)
{
	if (height == 0)
		return GP_ESUCCESS;	/* zero height: do not draw anything */

	return GP_TVLineXYY(context, x, y, y + height - 1, pixel);
}
