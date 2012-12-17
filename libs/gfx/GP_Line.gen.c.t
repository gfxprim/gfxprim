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

%% extends "base.c.t"

{% block descr %}Line drawing algorithm{% endblock %}

%% block body

#include "core/GP_Common.h"
#include "core/GP_GetPutPixel.h"
#include "core/GP_FnPerBpp.h"

#include "gfx/GP_VLine.h"
#include "gfx/GP_HLine.h"
#include "gfx/GP_Line.h"

/*
 * The classical Bresenham line drawing algorithm.
 * Please see http://en.wikipedia.org/wiki/Bresenham%27s_line_algorithm
 * for a nice and understandable description.
 */

%% for ps in pixelsizes

void GP_Line_Raw_{{ ps.suffix }}(GP_Context *context, int x0, int y0,
	int x1, int y1, GP_Pixel pixval)
{
	/* special cases: vertical line, horizontal line, single point */
	if (x0 == x1) {
		if (y0 == y1) {
			GP_PutPixel_Raw_Clipped_{{ ps.suffix }}(context,
					x0, y0, pixval);
			return;
		}
		GP_VLine_Raw(context, x0, y0, y1, pixval);
		return;
	}
	if (y0 == y1) {
		GP_HLine_Raw(context, x0, x1, y0, pixval);
		return;
	}

	/*
	 * Which axis is longer? Swap the coordinates if necessary so
	 * that the X axis is always the longer one and Y is shorter.
	 */
	int steep = abs(y1 - y0) / abs(x1 - x0);
	if (steep) {
		GP_SWAP(x0, y0);
		GP_SWAP(x1, y1);
	}
	if (x0 > x1) {
		GP_SWAP(x0, x1);
		GP_SWAP(y0, y1);
	}

	/* iterate over the longer axis, calculate values on the shorter */
	int deltax = x1 - x0;
	int deltay = abs(y1 - y0);

	/*
	 * start with error of 0.5 (multiplied by deltax for integer-only math),
	 * this reflects the fact that ideally, the coordinate should be
	 * in the middle of the pixel
	 */
	int error = deltax / 2;

	int y = y0, x;
	int ystep = (y0 < y1) ? 1 : -1;
	for (x = x0; x <= x1; x++) {

		if (steep)
			GP_PutPixel_Raw_Clipped_{{ ps.suffix }}(context, y, x,
								pixval);
		else
			GP_PutPixel_Raw_Clipped_{{ ps.suffix }}(context, x, y,
								pixval);

		error -= deltay;
		if (error < 0) {
			y += ystep;	/* next step on the shorter axis */
			error += deltax;
		}
	}
}

%% endfor

void GP_Line_Raw(GP_Context *context, GP_Coord x0, GP_Coord y0,
                 GP_Coord x1, GP_Coord y1, GP_Pixel pixel)
{
	GP_CHECK_CONTEXT(context);

	GP_FN_PER_BPP_CONTEXT(GP_Line_Raw, context, context, x0, y0, x1, y1,
	                      pixel);
}

void GP_Line(GP_Context *context, GP_Coord x0, GP_Coord y0,
             GP_Coord x1, GP_Coord y1, GP_Pixel pixel)
{
	GP_CHECK_CONTEXT(context);

	GP_TRANSFORM_POINT(context, x0, y0);
	GP_TRANSFORM_POINT(context, x1, y1);

	GP_Line_Raw(context, x0, y0, x1, y1, pixel);
}

%% endblock body
