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
 * Parameterized template for a triangle drawing function.
 * Parameters that must be #defined outside:
 *
 *	FN_ATTR
 *		(Optional.) Attributes of the function (e.g. "static").
 * 	FN_NAME
 * 		The name of the function to define.
 * 	SETPIXEL
 * 		Name of pixel drawing routine, with arguments
 * 		as defined in setpixel_generic.c.
 * 	LINE
 * 		Name of line drawing routine, with arguments
 * 		as defined in line_generic.c.
 */

extern void SETPIXEL(GP_TARGET_TYPE *target, GP_COLOR_TYPE color,
	int x, int y);
extern void LINE(GP_TARGET_TYPE *target, GP_COLOR_TYPE color,
	int x0, int y0, int x1, int y1);

#ifndef FN_ATTR
#define FN_ATTR
#endif

FN_ATTR void FN_NAME(GP_TARGET_TYPE *target, GP_COLOR_TYPE color,
	int x0, int y0, int x1, int y1, int x2, int y2)
{
	LINE(target, color, x0, y0, x1, y1);
	LINE(target, color, x0, y0, x2, y2);
	LINE(target, color, x1, y1, x2, y2);

#if 0
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
		LINE(target, color, Ax, Ay, Bx, By);
		return;
	}

	/* The direction of each side (whether X grows or decreases). */
	int ABxstep = (ABdx < 0) ? -1 : 1;
	int ACxstep = (ACdx < 0) ? -1 : 1;
	int BCxstep = (BCdx < 0) ? -1 : 1;

	/*
	 * Draw the triangle in a top-down, line-per line manner.
	 * The algorithm is the same as for GP_FillTriangle(), except that
	 * from each line, we draw only the starting and ending point,
	 * plus all points during transition from previous X to new X.
	 * (This also means we draw the line at B twice; once to finish
	 * the AB line, and one to start the AC line.)
	 */
	int y, ABx, ACx, BCx, ABerr, ACerr, BCerr;
	for (ABx = Ax, ACx = Ax, y = Ay; y <= By; y++) {
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
		SETPIXEL(target, color, BCx, y);
		for (;;) {
			ACerr = (ACx - Ax) * ACdy - (y - Ay) * ACdx;
			if (ACxstep > 0 ? ACerr >= 0 : ACerr <= 0) {
				SETPIXEL(target, color, ACx, y);
				break;
			}
			ACx += ACxstep;
			SETPIXEL(target, color, ACx, y);
		}
	}
	LINE(target, color, BCx, y-1, ACx, y-1);
#endif
}

#undef FN_NAME
#undef SETPIXEL
#undef LINE

