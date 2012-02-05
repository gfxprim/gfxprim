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

#include "core/GP_Transform.h"

#include "gfx/GP_HLine.h"
#include "gfx/GP_VLine.h"
#include "gfx/GP_Rect.h"

void GP_RectXYXY_Raw(GP_Context *context, GP_Coord x0, GP_Coord y0,
                     GP_Coord x1, GP_Coord y1, GP_Pixel pixel)
{
	GP_HLine_Raw(context, x0, x1, y0, pixel);
	GP_HLine_Raw(context, x0, x1, y1, pixel);
	GP_VLine_Raw(context, x0, y0, y1, pixel);
	GP_VLine_Raw(context, x1, y0, y1, pixel);
}

void GP_RectXYWH_Raw(GP_Context *context, GP_Coord x, GP_Coord y,
	             GP_Size w, GP_Size h, GP_Pixel pixel)
{
	if (w == 0 || h == 0)
		return;
	
	GP_RectXYXY_Raw(context, x, y, x + w - 1, y + h - 1, pixel);
}

void GP_RectXYXY(GP_Context *context, GP_Coord x0, GP_Coord y0,
                 GP_Coord x1, GP_Coord y1, GP_Pixel pixel)
{
	GP_CHECK_CONTEXT(context);
	
	GP_TRANSFORM_POINT(context, x0, y0);
	GP_TRANSFORM_POINT(context, x1, y1);

	GP_RectXYXY_Raw(context, x0, y0, x1, y1, pixel);
}

void GP_RectXYWH(GP_Context *context, GP_Coord x, GP_Coord y,
                 GP_Size w, GP_Size h, GP_Pixel pixel)
{
	if (w == 0 || h == 0)
		return;
	
	GP_RectXYXY(context, x, y, x + w - 1, y + h - 1, pixel);
}

void GP_FillRectXYXY_Raw(GP_Context *context, GP_Coord x0, GP_Coord y0,
                         GP_Coord x1, GP_Coord y1, GP_Pixel pixel)
{
	GP_CHECK_CONTEXT(context);

	if (y0 > y1)
		GP_SWAP(y0, y1);

	GP_Coord y;
	for (y = y0; y <= y1; y++)
		GP_HLine_Raw(context, x0, x1, y, pixel);
}

void GP_FillRectXYWH_Raw(GP_Context *context, GP_Coord x, GP_Coord y,
                         GP_Size w, GP_Size h, GP_Pixel pixel)
{
	if (w == 0 || h == 0)
		return;

	GP_FillRectXYXY_Raw(context, x, y, x + w - 1, y + h - 1, pixel);
}

void GP_FillRectXYXY(GP_Context *context, GP_Coord x0, GP_Coord y0,
                     GP_Coord x1, GP_Coord y1, GP_Pixel pixel)
{
	GP_CHECK_CONTEXT(context);
	
	GP_TRANSFORM_POINT(context, x0, y0);
	GP_TRANSFORM_POINT(context, x1, y1);

	GP_FillRect_Raw(context, x0, y0, x1, y1, pixel);
}

void GP_FillRectXYWH(GP_Context *context, GP_Coord x, GP_Coord y,
                     GP_Size w, GP_Size h, GP_Pixel pixel)
{
	if (w == 0 || h == 0)
		return;

	GP_FillRectXYXY(context, x, y, x + w - 1, y + h - 1, pixel);
}
