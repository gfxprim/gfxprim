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

#include "GP_pixel.h"
#include "GP_clip_rect.h"
#include "GP_line.h"
#include "GP_gfx.h"

/*
 * Adaptive implementation of GP_Line() function. It first identifies
 * the number of bits per pixel of the image, and then calls the appropriate
 * specialized function.
 */
void GP_Line(SDL_Surface * surf, long color, int x0, int y0, int x1, int y1)
{
	if (surf == NULL || surf->pixels == NULL)
		return;

	switch (surf->format->BytesPerPixel) {
	case 1:
		GP_Line_8bpp(surf, color, x0, y0, x1, y1);
		break;
	case 2:
		GP_Line_16bpp(surf, color, x0, y0, x1, y1);
		break;
	case 3:
		GP_Line_24bpp(surf, color, x0, y0, x1, y1);
		break;
	case 4:
		GP_Line_32bpp(surf, color, x0, y0, x1, y1);
		break;
	}
}

/*
 * Build the specialized GP_Line() variants for various bit depths:
 * GP_Line_8bpp(), GP_Line_16bpp(), GP_Line_24bpp(), GP_Line_32bpp()
 * for surfaces of 8, 16, 24 or 32 bits per pixel, respectively.
 */

#define FN_NAME		GP_Line_8bpp
#define SETPIXEL	GP_SetPixel_8bpp
#include "generic/line_generic.c"
#undef SETPIXEL
#undef FN_NAME

#define FN_NAME		GP_Line_16bpp
#define SETPIXEL	GP_SetPixel_16bpp
#include "generic/line_generic.c"
#undef SETPIXEL
#undef FN_NAME

#define FN_NAME		GP_Line_24bpp
#define SETPIXEL	GP_SetPixel_24bpp
#include "generic/line_generic.c"
#undef SETPIXEL
#undef FN_NAME

#define FN_NAME		GP_Line_32bpp
#define SETPIXEL	GP_SetPixel_32bpp
#include "generic/line_generic.c"
#undef SETPIXEL
#undef FN_NAME

/*
 * A bitdepth-adaptive version of GP_HLine().
 */
void GP_HLine(SDL_Surface *surf, long color, int x0, int x1, int y)
{
	if (surf == NULL || surf->format == NULL)
		return;

	switch (surf->format->BytesPerPixel) {
	case 1:
		GP_HLine_8bpp(surf, color, x0, x1, y);
		break;
	case 2:
		GP_HLine_16bpp(surf, color, x0, x1, y);
		break;
	case 3:
		GP_HLine_24bpp(surf, color, x0, x1, y);
		break;
	case 4:
		GP_HLine_32bpp(surf, color, x0, x1, y);
		break;
	}
}

/*
 * Build specialized GP_HLine() variants for various bit depths.
 */

#define FN_NAME		GP_HLine_8bpp
#define BYTES_PER_PIXEL	1
#define WRITE_PIXEL	GP_WRITE_PIXEL_1BYTE
#include "generic/hline_generic.c"
#undef WRITE_PIXEL
#undef BYTES_PER_PIXEL
#undef FN_NAME

#define FN_NAME		GP_HLine_16bpp
#define BYTES_PER_PIXEL	2
#define WRITE_PIXEL	GP_WRITE_PIXEL_2BYTES
#include "generic/hline_generic.c"
#undef WRITE_PIXEL
#undef BYTES_PER_PIXEL
#undef FN_NAME

#define FN_NAME		GP_HLine_24bpp
#define BYTES_PER_PIXEL	3
#define WRITE_PIXEL	GP_WRITE_PIXEL_3BYTES
#include "generic/hline_generic.c"
#undef WRITE_PIXEL
#undef BYTES_PER_PIXEL
#undef FN_NAME

#define FN_NAME		GP_HLine_32bpp
#define BYTES_PER_PIXEL	4
#define WRITE_PIXEL	GP_WRITE_PIXEL_4BYTES
#include "generic/hline_generic.c"
#undef WRITE_PIXEL
#undef BYTES_PER_PIXEL
#undef FN_NAME

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
	int xmin, xmax, ymin, ymax;
	GP_GET_CLIP_RECT(surf, xmin, xmax, ymin, ymax);

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
	Uint8 *p_start = GP_PIXEL_ADDR(surf, x, y0);
	Uint8 *p_end = p_start + (y1 - y0) * surf->pitch;

	/* Write pixels. */
	Uint8 * p;
	switch (bytes_per_pixel) {
	case 1:
		for (p = p_start; p <= p_end; p += pitch)
			GP_WRITE_PIXEL_1BYTE(p, color);
		break;
	
	case 2:
		for (p = p_start; p <= p_end; p += pitch)
			GP_WRITE_PIXEL_2BYTES(p, color);
		break;
	
	case 3:
		for (p = p_start; p <= p_end; p += pitch)
			GP_WRITE_PIXEL_3BYTES(p, color);
		break;

	case 4:
		for (p = p_start; p <= p_end; p += pitch)
			GP_WRITE_PIXEL_4BYTES(p, color);
		break;
	}
}

void GP_HLineWide(SDL_Surface *surf, long color, enum GP_LinePosition pos, uint8_t thickness, int x0, int x1, int y)
{
	switch (pos) {
		case GP_LINE_ABOVE:
			GP_FillRect(surf, color, x0, y + thickness, x1, y);
		break;
		case GP_LINE_BELOW:
			GP_FillRect(surf, color, x0, y - thickness, x1, y);
		break;
		case GP_LINE_CENTER:
			GP_FillRect(surf, color, x0, y - thickness, x1, y +  thickness);
		break;
	}
}

void GP_VLineWide(SDL_Surface *surf, long color, enum GP_LinePosition pos, uint8_t thickness, int x, int y0, int y1)
{
	switch (pos) {
		case GP_LINE_ABOVE:
			GP_FillRect(surf, color, x + thickness, y0, x, y1);
		break;
		case GP_LINE_BELOW:
			GP_FillRect(surf, color, x - thickness, y0, x, y1);
		break;
		case GP_LINE_CENTER:
			GP_FillRect(surf, color, x - thickness, y0, x + thickness, y1);
		break;
	}
}

