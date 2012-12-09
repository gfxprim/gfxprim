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
 * The classical Bresenham line drawing algorithm.
 * Please see http://en.wikipedia.org/wiki/Bresenham%27s_line_algorithm
 * for a nice and understandable description.
 */

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
	if (x0 == x1) { \
		if (y0 == y1) { \
			PUTPIXEL(context, x0, y0, pixval); \
			return; \
		} \
		int y = y0; \
		while (y != y1) { \
			PUTPIXEL(context, x0, y, pixval); \
			if (y0 > y1) y--; \
			else y++; \
		} \
		return; \
	} \
\
	int steep = abs(y1 - y0) / abs(x1 - x0); \
	if (steep) { \
		GP_SWAP(x0, y0); \
		GP_SWAP(x1, y1); \
	} \
	if (x0 > x1) { \
		GP_SWAP(x0, x1); \
		GP_SWAP(y0, y1); \
	} \
\
	int deltax = x1 - x0; \
	int deltay = abs(y1 - y0); \
	int error = deltax / 2; \
	int y = y0, x; \
	int ystep = (y0 < y1) ? 1 : -1; \
	for (x = x0; x <= x1; x++) { \
\
		if (steep) \
			PUTPIXEL(context, y, x, pixval); \
		else \
			PUTPIXEL(context, x, y, pixval); \
\
		error = error - deltay; \
		if (error < 0) { \
			y = y + ystep; \
			error = error + deltax; \
		} \
	} \
}
