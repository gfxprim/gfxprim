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

#include "GP_SDL.h"
#include "GP_SDL_backend.h"
#include "GP_writepixel.h"

#define FN_ATTR		GP_INTERNAL_FN
#define FN_NAME		GP_SDL_Line_8bpp
#define SETPIXEL	GP_SDL_SetPixel_8bpp
#include "generic/line_generic.c"

#define FN_ATTR		GP_INTERNAL_FN
#define FN_NAME		GP_SDL_Line_16bpp
#define SETPIXEL	GP_SDL_SetPixel_16bpp
#include "generic/line_generic.c"

#define FN_ATTR		GP_INTERNAL_FN
#define FN_NAME		GP_SDL_Line_24bpp
#define SETPIXEL	GP_SDL_SetPixel_24bpp
#include "generic/line_generic.c"

#define FN_ATTR		GP_INTERNAL_FN
#define FN_NAME		GP_SDL_Line_32bpp
#define SETPIXEL	GP_SDL_SetPixel_32bpp
#include "generic/line_generic.c"

#define FN_ATTR		GP_INTERNAL_FN
#define FN_NAME		GP_SDL_HLine_8bpp
#define BYTES_PER_PIXEL	1
#include "generic/hline_generic.c"

#define FN_ATTR		GP_INTERNAL_FN
#define FN_NAME		GP_SDL_HLine_16bpp
#define BYTES_PER_PIXEL	2
#include "generic/hline_generic.c"

#define FN_ATTR		GP_INTERNAL_FN
#define FN_NAME		GP_SDL_HLine_24bpp
#define BYTES_PER_PIXEL	3
#include "generic/hline_generic.c"

#define FN_ATTR		GP_INTERNAL_FN
#define FN_NAME		GP_SDL_HLine_32bpp
#define BYTES_PER_PIXEL	4
#include "generic/hline_generic.c"

#define FN_ATTR		GP_INTERNAL_FN
#define FN_NAME		GP_SDL_VLine_8bpp
#define WRITE_PIXEL	GP_WRITE_PIXEL_1BYTE
#include "generic/vline_generic.c"

#define FN_ATTR		GP_INTERNAL_FN
#define FN_NAME		GP_SDL_VLine_16bpp
#define WRITE_PIXEL	GP_WRITE_PIXEL_2BYTES
#include "generic/vline_generic.c"

#define FN_ATTR		GP_INTERNAL_FN
#define FN_NAME		GP_SDL_VLine_24bpp
#define WRITE_PIXEL	GP_WRITE_PIXEL_3BYTES
#include "generic/vline_generic.c"

#define FN_ATTR		GP_INTERNAL_FN
#define FN_NAME		GP_SDL_VLine_32bpp
#define WRITE_PIXEL	GP_WRITE_PIXEL_4BYTES
#include "generic/vline_generic.c"

void GP_SDL_Line(SDL_Surface *target, long color, int x0, int y0, int x1, int y1)
{
	switch (GP_BYTES_PER_PIXEL(target)) {
	case 1:
		GP_SDL_Line_8bpp(target, color, x0, y0, x1, y1);
		break;
	case 2:
		GP_SDL_Line_16bpp(target, color, x0, y0, x1, y1);
		break;
	case 3:
		GP_SDL_Line_24bpp(target, color, x0, y0, x1, y1);
		break;
	case 4:
		GP_SDL_Line_32bpp(target, color, x0, y0, x1, y1);
		break;
	}
}

void GP_SDL_HLine(SDL_Surface *target, long color, int x0, int x1, int y)
{
	switch (GP_BYTES_PER_PIXEL(target)) {
	case 1:
		GP_SDL_HLine_8bpp(target, color, x0, x1, y);
		break;
	case 2:
		GP_SDL_HLine_16bpp(target, color, x0, x1, y);
		break;
	case 3:
		GP_SDL_HLine_24bpp(target, color, x0, x1, y);
		break;
	case 4:
		GP_SDL_HLine_32bpp(target, color, x0, x1, y);
		break;
	}
}

void GP_SDL_VLine(SDL_Surface *target, long color, int x, int y0, int y1)
{
	switch (GP_BYTES_PER_PIXEL(target)) {
	case 1:
		GP_SDL_VLine_8bpp(target, color, x, y0, y1);
		break;
	case 2:
		GP_SDL_VLine_16bpp(target, color, x, y0, y1);
		break;
	case 3:
		GP_SDL_VLine_24bpp(target, color, x, y0, y1);
		break;
	case 4:
		GP_SDL_VLine_32bpp(target, color, x, y0, y1);
		break;
	}
}

#if 0
void GP_HLineWide(GP_TARGET_TYPE *target, GP_COLOR_TYPE color,
	enum GP_LinePosition pos, uint8_t thickness, int x0, int x1, int y)
{
	switch (pos) {
		case GP_LINE_ABOVE:
			GP_FillRect(target, color, x0, y + thickness, x1, y);
		break;
		case GP_LINE_BELOW:
			GP_FillRect(target, color, x0, y - thickness, x1, y);
		break;
		case GP_LINE_CENTER:
			GP_FillRect(target, color, x0, y - thickness, x1, y +  thickness);
		break;
	}
}

void GP_VLineWide(GP_TARGET_TYPE *target, GP_COLOR_TYPE color,
	enum GP_LinePosition pos, uint8_t thickness, int x, int y0, int y1)
{
	switch (pos) {
		case GP_LINE_ABOVE:
			GP_FillRect(target, color, x + thickness, y0, x, y1);
		break;
		case GP_LINE_BELOW:
			GP_FillRect(target, color, x - thickness, y0, x, y1);
		break;
		case GP_LINE_CENTER:
			GP_FillRect(target, color, x - thickness, y0, x + thickness, y1);
		break;
	}
}
#endif

