// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2009-2021 Cyril Hrubis <metan@ucw.cz>
 */

/**
 * @file gp_sdl_pixmap.h
 * @brief A SDL surface to GFXprim pixmap conversion.
 */
#ifndef BACKENDS_GP_SDL_PIXMAP_H
#define BACKENDS_GP_SDL_PIXMAP_H

#include <core/gp_pixel.h>
#include <core/gp_pixmap.h>

/**
 * @brief A SDL surface to GFXprim pixmap conversion.
 *
 * This function lets you use GFXprim together with SDL. All you need to do
 * is to initialize pixmap from surface. The usage is as follows:
 *
 * @code
 * gp_pixmap c;
 *
 * if (gp_pixmap_from_sdl_surface(&c, surface)) {
 *	error("Failed to match PIXEL_TYPE for given surface");
 *	exit(1);
 * }
 *
 * // The pixmap has gamma set to 1.0 which is unlikely correct
 * // Use gp_pixmap_correction_set() to assign correct correction.
 * @endcode
 *
 * Now you have initialized pixmap that shares the pixel buffer with
 * the SDL surface.
 *
 * TODO: Make use of the SDL_GetGammaRamp()?
 *
 * @param pixmap A pointer to a pixmap to be initialized.
 * @param surf A SDL surface to be mapped into pixmap.
 * @return A zero on success, non-zero if pixel type couldn't be mapped.
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
	pixmap->gamma = NULL;

	return 0;
}

#endif /* BACKENDS_GP_SDL_PIXMAP_H */
