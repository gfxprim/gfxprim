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
 * Parameterized template for a function that draws a filled ellipse.
 * Parameters that must be #defined outside:
 *
 * 	FN_ATTR
 * 		(Optional.) Function attributes (e.g. "static").
 * 	FN_NAME
 * 		Name of the function to define.
 * 	HLINE
 * 		Horizontal line drawing function to use.
 */

extern void HLINE(GP_TARGET_TYPE *target, GP_COLOR_TYPE color,
	int x0, int x1, int y);

#ifndef FN_ATTR
#define FN_ATTR
#endif

/*
 * Draws a filled ellipse centered in (xcenter, ycenter), with radii a and b.
 * The target surface clipping rectangle is honored; drawing over
 * surface boundary is safe.
 * If either of the radii is zero or negative, the call has no effect.
 */
FN_ATTR void FN_NAME(GP_TARGET_TYPE *target, GP_COLOR_TYPE color,
	int xcenter, int ycenter, int a, int b)
{
	if (a <= 0 || b <= 0)
		return;

	/* Precompute quadratic terms. */
	int a2 = a*a;
	int b2 = b*b;

	/*
	 * Draw the ellipse. The algorithm is exactly the same
	 * as with GP_Ellipse() except that we draw a line between
	 * each two points at each side of the X axis.
	 */
	int x, y, error;
	for (x = 0, error = -b2*a, y = b; y >= 0; y--) {

		while (error < 0) {
			error += b2 * (2*x + 1);
			x++;
		}
		error += a2 * (-2*y + 1);

		/* Draw two horizontal lines reflected across Y. */
		HLINE(target, color, xcenter-x+1, xcenter+x-1, ycenter-y);
		HLINE(target, color, xcenter-x+1, xcenter+x-1, ycenter+y);
	}
}

#undef FN_ATTR
#undef FN_NAME
#undef HLINE
