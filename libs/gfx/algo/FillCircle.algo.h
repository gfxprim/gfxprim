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
 * A filled circle drawing algorithm.
 *
 * A filled circle is drawn in the same way as an unfilled one,
 * in a top-down, line per line manner, except that we don't need to draw
 * four points in each X step. Instead, we just iterate X
 * until we accumulate enough Y changes to reach the next line,
 * and then draw the full line.
 */

/*
 * This macro defines a filled circle drawing function.
 * Arguments:
 *     CONTEXT_T - user-defined type of drawing context (passed to HLINE)
 *     PIXVAL_T  - user-defined pixel value type (passed to HLINE)
 *     HLINE     - horizontal line drawing function f(context, x0, x1, y, pixval)
 *     FN_NAME   - name of the function to be defined
 */
#define DEF_FILLCIRCLE_FN(FN_NAME, CONTEXT_T, PIXVAL_T, HLINE) \
void FN_NAME(CONTEXT_T context, int xcenter, int ycenter, \
	unsigned int r, PIXVAL_T pixval) \
{ \
	/* for r == 0, circle degenerates to a point */ \
	if (r == 0) { \
		HLINE(context, xcenter, xcenter, ycenter, pixval); \
		return; \
	} \
\
	int x, y, error; \
	for (x = 0, error = -r, y = r; y >= 0; y--) { \
		while (error < 0) { \
			error += 2*x + 1; \
			x++; \
		} \
		error += -2*y + 1; \
		HLINE(context, xcenter-x+1, xcenter+x-1, ycenter-y, pixval); \
		HLINE(context, xcenter-x+1, xcenter+x-1, ycenter+y, pixval); \
	} \
}
