// SPDX-License-Identifier: GPL-2.1-or-later
/*
 * Copyright (C) 2009-2013 Cyril Hrubis <metan@ucw.cz>
 */

#ifndef FILTERS_GP_SEPIA_H
#define FILTERS_GP_SEPIA_H

#include <filters/gp_filter.h>

int gp_filter_sepia_ex(const gp_pixmap *const src,
                       gp_coord x_src, gp_coord y_src,
                       gp_size w_src, gp_size h_src,
                       gp_pixmap *dst,
                       gp_coord x_dst, gp_coord y_dst,
                       gp_progress_cb *callback);

static inline int gp_filter_sepia(const gp_pixmap *const src,
                                 gp_pixmap *dst,
                                 gp_progress_cb *callback)
{
	return gp_filter_sepia_ex(src, 0, 0, src->w, src->h,
	                          dst, 0, 0, callback);
}

gp_pixmap *gp_filter_sepia_ex_alloc(const gp_pixmap *const src,
                                    gp_coord x_src, gp_coord y_src,
                                    gp_size w_src, gp_size h_src,
                                    gp_pixel_type dst_pixel_type,
                                    gp_progress_cb *callback);

static inline gp_pixmap *gp_filter_sepia_alloc(const gp_pixmap *const src,
                                               gp_pixel_type dst_pixel_type,
                                               gp_progress_cb *callback)
{
	return gp_filter_sepia_ex_alloc(src, 0, 0, src->w, src->h,
	                                dst_pixel_type, callback);
}

#endif /* FILTERS_GP_SEPIA_H */
