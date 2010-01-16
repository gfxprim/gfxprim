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

#ifndef GP_SDL_H
#define GP_SDL_H

#include <SDL/SDL.h>

void GP_SDL_SetPixel(SDL_Surface *target, long color, int x, int y);
void GP_SDL_SetPixel_8bpp(SDL_Surface *target, long color, int x, int y);
void GP_SDL_SetPixel_16bpp(SDL_Surface *target, long color, int x, int y);
void GP_SDL_SetPixel_24bpp(SDL_Surface *target, long color, int x, int y);
void GP_SDL_SetPixel_32bpp(SDL_Surface *target, long color, int x, int y);

long GP_SDL_GetPixel(SDL_Surface *target, int x, int y);

#define GP_SDL_PutPixel GP_SDL_SetPixel

void GP_SDL_Line(SDL_Surface *target, long color, int x0, int y0, int x1, int y1);
void GP_SDL_Line_8bpp(SDL_Surface *target, long color, int x0, int y0, int x1, int y1);
void GP_SDL_Line_16bpp(SDL_Surface *target, long color, int x0, int y0, int x1, int y1);
void GP_SDL_Line_24bpp(SDL_Surface *target, long color, int x0, int y0, int x1, int y1);
void GP_SDL_Line_32bpp(SDL_Surface *target, long color, int x0, int y0, int x1, int y1);

void GP_SDL_HLine(SDL_Surface *target, long color, int x0, int x1, int y);
void GP_SDL_HLine_8bpp(SDL_Surface *target, long color, int x0, int x1, int y);
void GP_SDL_HLine_16bpp(SDL_Surface *target, long color, int x0, int x1, int y);
void GP_SDL_HLine_24bpp(SDL_Surface *target, long color, int x0, int x1, int y);
void GP_SDL_HLine_32bpp(SDL_Surface *target, long color, int x0, int x1, int y);

void GP_SDL_VLine(SDL_Surface *target, long color, int x, int y0, int y1);
void GP_SDL_VLine_8bpp(SDL_Surface *target, long color, int x, int y0, int y1);
void GP_SDL_VLine_16bpp(SDL_Surface *target, long color, int x, int y0, int y1);
void GP_SDL_VLine_24bpp(SDL_Surface *target, long color, int x, int y0, int y1);
void GP_SDL_VLine_32bpp(SDL_Surface *target, long color, int x, int y0, int y1);

void GP_SDL_Circle(SDL_Surface *target, long color, int x, int y, int r);
void GP_SDL_FillCircle(SDL_Surface *target, long color, int x, int y, int r);
void GP_SDL_Ellipse(SDL_Surface *target, long color, int x, int y, int a, int b);
void GP_SDL_FillEllipse(SDL_Surface *target, long color, int x, int y, int a, int b);
void GP_SDL_Rect(SDL_Surface *target, long color, int x0, int y0, int x1, int y1);
void GP_SDL_FillRect(SDL_Surface *target, long color, int x0, int y0, int x1, int y1);
void GP_SDL_Triangle(SDL_Surface *target, long color, int x0, int y0, int x1, int y1, int x2, int y2);
void GP_SDL_FillTriangle(SDL_Surface *target, long color, int x0, int y0, int x1, int y1, int x2, int y2);

#endif

