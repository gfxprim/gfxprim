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

/* swaps endpoints of the line */
#define SWAP_ENDPOINTS do { \
	GP_SWAP(x0, x1); \
	GP_SWAP(y0, y1); \
	deltax = -deltax; \
	deltay = -deltay; \
} while(0);

/* starts a loop that iterates over every X in the line */
#define FOR_EACH_X for (x = x0, y = y0, error = deltax/2; x <= x1; x++)

/* starts a loop that iterates over every Y in the line */
#define FOR_EACH_Y for (x = x0, y = y0, error = deltay/2; y <= y1; y++)

/* updates Y to reflect increasing X by 1; as the change in Y can be
 * a fraction, a rounding error is maintained and if it overflows,
 * Y is increased or decreased by 1. */
#define NEXT_Y do { \
	error -= abs(deltay); \
	if (error < 0) { \
		deltay > 0 ? y++ : y--; \
		error += deltax; \
	} \
} while(0);

#define NEXT_X do { \
	error -= abs(deltax); \
	if (error < 0) { \
		deltax > 0 ? x++ : x--; \
		error += deltay; \
	} \
} while(0); 

GP_RetCode GP_Line(GP_Context *context, int x0, int y0, int x1, int y1,
                   GP_Color color)
{
	GP_CHECK_CONTEXT(context);

	GP_TRANSFORM_X(context, x0);
	GP_TRANSFORM_Y(context, y0);
	GP_TRANSFORM_X(context, x1);
	GP_TRANSFORM_Y(context, y1);

	/* The steepness of the line */
	int deltax = x1 - x0;
	int deltay = y1 - y0;

	/* Current X and Y coordinate, and cummulative rounding error. */
	int x, y, error;

	/* If X changes faster than Y (i.e. the line is closer to horizontal
	 * than to vertical), iterate X one by one and calculate Y;
	 * in some points, multiple X will lead to the same value of Y.
	 */
	if (abs(deltax) > abs(deltay)) {

		/* always draw in the direction of increasing X */
		if (deltax < 0) {
			SWAP_ENDPOINTS;
		}

		FOR_EACH_X {
			GP_PutPixel(context, x, y, color);
			NEXT_Y;
		}

	/* If Y changes faster than X (the line is closer to vertical
	 * than to horizontal), iterate Y and calculate X.
	 */
	} else {

		/* always draw in the direction of increasing Y */
		if (deltay < 0) {
			SWAP_ENDPOINTS;
		}

		FOR_EACH_Y {
			GP_PutPixel(context, x, y, color);
			NEXT_X;
		}
	}

	//TODO: see GP_Circle.c
	return GP_ESUCCESS;
}

size_t GP_CalcLinePoints(int x0, int y0, int x1, int y1,
	int *points, size_t maxlen)
{
	GP_CHECK(points != NULL || maxlen == 0);
	GP_CHECK(maxlen % 2 == 0);
	
	size_t i = 0;

	int deltax = x1 - x0;
	int deltay = y1 - y0;

	int x, y, error;

	if (abs(deltax) > abs(deltay)) {	/* X changes faster */

		if (deltax < 0) {
			SWAP_ENDPOINTS;
		}

		FOR_EACH_X {
			if (i < maxlen) {
				points[i++] = x;
				points[i++] = y;
			}
			NEXT_X;
		}
	} else {				/* Y changes faster */

		if (deltay < 0) {
			SWAP_ENDPOINTS;
		}

		FOR_EACH_Y {
			if (i < maxlen) {
				points[i++] = x;
				points[i++] = y;
			}
			NEXT_Y;
		}
	}

	return i;
}
