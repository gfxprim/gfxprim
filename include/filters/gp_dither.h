// SPDX-License-Identifier: GPL-2.1-or-later
/*
 * Copyright (C) 2009-2013 Cyril Hrubis <metan@ucw.cz>
 */

/*

  Dithering algorithms.

 */

#ifndef FILTERS_GP_DITHER_H
#define FILTERS_GP_DITHER_H

#include <filters/gp_filter.h>

/*
 * Classical Floyd-Steinberg. Produces good results and is a little faster than
 * the Hilbert-Peano dithering.
 *
 * The error is distributed to the neighbor pixels as follows:
 * (X denotes current position)
 *
 *
 *          |       |
 *          |   X   |  7/16
 *          |       |
 *   -----------------------
 *          |       |
 *     3/16 |  5/16 |  1/16
 *          |       |
 *
 */

/*
 * Converts RGB888 24bit image to any RGB or Grayscale bitmap.
 *
 * The source pixel_type MUST BE GP_PIXEL_RGB888.
 *
 * The destination must be at least as large as source.
 *
 * If operation was aborted from within a callback, non-zero is returned.
 */
int gp_filter_floyd_steinberg(const gp_pixmap *src,
                              gp_pixmap *dst,
                              gp_progress_cb *callback);

/*
 * If malloc() has failed, or operation was aborted by a callback, NULL is
 * returned.
 */
gp_pixmap *gp_filter_floyd_steinberg_alloc(const gp_pixmap *src,
                                           gp_pixel_type pixel_type,
                                           gp_progress_cb *callback);

/*
 * Hilbert-Peano space filling curve based dithering.
 *
 * The error value is distributed around the Hilbert curve.
 *
 * This dithering introduces a little more noisy result but doesn't create
 * repeating patterns like Floyd-Steinberg which looks generally better to
 * human eye. On the other hand edges tend to be less sharp.
 */

/*
 * Converts RGB888 24bit image to any RGB or Grayscale bitmap.
 *
 * The source pixel_type MUST BE GP_PIXEL_RGB888.
 *
 * The destination must be at least as large as source.
 *
 * If the operation was aborted from within a callback, non-zero is returned.
 */
int gp_filter_hilbert_peano(const gp_pixmap *src,
                            gp_pixmap *dst,
                            gp_progress_cb *callback);

/*
 * If malloc() has failed, or operation was aborted by a callback, NULL is
 * returned.
 */
gp_pixmap *gp_filter_hilbert_peano_alloc(const gp_pixmap *src,
                                         gp_pixel_type pixel_type,
                                         gp_progress_cb *callback);

#endif /* FILTERS_GP_DITHER_H */
