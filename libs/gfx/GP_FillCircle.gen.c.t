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
 * Copyright (C) 2009-2012 Jiri "BlueBear" Dluhos                            *
 *                         <jiri.bluebear.dluhos@gmail.com>                  *
 *                                                                           *
 * Copyright (C) 2009-2013 Cyril Hrubis <metan@ucw.cz>                       *
 *                                                                           *
 *****************************************************************************/

%% extends "base.c.t"

{% block descr %}A filled circle drawing algorithm.{% endblock %}

%% block body

#include "core/GP_GetPutPixel.h"
#include "core/GP_Transform.h"
#include "core/GP_FnPerBpp.h"
#include "gfx/GP_HLine.h"
#include "gfx/GP_Circle.h"

/*
 * A filled circle drawing algorithm.
 *
 * A filled circle is drawn in the same way as an unfilled one,
 * in a top-down, line per line manner, except that we don't need to draw
 * four points in each X step. Instead, we just iterate X
 * until we accumulate enough Y changes to reach the next line,
 * and then draw the full line. The top and bottom half are mirrored.
 */

%% for ps in pixelsizes

static void GP_FillCircle_Raw_{{ ps.suffix }}(GP_Context *context,
	GP_Coord xcenter, GP_Coord ycenter, GP_Size r, GP_Pixel pixel)
{
	/* for r == 0, circle degenerates to a point */
	if (r == 0) {
		GP_PutPixel_Raw_{{ ps.suffix }}(context, xcenter, ycenter, pixel);
		return;
	}

	int x, y, error;
	for (x = 0, error = -r, y = r; y >= 0; y--) {
		while (error < 0) {
			error += 2*x + 1;
			x++;
		}
		error += -2*y + 1;
		GP_HLine_Raw_{{ ps.suffix }}(context, xcenter-x+1, xcenter+x-1, ycenter-y, pixel);
		GP_HLine_Raw_{{ ps.suffix }}(context, xcenter-x+1, xcenter+x-1, ycenter+y, pixel);
	}
}

%% endfor

void GP_FillCircle_Raw(GP_Context *context, GP_Coord xcenter, GP_Coord ycenter,
                       GP_Size r, GP_Pixel pixel)
{
	GP_CHECK_CONTEXT(context);

	GP_FN_PER_BPP_CONTEXT(GP_FillCircle_Raw, context, context,
	                      xcenter, ycenter, r, pixel);
}

void GP_FillCircle(GP_Context *context, GP_Coord xcenter, GP_Coord ycenter,
                   GP_Size r, GP_Pixel pixel)
{
	GP_CHECK_CONTEXT(context);

	GP_TRANSFORM_POINT(context, xcenter, ycenter);

	GP_FillCircle_Raw(context, xcenter, ycenter, r, pixel);
}

%% endblock body
