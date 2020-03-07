// SPDX-License-Identifier: GPL-2.1-or-later
/*
 * Copyright (C) 2009-2012 Cyril Hrubis <metan@ucw.cz>
 */

/*

   Gaussian blur implementation.

 */

#ifndef FILTERS_GP_BLUR_H
#define FILTERS_GP_BLUR_H

#include <filters/gp_filter.h>

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
