// SPDX-License-Identifier: GPL-2.1-or-later
/*
 * Copyright (C) 2009-2012 Cyril Hrubis <metan@ucw.cz>
 */

 /*

    Additive Gaussian noise filters.

    The sigma and mu parameters define the noise parameters. The sigma defines
    amount of randomness, the mu defines offset. Both are defined as a ratio of
    the particular channel size.

  */

#ifndef FILTERS_GP_GAUSSIAN_NOISE_H
#define FILTERS_GP_GAUSSIAN_NOISE_H

#include <filters/gp_filter.h>

int gp_filter_gaussian_noise_add_ex(const gp_pixmap *src,
                                    gp_coord x_src, gp_coord y_src,
                                    gp_size w_src, gp_size h_src,
                                    gp_pixmap *dst,
                                    gp_coord x_dst, gp_coord y_dst,
                                    float sigma, float mu,
                                    gp_progress_cb *callback);

gp_pixmap *gp_filter_gaussian_noise_add_ex_alloc(const gp_pixmap *src,
                                                 gp_coord x_src, gp_coord y_src,
                                                 gp_size w_src, gp_size h_src,
                                                 float sigma, float mu,
                                                 gp_progress_cb *callback);

static inline int gp_filter_gaussian_noise_add(const gp_pixmap *src,
                                               gp_pixmap *dst,
                                               float sigma, float mu,
                                               gp_progress_cb *callback)
{
	return gp_filter_gaussian_noise_add_ex(src, 0, 0, src->w, src->h,
	                                       dst, 0, 0, sigma, mu, callback);
}

static inline gp_pixmap *
gp_filter_gaussian_noise_add_alloc(const gp_pixmap *src,
                                  float sigma, float mu,
                                  gp_progress_cb *callback)
{
	return gp_filter_gaussian_noise_add_ex_alloc(src, 0, 0, src->w, src->h,
	                                             sigma, mu, callback);
}

#endif /* FILTERS_GP_GAUSSIAN_NOISE_H */
