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

#include "GP.h"
#include "GP_SDL.h"
#include "GP_SDL_backend.h"
#include "GP_writepixel.h"

#define FN_NAME		GP_SDL_VLine_8bpp
#define WRITE_PIXEL	GP_WritePixel8bpp
#include "generic/vline_generic.c"

#define FN_NAME		GP_SDL_VLine_16bpp
#define WRITE_PIXEL	GP_WritePixel16bpp
#include "generic/vline_generic.c"

#define FN_NAME		GP_SDL_VLine_24bpp
#define WRITE_PIXEL	GP_WritePixel24bpp
#include "generic/vline_generic.c"

#define FN_NAME		GP_SDL_VLine_32bpp
#define WRITE_PIXEL	GP_WritePixel32bpp
#include "generic/vline_generic.c"

void GP_SDL_Line(SDL_Surface *target, long color, int x0, int y0, int x1, int y1)
{
	GP_Context context;
	GP_SDL_ContextFromSurface(target, &context);

	GP_Line(&context, x0, y0, x1, y1, color);
}

void GP_SDL_HLine(SDL_Surface *target, long color, int x0, int x1, int y)
{
	GP_Context context;
	GP_SDL_ContextFromSurface(target, &context);

	GP_HLine(&context, x0, x1, y, color);
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

