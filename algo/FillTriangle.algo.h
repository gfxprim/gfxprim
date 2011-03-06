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
 * A triangle drawing algorithm.
 *
 * The triangle is drawn in a top-down, line-per line manner.
 *
 * First, the vertices of the triangle are sorted vertically: A is the topmost,
 * C is the bottommost, B is somewhere between them.
 *
 * The triangle is subdivided into two parts:
 * from A to B (between AB and AC)
 * and from B to C (between BC and AC).
 *
 * For each line, the value of X is calculated for each side,
 * yielding a point on each side, and a horizontal line is drawn
 * between the points.
 *
 * The value of X for each side is calculated from the line equation
 * of the side. For example, for AB we have:
 *
 * ABx = (y - Ay) * ABdx / ABdy + Ax            and thus
 * ABx * ABdy = (y - Ay) * ABdx + Ax * ABdy     and thus
 * (ABx - Ax) * ABdy - (y - Ay) * ABdx = 0
 *
 * For integer-only operation, the expression will not be exactly 0;
 * instead, we will have an error term we try to minimize:
 *
 * ABerr = (ABx - Ax) * ABdy - (y - Ay) * ABdx       and similarly
 * ACerr = (ACx - Ax) * ACdy - (y - Ay) * ACdx
 * BCerr = (BCx - Bx) * BCdy - (y - By) * BCdx
 *
 * In the drawing loop, we calculate each error term from the previous
 * value by increasing or decreasing X (according to the line direction)
 * until the error term crosses the zero boundary.
 */

#define DEF_FILLTRIANGLE_FN(FN_NAME, CONTEXT_T, PIXVAL_T, HLINE, PUTPIXEL) \
void FN_NAME(CONTEXT_T context, int x0, int y0, int x1, int y1, \
		int x2, int y2, PIXVAL_T pixval) \
{ \
\
	/* Sort the three points according to the Y coordinate. */ \
	int Ax, Ay, Bx, By, Cx, Cy; \
	if (y0 < y1) { \
		if (y1 < y2) {					/* 0, 1, 2 */ \
			Ax = x0, Ay = y0; \
			Bx = x1, By = y1; \
			Cx = x2, Cy = y2; \
		} else { \
			if (y2 < y0) {				/* 2, 0, 1 */ \
				Ax = x2, Ay = y2; \
				Bx = x0, By = y0; \
				Cx = x1, Cy = y1; \
			} else {				/* 0, 2, 1 */ \
				Ax = x0, Ay = y0; \
				Bx = x2, By = y2; \
				Cx = x1, Cy = y1; \
			} \
		} \
	} else { \
		if (y1 < y2) { \
			if (y2 < y0) {				/* 1, 2, 0 */ \
				Ax = x1, Ay = y1; \
				Bx = x2, By = y2; \
				Cx = x0, Cy = y0; \
			} else {				/* 1, 0, 2 */ \
				Ax = x1, Ay = y1; \
				Bx = x0, By = y0; \
				Cx = x2, Cy = y2; \
			} \
		} else {					/* 2, 1, 0 */ \
			Ax = x2, Ay = y2; \
			Bx = x1, By = y1; \
			Cx = x0, Cy = y0; \
		} \
	} \
\
	/* Special case: Ay = By = Cy, triangle degenerates to a line. */ \
	if (Ay == Cy) { \
		return HLINE(context, \
		                GP_MIN(Ax, GP_MIN(Bx, Cx)), \
		                GP_MAX(Ax, GP_MAX(Bx, Cx)), \
		                Ay, pixval); \
	} \
\
	int ABpoints[Cy-Ay+1], ACpoints[Cy-Ay+1], BCpoints[Cy-By+1]; \
\
	if (Bx < Ax) { \
		GP_RasterizeLine(Ax, Ay, Bx, By, ABpoints, Ay, Cy, GP_KEEP_XMIN); \
		GP_RasterizeLine(Ax, Ay, Cx, Cy, ACpoints, Ay, Cy, GP_KEEP_XMAX); \
		GP_RasterizeLine(Bx, By, Cx, Cy, BCpoints, By, Cy, GP_KEEP_XMIN); \
	} else { \
		GP_RasterizeLine(Ax, Ay, Bx, By, ABpoints, Ay, Cy, GP_KEEP_XMAX); \
		GP_RasterizeLine(Ax, Ay, Cx, Cy, ACpoints, Ay, Cy, GP_KEEP_XMIN); \
		GP_RasterizeLine(Bx, By, Cx, Cy, BCpoints, By, Cy, GP_KEEP_XMAX); \
	} \
\
	int y; \
\
	for (y = Ay; y < By; y++) { \
		HLINE(context, ABpoints[y - Ay], ACpoints[y - Ay], y, pixval); \
	} \
\
	for (y = By; y <= Cy; y++) { \
		HLINE(context, BCpoints[y - By], ACpoints[y - Ay], y, pixval); \
	} \
}

