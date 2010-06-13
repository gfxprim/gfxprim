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

inline struct GP_ClipInfo *GP_SDL_ClipInfoFromSurface(SDL_Surface *surf,
		struct GP_ClipInfo *clip)
{
	clip->min_row = surf->clip_rect.y;
	clip->max_row = surf->clip_rect.y + surf->clip_rect.h - 1;
	clip->min_column = surf->clip_rect.x;
	clip->max_column = surf->clip_rect.x + surf->clip_rect.w - 1;

	if (clip->max_row < clip->min_row) {
		clip->max_row = clip->min_row;
	}
	if (clip->max_column < clip->min_column) {
		clip->max_column = clip->min_column;
	}

	return clip;
}

