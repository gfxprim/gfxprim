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

  GP_Context rotations and mirroring.

 */

#ifndef FILTERS_GP_ROTATE_H
#define FILTERS_GP_ROTATE_H

#include "core/GP_Context.h"
#include "GP_Filter.h"

/*
 * Mirror horizontally.
 * 
 * Works 'in place'. The contexts must have equal pixel_type and size.
 *
 * This is semi-internal function without any assertions on destination.
 */
int GP_FilterMirrorH_Raw(const GP_Context *src, GP_Context *dst,
                         GP_ProgressCallback *callback);

/*
 * Mirrors bitmap horizontally.
 *
 * The dst must be at least as big as source.
 *
 * The filter works 'in-place' which means that src and dst
 * may be very same context. Note that when aborting in-place operation
 * the image buffer gets into an inconsistent state.
 *
 * Returns zero on success, non-zero if operation was aborted.
 */
int GP_FilterMirrorH(const GP_Context *src, GP_Context *dst,
                     GP_ProgressCallback *callback);

/*
 * Mirrors bitmap horizontally.
 *
 * Returns pointer to newly allocated context, or NULL if malloc() has failed
 * or operation was aborted from withing a callback.
 */
GP_Context *GP_FilterMirrorH_Alloc(const GP_Context *src,
                                   GP_ProgressCallback *callback);

/*
 * Mirror vertically
 *
 * Works 'in place'. The contexts must have equal pixel_type and size.
 *
 * This is semi-internal function without any assertions on destination.
 */
int GP_FilterMirrorV_Raw(const GP_Context *src, GP_Context *dst,
                         GP_ProgressCallback *callback);

/*
 * Mirrors bitmap vertically.
 *
 * The dst must be at least as big as source.
 *
 * The filter works 'in-place' which means that src and dst
 * may be very same context. Note that when aborting in-place operation
 * the image buffer gets into an inconsistent state.
 *
 * Returns zero on success, non-zero if operation was aborted.
 */
int GP_FilterMirrorV(const GP_Context *src, GP_Context *dst,
                     GP_ProgressCallback *callback);

/*
 * Mirrors bitmap vertically.
 *
 * Returns pointer to newly allocated context, or NULL if malloc() has failed
 * or operation was aborted from withing a callback.
 */
GP_Context *GP_FilterMirrorV_Alloc(const GP_Context *src,
                                   GP_ProgressCallback *callback);

/*
 * Rotate context by 90, 180 and 270.
 *
 * Doesn't work 'in place'. The contexts must have equal pixel_type size must 
 * match the rotated size (is equal for 180 and swapped for 90 and 270).
 *
 * These are semi-internal functions without any assertions on destination.
 */
int GP_FilterRotate90_Raw(const GP_Context *src, GP_Context *dst,
                          GP_ProgressCallback *callback);

int GP_FilterRotate180_Raw(const GP_Context *src, GP_Context *dst,
                           GP_ProgressCallback *callback);

int GP_FilterRotate270_Raw(const GP_Context *src, GP_Context *dst,
                           GP_ProgressCallback *callback);

/*
 * Rotate the context by 90, 180, 270.
 *
 * Returns pointer to destination bitmap or NULL if allocation failed.
 */
int GP_FilterRotate90(const GP_Context *src, GP_Context *dst,
                      GP_ProgressCallback *callback);

GP_Context *GP_FilterRotate90_Alloc(const GP_Context *src,
                                    GP_ProgressCallback *callback);

int GP_FilterRotate180(const GP_Context *src, GP_Context *dst,
                       GP_ProgressCallback *callback);

GP_Context *GP_FilterRotate180_Alloc(const GP_Context *src,
                                    GP_ProgressCallback *callback);

int GP_FilterRotate270(const GP_Context *src, GP_Context *dst,
                       GP_ProgressCallback *callback);

GP_Context *GP_FilterRotate270_Alloc(const GP_Context *src,
                                     GP_ProgressCallback *callback);

/*
 * Calls a symmetry filter on bitmap. 
 *
 * If dst is NULL, new bitmap is allocated.
 *
 * Returns pointer to destination bitmap or NULL if allocation failed.
 */
typedef enum GP_FilterSymmetries {
	GP_ROTATE_90 = 0,
	GP_ROTATE_CW = GP_ROTATE_90,
	GP_ROTATE_180,
	GP_ROTATE_270,
	GP_ROTATE_CCW = GP_ROTATE_270,
	GP_MIRROR_H,
	GP_MIRROR_V,
} GP_FilterSymmetries;

/*
 * NULL-terminated array of symmetry names (C strings).
 */
extern const char **GP_FilterSymmetryNames;

/*
 * Symmetry by name (as defined in GP_FilerSymmetryNames).
 * 
 * Returns either one of the GP_FilterSymmetries enums or -1 in case of
 * failure.
 */
int GP_FilterSymmetryByName(const char *symmetry);

int GP_FilterSymmetry(const GP_Context *src, GP_Context *dst,
                      GP_FilterSymmetries symmetry,
		      GP_ProgressCallback *callback);

GP_Context *GP_FilterSymmetry_Alloc(const GP_Context *src,
                                   GP_FilterSymmetries symmetry,
                                   GP_ProgressCallback *callback);

#endif /* FILTERS_GP_ROTATE_H */
