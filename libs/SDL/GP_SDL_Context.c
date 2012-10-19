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
 * Copyright (C) 2009-2012 Cyril Hrubis <metan@ucw.cz>                       *
 *                                                                           *
 *****************************************************************************/

#include "GP_Core.h"

#include "../../config.h"

#ifdef HAVE_LIBSDL

#include "GP_SDL.h"

int GP_SDL_ContextFromSurface(GP_Context *context, SDL_Surface *surf)
{
	if (surf == NULL || surf->pixels == NULL || context == NULL)
		return 1;

	/* sanity checks on the SDL surface */
	if (surf->format->BytesPerPixel == 0 || surf->format->BytesPerPixel > 4)
		return 1;
	
	enum GP_PixelType pixeltype = GP_PixelRGBMatch(surf->format->Rmask,
	                                               surf->format->Gmask,
						       surf->format->Bmask,
						       surf->format->Ashift,
						       surf->format->BitsPerPixel);

	if (pixeltype == GP_PIXEL_UNKNOWN)
		return 1;

	/* basic structure and size */
	context->pixels = surf->pixels;
	context->bpp = 8 * surf->format->BytesPerPixel;
	context->pixel_type = pixeltype;
	context->bytes_per_row = surf->pitch;
	context->w = surf->w;
	context->h = surf->h;

	/* orientation */
	context->axes_swap = 0;
	context->x_swap = 0;
	context->y_swap = 0;

	return 0;
}

#endif /* HAVE_LIBSDL */
