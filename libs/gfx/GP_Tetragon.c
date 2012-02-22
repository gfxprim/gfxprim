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

#include "gfx/GP_Line.h"
#include "gfx/GP_Polygon.h"
#include "gfx/GP_Tetragon.h"

void GP_Tetragon_Raw(GP_Context *context, GP_Coord x0, GP_Coord y0,
                     GP_Coord x1, GP_Coord y1, GP_Coord x2, GP_Coord y2,
                     GP_Coord x3, GP_Coord y3, GP_Pixel pixel)
{
	GP_CHECK_CONTEXT(context);

	GP_Line_Raw(context, x0, y0, x1, y1, pixel);
	GP_Line_Raw(context, x1, y1, x2, y2, pixel);
	GP_Line_Raw(context, x2, y2, x3, y3, pixel);
	GP_Line_Raw(context, x3, y3, x0, y0, pixel);
}

void GP_Tetragon(GP_Context *context, GP_Coord x0, GP_Coord y0,
                 GP_Coord x1, GP_Coord y1, GP_Coord x2, GP_Coord y2,
                 GP_Coord x3, GP_Coord y3, GP_Pixel pixel)
{
	GP_CHECK_CONTEXT(context);
	
	GP_TRANSFORM_POINT(context, x0, y0);
	GP_TRANSFORM_POINT(context, x1, y1);
	GP_TRANSFORM_POINT(context, x2, y2);
	GP_TRANSFORM_POINT(context, x3, y3);

	GP_Tetragon_Raw(context, x0, y0, x1, y1, x2, y2, x3, y3, pixel);
}

void GP_FillTetragon_Raw(GP_Context *context, GP_Coord x0, GP_Coord y0,
                         GP_Coord x1, GP_Coord y1, GP_Coord x2, GP_Coord y2,
                         GP_Coord x3, GP_Coord y3, GP_Pixel pixel)
{
	const GP_Coord xy[8] = {x0, y0, x1, y1, x2, y2, x3, y3};
	
	GP_FillPolygon_Raw(context, 4, xy, pixel);
}

void GP_FillTetragon(GP_Context* context, GP_Coord x0, GP_Coord y0,
                     GP_Coord x1, GP_Coord y1, GP_Coord x2, GP_Coord y2,
                     GP_Coord x3, GP_Coord y3, GP_Pixel pixel)
{
	GP_CHECK_CONTEXT(context);
	
	GP_TRANSFORM_POINT(context, x0, y0);
	GP_TRANSFORM_POINT(context, x1, y1);
	GP_TRANSFORM_POINT(context, x2, y2);
	GP_TRANSFORM_POINT(context, x3, y3);

	const GP_Coord xy[8] = {x0, y0, x1, y1, x2, y2, x3, y3};

	GP_FillPolygon_Raw(context, 4, xy, pixel);
}
