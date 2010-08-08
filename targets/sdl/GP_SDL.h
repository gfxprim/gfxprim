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

#include "GP.h"
#include "GP_SDL_Context.h"

#define GP_SDL_BYTES_PER_PIXEL(target) (target->format->BytesPerPixel)

#define GP_SDL_BYTES_PER_LINE(target) (target->pitch)

#define GP_SDL_PIXELS(target) ((uint8_t *)(target->pixels))

#define GP_SDL_PIXEL_ADDR(target, x, y) ( \
	GP_SDL_PIXELS(target) \
	+ y * GP_SDL_BYTES_PER_LINE(target) \
	+ x * GP_SDL_BYTES_PER_PIXEL(target) \
)

#endif /* GP_SDL_H */
