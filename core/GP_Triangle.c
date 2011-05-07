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

void GP_Triangle(GP_Context *context, int x0, int y0, int x1, int y1,
                 int x2, int y2, GP_Pixel pixel)
{
	GP_CHECK_CONTEXT(context);

	GP_Line(context, x0, y0, x1, y1, pixel);
	GP_Line(context, x0, y0, x2, y2, pixel);
	GP_Line(context, x1, y1, x2, y2, pixel);
}

void GP_TTriangle(GP_Context *context, int x0, int y0, int x1, int y1,
                  int x2, int y2, GP_Pixel pixel)
{
	GP_CHECK_CONTEXT(context);
	
	GP_TRANSFORM_POINT(context, x0, y0);
	GP_TRANSFORM_POINT(context, x1, y1);
	GP_TRANSFORM_POINT(context, x2, y2);

	GP_Triangle(context, x0, y0, x1, y1, x2, y2, pixel);
}

void GP_FillTriangle(GP_Context * context, int x0, int y0, int x1, int y1,
                     int x2, int y2, GP_Pixel pixel)
{
	GP_CHECK_CONTEXT(context);

	int coords[6] = { x0, y0, x1, y1, x2, y2 };
	GP_FillPolygon(context, 3, &coords, pixel);
}

void GP_TFillTriangle(GP_Context* context, int x0, int y0, int x1, int y1,
                      int x2, int y2, GP_Pixel pixel)
{
	GP_CHECK_CONTEXT(context);
	
	GP_TRANSFORM_POINT(context, x0, y0);
	GP_TRANSFORM_POINT(context, x1, y1);
	GP_TRANSFORM_POINT(context, x2, y2);
	
	int coords[6] = { x0, y0, x1, y1, x2, y2 };
	GP_FillPolygon(context, 3, &coords, pixel);
}
