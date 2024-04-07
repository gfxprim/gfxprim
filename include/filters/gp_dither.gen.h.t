// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2009-2023 Cyril Hrubis <metan@ucw.cz>
 */

/**
 * @file gp_dither.gen.h
 * @brief Dithering algorithms.
 */
#ifndef FILTERS_GP_DITHER_H
#define FILTERS_GP_DITHER_H

#include <filters/gp_filter.h>

/**
 * @brief Dithering types.
 */
typedef enum gp_dither_type {
	/**
	 * @brief Classical Floyd-Steinberg.
	 *
	 * Error distribution:
	 * ```
	 *   |      |   X   |  7/16 |
	 *   |------|-------|-------|
	 *   | 3/16 |  5/16 |  1/16 |
	 * ```
	 */
	GP_DITHER_FLOYD_STEINBERG,
	/**
	 * @brief Modified Atkinson dithering.
	 *
	 * Distributes only 7/8 of the error, result has best contrast, but
	 * large areas may be "washed out" and the image tends to be darker.
	 *
	 * I've found that the original Atkinson that distributes 6/8 of the
	 * error tends to be too dark and that distributing 7/8 works better.
	 *
	 * Error distribution:
	 *```
	 *   |      |   X   |  1/8  |  1/8 |
	 *   |------|-------|-------|------|
	 *   | 1/8  |  1/8  |  1/8  |      |
	 *   |------|-------|-------|------|
	 *   | 1/8  |  1/8  |       |      |
	 *      ^
	 *   This is zero in the original Atkinson.
	 *```
	 */
	GP_DITHER_ATKINSON,
	/**
	 * @brief A sierra dithering.
	 *
	 * Sierra, produces shaper than Floyd-Steinberg, sometimes with less
	 * patterns. Generally preferable for small image sizes.
	 *
	 * Error distribution:
	 *```
	 *   |      |      |  X   | 5/32 | 3/32 |
	 *   |------|------|------|------|------|
	 *   | 2/32 | 4/32 | 5/32 | 4/32 | 2/32 |
	 *   |------|------|------|------|------|
	 *   |      | 2/32 | 3/32 | 2/32 |      |
	 *```
	 */
	GP_DITHER_SIERRA,
	/**
	 * @brief A sierra lite dithering.
	 *
	 * Sierra Lite, comparable to Floyd-Steinberg,
	 * result is sometimes slightly sharper.
	 *
	 * Error distribution:
	 *```
	 *    |     |  X  | 2/4 |
	 *    |-----|-----|-----|
	 *    | 1/4 | 1/4 |     |
	 *```
	 */
	GP_DITHER_SIERRA_LITE,
	/**
	 * @brief A hilbert-peano dithering.
	 *
	 * Distributes error along Hilbert curve, produces no patterns on
	 * photos, but the end result is less sharp. Does not work well when
	 * large areas filed with the same color, since the hilbert curve
	 * end up being visible in the result.
	 */
	GP_DITHER_HILBERT_PEANO,
	/**
	 * @brief Number of dithering types.
	 *
	 * Last valid types is GP_DITHER_MAX-1.
	 */
	GP_DITHER_MAX,
} gp_dither_type;

/**
 * @brief Returns dithering type by name.
 *
 * The lookup is not case sensitive and also works for two letter acronyms of
 * the ditherings. The indended usage for two letter acronyms are shorter
 * command line options, e.g. '-d at'.
 *
 * |  Canonical name  | Acronym |
 * |------------------|---------|
 * |  Floyd Steinberg |   fs    |
 * |  Atkinson        |   at    |
 * |  Sierra          |   si    |
 * |  Sierra Lite     |   sl    |
 * |  Hilbert Peano   |   hp    |
 *
 * @param dither_name A dithering name.
 *
 * @return Dithering type or GP_DITHER_MAX on invalid name.
 */
gp_dither_type gp_dither_type_by_name(const char *dither_name);

/**
 * @brief Returns dithering name for a given dithering type.
 *
 * @param dither_type A dithering type id.
 *
 * @return A canonical name for a dithering.
 */
const char *gp_dither_type_name(gp_dither_type dither_type);

/**
 * @brief Runs dithering from src->pixel_type to dst->pixel_type.
 *
 * @return Zero on success, non-zero on invalid type or aborted filter.
 */
int gp_filter_dither(gp_dither_type type,
                     const gp_pixmap *src, gp_pixmap *dst,
                     gp_progress_cb *callback);

/**
 * @brief Allocates dst and runs dithering from src->pixel_type to pixel_type.
 *
 * @return Newly allocated pixmap on success, NULL on invalid type or aborted filter.
 */
gp_pixmap *gp_filter_dither_alloc(gp_dither_type type,
                                  const gp_pixmap *src,
                                  gp_pixel_type pixel_type,
                                  gp_progress_cb *callback);

@ for name in ['floyd_steinberg', 'atkinson', 'sierra', 'sierra_lite', 'hilbert_peano']:
/**
 * @brief Runs {{ name }} dithering from src::pixel_type to dst::pixel_type.
 *
 * @return Zero on success, non-zero on invalid type or aborted filter.
 */
int gp_filter_{{ name }}(const gp_pixmap *src,
                         gp_pixmap *dst,
                         gp_progress_cb *callback);

/**
 * @brief Allocates dst and runs {{ name }} dithering from src->pixel_type to dst->pixel_type.
 *
 * @return Newly allocated pixmap on success, NULL on invalid type or aborted filter.
 */
static inline gp_pixmap *gp_filter_{{ name }}_alloc(const gp_pixmap *src,
                                                    gp_pixel_type pixel_type,
                                                    gp_progress_cb *callback)
{
	return gp_filter_dither_alloc({{ 'GP_DITHER_' + name.upper() }}, src, pixel_type, callback);
}

@ end
@
#endif /* FILTERS_GP_DITHER_H */
