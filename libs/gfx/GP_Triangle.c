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
#include "gfx/GP_Triangle.h"

void GP_Triangle_Raw(GP_Pixmap *pixmap, GP_Coord x0, GP_Coord y0,
                     GP_Coord x1, GP_Coord y1,
                     GP_Coord x2, GP_Coord y2, GP_Pixel pixel)
{
	GP_Line_Raw(pixmap, x0, y0, x1, y1, pixel);
	GP_Line_Raw(pixmap, x0, y0, x2, y2, pixel);
	GP_Line_Raw(pixmap, x1, y1, x2, y2, pixel);
}

void GP_Triangle(GP_Pixmap *pixmap, GP_Coord x0, GP_Coord y0,
                 GP_Coord x1, GP_Coord y1,
                 GP_Coord x2, GP_Coord y2, GP_Pixel pixel)
{
	GP_CHECK_PIXMAP(pixmap);

	GP_TRANSFORM_POINT(pixmap, x0, y0);
	GP_TRANSFORM_POINT(pixmap, x1, y1);
	GP_TRANSFORM_POINT(pixmap, x2, y2);

	GP_Triangle_Raw(pixmap, x0, y0, x1, y1, x2, y2, pixel);
}

void GP_FillTriangle_Raw(GP_Pixmap *pixmap, GP_Coord x0, GP_Coord y0,
                         GP_Coord x1, GP_Coord y1,
                         GP_Coord x2, GP_Coord y2, GP_Pixel pixel)
{
	const GP_Coord coords[6] = {x0, y0, x1, y1, x2, y2};

	GP_FillPolygon_Raw(pixmap, 3, coords, pixel);
}

void GP_FillTriangle(GP_Pixmap* pixmap, GP_Coord x0, GP_Coord y0,
                     GP_Coord x1, GP_Coord y1,
                     GP_Coord x2, GP_Coord y2, GP_Pixel pixel)
{
	GP_CHECK_PIXMAP(pixmap);

	GP_TRANSFORM_POINT(pixmap, x0, y0);
	GP_TRANSFORM_POINT(pixmap, x1, y1);
	GP_TRANSFORM_POINT(pixmap, x2, y2);

	GP_FillTriangle_Raw(pixmap, x0, y0, x1, y1, x2, y2, pixel);
}
