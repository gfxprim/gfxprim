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

#include "core/GP_AngleUtils.h"

#include <math.h>

/*
 * This macro defines a partial ellipse drawing function.
 * Arguments:
 *     CONTEXT_T - user-defined type of drawing context (passed to PUTPIXEL)
 *     PIXVAL_T  - user-defined pixel value type (passed to PUTPIXEL)
 *     PUTPIXEL  - a pixel drawing function f(context, x, y, pixval)
 *     FN_NAME   - name of the function to be defined
 */
#define DEF_PARTIAL_ELLIPSE_FN(FN_NAME, CONTEXT_T, PIXVAL_T, PUTPIXEL) \
void FN_NAME(CONTEXT_T context, int xcenter, int ycenter, int a, int b, \
	int start, int end, PIXVAL_T pixval) \
{ \
	double startAngle = GP_NormalizeAngle(2*M_PI*(start / 360000.0)); \
	double endAngle = GP_NormalizeAngle(2*M_PI*(end / 360000.0)); \
\
	int x; \
	for (x = -a; x <= a; x++) { \
		double angle = acos(((double) x) / a); \
		double y = floor(b*sin(angle)); \
		if (GP_AngleInRange(angle, startAngle, endAngle)) { \
			PUTPIXEL(context, xcenter+x, ycenter-y, pixval); \
		} \
		if (GP_AngleInRange(2*M_PI - angle, startAngle, endAngle)) { \
			PUTPIXEL(context, xcenter+x, ycenter+y, pixval); \
		} \
	} \
\
	int y; \
	for (y = -b; y <= b; y++) { \
		double angle = asin(((double) y) / b); \
		double x = floor(a*cos(angle)); \
		if (GP_AngleInRange(angle, startAngle, endAngle)) { \
			PUTPIXEL(context, xcenter+x, ycenter-y, pixval); \
		} \
		if (GP_AngleInRange(M_PI - angle, startAngle, endAngle)) { \
			PUTPIXEL(context, xcenter-x, ycenter-y, pixval); \
		} \
	} \
}

