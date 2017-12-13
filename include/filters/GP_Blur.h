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

   Gaussian blur implementation.

 */

#ifndef FILTERS_GP_BLUR_H
#define FILTERS_GP_BLUR_H

#include <filters/GP_Filter.h>

/*
 * Gaussian blur implemented using linear separable convolution.
 *
 * The x_sigma defines the blur size in horizontal direction and y_sigma
 * defines blur on vertical direction.
 */

int gp_filter_gaussian_blur_ex(const gp_pixmap *src,
                               gp_coord x_src, gp_coord y_src,
                               gp_size w_src, gp_size h_src,
                               gp_pixmap *dst,
                               gp_coord x_dst, gp_coord y_dst,
                               float x_sigma, float y_sigma,
                               gp_progress_cb *callback);

gp_pixmap *gp_filter_gaussian_blur_ex_alloc(const gp_pixmap *src,
                                            gp_coord x_src, gp_coord y_src,
                                            gp_size w_src, gp_size h_src,
                                            float x_sigma, float y_sigma,
                                            gp_progress_cb *callback);

static inline int gp_filter_gaussian_blur(const gp_pixmap *src, gp_pixmap *dst,
                                          float x_sigma, float y_sigma,
                                          gp_progress_cb *callback)
{
	return gp_filter_gaussian_blur_ex(src, 0, 0, src->w, src->h, dst, 0, 0,
	                                  x_sigma, y_sigma, callback);
}

static inline gp_pixmap *gp_filter_gaussian_blur_alloc(const gp_pixmap *src,
                                                       float x_sigma, float y_sigma,
                                                       gp_progress_cb *callback)
{
	return gp_filter_gaussian_blur_ex_alloc(src, 0, 0, src->w, src->h,
	                                        x_sigma, y_sigma, callback);
}

#endif /* FILTERS_GP_BLUR_H */
