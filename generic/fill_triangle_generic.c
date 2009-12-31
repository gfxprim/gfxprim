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

void FN_NAME(TARGET_TYPE *target, long color, int x0, int y0, int x1, int y1, int x2, int y2)
{
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
	if (ACdx == 0 && ACdy == 0) {
		GP_Line(target, color, Ax, Ay, Bx, By);
		return;
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
	 * ABerr = (ABx - Ax) * ABdy - (y - Ay) * ABdx
	 *
	 * Because sides are straight lines, we know that X is either
	 * monotonically growing, or decreasing. Therefore we can calculate
	 * the next value of X from the previous value simply by increasing
	 * or decreasing it until the error term crosses the zero boundary.
	 */
	int y, ABx, ACx, BCx, ABerr, ACerr, BCerr;
	for (ABx = Ax, ACx = Ax, y = Ay; y < By; y++) {
		for (;;) {
			ABerr = (ABx - Ax) * ABdy - (y - Ay) * ABdx;
			if (ABxstep > 0 ? ABerr >= 0 : ABerr <= 0) {
				SETPIXEL(target, color, ABx, y);
				break;
			}
			ABx += ABxstep;
			SETPIXEL(target, color, ABx, y);
		}
		for (;;) {
			ACerr = (ACx - Ax) * ACdy - (y - Ay) * ACdx;
			if (ACxstep > 0 ? ACerr >= 0 : ACerr <= 0) {
				SETPIXEL(target, color, ACx, y);
				break;
			}
			ACx += ACxstep;
			SETPIXEL(target, color, ACx, y);
		}
		GP_HLine(target, color, ABx, ACx, y);
	}
	for (BCx = Bx, y = By; y <= Cy; y++) {
		for (;;) {
			BCerr = (BCx - Bx) * BCdy - (y - By) * BCdx;
			if (BCxstep > 0 ? BCerr >= 0 : BCerr <= 0) {
				SETPIXEL(target, color, BCx, y);
				break;
			}
			BCx += BCxstep;
			SETPIXEL(target, color, BCx, y);
		}
		for (;;) {
			ACerr = (ACx - Ax) * ACdy - (y - Ay) * ACdx;
			if (ACxstep > 0 ? ACerr >= 0 : ACerr <= 0) {
				SETPIXEL(target, color, ACx, y);
				break;
			}
			ACx += ACxstep;
			SETPIXEL(target, color, ACx, y);
		}
		GP_HLine(target, color, BCx, ACx, y);
	}
}

