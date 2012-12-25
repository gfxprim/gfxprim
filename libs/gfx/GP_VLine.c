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

#include "core/GP_Common.h"
#include "core/GP_GetPutPixel.h"
#include "core/GP_FnPerBpp.h"

#include "gfx/GP_VLine.h"
#include "gfx/GP_HLine.h"

/* 
 * Ensures that coordinates are in correct order, and clips them.
 * Exits immediately if the line is completely clipped out.
 */
#define ORDER_AND_CLIP_COORDS do {             \
	if (y0 > y1)                           \
		GP_SWAP(y0, y1);               \
	if (x < 0 || x >= (int) context->w ||  \
	    y1 < 0 || y0 >= (int) context->h)  \
		return;                        \
	y0 = GP_MAX(y0, 0);                    \
	y1 = GP_MIN(y1, (int) context->h - 1); \
} while (0)

void GP_VLineXYY_Raw(GP_Context *context, GP_Coord x, GP_Coord y0,
                     GP_Coord y1, GP_Pixel pixel)
{
	GP_CHECK_CONTEXT(context);
	
	ORDER_AND_CLIP_COORDS;

	GP_FN_PER_BPP_CONTEXT(GP_VLine_Raw, context, context, x, y0, y1, pixel);
}

void GP_VLineXYH_Raw(GP_Context *context, GP_Coord x, GP_Coord y, GP_Size h,
                     GP_Pixel pixel)
{
	if (h == 0)
		return;

	GP_VLineXYY(context, x, y, y + h - 1, pixel);
}

void GP_VLineXYY(GP_Context *context, GP_Coord x, GP_Coord y0,
                  GP_Coord y1, GP_Pixel pixel)
{
	GP_CHECK_CONTEXT(context);
	
	if (context->axes_swap) {
		GP_TRANSFORM_Y(context, x);
		GP_TRANSFORM_X(context, y0);
		GP_TRANSFORM_X(context, y1);
		GP_HLine_Raw(context, y0, y1, x, pixel);
	} else {
		GP_TRANSFORM_X(context, x);
		GP_TRANSFORM_Y(context, y0);
		GP_TRANSFORM_Y(context, y1);
		GP_VLine_Raw(context, x, y0, y1, pixel);
	}
}

void GP_VLineXYH(GP_Context *context, GP_Coord x, GP_Coord y, GP_Size h,
                 GP_Pixel pixel)
{
	if (h == 0)
		return;

	GP_VLineXYY(context, x, y, y + h - 1, pixel);
}
