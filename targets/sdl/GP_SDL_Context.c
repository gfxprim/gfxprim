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

#include "GP.h"
#include "GP_SDL.h"

/* Checks whether pixel color component masks in the given surface are equal
 * to specified. Returns nonzero if they match, zero otherwise.
 */
static int check_pixel_masks(SDL_Surface *surf, unsigned int rmask,
	unsigned int gmask, unsigned int bmask, unsigned int amask)
{
	return (surf->format->Rmask == rmask
		&& surf->format->Gmask == gmask
		&& surf->format->Bmask == bmask
		&& surf->format->Ashift == amask);
}

/* Detects the pixel type of the SDL surface.
 * Returns the pixel type, or GP_PIXEL_UNKNOWN if the type was not recognized.
 */
static enum GP_PixelType find_surface_pixel_type(SDL_Surface *surf)
{
	switch (surf->format->BytesPerPixel) {
		case 1:
			if (check_pixel_masks(surf, 0, 0, 0, 0)) {
				return GP_PIXEL_PAL8;
			}
			break;
		case 2:
			if (check_pixel_masks(surf, 0x7c00, 0x03e0, 0x001f, 0)) {
				return GP_PIXEL_RGB555;
			}
			if (check_pixel_masks(surf, 0xf800, 0x07e0, 0x001f, 0)) {
				return GP_PIXEL_RGB565;
			}
			break;
		case 3:
			if (check_pixel_masks(surf, 0xff0000, 0xff00, 0xff, 0)) {
				return GP_PIXEL_RGB888;
			}
			if (check_pixel_masks(surf, 0xff, 0xff00, 0xff0000, 0)) {
				return GP_PIXEL_BGR888;
			}
			break;
		case 4:
			if (check_pixel_masks(surf, 0xff0000, 0xff00, 0xff, 0)) {
				return GP_PIXEL_XRGB8888;
			}
			if (check_pixel_masks(surf, 0xff, 0xff00, 0xff0000, 0)) {
				return GP_PIXEL_XBGR8888;
			}
			if (check_pixel_masks(surf, 0xff000000, 0xff0000, 0xff00, 0)) {
				return GP_PIXEL_RGBX8888;
			}
			if (check_pixel_masks(surf, 0xff00, 0xff0000, 0xff000000, 0)) {
				return GP_PIXEL_BGRX8888;
			}
			if (check_pixel_masks(surf, 0xff0000, 0xff00, 0xff, 0xff000000)) {
				return GP_PIXEL_ARGB8888;
			}
			if (check_pixel_masks(surf, 0xff, 0xff00, 0xff0000, 0xff000000)) {
				return GP_PIXEL_ABGR8888;
			}
			if (check_pixel_masks(surf, 0xff000000, 0xff0000, 0xff00, 0xff)) {
				return GP_PIXEL_RGBA8888;
			}
			if (check_pixel_masks(surf, 0xff00, 0xff0000, 0xff000000, 0xff)) {
				return GP_PIXEL_BGRA8888;
			}
			break;

	}
	return GP_PIXEL_UNKNOWN;

}

inline GP_RetCode GP_SDL_ContextFromSurface(
		GP_Context *context, SDL_Surface *surf)
{
	if (surf == NULL || context == NULL) {
		return GP_ENULLPTR;
	}

	/* basic structure and size */
	context->pixels = surf->pixels;
	context->bits_per_pixel = 8 * surf->format->BytesPerPixel;
	context->pixel_type = find_surface_pixel_type(surf);
	context->bytes_per_row = surf->pitch;
	context->cols = surf->w;
	context->rows = surf->h;

	/* orientation */
	context->axes_swap = 0;
	context->x_swap = 0;
	context->y_swap = 0;

	/* clipping */
	context->clip_row_min = surf->clip_rect.y;
	context->clip_row_max = surf->clip_rect.y + surf->clip_rect.h - 1;
	context->clip_col_min = surf->clip_rect.x;
	context->clip_col_max = surf->clip_rect.x + surf->clip_rect.w - 1;

	if (context->pixel_type == GP_PIXEL_UNKNOWN) {
		return GP_ENOIMPL;
	}

	return GP_ESUCCESS;
}
