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

#include <stdlib.h>

/* Produce low-level definitions of GP_Line() optimized for various
 * bit depths.
 */
#define CONTEXT_T GP_Context *
#define COLOR_T GP_Pixel
	#define PUTPIXEL GP_PutPixel8bpp
	#define FN_NAME GP_Line8bpp
		#include "algo/Line.algo.c"
	#undef PUTPIXEL
	#undef FN_NAME
	#define PUTPIXEL GP_PutPixel16bpp
	#define FN_NAME GP_Line16bpp
		#include "algo/Line.algo.c"
	#undef PUTPIXEL
	#undef FN_NAME
	#define PUTPIXEL GP_PutPixel24bpp
	#define FN_NAME GP_Line24bpp
		#include "algo/Line.algo.c"
	#undef PUTPIXEL
	#undef FN_NAME
	#define PUTPIXEL GP_PutPixel32bpp
	#define FN_NAME GP_Line32bpp
		#include "algo/Line.algo.c"
	#undef PUTPIXEL
	#undef FN_NAME
#undef CONTEXT_T
#undef COLOR_T

GP_RetCode GP_Line(GP_Context *context, int x0, int y0, int x1, int y1,
                   GP_Color color)
{
	GP_CHECK_CONTEXT(context);

	GP_Pixel pixel;
	pixel.type = context->pixel_type;
	GP_ColorToPixel(color, &pixel);

	switch (context->bits_per_pixel) {
	case 8:
		GP_Line8bpp(context, x0, y0, x1, y1, pixel);
		break;

	case 16:
		GP_Line16bpp(context, x0, y0, x1, y1, pixel);
		break;
	
	case 24:
		GP_Line24bpp(context, x0, y0, x1, y1, pixel);
		break;
	
	case 32:
		GP_Line32bpp(context, x0, y0, x1, y1, pixel);
		break;
	
	default:
		return GP_ENOIMPL;
	}

	return GP_ESUCCESS;
}

GP_RetCode GP_TLine(GP_Context *context, int x0, int y0, int x1, int y1,
                    GP_Color color)
{
	GP_TRANSFORM_POINT(context, x0, y0);
	GP_TRANSFORM_POINT(context, x1, y1);
	return GP_Line(context, x0, y0, x1, y1, color);
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
