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
 * Copyright (C) 2009-2013 Cyril Hrubis <metan@ucw.cz>                       *
 *                                                                           *
 *****************************************************************************/

#ifndef FILTERS_GP_SEPIA_H
#define FILTERS_GP_SEPIA_H

#include <filters/GP_Filter.h>

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
