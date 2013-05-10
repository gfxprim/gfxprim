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
 * Copyright (C) 2009-2013 Cyril Hrubis <metan@ucw.cz>                       *
 *                                                                           *
 *****************************************************************************/

%% extends "base.c.t"

{% block descr %}A filled ellipse drawing algorithm.{% endblock %}

%% block body

#include "core/GP_GetPutPixel.h"
#include "core/GP_FnPerBpp.h"
#include "gfx/GP_HLine.h"
#include "gfx/GP_VLine.h"
#include "gfx/GP_Ellipse.h"

/*
 * A filled ellipse drawing algorithm.
 *
 * The algorithm is exactly the same as with GP_Ellipse() except that
 * we draw a line between each two points at each side of the X axis;
 * therefore, we don't need to draw any points during iterations of X,
 * we just iterate X until Y reaches next line, and then draw the full line.
 */

%% for ps in pixelsizes

static void GP_FillEllipse_Raw_{{ ps.suffix }}(GP_Context *context, GP_Coord xcenter,
		GP_Coord ycenter, GP_Size a, GP_Size b, GP_Pixel pixel)
{
	/* Precompute quadratic terms. */
	int a2 = a*a;
	int b2 = b*b;

	/* Handle special case */
	if (a == 0) {
		GP_VLine_Raw_{{ ps.suffix }}(context, xcenter, ycenter - b, ycenter + b, pixel);
		return;
	}
	
	int x, y, error;
	for (x = 0, error = -b2*a, y = b; y >= 0; y--) {
		while (error < 0) {
			error += b2 * (2*x + 1);
			x++;
		}
		error += a2 * (-2*y + 1);
		
		/* Draw two horizontal lines reflected across Y. */
		GP_HLine_Raw_{{ ps.suffix }}(context, xcenter-x+1, xcenter+x-1, ycenter-y, pixel);
		GP_HLine_Raw_{{ ps.suffix }}(context, xcenter-x+1, xcenter+x-1, ycenter+y, pixel);
	}
}

%% endfor

void GP_FillEllipse_Raw(GP_Context *context, GP_Coord xcenter, GP_Coord ycenter,
	                GP_Size a, GP_Size b, GP_Pixel pixel)
{
	GP_CHECK_CONTEXT(context);

	GP_FN_PER_BPP_CONTEXT(GP_FillEllipse_Raw, context, context,
	                      xcenter, ycenter, a, b, pixel);
}

void GP_FillEllipse(GP_Context *context, GP_Coord xcenter, GP_Coord ycenter,
                    GP_Size a, GP_Size b, GP_Pixel pixel)
{
	GP_CHECK_CONTEXT(context);
	
	GP_TRANSFORM_POINT(context, xcenter, ycenter);
	GP_TRANSFORM_SWAP(context, a, b);
	
	GP_FillEllipse_Raw(context, xcenter, ycenter, a, b, pixel);
}

%% endblock body
