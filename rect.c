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

#include "GP_line.h"
#include "GP_gfx.h"

/*
 * Draws a rectangle from (x0, y0) to (x1, y1).
 */
void GP_Rect(SDL_Surface *surf, long color, int x0, int y0, int x1, int y1)
{
	GP_HLine(surf, color, x0, x1, y0);
	GP_HLine(surf, color, x0, x1, y1);
	GP_VLine(surf, color, x0, y0, y1);
	GP_VLine(surf, color, x1, y0, y1);
}

/*
 * Draws a solid filled rectangle from (x0, y0) to (x1, y1).
 */
void GP_FillRect(SDL_Surface *surf, long color, int x0, int y0, int x1, int y1)
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
	SDL_FillRect(surf, &rect, color);
}

