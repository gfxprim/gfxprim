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

inline GP_Context *GP_SDL_ContextFromSurface(
		GP_Context *context, SDL_Surface *surf)
{
	GP_CHECK(surf != NULL);

	if (context == NULL) {
		context = (GP_Context *) malloc(sizeof(*context));
	}

	context->pixels = surf->pixels;
	context->bits_per_pixel = 8 * surf->format->BytesPerPixel;
	context->bytes_per_row = surf->pitch;
	context->columns = surf->w;
	context->rows = surf->h;
	context->rows_are_vertical = 0;
	context->clip_row_min = surf->clip_rect.y;
	context->clip_row_max = surf->clip_rect.y + surf->clip_rect.h - 1;
	context->clip_column_min = surf->clip_rect.x;
	context->clip_column_max = surf->clip_rect.x + surf->clip_rect.w - 1;

	return context;
}
