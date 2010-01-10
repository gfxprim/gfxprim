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
 * Parameterized template for function for drawing vertical lines.
 * To be #included from line.c.
 * Parameters that must be #defined outside:
 *
 * 	FN_NAME
 * 		Name of the function.
 * 	WRITE_PIXEL
 * 		A pixel writing routine to use. Must have form
 * 		void WRITE_PIXEL(uint8_t *p, long color).
 */

void FN_NAME(GP_TARGET_TYPE *target, GP_COLOR_TYPE color, int x, int y0, int y1)
{
	if (target == NULL || GP_PIXELS(target) == NULL)
		return;

	/* Ensure that y0 <= y1, swap coordinates if needed. */
	if (y0 > y1) {
		FN_NAME(target, color, x, y1, y0);
		return;
	}

	/* Get the clipping rectangle. */
	int xmin, xmax, ymin, ymax;
	GP_GET_CLIP_RECT(target, xmin, xmax, ymin, ymax);

	/* Check whether the line is not completely clipped out. */
	if (x < xmin || x > xmax || y0 > ymax || y1 < xmin)
		return;

	/* Clip the start and end of the line. */
	if (y0 < ymin) {
		y0 = ymin;
	}
	if (y1 > ymax) {
		y1 = ymax;
	}

	int bytes_per_line = GP_BYTES_PER_LINE(target);

	/* Get the starting and ending address of the line. */
	uint8_t *p_start = GP_PIXEL_ADDR(target, x, y0);
	uint8_t *p_end = p_start + (y1 - y0) * bytes_per_line;

	/* Write pixels. */
	uint8_t * p;
	for (p = p_start; p <= p_end; p += bytes_per_line) {
		WRITE_PIXEL(p, color);
	}
}

