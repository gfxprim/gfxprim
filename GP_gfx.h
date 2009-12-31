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

#ifndef GP_GFX_H
#define GP_GFX_H

#include <SDL/SDL.h>

void GP_Clear(SDL_Surface *surf, long color);

void GP_Circle(SDL_Surface *surf, long color, int xcenter, int ycenter, int r);
void GP_FillCircle(SDL_Surface *surf, long color, int xcenter, int ycenter, int r);

void GP_Ellipse(SDL_Surface *surf, long color, int xcenter, int ycenter, int a, int b);
void GP_FillEllipse(SDL_Surface *surf, long color, int xcenter, int ycenter, int a, int b);

void GP_Rect(SDL_Surface *surf, long color, int x0, int y0, int x1, int y1);
void GP_FillRect(SDL_Surface *surf, long color, int x0, int y0, int x1, int y1);

void GP_Triangle(SDL_Surface *surf, long color, int x0, int y0, int x1, int y1, int x2, int y2);
void GP_FillTriangle(SDL_Surface *surf, long color, int x0, int y0, int x1, int y1, int x2, int y2);

/* Specializations for known bit depth */
void GP_Triangle_8bpp(SDL_Surface *surf, long color, int x0, int y0, int x1, int y1, int x2, int y2);
void GP_Triangle_16bpp(SDL_Surface *surf, long color, int x0, int y0, int x1, int y1, int x2, int y2);
void GP_Triangle_24bpp(SDL_Surface *surf, long color, int x0, int y0, int x1, int y1, int x2, int y2);
void GP_Triangle_32bpp(SDL_Surface *surf, long color, int x0, int y0, int x1, int y1, int x2, int y2);
void GP_FillTriangle_8bpp(SDL_Surface *surf, long color, int x0, int y0, int x1, int y1, int x2, int y2);
void GP_FillTriangle_16bpp(SDL_Surface *surf, long color, int x0, int y0, int x1, int y1, int x2, int y2);
void GP_FillTriangle_24bpp(SDL_Surface *surf, long color, int x0, int y0, int x1, int y1, int x2, int y2);
void GP_FillTriangle_32bpp(SDL_Surface *surf, long color, int x0, int y0, int x1, int y1, int x2, int y2);

#endif /* GP_GFX_H */

