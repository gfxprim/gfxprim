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
 * Function that draws a filled rectangle from horizontal lines.
 * Following arguments must be #defined before including this:
 *
 *     CONTEXT_T - user-defined type of drawing context (passed to LINE/HLINE)
 *     PIXVAL_T  - user-defined pixel value type (passed to LINE/HLINE)
 *     LINE      - line drawing function f(context, x0, y0, x1, y1, pixval)
 *     HLINE     - horizontal line drawing function f(context, x0, x1, y, pixval)
 *     FN_NAME   - name of the function to be defined
 *
 * Also, GP_MIN and GP_MAX macros are used.
 */

void FN_NAME(CONTEXT_T context, int x0, int y0, int x1, int y1,
		int x2, int y2, PIXVAL_T pixval)
{
	/*
	 * FIXME: For some triangles (maybe when abs(dx) > abs(dy)?),
	 * there are 'missing' pixels around the boundary (at the start/end
	 * of every horizontal line). I really don't know why. Sorry. :(
	 * This hack "fixes" this by drawing three lines around the boundary.
	 */
	LINE(context, x0, y0, x1, y1, pixval);
	LINE(context, x0, y0, x2, y2, pixval);
	LINE(context, x1, y1, x2, y2, pixval);

	/*
	 * Sort the three points according to the Y coordinate.
	 * A is the topmost, B is between them, C is the bottommost.
	 */
	int Ax, Ay, Bx, By, Cx, Cy;
	if (y0 < y1) {
		if (y1 < y2) {					/* 0, 1, 2 */
			Ax = x0, Ay = y0;
			Bx = x1, By = y1;
			Cx = x2, Cy = y2;
		} else {
			if (y2 < y0) {				/* 2, 0, 1 */
				Ax = x2, Ay = y2;
				Bx = x0, By = y0;
				Cx = x1, Cy = y1;
			} else {				/* 0, 2, 1 */
				Ax = x0, Ay = y0;
				Bx = x2, By = y2;
				Cx = x1, Cy = y1;
			}
		}
	} else {
		if (y1 < y2) {
			if (y2 < y0) {				/* 1, 2, 0 */
				Ax = x1, Ay = y1;
				Bx = x2, By = y2;
				Cx = x0, Cy = y0;
			} else {				/* 1, 0, 2 */
				Ax = x1, Ay = y1;
				Bx = x0, By = y0;
				Cx = x2, Cy = y2;
			}
		} else {					/* 2, 1, 0 */
			Ax = x2, Ay = y2;
			Bx = x1, By = y1;
			Cx = x0, Cy = y0;
		}
	}

	/* Calculate delta X and delta Y per each side. */
	int ABdx = Bx - Ax;
	int ABdy = By - Ay;
	int ACdx = Cx - Ax;
	int ACdy = Cy - Ay;
	int BCdx = Cx - Bx;
	int BCdy = Cy - By;

	/* Handle degenerate cases. */
	if (ACdy == 0) {

		/* All three vertices in one horizontal line. */
		return HLINE(context,
		                GP_MIN(Ax, GP_MIN(Bx, Cx)),
		                GP_MAX(Ax, GP_MAX(Bx, Cx)),
		                Ay, pixval);
	}

	/* The direction of each side (whether X grows or decreases). */
	int ABxstep = (ABdx < 0) ? -1 : 1;
	int ACxstep = (ACdx < 0) ? -1 : 1;
	int BCxstep = (BCdx < 0) ? -1 : 1;

	/*
	 * Draw the triangle in a top-down, line-per line manner.
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
	int y, ABx, ACx, BCx, old_ABx, old_ACx, old_BCx, ABerr, ACerr, BCerr;

	/* Top part of the triangle (from Ay to By). */
	ABerr = abs(ABdy)/2;
	ACerr = abs(ACdy)/2;
	for (ABx = Ax, ACx = Ax, y = Ay; y < By; y++) {

		HLINE(context, ABx, ACx, y, pixval);

		old_ABx = ABx;
		old_ACx = ACx;

		while (ABerr < abs(ABdx) || ABdy == 0) {
			ABx += ABxstep;
			ABerr += abs(ABdy);
		}

		while (ACerr < abs(ACdx) || ACdy == 0) {
			ACx += ACxstep;
			ACerr += abs(ACdy);
		}

		/*
		 * The value of ABerr and ACerr for the next loop iteration.
		 * (from the equation above, with y = y+1).
		 */
		ABerr -= abs(ABdx);
		ACerr -= abs(ACdx);
	}

	/* A special case where B and C are at the same height. */
	if (BCdy == 0) {
		HLINE(context, Bx, Cx, y, pixval);
		return;
	}

	/* Bottom part (from By to Cy). */
	for (BCx = Bx, y = By, BCerr = abs(BCdy)/2; y <= Cy; y++) {
		old_BCx = BCx;
		old_ACx = ACx;

		HLINE(context, BCx, ACx, y, pixval);

		while (BCerr < abs(BCdx) || BCdy == 0) {
			BCx += BCxstep;
			BCerr += abs(BCdy);
		}

		while (ACerr < abs(ACdx) || ACdy == 0) {
			ACx += ACxstep;
			ACerr += abs(ACdy);
		}

		/*
		 * The value of ACerr and BCerr for the next loop iteration.
		 */
		ACerr -= abs(ACdx);
		BCerr -= abs(BCdx);
	}
}

