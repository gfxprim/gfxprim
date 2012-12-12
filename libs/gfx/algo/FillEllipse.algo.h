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
 * Copyright (C) 2009-2012 Cyril Hrubis <metan@ucw.cz>                       *
 *                                                                           *
 *****************************************************************************/

/*
 * A filled ellipse drawing algorithm.
 *
 * The algorithm is exactly the same as with GP_Ellipse() except that
 * we draw a line between each two points at each side of the X axis;
 * therefore, we don't need to draw any points during iterations of X,
 * we just iterate X until Y reaches next line, and then draw the full line.
 */

/*
 * This macro defines a filled ellipse drawing function.
 * Arguments:
 *     CONTEXT_T - user-defined type of drawing context (passed to HLINE)
 *     PIXVAL_T  - user-defined pixel value type (passed to HLINE)
 *     HLINE     - horizontal line drawing function f(context, x0, x1, y, pixval)
 *     FN_NAME   - name of the function to be defined
 */
#define DEF_FILLELLIPSE_FN(FN_NAME, CONTEXT_T, PIXVAL_T, HLINE) \
static void FN_NAME(CONTEXT_T context, int xcenter, int ycenter, \
	unsigned int a, unsigned int b, PIXVAL_T pixval) \
{ \
	/* Precompute quadratic terms. */ \
	int a2 = a*a; \
	int b2 = b*b; \
\
	int x, y, error; \
	for (x = 0, error = -b2*a, y = b; y >= 0; y--) { \
		while (error < 0) { \
			error += b2 * (2*x + 1); \
			x++; \
		} \
		error += a2 * (-2*y + 1); \
\
		/* Draw two horizontal lines reflected across Y. */ \
		HLINE(context, xcenter-x+1, xcenter+x-1, ycenter-y, pixval); \
		HLINE(context, xcenter-x+1, xcenter+x-1, ycenter+y, pixval); \
	} \
}
