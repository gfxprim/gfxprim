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

/* A horizontal line drawing algorithm. */

/* Ensures that coordinates are in correct order, and clips them.
 * Exits immediately if the line is completely clipped out.
 */
#define ORDER_AND_CLIP_COORDS do { \
	if (x0 > x1) GP_SWAP(x0, x1); \
	if (y < (int) context->clip_h_min \
		|| y > (int) context->clip_h_max \
		|| x0 > (int) context->clip_w_max \
		|| x1 < (int) context->clip_w_min) { \
		return; \
	} \
	x0 = GP_MAX(x0, (int) context->clip_w_min); \
	x1 = GP_MIN(x1, (int) context->clip_w_max); \
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
#define DEF_HLINE_FN(FN_NAME, CONTEXT_T, PIXEL_T, PIXEL_ADDRESS, WRITE_PIXELS) \
void FN_NAME(GP_Context *context, int x0, int x1, int y, GP_Pixel pixel) \
{ \
	ORDER_AND_CLIP_COORDS; \
\
	size_t length = 1 + x1 - x0; \
	void *start = GP_PIXEL_ADDRESS(context, y, x0); \
\
	WRITE_PIXELS(start, length, pixel); \
}

/*
 * Not byte aligned pixels
 */
#define DEF_HLINE_BU_FN(FN_NAME, CONTEXT_T, PIXEL_T, PIXEL_ADDRESS, WRITE_PIXELS) \
void FN_NAME(GP_Context *context, int x0, int x1, int y, GP_Pixel pixel) \
{ \
	ORDER_AND_CLIP_COORDS; \
\
	size_t length = 1 + x1 - x0; \
	void *start = GP_PIXEL_ADDRESS(context, y, x0); \
\
	WRITE_PIXELS(start, x0 % 8, length, pixel); \
}
