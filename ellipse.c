/*
 * This file is part of the gfxprim library.
 *
 * Copyright (c) 2009 Jiri "BlueBear" Dluhos <jiri.bluebear.dluhos@gmail.com>,
 *                    Cyril Hrubis (Metan) <metan@ucw.cz>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDERS
 * OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "GP_pixel.h"
#include "GP_line.h"
#include "GP_gfx.h"

/*
 * Plots 4 pixels mirrored around the common center.
 */
static inline void GP_4MirroredPixels(SDL_Surface *surf, long color,
				int xcenter, int ycenter, int x, int y)
{
	GP_SetPixel(surf, color, xcenter-x, ycenter-y);
	GP_SetPixel(surf, color, xcenter+x, ycenter-y);
	GP_SetPixel(surf, color, xcenter-x, ycenter+y);
	GP_SetPixel(surf, color, xcenter+x, ycenter+y);
}

/*
 * Draws an ellipse centered in (xcenter, ycenter), with radii a and b.
 * The target surface clipping rectangle is honored; drawing over
 * surface boundary is safe.
 * If either of the radii is zero or negative, the call has no effect.
 */
void GP_Ellipse(SDL_Surface *surf, long color, int xcenter, int ycenter,
		int a, int b)
{
	if (surf == NULL || surf->pixels == NULL)
		return;
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

			GP_SetPixel(surf, color, xcenter-x+1, ycenter-y);
			GP_SetPixel(surf, color, xcenter+x-1, ycenter-y);
			GP_SetPixel(surf, color, xcenter-x+1, ycenter+y);
			GP_SetPixel(surf, color, xcenter+x-1, ycenter+y);
		}

		/* Calculate error(y-1) from error(y). */
		error += -2*y*a2 + a2;

		GP_SetPixel(surf, color, xcenter-x+1, ycenter-y);
		GP_SetPixel(surf, color, xcenter+x-1, ycenter-y);
		GP_SetPixel(surf, color, xcenter-x+1, ycenter+y);
		GP_SetPixel(surf, color, xcenter+x-1, ycenter+y);
	}
}

/*
 * Draws a filled ellipse centered in (xcenter, ycenter), with radii a and b.
 * The target surface clipping rectangle is honored; drawing over
 * surface boundary is safe.
 * If either of the radii is zero or negative, the call has no effect.
 */
void GP_FillEllipse(SDL_Surface *surf, long color, int xcenter, int ycenter,
			int a, int b)
{
	if (surf == NULL || surf->pixels == NULL)
		return;
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
		GP_HLine(surf, color, xcenter-x+1, xcenter+x-1, ycenter-y);
		GP_HLine(surf, color, xcenter-x+1, xcenter+x-1, ycenter+y);
	}
}

