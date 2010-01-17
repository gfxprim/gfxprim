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

#define FN_ATTR		static
#define FN_NAME 	GP_SDL_Rect_8bpp
#define HLINE 		GP_SDL_HLine_8bpp
#define VLINE 		GP_SDL_VLine_8bpp
#include "generic/rect_generic.c"

#define FN_ATTR		static
#define FN_NAME		GP_SDL_Rect_16bpp
#define HLINE		GP_SDL_HLine_16bpp
#define VLINE		GP_SDL_VLine_16bpp
#include "generic/rect_generic.c"

#define FN_ATTR		static
#define FN_NAME		GP_SDL_Rect_24bpp
#define HLINE		GP_SDL_HLine_24bpp
#define VLINE		GP_SDL_VLine_24bpp
#include "generic/rect_generic.c"

#define FN_ATTR		static
#define FN_NAME		GP_SDL_Rect_32bpp
#define HLINE		GP_SDL_HLine_32bpp
#define VLINE		GP_SDL_VLine_32bpp
#include "generic/rect_generic.c"

void GP_SDL_Rect(GP_TARGET_TYPE *target, GP_COLOR_TYPE color, int x0, int y0, int x1, int y1)
{
	switch (GP_BYTES_PER_PIXEL(target)) {
	case 1:
		GP_SDL_Rect_8bpp(target, color, x0, y0, x1, y1);
		break;
	
	case 2:
		GP_SDL_Rect_16bpp(target, color, x0, y0, x1, y1);
		break;
	
	case 3:
		GP_SDL_Rect_24bpp(target, color, x0, y0, x1, y1);
		break;
	
	case 4:
		GP_SDL_Rect_32bpp(target, color, x0, y0, x1, y1);
		break;
	}
}

void GP_SDL_FillRect(GP_TARGET_TYPE *target, GP_COLOR_TYPE color, int x0, int y0, int x1, int y1)
{
	int top, left, bottom, right;

	if (y0 <= y1) {
		top    = y0;
		bottom = y1;
	} else {
		top    = y1;
		bottom = y0;
	}

	if (x0 <= x1) {
		left  = x0;
		right = x1;
	} else {
		left  = x1;
		right = x0;
	}

	SDL_Rect rect = {left, top, right - left + 1, bottom - top + 1};
	SDL_FillRect(target, &rect, color);
}

