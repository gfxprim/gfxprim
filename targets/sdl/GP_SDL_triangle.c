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

#define FN_NAME		GP_SDL_Triangle_8bpp
#define LINE		GP_SDL_Line_8bpp
#define SETPIXEL	GP_SDL_SetPixel_8bpp
#include "generic/triangle_generic.c"

#define FN_NAME		GP_SDL_Triangle_16bpp
#define LINE		GP_SDL_Line_16bpp
#define SETPIXEL	GP_SDL_SetPixel_16bpp
#include "generic/triangle_generic.c"

#define FN_NAME		GP_SDL_Triangle_24bpp
#define LINE		GP_SDL_Line_24bpp
#define SETPIXEL	GP_SDL_SetPixel_24bpp
#include "generic/triangle_generic.c"

#define FN_NAME		GP_SDL_Triangle_32bpp
#define LINE		GP_SDL_Line_32bpp
#define SETPIXEL	GP_SDL_SetPixel_32bpp
#include "generic/triangle_generic.c"

#define FN_NAME		GP_SDL_FillTriangle_8bpp
#define HLINE		GP_SDL_HLine_8bpp
#include "generic/fill_triangle_generic.c"

#define FN_NAME		GP_SDL_FillTriangle_16bpp
#define HLINE		GP_SDL_HLine_16bpp
#include "generic/fill_triangle_generic.c"

#define FN_NAME		GP_SDL_FillTriangle_24bpp
#define HLINE		GP_SDL_HLine_24bpp
#include "generic/fill_triangle_generic.c"

#define FN_NAME		GP_SDL_FillTriangle_32bpp
#define HLINE		GP_SDL_HLine_32bpp
#include "generic/fill_triangle_generic.c"

void GP_SDL_Triangle(GP_TARGET_TYPE *target, GP_COLOR_TYPE color,
	int x0, int y0, int x1, int y1, int x2, int y2)
{
	switch (GP_BYTES_PER_PIXEL(target)) {
	case 1:
		GP_SDL_Triangle_8bpp(target, color, x0, y0, x1, y1, x2, y2);
		break;
	case 2:
		GP_SDL_Triangle_16bpp(target, color, x0, y0, x1, y1, x2, y2);
		break;
	case 3:
		GP_SDL_Triangle_24bpp(target, color, x0, y0, x1, y1, x2, y2);
		break;
	case 4:
		GP_SDL_Triangle_32bpp(target, color, x0, y0, x1, y1, x2, y2);
		break;
	}
}

void GP_SDL_FillTriangle(GP_TARGET_TYPE *target, GP_COLOR_TYPE color,
	int x0, int y0, int x1, int y1, int x2, int y2)
{
	switch (GP_BYTES_PER_PIXEL(target)) {
	case 1:
		GP_SDL_FillTriangle_8bpp(target, color, x0, y0, x1, y1, x2, y2);
		break;
	case 2:
		GP_SDL_FillTriangle_16bpp(target, color, x0, y0, x1, y1, x2, y2);
		break;
	case 3:
		GP_SDL_FillTriangle_24bpp(target, color, x0, y0, x1, y1, x2, y2);
		break;
	case 4:
		GP_SDL_FillTriangle_32bpp(target, color, x0, y0, x1, y1, x2, y2);
		break;
	}
}

