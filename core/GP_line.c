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

#include "GP.h"

#include <stdlib.h>

void GP_Line(GP_Context *context, int x0, int y0, int x1, int y1,
	uint32_t color)
{
	GP_CHECK_CONTEXT(context);

	/* The steepness of the line */
	int deltax = x1 - x0;
	int deltay = y1 - y0;

	/*
	 * Subdivide lines to octants according to two boolean factors:
	 *
	 * - whether Y is increasing/decreasing (delta Y is positive/negative)
	 * - whether X or Y is changing faster.
	 *
	 * For each octant:
	 * When drawing, the faster-changing coordinate is iterated one by one,
	 * and the other (which changes slower) is calculated.
	 */
	int x = x0, y = y0;
	int error;
	if (abs(deltax) > abs(deltay)) {	/* X changes faster */

		error = abs(deltax) / 2;

		if (deltax >= 0) {		/* X increases */
			if (deltay > 0) {	/* Y increases */

				for (; x <= x1; x++) {
					GP_PutPixel(context, x, y, color);
					error -= deltay;
					if (error < 0) { 
						y++;
						error += deltax;
					}
				}
			} else {		/* Y decreases */

				for (; x <= x1; x++) {
					GP_PutPixel(context, x, y, color);
					error -= -deltay;
					if (error < 0) { 
						y--;
						error += deltax;
					}
				}
			}
		} else {			/* X decreases */

			/*
			 * Swap endpoints and draw with increasing X.
			 * This ensures that the pixels plotted are exactly
			 * the same as with the opposite direction.
			 */
			GP_Line(context, x1, y1, x0, y0, color);
			return;
		}
	} else {				/* Y changes faster */

		error = abs(deltay) / 2;

		if (deltay >= 0) {		/* Y increases */
			if (deltax > 0) {	/* X increases */
				for (; y <= y1; y++) {
					GP_PutPixel(context, x, y, color);
					error -= deltax;
					if (error < 0) {
						x++;
						error += deltay;
					}
				}
			} else {		/* X decreases */

				for (; y <= y1; y++) {
					GP_PutPixel(context, x, y, color);
					error -= -deltax;
					if (error < 0) {
						x--;
						error += deltay;
					}
				}
			}
		} else {			/* Y decreases */

			/*
			 * Swap endpoints and draw the line with increasing Y.
			 * This ensures that the pixels plotted are exactly
			 * the same as with the opposite direction.
			 */
			GP_Line(context, x1, y1, x0, y0, color);
			return;
		}
	}
}
