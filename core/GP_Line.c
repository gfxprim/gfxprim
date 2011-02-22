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
#include "algo/Line.algo.h"
#include "GP_FnPerBpp.h"

DEF_LINE_FN(GP_Line1bpp, GP_Context *, GP_Pixel, GP_PutPixel1bpp)
DEF_LINE_FN(GP_Line8bpp, GP_Context *, GP_Pixel, GP_PutPixel8bpp)
DEF_LINE_FN(GP_Line16bpp, GP_Context *, GP_Pixel, GP_PutPixel16bpp)
DEF_LINE_FN(GP_Line24bpp, GP_Context *, GP_Pixel, GP_PutPixel24bpp)
DEF_LINE_FN(GP_Line32bpp, GP_Context *, GP_Pixel, GP_PutPixel32bpp)

GP_RetCode GP_Line(GP_Context *context, int x0, int y0, int x1, int y1,
                   GP_Pixel pixel)
{
	if (!context)
		return GP_ENULLPTR;
	if (!GP_IS_CONTEXT_VALID(context))
		return GP_EBADCONTEXT;

	GP_FN_PER_BPP(GP_Line, x0, y0, x1, y1, pixel);

	return GP_ESUCCESS;
}

GP_RetCode GP_TLine(GP_Context *context, int x0, int y0, int x1, int y1,
                    GP_Pixel pixel)
{
	if (!context)
		return GP_ENULLPTR;
	if (!GP_IS_CONTEXT_VALID(context))
		return GP_EBADCONTEXT;

	GP_TRANSFORM_POINT(context, x0, y0);
	GP_TRANSFORM_POINT(context, x1, y1);
	return GP_Line(context, x0, y0, x1, y1, pixel);
}

size_t GP_CalcLinePoints(int x0, int y0, int x1, int y1,
	int *points, size_t maxlen)
{
	GP_CHECK(points != NULL || maxlen == 0);
	GP_CHECK(maxlen % 2 == 0);
	
	size_t i = 0;

	int deltax = x1 - x0;
	int deltay = y1 - y0;

	int x, y, error;

	if (abs(deltax) > abs(deltay)) {	/* X changes faster */

		if (deltax < 0) {
			SWAP_ENDPOINTS;
		}

		FOR_EACH_X {
			if (i < maxlen) {
				points[i++] = x;
				points[i++] = y;
			}
			NEXT_X;
		}
	} else {				/* Y changes faster */

		if (deltay < 0) {
			SWAP_ENDPOINTS;
		}

		FOR_EACH_Y {
			if (i < maxlen) {
				points[i++] = x;
				points[i++] = y;
			}
			NEXT_Y;
		}
	}

	return i;
}
