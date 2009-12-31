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
 * File to be #included from line.c.
 *
 * This file is a macro template. With each inclusion, you get a definition
 * of a horizontal line drawing function in form:
 *
 * void FN_NAME(SDL_Surface * surf, long color, int x0, int x1, int y)
 *
 * These arguments must be #defined in the including file:
 *
 * 	FN_NAME
 * 		Name of the function.
 * 	BYTES_PER_PIXEL
 * 		Number of bytes per pixel of the target.
 * 	WRITE_PIXEL
 * 		A pixel writing routine to use. Must have form
 * 		void WRITE_PIXEL(uint8_t *p, long color).
 */

void FN_NAME(SDL_Surface *surf, long color, int x0, int x1, int y)
{
	if (surf == NULL || surf->format == NULL)
		return;

	/* Ensure that x0 <= x1, swap coordinates if needed. */
	if (x0 > x1) {
		FN_NAME(surf, color, x1, x0, y);
		return;
	}

	/* Get the clipping rectangle. */
	int xmin, xmax, ymin, ymax;
	GP_GET_CLIP_RECT(surf, xmin, xmax, ymin, ymax);

	/* Check whether the line is not completely clipped out. */
	if (y < ymin || y > ymax || x0 > xmax || x1 < xmin)
		return;

	/* Clip the start and end of the line. */
	if (x0 < xmin) {
		x0 = xmin;
	}
	if (x1 > xmax) {
		x1 = xmax;
	}

	/* Get the starting and ending address of the line. */
	uint8_t *p_start = GP_PIXEL_ADDR(surf, x0, y);
	uint8_t *p_end = p_start + (x1 - x0) * BYTES_PER_PIXEL;

	/* Write pixels. */
	uint8_t * p;
	for (p = p_start; p <= p_end; p += BYTES_PER_PIXEL)
		WRITE_PIXEL(p, color);
}

