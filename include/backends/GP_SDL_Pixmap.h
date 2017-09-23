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
 * Copyright (C) 2009-2012 Cyril Hrubis <metan@ucw.cz>                       *
 *                                                                           *
 *****************************************************************************/

#ifndef BACKENDS_GP_SDL_PIXMAP_H
#define BACKENDS_GP_SDL_PIXMAP_H

#include <SDL/SDL.h>
#include <core/GP_Pixmap.h>

/*
 * This function lets you use GFXprim together with SDL. All you need to do
 * is to initialize pixmap from surface. The usage is as follows:
 *
 * ...
 *
 * GP_Pixmap c;
 *
 * if (GP_PixmapFromSDLSurface(&c, surface)) {
 *	error("Failed to match PIXEL_TYPE for given surface");
 *	exit(1);
 * }
 *
 * ...
 *
 * Now you have initialized pixmap that shares the pixel buffer with
 * the SDL surface.
 */
int GP_PixmapFromSDLSurface(GP_Pixmap *c, const SDL_Surface *surf);

#endif /* BACKENDS_GP_SDL_PIXMAP_H */
