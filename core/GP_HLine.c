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

DEF_HLINE_FN(GP_HLine8bpp, GP_Context *, GP_Pixel, GP_PIXEL_ADDRESS, GP_WritePixels8bpp)
DEF_HLINE_FN(GP_HLine16bpp, GP_Context *, GP_Pixel, GP_PIXEL_ADDRESS, GP_WritePixels16bpp)
DEF_HLINE_FN(GP_HLine24bpp, GP_Context *, GP_Pixel, GP_PIXEL_ADDRESS, GP_WritePixels24bpp)
DEF_HLINE_FN(GP_HLine32bpp, GP_Context *, GP_Pixel, GP_PIXEL_ADDRESS, GP_WritePixels32bpp)

GP_RetCode GP_HLineXXY(GP_Context *context, int x0, int x1, int y, GP_Pixel pixel)
{
	if (!context)
		return GP_ENULLPTR;
	if (!GP_IS_CONTEXT_VALID(context))
		return GP_EBADCONTEXT;

	GP_FN_PER_BPP(GP_HLine, x0, x1, y, pixel);

	return GP_ESUCCESS;
}

GP_RetCode GP_HLineXYW(GP_Context *context, int x, int y, unsigned int w,
	GP_Pixel pixel)
{
	if (w == 0)
		return GP_ESUCCESS;	/* zero width: do not draw anything */

	return GP_HLineXXY(context, x, y, x + w - 1, pixel);
}

GP_RetCode GP_THLineXXY(GP_Context *context, int x0, int x1, int y, GP_Pixel pixel)
{
	if (!context)
		return GP_ENULLPTR;
	if (!GP_IS_CONTEXT_VALID(context))
		return GP_EBADCONTEXT;

	if (context->axes_swap) {
		GP_TRANSFORM_Y(context, x0);
		GP_TRANSFORM_Y(context, x1);
		GP_TRANSFORM_X(context, y);
		return GP_VLine(context, y, x0, x1, pixel);
	}

	GP_TRANSFORM_X(context, x0);
	GP_TRANSFORM_X(context, x1);
	GP_TRANSFORM_Y(context, y);
	return GP_HLine(context, x0, x1, y, pixel);
}

GP_RetCode GP_THLineXYW(GP_Context *context, int x, int y, unsigned int w,
	GP_Pixel pixel)
{
	if (w == 0)
		return GP_ESUCCESS;	/* zero width: do not draw anything */

	return GP_THLineXXY(context, x, y, x + w - 1, pixel);
}
