// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2009-2013 Cyril Hrubis <metan@ucw.cz>
 */

/*

  gp_pixmap rotations and mirroring.

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

/*
 * Calls a symmetry filter on bitmap.
 *
 * If dst is NULL, new bitmap is allocated.
 *
 * Returns pointer to destination bitmap or NULL if allocation failed.
 */
typedef enum gp_filter_symmetries {
	GP_ROTATE_90 = 0,
	GP_ROTATE_CW = GP_ROTATE_90,
	GP_ROTATE_180,
	GP_ROTATE_270,
	GP_ROTATE_CCW = GP_ROTATE_270,
	GP_MIRROR_H,
	GP_MIRROR_V,
} gp_filter_symmetries;

/*
 * NULL-terminated array of symmetry names (C strings).
 */
extern const char **gp_filter_symmetry_names;

/*
 * Symmetry by name (as defined in gp_filer_symmetry_names).
 *
 * Returns either one of the gp_filter_symmetries enums or -1 in case of
 * failure.
 */
int gp_filter_symmetry_by_name(const char *symmetry);

int gp_filter_symmetry(const gp_pixmap *src, gp_pixmap *dst,
                       gp_filter_symmetries symmetry,
                       gp_progress_cb *callback);

gp_pixmap *gp_filter_symmetry_alloc(const gp_pixmap *src,
                                    gp_filter_symmetries symmetry,
                                    gp_progress_cb *callback);

#endif /* FILTERS_GP_ROTATE_H */
