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

#define GP_SDL_BYTES_PER_PIXEL(target) (target->format->BytesPerPixel)

#define GP_SDL_BYTES_PER_LINE(target) (target->pitch)

#define GP_SDL_PIXELS(target) ((uint8_t *)(target->pixels))

#define GP_SDL_PIXEL_ADDR(target, x, y) ( \
	GP_SDL_PIXELS(target) \
	+ y * GP_SDL_BYTES_PER_LINE(target) \
	+ x * GP_SDL_BYTES_PER_PIXEL(target) \
)

/* Most drawing functions have this form. */
#define GP_DEF_DRAWING_FN(FN, ...) \
void FN(SDL_Surface *target, long color, __VA_ARGS__); \

/* The basic drawing functions: pixel, lines, simple shapes. */
GP_DEF_DRAWING_FN(GP_SDL_SetPixel, int x, int y);
GP_DEF_DRAWING_FN(GP_SDL_Line, int x0, int y0, int x1, int y1);
GP_DEF_DRAWING_FN(GP_SDL_HLine, int x0, int x1, int y);
GP_DEF_DRAWING_FN(GP_SDL_VLine, int x, int y0, int y1);
GP_DEF_DRAWING_FN(GP_SDL_Circle, int x, int y, int r);
GP_DEF_DRAWING_FN(GP_SDL_FillCircle, int x, int y, int r);
GP_DEF_DRAWING_FN(GP_SDL_Ellipse, int x, int y, int a, int b);
GP_DEF_DRAWING_FN(GP_SDL_FillEllipse, int x, int y, int a, int b);
GP_DEF_DRAWING_FN(GP_SDL_Rect, int x0, int y0, int x1, int y1);
GP_DEF_DRAWING_FN(GP_SDL_FillRect, int x0, int y0, int x1, int y1);
GP_DEF_DRAWING_FN(GP_SDL_Triangle, int x0, int y0, int x1, int y1, int x2, int y2);
GP_DEF_DRAWING_FN(GP_SDL_FillTriangle, int x0, int y0, int x1, int y1, int x2, int y2);

/* A frequently used alias. */
#define GP_SDL_PutPixel GP_SDL_SetPixel

long GP_SDL_GetPixel(SDL_Surface *target, int x, int y);

#include "GP_text.h"

GP_DEF_DRAWING_FN(GP_SDL_Text, int x, int y, GP_TextStyle *style, const char *str);

#endif
