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
 * Parameterized template for ellipse drawing function.
 * Parameters that must be #defined outside:
 *
 * 	FN_ATTR
 * 		(Optional.) Attributes of the function (e.g. "static").
 * 	FN_NAME
 * 		Name of the function to define.
 * 	SETPIXEL
 * 		Routine to be used for drawing pixels.
 */

extern void SETPIXEL(GP_TARGET_TYPE *target, GP_COLOR_TYPE color,
	int x, int y);

#ifndef FN_ATTR
#define FN_ATTR
#endif

/*
 * Draws an ellipse centered in (xcenter, ycenter), with radii a and b.
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
	 * Draw the ellipse from top to down. The ellipse is
	 * X- and Y-symmetrical so we only need to calculate
	 * a 1/4 of points. In this case, we calculate the
	 * top-right quadrant (Y in range <b, 0>, X in <0, a>).
	 *
	 * Because ellipses are not symmetrical diagonally
	 * (differently from circles), we have to enumerate
	 * points both along the Y and X axis, as each enumeration
	 * gives us only half of the points.
	 *
	 * Algorithm:
	 * From the canonical implicit equation of an ellipse:
	 *
	 * x^2/a^2 + y^2/b^2 = 1             and therefore
	 * x^2*b^2 + y^2*a^2 - a^2*b^2 = 0
	 *
	 * which has an exact solution for a non-integer x.
	 * For an integer approximation, we need to find
	 * an integer x so that
	 *
	 * x^2*b^2 + y^2*a^2 - a^2*b^2 = error
	 *
	 * where error is as close to 0 as possible.
	 *
	 * Optimization:
	 * We can save a significant amount of multiplications
	 * by calculating next error values from the previous ones.
	 * For error(x+1):
	 *
	 * error(x+1) = (x+1)^2*b^2 + y^2*a^2 + a^2*b^2
	 *
	 * which can be rewritten as (after expanding (x+1)^2):
	 *
	 * error(x+1) = x^2*b^2 + 2*x^2*b^2 + b^2 + y^2*a^2 + a^2*b^2
	 *
	 * and, after substituting error(x) which we already know:
	 *
	 * error(x+1) = error(x) + 2*x*b^2 + b^2
	 *
	 * The same applies to x-1, y+1 and y-1.
	 */
	
	int x, y, error;
	for (x = 0, error = -b2*a, y = b; y >= 0; y--) {
		while (error < 0) {

			/* Calculate error(x+1) from error(x). */
			error += 2*x*b2 + b2;
			x++;

			SETPIXEL(target, color, xcenter-x+1, ycenter-y);
			SETPIXEL(target, color, xcenter+x-1, ycenter-y);
			SETPIXEL(target, color, xcenter-x+1, ycenter+y);
			SETPIXEL(target, color, xcenter+x-1, ycenter+y);
		}

		/* Calculate error(y-1) from error(y). */
		error += -2*y*a2 + a2;

		SETPIXEL(target, color, xcenter-x+1, ycenter-y);
		SETPIXEL(target, color, xcenter+x-1, ycenter-y);
		SETPIXEL(target, color, xcenter-x+1, ycenter+y);
		SETPIXEL(target, color, xcenter+x-1, ycenter+y);
	}
}

#undef FN_ATTR
#undef FN_NAME
#undef SETPIXEL

