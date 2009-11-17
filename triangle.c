/*
 * This file is part of the gfxprim library.
 *
 * Copyright (c) 2009 Jiri "BlueBear" Dluhos <jiri.bluebear.dluhos@gmail.com>,
 *                    Cyril Hrubis (Metan) <metan@ucw.cz>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDERS
 * OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "GP.h"

#include <assert.h>
#include <stdio.h>

void GP_Triangle(SDL_Surface * surf, long color, int x0, int y0, int x1, int y1, int x2, int y2)
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
		GP_Line(surf, color, Ax, Ay, Bx, By);
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
				GP_SetPixel(surf, color, ABx, y);
				break;
			}
			ABx += ABxstep;
			GP_SetPixel(surf, color, ABx, y);
		}
		for (;;) {
			ACerr = (ACx - Ax) * ACdy - (y - Ay) * ACdx;
			if (ACxstep > 0 ? ACerr >= 0 : ACerr <= 0) {
				GP_SetPixel(surf, color, ACx, y);
				break;
			}
			ACx += ACxstep;
			GP_SetPixel(surf, color, ACx, y);
		}
	}
	for (BCx = Bx, y = By; y <= Cy; y++) {
		for (;;) {
			BCerr = (BCx - Bx) * BCdy - (y - By) * BCdx;
			if (BCxstep > 0 ? BCerr >= 0 : BCerr <= 0) {
				GP_SetPixel(surf, color, BCx, y);
				break;
			}
			BCx += BCxstep;
			GP_SetPixel(surf, color, BCx, y);
		}
		GP_SetPixel(surf, color, BCx, y);
		for (;;) {
			ACerr = (ACx - Ax) * ACdy - (y - Ay) * ACdx;
			if (ACxstep > 0 ? ACerr >= 0 : ACerr <= 0) {
				GP_SetPixel(surf, color, ACx, y);
				break;
			}
			ACx += ACxstep;
			GP_SetPixel(surf, color, ACx, y);
		}
	}
	GP_HLine(surf, color, BCx, ACx, y-1);
}

void GP_FillTriangle(SDL_Surface * surf, long color, int x0, int y0, int x1, int y1, int x2, int y2)
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
		GP_Line(surf, color, Ax, Ay, Bx, By);
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
				GP_SetPixel(surf, color, ABx, y);
				break;
			}
			ABx += ABxstep;
			GP_SetPixel(surf, color, ABx, y);
		}
		for (;;) {
			ACerr = (ACx - Ax) * ACdy - (y - Ay) * ACdx;
			if (ACxstep > 0 ? ACerr >= 0 : ACerr <= 0) {
				GP_SetPixel(surf, color, ACx, y);
				break;
			}
			ACx += ACxstep;
			GP_SetPixel(surf, color, ACx, y);
		}
		GP_HLine(surf, color, ABx, ACx, y);
	}
	for (BCx = Bx, y = By; y <= Cy; y++) {
		for (;;) {
			BCerr = (BCx - Bx) * BCdy - (y - By) * BCdx;
			if (BCxstep > 0 ? BCerr >= 0 : BCerr <= 0) {
				GP_SetPixel(surf, color, BCx, y);
				break;
			}
			BCx += BCxstep;
		}
		for (;;) {
			ACerr = (ACx - Ax) * ACdy - (y - Ay) * ACdx;
			if (ACxstep > 0 ? ACerr >= 0 : ACerr <= 0) {
				GP_SetPixel(surf, color, ACx, y);
				break;
			}
			ACx += ACxstep;
			GP_SetPixel(surf, color, ACx, y);
		}
		GP_HLine(surf, color, BCx, ACx, y);
	}
}

