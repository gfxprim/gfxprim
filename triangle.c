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
    GP_Line(surf, color, x0, y0, x1, y1);
    GP_Line(surf, color, x1, y1, x2, y2);
    GP_Line(surf, color, x2, y2, x0, y0);
}

#if 0
/*
 * Draws a filled triangle.
 */
void GP_FillTriangle(SDL_Surface * surf, long color, int x0, int y0, int x1, int y1, int x2, int y2)
{

/* Algorithm:
 *
 * First, the three points (vertices) are sorted according to the Y coordinate
 * to the top (xtop, ytop), bottom (xbot, ybot) and midpoint (xmid, ymid).
 *
 * The three sides of the triangle are classified:
 *      - the "top" side (from the top point to the midpoint)
 *      - the "bottom" side (from the midpoint to the bottom point)
 *      - the "long" side (from the top point to the bottom point)
 *
 * The triangle is subdivided into two sub-triangles:
 *      - the upper subtriangle (with Y coordinates from ytop to ymid),
 *        bounded by the "top" side and by part of the "long" side
 *      - the lower subtriangle (with Y coordinates from ymid to ybot),
 *        bounded by the "bottom" side and the rest of the "long" side.
 *
 * Both subtriangles are filled by horizontal lines in top-down fashion,
 * by iterating the Y coordinate from ytop to ymid and from ymid to ybot.
 * For each line, the starting and ending X coordinate (xstart, xend)
 * are calculated as follows:
 *      - from the equation of each side, we have
 *        (by convention, we start at either top, or bottom side
 *        and end at the long side):
 *
 *        Upper subtriangle:
 *                xstart = (y - ytop) * (xmid - xtop) / (ymid - ytop)
 *        Lower subtriangle:
 *                xstart = (y - ytop) * (xbot - xmid) / (ybot - ymid)
 *        For both:
 *                xend = (y - ytop) * (xbot - xtop) / (ybot - ytop)
 *
 *        where xstart, xend are real numbers.
 *      - for integer math, we need to adjust the calculation a bit.
 *        Rearranging the equations to get rid of the division is easy:
 *
 *        Upper subtriangle:
 *                xstart * (ymid - ytop) = (y - ytop) * (xmid - xtop)
 *        Lower subtriangle:
 *                xstart * (ybot - ymid) = (y - ytop) * (xbot - xmid)
 *        For both:
 *                xend * (ybot - ytop) = (y - ytop) * (xbot - xtop)
 *
 *        By moving all items to the left side, we have the error term
 *        we want to have as close to zero as possible when approximating
 *        the integer solution:
 *
 *        Upper subtriangle:
 *                xstart_err = xstart * (ymid - ytop) - (y - ytop) * (xmid - xtop) ~= 0
 *        Lower subtriangle:
 *                xstart_err = xstart * (ybot - ymid) - (y - ytop) * (xbot - xmid) ~= 0
 *        Both:
 *                xend_err = xend * (ybot - ytop) - (y - ytop) * (xbot - xtop) ~= 0
 *
 *        Note that there are many constants that only need to be
 *        calculated once:
 *
 *            ymid - ytop = top_height      // Y-length of the top side
 *            ybot - ytop = long_height     // Y-length of the long side
 *            ybot - ymid = bottom_height   // Y-length of the bottom side
 *            xmid - xtop = top_width       // X-length of the top side
 *            xbot - xtop = long_height     // X-length of the long side
 *            xbot - xmid = bottom_height   // X-length of the bottom side
 *
 *        So, we can simplify the equations to:
 *
 *        Upper subtriangle:
 *                 xstart_err = xstart * top_height - (y - ytop) * top_width ~= 0
 *        Lower subtriangle:
 *                 xstart_err = xstart * bottom_height - (y - ytop) * bottom_width ~= 0
 *        Both:
 *                 xend_err = xend * long_height - (y - ytop) * long_width ~= 0
 *
 *        Also note that because all sides are straight lines, we know that
 *        xstart and xend are either monotonically increasing, or decreasing.
 *        This means that once we know what is the case for every side,
 *        we can calculate xstart/xend on every line simply by taking the previous
 *        value and incrementing/decrementing it until the error term
 *        reaches or crosses the zero value.
 *
 *        EXAMPLE: let's draw a top subtriangle (the bottom is equivalent)
 *        when top_width < 0 (i.e. the midpoint lies on the left from
 *        the top point), and long_height > 0 (the bottom point lies
 *        on the right from the top point). The rendering code would be:
 *
 *        for (xstart = 0, xend = 0, y = 0; y <= top_height; y++) {
 *                GP_HLine(surf, color, xtop + xstart, ytop + y, xtop + xend);
 *                while (xstart * top_height - top_width * y > 0)
 *                        xstart--;
 *                while (xend * long_height - long_width * y < 0)
 *                        xend++;
 *        }
 *
 *        When looking at the loops, we can see that all the multiplications
 *        consist of a constant and a value that is iterated in the loop.
 *        This allows us to remove the multiplications completely; the
 *        xstart_err and xend_err can be calculated by gradual accumulation,
 *        reusing values from the previous loop cycle:
 *
 *        xstart_err = abs(top_width) / 2;
 *        xend_err = abs(long_width) / 2;
 *        for (y = 0, xstart = 0, xend = 0; y <= top_height; y++) {
 *                GP_HLine(surf, color, xtop + xstart, ytop + y, xtop + xend);
 *                while (xstart_err > 0) {
 *                        xstart_err -= top_height;
 *                        xstart--;
 *                }
 *                xstart_err += -top_width;
 *                while (xend_err > 0) {
 *                        xend_err -= long_height;
 *                        xend++;
 *                }
 *                xend_err += long_width;
 *        }
 *
 *        (The initial values of xstart_err and xend_err are traditionally
 *        chosen to half of the delta of one loop step.)
 */

	/* Sort the three points according to the Y coordinate. */
	int ytop, ymid, ybot;
	int xtop, xmid, xbot;
	if (y0 < y1) {
		if (y1 < y2) {					/* 0, 1, 2 */
			xtop = x0, ytop = y0;
			xmid = x1, ymid = y1;
			xbot = x2, ybot = y2;
		} else {
			if (y2 < y0) {				/* 2, 0, 1 */
				xtop = x2, ytop = y2;
				xmid = x0, ymid = y0;
				xbot = x1, ybot = y1;
			} else {				/* 0, 2, 1 */
				xtop = x0, ytop = y0;
				xmid = x2, ymid = y2;
				xbot = x1, ybot = y1;
			}
		}
	} else {
		if (y1 < y2) {
			if (y2 < y0) {				/* 1, 2, 0 */
				xtop = x1, ytop = y1;
				xmid = x2, ymid = y2;
				xbot = x0, ybot = y0;
			} else {				/* 1, 0, 2 */
				xtop = x1, ytop = y1;
				xmid = x0, ymid = y0;
				xbot = x2, ybot = y2;
			}
		} else {					/* 2, 1, 0 */
			xtop = x2, ytop = y2;
			xmid = x1, ymid = y1;
			xbot = x0, ybot = y0;
		}
	}

	/*
	 * Calculate the Y-axis length ("height") and X-axis
	 * length ("width") of each side.
	 */
	int top_height = ymid - ytop;
	int bottom_height = ybot - ymid;
	int long_height = ybot - ytop;
	int top_width = xmid - xtop;
	int bottom_width = xbot - xmid;
	int long_width = xbot - xtop;

	int y, xstart, xend, xstart_err, xend_err;

/*
 * Updates xstart to the next position, moving it to the left or right
 * until the xstart_err term reaches (or crosses) zero.
 * SIDE is either "top" or "bottom", according to which side of the triangle
 * we are handling.
 */
#define xstart_step_left(SIDE) { \
	while (xstart_err > 0) { \
		xstart_err -= SIDE##_height; \
		xstart--; \
	} \
	xstart_err += -SIDE##_width; \
}
#define xstart_step_right(SIDE) { \
	while (xstart_err > 0) { \
		xstart_err -= SIDE##_height; \
		xstart++; \
	} \
	xstart_err += SIDE##_width; \
}

/*
 * Updates xend to the next position, moving it to the left or right
 * until the xend_err term reaches (or crosses) zero.
 */
#define xend_step_left { \
	while (xend_err > 0) { \
		xend_err -= long_height; \
		xend--; \
	} \
	xend_err += -long_width; \
}
#define xend_step_right { \
	while (xend_err > 0) { \
		xend_err -= long_height; \
		xend++; \
	} \
	xend_err += long_width; \
}

#define top_subtriangle_loop for(; y < top_height; y++)

	/* top subtriangle */
	xstart_err = -abs(top_width)/2;
	xend_err = -abs(long_width)/2;
	y = 0;
	xstart = xtop;
	xend = xtop;
	if (top_width > 0) {
		if (long_width > 0) {
			top_subtriangle_loop {
				xstart_step_right(top);
				xend_step_right;
				GP_HLine(surf, color, xstart, xend, ytop + y);
			}
		} else {
			top_subtriangle_loop {
				xstart_step_right(top);
				xend_step_left;
				GP_HLine(surf, color, xstart, xend, ytop + y);
			}
		}
	} else {
		if (long_width > 0) {
			top_subtriangle_loop {
				xstart_step_left(top);
				xend_step_right;
				GP_HLine(surf, color, xstart, xend, ytop + y);
			}
		} else {
			top_subtriangle_loop {
				xstart_step_left(top);
				xend_step_left;
				GP_HLine(surf, color, xstart, xend, ytop + y);
			}
		}
	}

	/* triangle with a flat bottom is a special case */
	if (bottom_height == 0) return;

#define bottom_subtriangle_loop for(; y < long_height; y++)

	/* bottom subtriangle */
	xstart_err = -abs(bottom_width)/2;
	xstart = xmid;
	if (bottom_width > 0) {
		if (long_width > 0) {
			bottom_subtriangle_loop {
				xstart_step_right(bottom);
				xend_step_right;
				GP_HLine(surf, color, xstart, xend, ytop + y);
			}
		} else {
			bottom_subtriangle_loop {
				xstart_step_right(bottom);
				xend_step_left;
				GP_HLine(surf, color, xstart, xend, ytop + y);
			}
		}
	} else {
		if (long_width > 0) {
			bottom_subtriangle_loop {
				xstart_step_left(bottom);
				xend_step_right;
				GP_HLine(surf, color, xstart, xend, ytop + y);
			}
		} else {
			bottom_subtriangle_loop {
				xstart_step_left(bottom);
				xend_step_left;
				GP_HLine(surf, color, xstart, xend, ytop + y);
			}
		}
	}
}
#endif

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

	int ABxstep = (ABdx < 0) ? -1 : 1;
	int ACxstep = (ACdx < 0) ? -1 : 1;
	int BCxstep = (BCdx < 0) ? -1 : 1;

//	printf("GP_Triangle(): ----\n");
//	printf("GP_Triangle(): A = (%d, %d), B = (%d, %d), C = (%d, %d)\n", Ax, Ay, Bx, By, Cx, Cy);
//	printf("GP_Triangle(): ABd = (%d, %d), ACd = (%d, %d), BCd = (%d, %d)\n", ABdx, ABdy, ACdx, ACdy, BCdx, BCdy);

	int y, ABx, ACx, BCx, ABerr, ACerr, BCerr;
	for (ABx = Ax, ACx = Ax, y = Ay; y < By; y++) {
		for (;;) {
			ABerr = (ABx - Ax) * ABdy - (y - Ay) * ABdx;
			if (ABxstep > 0 ? ABerr >= 0 : ABerr <= 0)
				break;
			ABx += ABxstep;
		}
		for (;;) {
			ACerr = (ACx - Ax) * ACdy - (y - Ay) * ACdx;
			if (ACxstep > 0 ? ACerr >= 0 : ACerr <= 0)
				break;
			ACx += ACxstep;
		}
		GP_HLine(surf, color, ABx, ACx, y);
	}
	for (BCx = Bx, y = By; y <= Cy; y++) {
		for (;;) {
			BCerr = (BCx - Bx) * BCdy - (y - By) * BCdx;
			if (BCxstep > 0 ? BCerr >= 0 : BCerr <= 0)
				break;
			BCx += BCxstep;
		}
		for (;;) {
			ACerr = (ACx - Ax) * ACdy - (y - Ay) * ACdx;
			if (ACxstep > 0 ? ACerr >= 0 : ACerr <= 0)
				break;
			ACx += ACxstep;
		}
		GP_HLine(surf, color, BCx, ACx, y);
	}
}

