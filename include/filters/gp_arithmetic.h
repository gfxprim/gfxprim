// SPDX-License-Identifier: GPL-2.1-or-later
/*
 * Copyright (C) 2009-2012 Cyril Hrubis <metan@ucw.cz>
 */

/*

  Arithmetic filters - compute products of two bitmaps.

 */

#ifndef FILTERS_GP_ARITHMETIC_H
#define FILTERS_GP_ARITHMETIC_H

#include <filters/gp_filter.h>

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
