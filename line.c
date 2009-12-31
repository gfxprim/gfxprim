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
#include "GP_line.h"
#include "GP_gfx.h"

/*
 * The generic GP_Line() function; first identifies the bits per pixel,
 * and then calls the appropriate specialized function.
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
#include "templates/line_tmpl.c"
#undef SETPIXEL
#undef FN_NAME

#define FN_NAME		GP_Line_16bpp
#define SETPIXEL	GP_SetPixel_16bpp
#include "templates/line_tmpl.c"
#undef SETPIXEL
#undef FN_NAME

#define FN_NAME		GP_Line_24bpp
#define SETPIXEL	GP_SetPixel_24bpp
#include "templates/line_tmpl.c"
#undef SETPIXEL
#undef FN_NAME

#define FN_NAME		GP_Line_32bpp
#define SETPIXEL	GP_SetPixel_32bpp
#include "templates/line_tmpl.c"
#undef SETPIXEL
#undef FN_NAME

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
	Uint8 *p_start = GP_PIXEL_ADDR(surf, x0, y);
	Uint8 *p_end = p_start + (x1 - x0) * bytes_per_pixel;

	/* Write pixels. */
	Uint8 * p;
	switch (bytes_per_pixel) {
	case 1:
		for (p = p_start; p <= p_end; p++)
			GP_WRITE_PIXEL_1BYTE(p, color);
		break;
	
	case 2:
		for (p = p_start; p <= p_end; p += 2)
			GP_WRITE_PIXEL_2BYTES(p, color);
		break;
	
	case 3:
		for (p = p_start; p <= p_end; p += 3) {
			GP_WRITE_PIXEL_3BYTES(p, color);
		}
		break;

	case 4:
		for (p = p_start; p <= p_end; p += 4)
			GP_WRITE_PIXEL_4BYTES(p, color);
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

