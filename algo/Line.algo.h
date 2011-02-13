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
 * A Bresenham line-drawing algorithm.
 */

/* swaps endpoints of the line */
#define SWAP_ENDPOINTS do { \
	GP_SWAP(x0, x1); \
	GP_SWAP(y0, y1); \
	deltax = -deltax; \
	deltay = -deltay; \
} while (0)

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
} while (0)

#define NEXT_X do { \
	error -= abs(deltax); \
	if (error < 0) { \
		deltax > 0 ? x++ : x--; \
		error += deltay; \
	} \
} while (0) 

/*
 * This macro defines a line drawing function.
 * Arguments:
 *     CONTEXT_T - user-defined type of drawing context (passed to PUTPIXEL)
 *     PIXVAL_T  - user-defined pixel value type (passed to PUTPIXEL)
 *     PUTPIXEL  - a pixel drawing function f(context, x, y, pixval)
 *     FN_NAME   - name of the function to be defined
 */
#define DEF_LINE_FN(FN_NAME, CONTEXT_T, PIXVAL_T, PUTPIXEL) \
void FN_NAME(CONTEXT_T context, int x0, int y0, int x1, int y1, \
	PIXVAL_T pixval) \
{ \
	/* The steepness of the line */ \
	int deltax = x1 - x0; \
	int deltay = y1 - y0; \
\
	/* Current X and Y coordinate, and cummulative rounding error. */ \
	int x, y, error; \
\
	/* If X changes faster than Y (i.e. the line is closer to horizontal \
	 * than to vertical), iterate X one by one and calculate Y; \
	 * in some points, multiple X will lead to the same value of Y. \
	 */ \
	if (abs(deltax) > abs(deltay)) { \
\
		/* always draw in the direction of increasing X */ \
		if (deltax < 0) { \
			SWAP_ENDPOINTS; \
		} \
\
		FOR_EACH_X { \
			PUTPIXEL(context, x, y, pixval); \
			NEXT_Y; \
		} \
\
	/* If Y changes faster than X (the line is closer to vertical \
	 * than to horizontal), iterate Y and calculate X. \
	 */ \
	} else { \
\
		/* always draw in the direction of increasing Y */ \
		if (deltay < 0) { \
			SWAP_ENDPOINTS; \
		} \
\
		FOR_EACH_Y { \
			PUTPIXEL(context, x, y, pixval); \
			NEXT_X; \
		} \
	} \
}

