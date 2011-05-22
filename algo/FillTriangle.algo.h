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
 * Then, the lines AB, AC and BC are fictionally drawn using the Bresenham
 * algorithm, tracking their starting/ending X value for each scanline.
 * (See GP_LineTrack.h.)
 *
 * NOTE: For correct drawing, we need to fill each scanline
 * from its very first X point to its very last X point, so we need to
 * remember which side is on the left (we take its first X point)
 * and which is on the right (we take its last X point); this is easily
 * determined by comparing Bx against Ax.
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
	const int ABdx = abs(Bx - Ax); \
	const int ABsx = GP_SIGN(Bx - Ax); \
	const int ABdy = abs(By - Ay); \
	const int ACdx = abs(Cx - Ax); \
	const int ACsx = GP_SIGN(Cx - Ax); \
	const int ACdy = abs(Cy - Ay); \
	const int BCdx = abs(Cx - Bx); \
	const int BCsx = GP_SIGN(Cx - Bx); \
	const int BCdy = abs(Cy - By); \
	int ABerr = ABdx, ACerr = ACdx, BCerr = BCdx; \
	int ABx = Ax, ACx = Ax, BCx = Bx; \
	int y, xstart, xend; \
\
	if (ABdy == 0) \
		goto bottom_part; \
\
	/* Where the break between the top and bottom part occurs;
	 * this is normally right above the By, but there is a special case:
	 * when By == Cy, we need to draw everything.
	 */ \
	const int ybreak = (By == Cy) ? By : By - 1; \
\
	for (y = Ay; y <= ybreak; y++) { \
		xstart = GP_MIN(ABx, ACx); \
		xend = GP_MAX(ABx, ACx); \
		for (;;) { \
			if (ABerr >= ABdx) { \
				ABerr -= 2*ABdx; \
				break; \
			} \
			ABx += ABsx; \
			ABerr += 2*ABdy; \
		} \
		xstart = GP_MIN(ABx, xstart); \
		xend = GP_MAX(ABx, xend); \
		for (;;) { \
			if (ACerr >= ACdx) { \
				ACerr -= 2*ACdx; \
				break; \
			} \
			ACx += ACsx; \
			ACerr += 2*ACdy; \
		} \
		xstart = GP_MIN(ACx, xstart); \
		xend = GP_MAX(ACx, xend); \
		HLINE(context, xstart, xend, y, pixval); \
	} \
\
bottom_part: \
\
	if (BCdy == 0) \
		goto end; \
\
	for (y = By; y <= Cy; y++) { \
		xstart = GP_MIN(BCx, ACx); \
		xend = GP_MAX(BCx, ACx); \
		for (;;) { \
			if (BCerr >= BCdx) { \
				BCerr -= 2*BCdx; \
				break; \
			} \
			BCx += BCsx; \
			BCerr += 2*BCdy; \
		} \
		xstart = GP_MIN(xstart, BCx); \
		xend = GP_MAX(xend, BCx); \
		for (;;) { \
			if (ACerr >= ACdx) { \
				ACerr -= 2*ACdx; \
				break; \
			} \
			ACx += ACsx; \
			ACerr += 2*ACdy; \
		} \
		xstart = GP_MIN(xstart, ACx); \
		xend = GP_MAX(xend, ACx); \
		HLINE(context, xstart, xend, y, pixval); \
	} \
\
end: \
	return; \
}
