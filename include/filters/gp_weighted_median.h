// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2009-2012 Cyril Hrubis <metan@ucw.cz>
 */

 /*


  */

#ifndef FILTERS_GP_WEIGHTED_MEDIAN_H
#define FILTERS_GP_WEIGHTED_MEDIAN_H

#include <filters/gp_filter.h>

typedef struct gp_median_weights {
	unsigned int w;
	unsigned int h;
	unsigned int *weights;
} gp_median_weights;

int gp_filter_weighted_median_ex(const gp_pixmap *src,
                                 gp_coord x_src, gp_coord y_src,
                                 gp_size w_src, gp_size h_src,
                                 gp_pixmap *dst,
                                 gp_coord x_dst, gp_coord y_dst,
                                 gp_median_weights *weights,
                                 gp_progress_cb *callback);

gp_pixmap *gp_filter_weighted_median_ex_alloc(const gp_pixmap *src,
                                              gp_coord x_src, gp_coord y_src,
                                              gp_size w_src, gp_size h_src,
                                              gp_median_weights *weights,
                                              gp_progress_cb *callback);

static inline int gp_filter_weighted_median(const gp_pixmap *src,
                                            gp_pixmap *dst,
                                            gp_median_weights *weights,
                                            gp_progress_cb *callback)
{
	return gp_filter_weighted_median_ex(src, 0, 0, src->w, src->h,
	                                    dst, 0, 0, weights, callback);
}

static inline gp_pixmap *gp_filter_weighted_median_alloc(const gp_pixmap *src,
                                                         gp_median_weights *weights,
                                                         gp_progress_cb *callback)
{
	return gp_filter_weighted_median_ex_alloc(src, 0, 0, src->w, src->h,
	                                          weights, callback);
}

#endif /* FILTERS_GP_WEIGHTED_MEDIAN_H */
