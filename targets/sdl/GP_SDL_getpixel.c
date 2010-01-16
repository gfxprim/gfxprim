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

long GP_SDL_GetPixel(SDL_Surface *target, int x, int y)
{
	/* Clip coordinates against the surface boundary */
	int xmin, ymin, xmax, ymax;
	GP_GET_CLIP_RECT(target, xmin, xmax, ymin, ymax);
	if (x < xmin || y < ymin || x > xmax || y > ymax)
		return 0;

	/* Compute the address of the pixel */
	int bytes_per_pixel = GP_BYTES_PER_PIXEL(target);
	uint8_t *p = GP_PIXEL_ADDR(target, x, y);

	switch (bytes_per_pixel) {
	case 1:
		return (long) *p;

	case 2:
		return (long) *((uint16_t *) p);

	case 3:
#ifdef __BIG_ENDIAN__
		return (long) ((p[0] << 16) | (p[1] << 8) | p[2]);
#else
		return (long) (p[0] | (p[1] << 8) | (p[2] << 16));
#endif

	case 4:
		return (long) *((uint32_t *) p);

	default:
		return 0;
	}
}

