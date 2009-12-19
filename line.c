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

/*
 * Draws a line from (x0, y0) to (x1, y1), inclusive.
 * Coordinates are automatically clipped both to surface boundaries and
 * to the surface clipping rectangle.
 */
void GP_Line(SDL_Surface *surf, long color, int x0, int y0, int x1, int y1)
{
	/* The steepness of the line */
	int deltax = x1 - x0;
	int deltay = y1 - y0;

	/*
	 * Subdivide lines to octants according to two boolean factors:
	 *
	 * - whether Y is increasing/decreasing (delta Y is positive/negative)
	 * - whether X or Y is changing faster.
	 *
	 * For each octant:
	 * When drawing, the faster-changing coordinate is iterated one by one,
	 * and the other (which changes slower) is calculated.
	 */
	int x = x0, y = y0;
	int error;
	if (abs(deltax) > abs(deltay)) {	/* X changes faster */

		error = abs(deltax) / 2;

		if (deltax >= 0) {		/* X increases */
			if (deltay > 0) {	/* Y increases */

				for (; x <= x1; x++) {
					GP_SetPixel(surf, color, x, y);
					error -= deltay;
					if (error < 0) { 
						y++;
						error += deltax;
					}
				}
			} else {		/* Y decreases */

				for (; x <= x1; x++) {
					GP_SetPixel(surf, color, x, y);
					error -= -deltay;
					if (error < 0) { 
						y--;
						error += deltax;
					}
				}
			}
		} else {			/* X decreases */

			/*
			 * Swap endpoints and draw with increasing X.
			 * This ensures that the pixels plotted are exactly
			 * the same as with the opposite direction.
			 */
			GP_Line(surf, color, x1, y1, x0, y0);
			return;
		}
	} else {				/* Y changes faster */

		error = abs(deltay) / 2;

		if (deltay >= 0) {		/* Y increases */
			if (deltax > 0) {	/* X increases */
				for (; y <= y1; y++) {
					GP_SetPixel(surf, color, x, y);
					error -= deltax;
					if (error < 0) {
						x++;
						error += deltay;
					}
				}
			} else {		/* X decreases */

				for (; y <= y1; y++) {
					GP_SetPixel(surf, color, x, y);
					error -= -deltax;
					if (error < 0) {
						x--;
						error += deltay;
					}
				}
			}
		} else {			/* Y decreases */

			/*
			 * Swap endpoints and draw the line with increasing Y.
			 * This ensures that the pixels plotted are exactly
			 * the same as with the opposite direction.
			 */
			GP_Line(surf, color, x1, y1, x0, y0);
			return;
		}
	}
}

/*
 * Draws a horizontal line from (x0, y) to (x1, y), inclusive.
 */
void GP_HLine(SDL_Surface *surf, long color, int x0, int x1, int y)
{
	if (surf == NULL || surf->format == NULL)
		return;

	/* Ensure that x0 <= x1, swap coordinates if needed. */
	if (x0 > x1) {
		GP_HLine(surf, color, x1, x0, y);
		return;
	}

	/* Get the clipping rectangle. */
	int xmin = surf->clip_rect.x;
	int xmax = surf->clip_rect.x + surf->clip_rect.w - 1;
	int ymin = surf->clip_rect.y;
	int ymax = surf->clip_rect.y + surf->clip_rect.h - 1;

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

	int bytes_per_pixel = surf->format->BytesPerPixel;

	/* Get the starting and ending address of the line. */
	Uint8 *p_start = ((Uint8 *) surf->pixels)
				+ y * surf->pitch
				+ x0 * bytes_per_pixel;
	Uint8 *p_end = p_start + (x1 - x0) * bytes_per_pixel;

	/* Write pixels. */
	Uint8 * p;
	switch (bytes_per_pixel) {
	case 1:
		for (p = p_start; p <= p_end; p++)
			*p = (Uint8) color;
		break;
	
	case 2:
		for (p = p_start; p <= p_end; p += 2)
			*(Uint16 *) p = (Uint16) color;
		break;
	
	case 3:
		for (p = p_start; p <= p_end; p += 3) {

#if SDL_BYTEORDER == SDL_BIG_ENDIAN
			p[0] = (color >> 16) & 0xff;
			p[1] = (color >> 8) & 0xff;
			p[2] = color & 0xff;
#else
			p[0] = color & 0xff;
			p[1] = (color >> 8) & 0xff;
			p[2] = (color >> 16) & 0xff;
#endif
		}
		break;

	case 4:
		for (p = p_start; p <= p_end; p += 4)
			*(Uint32 *)p = (Uint32) color;
		break;
	}
}

/*
 * Draws a vertical line from (x, y0) to (x, y1), inclusive.
 */
void GP_VLine(SDL_Surface *surf, long color, int x, int y0, int y1)
{
	if (surf == NULL || surf->format == NULL)
		return;

	/* Ensure that y0 <= y1, swap coordinates if needed. */
	if (y0 > y1) {
		GP_VLine(surf, color, x, y1, y0);
		return;
	}

	/* Get the clipping rectangle. */
	int xmin = surf->clip_rect.x;
	int xmax = surf->clip_rect.x + surf->clip_rect.w - 1;
	int ymin = surf->clip_rect.y;
	int ymax = surf->clip_rect.y + surf->clip_rect.h - 1;

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

	int bytes_per_pixel = surf->format->BytesPerPixel;
	int pitch = surf->pitch;

	/* Get the starting and ending address of the line. */
	Uint8 *p_start = ((Uint8 *) surf->pixels)
				+ y0 * surf->pitch
				+ x * bytes_per_pixel;
	Uint8 *p_end = p_start + (y1 - y0) * surf->pitch;

	/* Write pixels. */
	Uint8 * p;
	switch (bytes_per_pixel) {
	case 1:
		for (p = p_start; p <= p_end; p += pitch)
			*p = (Uint8) color;
		break;
	
	case 2:
		for (p = p_start; p <= p_end; p += pitch)
			*(Uint16 *) p = (Uint16) color;
		break;
	
	case 3:
		for (p = p_start; p <= p_end; p += pitch) {

#if SDL_BYTEORDER == SDL_BIG_ENDIAN
			p[0] = (color >> 16) & 0xff;
			p[1] = (color >> 8) & 0xff;
			p[2] = color & 0xff;
#else
			p[0] = color & 0xff;
			p[1] = (color >> 8) & 0xff;
			p[2] = (color >> 16) & 0xff;
#endif
		}
		break;

	case 4:
		for (p = p_start; p <= p_end; p += pitch)
			*(Uint32 *)p = (Uint32) color;
		break;
	}
}

