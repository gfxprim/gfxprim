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

/*
 * Circle drawing algorithm.
 *
 * The circle is drawn in a top-down manner. We start from the top, and
 * at each line, we iterate X until we accumulate enough changes to Y
 * to pass to the next line. In each step, 4 pixels are drawn:
 * (X, Y), (-X, Y), (X, -Y) and (-X, -Y).
 *
 * The math:
 * From the circle equation, for every point applies:
 *
 * x^2 + y^2 = r^2      ->       x^2 + y^2 - r^2 = 0
 *
 * which has an exact solution for a non-integer x.
 * For an integer approximation, we want to find x
 * for which
 *
 * x^2 + y^2 - r^2 = error
 *
 * where error should be as close to 0 as possible.
 * We find the x by incrementing its value until
 * we cross the zero error boundary.
 *
 * Optimization:
 * Significant amount of multiplications can be
 * saved when calculating error by re-using previous
 * error values. For error(x+1) we have:
 *
 * error(x+1) = (x+1)^2 + y^2 - r^2
 *
 * which can be expanded to (expanding (x+1)^2):
 *
 * error(x+1) = x^2 + 2*x + 1 + y^2 - r^2
 *
 * and after substituting the error(x) we already know:
 *
 * error(x+1) = error(x) + 2*x + 1
 *
 * The same can be done for calculating
 * error(y-1) from error(y).
 */

/*
 * This macro defines a circle drawing function.
 * Arguments:
 *     PIXMAP_T - user-defined type of drawing pixmap (passed to PUTPIXEL)
 *     PIXVAL_T  - user-defined pixel value type (passed to PUTPIXEL)
 *     PUTPIXEL  - a pixel drawing function f(pixmap, x, y, pixval)
 *     FN_NAME   - name of the function to be defined
 */
#define DEF_CIRCLE_SEG_FN(FN_NAME, PIXMAP_T, PIXVAL_T, PUTPIXEL) \
static void FN_NAME(PIXMAP_T pixmap, int xcenter, int ycenter, int r, \
	uint8_t seg_flags, PIXVAL_T pixval) \
{ \
	int x, y, error; \
\
	/* Special case for r == 0 */ \
	if (r == 0) { \
		if (seg_flags) \
			PUTPIXEL(pixmap, xcenter, ycenter, pixval); \
		return; \
	} \
\
	/* Clip negative r */ \
	r = r < 0 ? -r : r; \
\
	for (x = 0, error = -r, y = r; y >= 0; y--) { \
\
		/* Iterate X until we can pass to the next line. */ \
		while (error < 0) { \
			error += 2*x + 1; \
			x++; \
			if (seg_flags & GP_CIRCLE_SEG2) \
				PUTPIXEL(pixmap, xcenter-x+1, ycenter-y, pixval); \
			if (seg_flags & GP_CIRCLE_SEG1) \
				PUTPIXEL(pixmap, xcenter+x-1, ycenter-y, pixval); \
			if (seg_flags & GP_CIRCLE_SEG3) \
				PUTPIXEL(pixmap, xcenter-x+1, ycenter+y, pixval); \
			if (seg_flags & GP_CIRCLE_SEG4) \
				PUTPIXEL(pixmap, xcenter+x-1, ycenter+y, pixval); \
		} \
\
		/* Enough changes accumulated, go to next line. */ \
		error += -2*y + 1; \
		if (seg_flags & GP_CIRCLE_SEG2) \
			PUTPIXEL(pixmap, xcenter-x+1, ycenter-y, pixval); \
		if (seg_flags & GP_CIRCLE_SEG1) \
			PUTPIXEL(pixmap, xcenter+x-1, ycenter-y, pixval); \
		if (seg_flags & GP_CIRCLE_SEG3) \
			PUTPIXEL(pixmap, xcenter-x+1, ycenter+y, pixval); \
		if (seg_flags & GP_CIRCLE_SEG4) \
			PUTPIXEL(pixmap, xcenter+x-1, ycenter+y, pixval); \
	} \
}
