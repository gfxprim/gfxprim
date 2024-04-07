// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2011      Tomas Gavenciak <gavento@ucw.cz>
 * Copyright (C) 2011-2024 Cyril Hrubis <metan@ucw.cz>
 */

/**
 * @file gp_get_put_pixel.h
 * @brief Pixel manipulations.
 */
#ifndef CORE_GP_GETPUTPIXEL_H
#define CORE_GP_GETPUTPIXEL_H

#include <core/gp_types.h>
#include <core/gp_pixel.h>
#include <core/gp_pixel_pack.gen.h>
#include <core/gp_get_put_pixel.gen.h>

/**
 * @brief Gets a pixel value.
 * @ingroup gfx
 *
 * This is a safe and slow function that does coordinate transformations (see
 * #GP_TRANSFORM_POINT) and clipping, i.e. reads outside of the
 * gp_pixmap::pixels return 0.
 *
 * @param pixmap A pixmap to get the pixel from.
 * @param x A x coordinate to get the pixel from.
 * @param y A y coordinate to get the pixel from.
 *
 * @returns A pixel value at give coordinates or 0 for pixels outside bitmap.
 */
gp_pixel gp_getpixel(const gp_pixmap *pixmap, gp_coord x, gp_coord y);

/**
 * @brief Gets a pixel value.
 * @ingroup gfx
 *
 * Version of gp_getpixel() without transformations nor border checking. This
 * is faster than the non-raw variant but stil somewhat slow since the function
 * has to handle different types of #gp_pixel_pack. If you are going for speed
 * use the pixel pack specific function e.g. gp_getpixel_raw_32BPP().
 *
 * @warning If x or y are outside of gp_pixmap::pixels the function will read
 *          ouside of the buffer.
 *
 * @param pixmap A pixmap to put the pixel into.
 * @param x A x coordinate to put the pixel at.
 * @param y A y coordinate to put the pixel at.
 * @return A pixel value.
 */
static inline gp_pixel gp_getpixel_raw(const gp_pixmap *pixmap,
                                       gp_coord x, gp_coord y)
{
	GP_FN_RET_PER_PACK_PIXMAP(gp_getpixel_raw, pixmap, pixmap, x, y);

	GP_ABORT("Invalid pixmap pixel type");
}

/*
 * Version of GetPixel without transformations and with border checking.
 */
static inline gp_pixel gp_getpixel_raw_clipped(const gp_pixmap *pixmap,
                                               gp_coord x, gp_coord y)
{
	if (GP_PIXEL_IS_CLIPPED(pixmap, x, y))
		return 0;

	return gp_getpixel_raw(pixmap, x, y);
}

/**
 * @brief Puts a pixel value.
 * @ingroup gfx
 *
 * This is a safe and slow function that does coordinate transformations (see
 * #GP_TRANSFORM_POINT) and clipping i.e. writes outside of the
 * gp_pixmap::pixels are ignored.
 *
 * If you are going to loop over all pixels in a pixmap you should use
 * gp_putpixel_raw() variant that is not clipped nor transformed.
 *
 * @param pixmap A pixmap to put the pixel into.
 * @param x A x coordinate to put the pixel at.
 * @param y A y coordinate to put the pixel at.
 * @param p A pixel value.
 */
void gp_putpixel(gp_pixmap *pixmap, gp_coord x, gp_coord y, gp_pixel p);

/**
 * @brief Puts a pixel value.
 * @ingroup gfx
 *
 * Version of gp_putpixel() without transformations nor border checking. This
 * is faster than the non-raw variant but stil somewhat slow since the function
 * has to handle all different types of #gp_pixel_pack. If you are going for
 * speed use the pixel pack specific function e.g. gp_putpixel_raw_32BPP().
 *
 * @warning If x or y are outside of gp_pixmap::pixels the function will write
 *          ouside of the buffer.
 *
 * @param pixmap A pixmap to put the pixel into.
 * @param x A x coordinate to put the pixel at.
 * @param y A y coordinate to put the pixel at.
 * @param p A pixel value.
 */
static inline void gp_putpixel_raw(gp_pixmap *pixmap,
                                   gp_coord x, gp_coord y, gp_pixel p)
{
	GP_FN_PER_PACK_PIXMAP(gp_putpixel_raw, pixmap, pixmap, x, y, p);
}

/*
 * Version of PutPixel without transformation and with border checking.
 */
static inline void gp_putpixel_raw_clipped(gp_pixmap *pixmap,
                                           gp_coord x, gp_coord y, gp_pixel p)
{
	GP_FN_PER_PACK_PIXMAP(gp_putpixel_raw_clipped, pixmap, pixmap, x, y, p);
}

/**
 * @brief Returns a pixel offset in a byte (in pixels).
 *
 * This function is internally used in gp_sub_pixmap() to compute starting
 * offset for a subpixmap to a first pixel in a row. Returns zero for pixel
 * types aligned to bytes.
 *
 * @param pixmap A pixmap.
 * @param x A x coordinate in a pixmap.
 *
 * @return An offset to the first pixel in byte.
 */
uint8_t gp_pixel_addr_offset(const gp_pixmap *pixmap, gp_coord x);

#endif /* CORE_GP_GETPUTPIXEL_H */
