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

  Arithmetic filters - compute products of two bitmaps.

 */

#ifndef FILTERS_GP_ARITHMETIC_H
#define FILTERS_GP_ARITHMETIC_H

#include <filters/GP_Filter.h>

/*
 * add filter.
 *
 * Produces saturated (clamped) addtion.
 */
int gp_filter_add(const gp_pixmap *src_a,
                  const gp_pixmap *src_b,
                  gp_pixmap *dst,
                  gp_progress_cb *callback);

gp_pixmap *gp_filter_add_alloc(const gp_pixmap *src_a,
                               const gp_pixmap *src_b,
                               gp_progress_cb *callback);

/*
 * mul filter.
 *
 * Produces saturated (clamped) multiplication.
 */
int gp_filter_mul(const gp_pixmap *src_a,
                  const gp_pixmap *src_b,
                  gp_pixmap *dst,
                  gp_progress_cb *callback);

gp_pixmap *gp_filter_mul_alloc(const gp_pixmap *src_a,
                               const gp_pixmap *src_b,
                               gp_progress_cb *callback);

/*
 * diff filter.
 *
 * Produces symetric difference.
 * eg. dst = abs(src_a - src_b)
 */
int gp_filter_diff(const gp_pixmap *src_a,
                   const gp_pixmap *src_b,
                   gp_pixmap *dst,
                   gp_progress_cb *callback);

gp_pixmap *gp_filter_diff_alloc(const gp_pixmap *src_a,
                                const gp_pixmap *src_b,
                                gp_progress_cb *callback);

/*
 * maximum filter.
 */
int gp_filter_max(const gp_pixmap *src_a,
                 const gp_pixmap *src_b,
                 gp_pixmap *dst,
                 gp_progress_cb *callback);

gp_pixmap *gp_filter_max_alloc(const gp_pixmap *src_a,
                               const gp_pixmap *src_b,
                               gp_progress_cb *callback);

/*
 * minimum filter.
 */
int gp_filter_min(const gp_pixmap *src_a,
                  const gp_pixmap *src_b,
                  gp_pixmap *dst,
                  gp_progress_cb *callback);

gp_pixmap *gp_filter_min_alloc(const gp_pixmap *src_a,
                               const gp_pixmap *src_b,
                               gp_progress_cb *callback);

#endif /* FILTERS_GP_ARITHMETIC_H */
