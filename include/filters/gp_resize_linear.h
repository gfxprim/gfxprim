// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2009-2013 Cyril Hrubis <metan@ucw.cz>
 */

/*

  Bilinear interpolation.

 */

#ifndef FILTERS_GP_RESIZE_LINEAR_H
#define FILTERS_GP_RESIZE_LINEAR_H

#include <filters/gp_filter.h>
#include <filters/gp_resize.h>

int gp_filter_resize_linear_int(const gp_pixmap *src, gp_pixmap *dst,
                                gp_progress_cb *callback);

int gp_filter_resize_linear_lf_int(const gp_pixmap *src, gp_pixmap *dst,
                                   gp_progress_cb *callback);

static inline gp_pixmap *gp_filter_resize_linear_int_alloc(const gp_pixmap *src,
                                                           gp_size w, gp_size h,
                                                           gp_progress_cb *callback)
{
	return gp_filter_resize_alloc(src, w, h, GP_INTERP_LINEAR_INT, callback);
}

static inline gp_pixmap *gp_filter_resize_linear_lf_int_alloc(const gp_pixmap *src,
                                                              gp_size w, gp_size h,
                                                              gp_progress_cb *callback)
{
	return gp_filter_resize_alloc(src, w, h, GP_INTERP_LINEAR_LF_INT, callback);
}

#endif /* FILTERS_GP_RESIZE_LINEAR_H */
