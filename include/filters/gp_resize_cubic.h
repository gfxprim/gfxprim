// SPDX-License-Identifier: GPL-2.1-or-later
/*
 * Copyright (C) 2009-2013 Cyril Hrubis <metan@ucw.cz>
 */

/*

  Bicubic interpolation.

 */

#ifndef FILTERS_GP_RESIZE_CUBIC_H
#define FILTERS_GP_RESIZE_CUBIC_H

#include <filters/gp_filter.h>
#include <filters/gp_resize.h>

int gp_filter_resize_cubic_int(const gp_pixmap *src, gp_pixmap *dst,
                               gp_progress_cb *callback);

int gp_filter_resize_cubic(const gp_pixmap *src, gp_pixmap *dst,
                           gp_progress_cb *callback);

static inline gp_pixmap *gp_filter_resize_cubic_int_alloc(const gp_pixmap *src,
                                                          gp_size w, gp_size h,
                                                          gp_progress_cb *callback)
{
	return gp_filter_resize_alloc(src, w, h, GP_INTERP_CUBIC_INT, callback);
}

static inline gp_pixmap *gp_filter_resize_cubic_alloc(const gp_pixmap *src,
                                                    gp_size w, gp_size h,
                                                    gp_progress_cb *callback)
{
	return gp_filter_resize_alloc(src, w, h, GP_INTERP_CUBIC, callback);
}

#endif /* FILTERS_GP_RESIZE_CUBIC_H */
