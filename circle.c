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

#include "GP_pixel.h"
#include "GP_line.h"
#include "GP_gfx.h"

/*
 * Draws a circle with center at (xcenter, ycenter) and radius r (in pixels).
 * The clipping rectangle of the target surface is honored; overdrawing
 * over the surface edges is safe.
 */
void GP_Circle(SDL_Surface *surf, long color, int xcenter, int ycenter, int r)
{
	if (surf == NULL || surf->pixels == NULL)
		return;
	if (r < 0)
		return;

	/*
	 * Draw the circle in top-down order, line-per-line manner;
	 * Y is iterated from r to 0, the rest is mirrored.
	 * For each line, X is calculated and points at +X and -X are drawn. 
	 */
	int x, y, error;
	for (x = 0, error = -r, y = r; y >= 0; y--) {

		/*
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
		 * HINT: Significant amount of multiplications can be
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
		while (error < 0) {
			error += 2*x + 1;
			x++;

			GP_SetPixel(surf, color, xcenter-x+1, ycenter-y);
			GP_SetPixel(surf, color, xcenter+x-1, ycenter-y);
			GP_SetPixel(surf, color, xcenter-x+1, ycenter+y);
			GP_SetPixel(surf, color, xcenter+x-1, ycenter+y);
		}
		error += -2*y + 1;

		/* Draw four pixels on the circle diameter. */
		GP_SetPixel(surf, color, xcenter-x+1, ycenter-y);
		GP_SetPixel(surf, color, xcenter+x-1, ycenter-y);
		GP_SetPixel(surf, color, xcenter-x+1, ycenter+y);
		GP_SetPixel(surf, color, xcenter+x-1, ycenter+y);
	}
}

/*
 * Draws a solid filled circle centered at (xcenter, ycenter)
 * with radius of r pixels.
 * The target surface clipping rectangle is honored, and overdrawing over
 * the surface boundary is safe.
 */
void GP_FillCircle(SDL_Surface *surf, long color, int xcenter, int ycenter, int r)
{
	if (surf == NULL || surf->pixels == NULL)
		return;
	if (r < 0)
		return;

	/*
	 * Draw the circle in top-down, line-per-line manner.
	 * For each line, X is calculated and a horizontal line is drawn
	 * between +X and -X, and reflected around the Y axis.
	 * X is computed in the same way as for GP_Circle().
	 */
	int x, y, error;
	for (x = 0, error = -r, y = r; y >= 0; y--) {
		while (error < 0) {
			error += 2*x + 1;
			x++;
		}
		error += -2*y + 1;

		GP_HLine(surf, color, xcenter-x+1, xcenter+x-1, ycenter-y);
		GP_HLine(surf, color, xcenter-x+1, xcenter+x-1, ycenter+y);
	}
}

