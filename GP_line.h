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

#ifndef GP_LINE_H
#define GP_LINE_H

#include <SDL/SDL.h>
#include <stdint.h>

void GP_Line(SDL_Surface *surf, long color, int x0, int y0, int x1, int y1);
void GP_HLine(SDL_Surface *surf, long color, int x0, int x1, int y);
void GP_VLine(SDL_Surface *surf, long color, int x, int y0, int y1);

enum GP_LinePosition {
	GP_LINE_ABOVE,
	GP_LINE_BELOW,
	GP_LINE_CENTER, /* thickness is treated as radius */
};

void GP_HLineWide(SDL_Surface *surf, long color, enum GP_LinePosition pos, uint8_t thickness, int x0, int x1, int y);
void GP_VLineWide(SDL_Surface *surf, long color, enum GP_LinePosition pos, uint8_t thickness, int x, int y0, int y1);

#endif /* GP_LINE_H */

