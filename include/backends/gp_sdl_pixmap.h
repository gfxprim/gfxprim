// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2009-2021 Cyril Hrubis <metan@ucw.cz>
 */

#ifndef BACKENDS_GP_SDL_PIXMAP_H
#define BACKENDS_GP_SDL_PIXMAP_H

#include <core/gp_pixel.h>
#include <core/gp_pixmap.h>

/*
 * This function lets you use GFXprim together with SDL. All you need to do
 * is to initialize pixmap from surface. The usage is as follows:
 *
 * ...
 *
 * gp_pixmap c;
 *
 * if (gp_pixmap_from_sdl_surface(&c, surface)) {
 *	error("Failed to match PIXEL_TYPE for given surface");
 *	exit(1);
 * }
 *
 * ...
 *
 * Now you have initialized pixmap that shares the pixel buffer with
 * the SDL surface.
 */
static inline int gp_pixmap_from_sdl_surface(gp_pixmap *pixmap,
                                             const SDL_Surface *surf)
{
	/* sanity checks on the SDL surface */
	if (surf->format->BytesPerPixel == 0) {
		GP_WARN("Surface->BytesPerPixel == 0");
		return 1;
	}

	if (surf->format->BytesPerPixel > 4) {
		GP_WARN("Surface->BytesPerPixel > 4");
		return 1;
	}

	enum gp_pixel_type pixeltype = gp_pixel_rgb_match(surf->format->Rmask,
							  surf->format->Gmask,
							  surf->format->Bmask,
							  surf->format->Ashift,
							  surf->format->BitsPerPixel);

	if (pixeltype == GP_PIXEL_UNKNOWN)
		return 1;

	/* basic structure and size */
	pixmap->pixels = surf->pixels;
	pixmap->pixel_type = pixeltype;
	pixmap->bytes_per_row = surf->pitch;
	pixmap->w = surf->w;
	pixmap->h = surf->h;

	return 0;
}

#endif /* BACKENDS_GP_SDL_PIXMAP_H */
