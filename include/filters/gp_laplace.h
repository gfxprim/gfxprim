// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2009-2012 Cyril Hrubis <metan@ucw.cz>
 */

/*

  Laplace filter and Laplace-based filters.

 */

#ifndef FILTERS_GP_LAPLACE_H
#define FILTERS_GP_LAPLACE_H

#include <filters/gp_filter.h>

/*
 * Discrete Laplace, second-derivative filter.
 *
 * Implemented by separable linear convolution with kernels
 *
 * [1 -2 1] and [ 1 ]
 *              [-2 ]
 *              [ 1 ]
 */
int gp_filter_laplace(const gp_pixmap *src, gp_pixmap *dst,
                      gp_progress_cb *callback);

gp_pixmap *gp_filter_laplace_alloc(const gp_pixmap *src,
                                   gp_progress_cb *callback);

/*
 * Laplace based filter sharpening.
 *
 * This filter substract result of Laplace filter weigted by w from the
 * original image which amplifies edges.
 */
int gp_filter_edge_sharpening(const gp_pixmap *src, gp_pixmap *dst,
                              float w, gp_progress_cb *callback);

gp_pixmap *gp_filter_edge_sharpening_alloc(const gp_pixmap *src, float w,
                                           gp_progress_cb *callback);

#endif /* FILTERS_GP_LAPLACE_H */
