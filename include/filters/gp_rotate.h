// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2009-2025 Cyril Hrubis <metan@ucw.cz>
 */

/*

  gp_pixmap symmetrys and mirroring.

 */

#ifndef FILTERS_GP_ROTATE_H
#define FILTERS_GP_ROTATE_H

#include <core/gp_types.h>
#include <filters/gp_filter.h>

/*
 * Mirrors bitmap horizontally.
 *
 * The dst must be at least as big as source.
 *
 * The filter works 'in-place' which means that src and dst
 * may be very same pixmap. Note that when aborting in-place operation
 * the image buffer gets into an inconsistent state.
 *
 * Returns zero on success, non-zero if operation was aborted.
 */
int gp_filter_mirror_h(const gp_pixmap *src, gp_pixmap *dst,
                       gp_progress_cb *callback);

/*
 * Mirrors bitmap horizontally.
 *
 * Returns pointer to newly allocated pixmap, or NULL if malloc() has failed
 * or operation was aborted from withing a callback.
 */
gp_pixmap *gp_filter_mirror_h_alloc(const gp_pixmap *src,
                                    gp_progress_cb *callback);

/*
 * Mirrors bitmap vertically.
 *
 * The dst must be at least as big as source.
 *
 * The filter works 'in-place' which means that src and dst
 * may be very same pixmap. Note that when aborting in-place operation
 * the image buffer gets into an inconsistent state.
 *
 * Returns zero on success, non-zero if operation was aborted.
 */
int gp_filter_mirror_v(const gp_pixmap *src, gp_pixmap *dst,
                       gp_progress_cb *callback);

/*
 * Mirrors bitmap vertically.
 *
 * Returns pointer to newly allocated pixmap, or NULL if malloc() has failed
 * or operation was aborted from withing a callback.
 */
gp_pixmap *gp_filter_mirror_v_alloc(const gp_pixmap *src,
                                    gp_progress_cb *callback);

/*
 * Rotate the pixmap by 90, 180, 270.
 *
 * Returns pointer to destination bitmap or NULL if allocation failed.
 */
int gp_filter_rotate_90(const gp_pixmap *src, gp_pixmap *dst,
                        gp_progress_cb *callback);

gp_pixmap *gp_filter_rotate_90_alloc(const gp_pixmap *src,
                                     gp_progress_cb *callback);

int gp_filter_rotate_180(const gp_pixmap *src, gp_pixmap *dst,
                         gp_progress_cb *callback);

gp_pixmap *gp_filter_rotate_180_alloc(const gp_pixmap *src,
                                      gp_progress_cb *callback);

int gp_filter_rotate_270(const gp_pixmap *src, gp_pixmap *dst,
                         gp_progress_cb *callback);

gp_pixmap *gp_filter_rotate_270_alloc(const gp_pixmap *src,
                                      gp_progress_cb *callback);

int gp_filter_symmetry(const gp_pixmap *src, gp_pixmap *dst,
                       gp_symmetry symmetry,
                       gp_progress_cb *callback);

gp_pixmap *gp_filter_symmetry_alloc(const gp_pixmap *src,
                                    gp_symmetry symmetry,
                                  gp_progress_cb *callback);

#endif /* FILTERS_GP_ROTATE_H */
