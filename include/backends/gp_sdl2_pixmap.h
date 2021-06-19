// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2009-2012 Cyril Hrubis <metan@ucw.cz>
 */

#ifndef BACKENDS_GP_SDL_PIXMAP_H
#define BACKENDS_GP_SDL_PIXMAP_H

#include <SDL2/SDL.h>
#include <core/gp_pixmap.h>

/*
 * This function lets you use GFXprim together with SDL 2. All you need to do
 * is to initialize pixmap from surface. The usage is as follows:
 *
 * ...
 *
 * gp_pixmap c;
 *
 * if (gp_pixmap_from_sdl2_surface(&c, surface)) {
 *	error("Failed to match PIXEL_TYPE for given surface");
 *	exit(1);
 * }
 *
 * ...
 *
 * Now you have initialized pixmap that shares the pixel buffer with
 * the SDL 2 surface.
 */
int gp_pixmap_from_sdl2_surface(gp_pixmap *c, const SDL_Surface *surf);

#endif /* BACKENDS_GP_SDL_PIXMAP_H */
