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

/*
 * The elliptical arc algorithm; a generalization of an ellipse that allows
 * to draw only part of it. The math is exactly the same, only it has
 * additional tests of what part to draw.
 */

/*
 * This macro defines a function that draws a segment of an arc within
 * two horizontal quadrants. For a larger arc, two calls are needed.
 *
 * The 'direction' parameter specifies which two quadrants to work in:
 * if <0, the top two quadrants (y < 0) are used, if >0, the bottom two
 * (y > 0) are used.
 *
 * Arguments:
 *     CONTEXT_T - user-defined type of drawing context (passed to PUTPIXEL)
 *     PIXVAL_T  - user-defined pixel value type (passed to PUTPIXEL)
 *     PUTPIXEL  - a pixel drawing function f(context, x, y, pixval)
 *     FN_NAME   - name of the function to be defined
 */
#define DEF_ARCSEGMENT_FN(FN_NAME, CONTEXT_T, PIXVAL_T, PUTPIXEL) \
void FN_NAME(CONTEXT_T context, int xcenter, int ycenter, \
	unsigned int a, unsigned int b, int direction, \
	double start, double end, PIXVAL_T pixval) \
{ \
	/* Precompute quadratic terms. */ \
	int a2 = a*a; \
	int b2 = b*b; \
\
	/* Compute minimum and maximum value of X from the angles. */ \
	int x1 = (int)(cos(start)*a); \
	int x2 = (int)(cos(end)*a); \
	int xmin = GP_MIN(x1, x2); \
	int xmax = GP_MAX(x1, x2); \
\
	int x, y, error; \
	for (x = 0, error = -b2*a, y = b; y >= 0; y--) { \
		while (error < 0) { \
\
			/* Calculate error(x+1) from error(x). */ \
			error += 2*x*b2 + b2; \
			x++; \
\
			if (direction < 0) { \
				if ((-x+1) >= xmin && (-x+1) <= xmax) { \
					PUTPIXEL(context, xcenter-x+1, ycenter-y, pixval); \
				} \
				if ((x-1) >= xmin && (x-1) <= xmax) { \
					PUTPIXEL(context, xcenter+x-1, ycenter-y, pixval); \
				} \
			} \
			if (direction > 0) { \
				if ((-x+1) >= xmin && (-x+1) <= xmax) { \
					PUTPIXEL(context, xcenter-x+1, ycenter+y, pixval); \
				} \
				if ((x-1) >= xmin && (x-1) <= xmax) { \
					PUTPIXEL(context, xcenter+x-1, ycenter+y, pixval); \
				} \
			} \
		} \
\
		/* Calculate error(y-1) from error(y). */ \
		error += -2*y*a2 + a2; \
\
		if (direction < 0) { \
			if ((-x+1) >= xmin && (-x+1) <= xmax) { \
				PUTPIXEL(context, xcenter-x+1, ycenter-y, pixval); \
			} \
			if ((x-1) >= xmin && (x-1) <= xmax) { \
				PUTPIXEL(context, xcenter+x-1, ycenter-y, pixval); \
			} \
		} \
		if (direction > 0) { \
			if ((-x+1) >= xmin && (-x+1) <= xmax) { \
				PUTPIXEL(context, xcenter-x+1, ycenter+y, pixval); \
			} \
			if ((x-1) >= xmin && (x-1) <= xmax) { \
				PUTPIXEL(context, xcenter+x-1, ycenter+y, pixval); \
			} \
		} \
	} \
}
