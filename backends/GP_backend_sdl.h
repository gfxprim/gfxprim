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

#ifndef GP_BACKEND_SDL_H
#define GP_BACKEND_SDL_H

#include <SDL/SDL.h>

/*
 * These definitions are used for interfacing with SDL.
 */

/*
 * The target object for drawing routines, which is SDL_Surface.
 */
#define GP_TARGET_TYPE	SDL_Surface

/*
 * Loads the clipping rectangle of the given surface into variables
 * whose names are passed in 'xmin', 'xmax', 'ymin', 'ymax'.
 */
#define GP_GET_CLIP_RECT(target, xmin, xmax, ymin, ymax) { \
	xmin = target->clip_rect.x; \
	xmax = target->clip_rect.x + target->clip_rect.w - 1; \
	ymin = target->clip_rect.y; \
	ymax = target->clip_rect.y + target->clip_rect.h - 1; \
}

/*
 * Determines the number of bytes per pixel of the target.
 */
#define GP_BYTES_PER_PIXEL(target) (target->format->BytesPerPixel)

/*
 * Determines the number of bytes per line of the target.
 */
#define GP_BYTES_PER_LINE(target) (target->pitch)

/*
 * Returns the pointer to the pixel data of the target.
 */
#define GP_PIXELS(target) ((uint8_t *)(target->pixels))

/*
 * Computes the address of a pixel at coordinates (x, y)
 * in the specified surface (the coordinates must lie within
 * the surface).
 * The result is a pointer of type uint8_t *.
 */
#define GP_PIXEL_ADDR(target, x, y) ( \
	GP_PIXELS(target) + y * GP_BYTES_PER_LINE(target) \
		+ x * GP_BYTES_PER_PIXEL(target))

#endif /* GP_BACKEND_SDL_H */

