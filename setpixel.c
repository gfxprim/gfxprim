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

#include "GP_pixel.h"

/*
 * Sets a pixel at coordinates (x, y) to the given color.
 * If the coordinates lie outside the surface boundaries or outside its
 * clipping rectangle, the call has no effect.
 */
void GP_SetPixel(SDL_Surface *surf, long color, int x, int y)
{
	int bytes_per_pixel = surf->format->BytesPerPixel;

	if (surf == NULL || surf->pixels == NULL)
		return;

	/* Clip coordinates against the clip rectangle of the surface */
	int x1 = surf->clip_rect.x;
	int y1 = surf->clip_rect.y;
	int x2 = surf->clip_rect.x + surf->clip_rect.w;
	int y2 = surf->clip_rect.y + surf->clip_rect.h;

	if (x < x1 || y < y1 || x >= x2 || y >= y2)
		return;

	/* Compute the address of the pixel */
	uint8_t *p = GP_PIXEL_ADDR(surf, x, y);

	switch (bytes_per_pixel) {
	case 1:
		GP_WRITE_PIXEL_1BYTE(p, color);
		break;
	
	case 2:
		GP_WRITE_PIXEL_2BYTES(p, color);
		break;

	case 3:
		GP_WRITE_PIXEL_3BYTES(p, color);
		break;

	case 4:
		GP_WRITE_PIXEL_4BYTES(p, color);
		break;
	}
}

/*
 * Here we produce specializations of GP_SetPixel() for surfaces
 * of known bit depth:
 *
 * GP_SetPixel_8bpp(), GP_SetPixel_16bpp(), GP_SetPixel_24bpp(),
 * GP_SetPixel_32bpp().
 */

#define FN_NAME		GP_SetPixel_8bpp
#define WRITE_PIXEL	GP_WRITE_PIXEL_1BYTE
#include "setpixel.tmpl.c"
#undef FN_NAME
#undef WRITE_PIXEL

#define FN_NAME		GP_SetPixel_16bpp
#define WRITE_PIXEL	GP_WRITE_PIXEL_2BYTES
#include "setpixel.tmpl.c"
#undef FN_NAME
#undef WRITE_PIXEL

#define FN_NAME		GP_SetPixel_24bpp
#define WRITE_PIXEL	GP_WRITE_PIXEL_3BYTES
#include "setpixel.tmpl.c"
#undef FN_NAME
#undef WRITE_PIXEL

#define FN_NAME		GP_SetPixel_32bpp
#define WRITE_PIXEL	GP_WRITE_PIXEL_4BYTES
#include "setpixel.tmpl.c"
#undef FN_NAME
#undef WRITE_PIXEL

