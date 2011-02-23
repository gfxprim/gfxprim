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

/* A vertical line drawing algorithm. */

/* Ensures that coordinates are in correct order, and clips them.
 * Exits immediately if the line is completely clipped out.
 */
#define ORDER_AND_CLIP_COORDS do { \
	if (y0 > y1) GP_SWAP(y0, y1); \
	if (x < (int) context->clip_w_min \
	    || x > (int) context->clip_w_max \
	    || y1 < (int) context->clip_h_min \
	    || y0 > (int) context->clip_h_max) { \
		return; \
	} \
	y0 = GP_MAX(y0, (int) context->clip_h_min); \
	y1 = GP_MIN(y1, (int) context->clip_h_max); \
} while (0)

/*
 * This macro defines a horizontal line drawing function.
 * Arguments:
 *     CONTEXT_T - user-defined type of drawing context (passed to PUTPIXEL)
 *     PIXVAL_T  - user-defined pixel value type (passed to PUTPIXEL)
 *     PIXEL_ADDRESS  - a function that returns a pointer to a pixel in memory,
 *                      in form f(context, y, x)
 *     WRITE_PIXELS   - a function that fills a linear block with pixel value,
 *                      in form f(start, length, pixel)
 *     FN_NAME   - name of the function to be defined
 */
#define DEF_VLINE_FN(FN_NAME, CONTEXT_T, PIXEL_T, PUT_PIXEL) \
void FN_NAME(CONTEXT_T context, int x, int y0, int y1, PIXEL_T pixel) \
{ \
	ORDER_AND_CLIP_COORDS; \
	int y; \
\
	for (y = y0; y <= y1; y++) \
		PUT_PIXEL(context, x, y, pixel); \
}
