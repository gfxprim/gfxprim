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
