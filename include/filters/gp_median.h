// SPDX-License-Identifier: GPL-2.1-or-later
/*
 * Copyright (C) 2009-2012 Cyril Hrubis <metan@ucw.cz>
 */

 /*

   Constant time Median Filter (the computational complexity is independent of
   radius).

   The xmed and ymed are radius values for x and y. The algorithm uses xmed
   respectively ymed pixel neighbors from each side so the result is median of
   rectangle of 2 * xmed + 1 x 2 * ymed + 1 pixels.

  */

#ifndef FILTERS_GP_MEDIAN_H
#define FILTERS_GP_MEDIAN_H

#include <filters/gp_filter.h>

int gp_filter_median_ex(const gp_pixmap *src,
                        gp_coord x_src, gp_coord y_src,
                        gp_size w_src, gp_size h_src,
                        gp_pixmap *dst,
                        gp_coord x_dst, gp_coord y_dst,
                        int xmed, int ymed,
                        gp_progress_cb *callback);

gp_pixmap *gp_filter_median_ex_alloc(const gp_pixmap *src,
                                     gp_coord x_src, gp_coord y_src,
                                     gp_size w_src, gp_size h_src,
                                     int xmed, int ymed,
                                     gp_progress_cb *callback);

static inline int gp_filter_median(const gp_pixmap *src,
                                   gp_pixmap *dst,
                                   int xmed, int ymed,
                                   gp_progress_cb *callback)
{
	return gp_filter_median_ex(src, 0, 0, src->w, src->h,
	                           dst, 0, 0, xmed, ymed, callback);
}

static inline gp_pixmap *gp_filter_median_alloc(const gp_pixmap *src,
                                                int xmed, int ymed,
                                                gp_progress_cb *callback)
{
	return gp_filter_median_ex_alloc(src, 0, 0, src->w, src->h,
	                                 xmed, ymed, callback);
}

#endif /* FILTERS_GP_MEDIAN_H */
