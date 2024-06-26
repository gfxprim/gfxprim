// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2011      Tomas Gavenciak <gavento@ucw.cz>
 * Copyright (C) 2023-2024 Cyril Hrubis <metan@ucw.cz>
 */

/**
 * @file gp_convert.h
 * @brief A pixel value conversions.
 */

#ifndef CORE_GP_CONVERT_H
#define CORE_GP_CONVERT_H

#include <core/gp_pixmap.h>
#include <core/gp_pixel.h>
#include <core/gp_convert.gen.h>
#include <core/gp_convert_scale.gen.h>

/**
 * @brief Converts a color specified by its R, G, B components to a specified type.
 *
 * @param r A red channel value.
 * @param g A green channel value.
 * @param b A blue channel value.
 * @param type A pixel type for the resulting pixel value.
 *
 * @return A pixel value.
 */
static inline gp_pixel gp_rgb_to_pixel(uint8_t r, uint8_t g, uint8_t b,
                                       gp_pixel_type type)
{
	gp_pixel p = GP_PIXEL_CREATE_RGB888(r, g, b);
	return gp_RGB888_to_pixel(p, type);
}

/**
 * @brief Converts a color specified by its R, G, B, A components to a specified type.
 *
 * @param r A red channel value.
 * @param g A green channel value.
 * @param b A blue channel value.
 * @param a An alpha channel value.
 * @param type A pixel type for the resulting pixel value.
 *
 * @return A pixel value.
 */
static inline gp_pixel gp_rgba_to_pixel(uint8_t r, uint8_t g, uint8_t b,
                                        uint8_t a, gp_pixel_type type)
{
	gp_pixel p = GP_PIXEL_CREATE_RGBA8888(r, g, b, a);
	return gp_RGBA8888_to_pixel(p, type);
}

/*
 * Converts a color specified by its R, G, B components to a pixel value
 * compatible with the specified pixmap.
 */
static inline gp_pixel gp_rgb_to_pixmap_pixel(uint8_t r, uint8_t g, uint8_t b,
				              const gp_pixmap *pixmap)
{
	return gp_rgb_to_pixel(r, g, b, pixmap->pixel_type);
}

/*
 * Converts a color specified by its R, G, B, A components to a pixel value
 * compatible with the specified pixmap.
 */
static inline gp_pixel gp_rgba_to_pixmap_pixel(uint8_t r, uint8_t g,
                                               uint8_t b, uint8_t a,
					       const gp_pixmap *pixmap)
{
	return gp_rgba_to_pixel(r, g, b, a, pixmap->pixel_type);
}

/*
 * Convert between any pixel types (excl. palette types) via RGBA8888
 */
static inline gp_pixel gp_convert_pixel(gp_pixel pixel, gp_pixel_type from,
                                        gp_pixel_type to)
{
	return gp_RGBA8888_to_pixel(gp_pixel_to_RGBA8888(pixel, from), to);
}

/*
 * Convert between pixel types of given pixmaps (excl. palette types) via
 * RGBA8888.
 */
static inline gp_pixel gp_convert_pixmap_pixel(gp_pixel pixel,
                                               const gp_pixmap *from,
					       const gp_pixmap *to)
{
	return gp_RGBA8888_to_pixel(gp_pixel_to_RGBA8888(pixel, from->pixel_type),
	                            to->pixel_type);
}

#endif /* CORE_GP_CONVERT_H */
