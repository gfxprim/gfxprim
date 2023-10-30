// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2009-2023 Cyril Hrubis <metan@ucw.cz>
 */

/*

  Dithering algorithms.

 */

#ifndef FILTERS_GP_DITHER_H
#define FILTERS_GP_DITHER_H

#include <filters/gp_filter.h>

typedef enum gp_dither_type {
	/*
	 * Classical Floyd-Steinberg.
	 *
	 *          |   X   |  7/16
	 *   -----------------------
	 *     3/16 |  5/16 |  1/16
	 */
	GP_DITHER_FLOYD_STEINBERG,
	/*
	 * Modified Atkinson dithering.
	 *
	 * Distributes only 7/8 of the error, result has best contrast, but
	 * large areas may be "washed out" and the image tends to be darker.
	 *
	 * I've found that the original Atkinson that distributes 6/8 of the
	 * error tends to be too dark and that distributing 7/8 works better.
	 *
	 *          |   X   |  1/8  |  1/8
	 *   -------------------------------
	 *     1/8  |  1/8  |  1/8  |
	 *   -------------------------------
	 *     1/8  |  1/8  |       |
	 *      ^
	 *   This is zero in the original Atkinson.
	 */
	GP_DITHER_ATKINSON,
	/*
	 *
	 * Sierra, produces shaper than Floyd-Steinberg, sometimes with less
	 * patterns. Generally preferable for small image sizes.
	 *
	 *          |       |   X   |  5/32 |  3/32
	 *   ---------------------------------------
	 *    2/32  |  4/32 |  5/32 |  4/32 |  2/32
	 *   ---------------------------------------
	 *          |  2/32 |  3/32 |  2/32 |
	 */
	GP_DITHER_SIERRA,
	/*
	 * Sierra Lite, comparable to Floyd-Steinberg,
	 * result is sometimes slightly sharper.
	 *
	 *          |   X   |  2/4
	 *   -----------------------
	 *     1/4  |  1/4  |
	 */
	GP_DITHER_SIERRA_LITE,
	/*
	 * Distributes error along Hilbert curve, produces no patterns on
	 * photos, but the end result is less sharp. Does not work well when
	 * large areas filed with the same color, since the hilbert curve
	 * end up being visible in the result.
	 */
	GP_DITHER_HILBERT_PEANO,
	/*
	 * Number of dithering types.
	 */
	GP_DITHER_MAX,
} gp_dither_type;

/**
 * @brief Returns dithering type by name.
 *
 * @return Dithering type or GP_DITHER_MAX on invalid name.
 */
gp_dither_type gp_dither_type_by_name(const char *dither_name);

/**
 * @brier Returns dithering name for a given dithering type.
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
 * @brief Allocated dst and runs dithering from src->pixel_type to pixel_type.
 *
 * @return Newly allocated pixmap on success, NULL on invalid type or aborted filter.
 */
gp_pixmap *gp_filter_dither_alloc(gp_dither_type type,
                                  const gp_pixmap *src,
                                  gp_pixel_type pixel_type,
                                  gp_progress_cb *callback);

@ for name in ['floyd_steinberg', 'atkinson', 'sierra', 'sierra_lite', 'hilbert_peano']:
int gp_filter_{{ name }}(const gp_pixmap *src,
                         gp_pixmap *dst,
                         gp_progress_cb *callback);

static inline gp_pixmap *gp_filter_{{ name }}_alloc(const gp_pixmap *src,
                                                    gp_pixel_type pixel_type,
                                                    gp_progress_cb *callback)
{
	return gp_filter_dither_alloc({{ 'GP_DITHER_' + name.upper() }}, src, pixel_type, callback);
}

@ end
@
#endif /* FILTERS_GP_DITHER_H */
