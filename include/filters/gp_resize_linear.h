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
