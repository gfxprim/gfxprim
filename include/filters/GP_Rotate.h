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
 * Copyright (C) 2009-2011 Cyril Hrubis <metan@ucw.cz>                       *
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
 */
void GP_FilterMirrorH_Raw(const GP_Context *src, GP_Context *dst,
                          GP_ProgressCallback *callback);

GP_Context *GP_FilterMirrorH(const GP_Context *src,
                             GP_ProgressCallback *callback);

/*
 * Mirror vertically
 *
 * Works 'in place'. The contexts must have equal pixel_type and size.
 */
void GP_FilterMirrorV_Raw(const GP_Context *src, GP_Context *dst,
                          GP_ProgressCallback *callback);

GP_Context *GP_FilterMirrorV(const GP_Context *src,
                             GP_ProgressCallback *callback);

/*
 * Rotate context by 90, 180 and 270.
 *
 * Doesn't work 'in place'. The contexts must have equal pixel_type size must 
 * match the rotated size size (is equal for 180 and swapped for 90 and 270).
 */
void GP_FilterRotate90_Raw(const GP_Context *src, GP_Context *dst,
                           GP_ProgressCallback *callback);

void GP_FilterRotate180_Raw(const GP_Context *src, GP_Context *dst,
                            GP_ProgressCallback *callback);

void GP_FilterRotate270_Raw(const GP_Context *src, GP_Context *dst,
                            GP_ProgressCallback *callback);

typedef enum GP_FilterRotation {
	GP_ROTATE_90,
	GP_ROTATE_CW = GP_ROTATE_90,
	GP_ROTATE_180,
	GP_ROTATE_270,
	GP_ROTATE_CCW = GP_ROTATE_270,
	GP_MIRROR_H,
	GP_MIRROR_V,
} GP_FilterRotation;

void GP_FilterRotate_Raw(const GP_Context *src, GP_Context *dst,
                         GP_FilterRotation rotation,
                         GP_ProgressCallback *callback);

GP_Context *GP_FilterRotate(const GP_Context *context,
                            GP_FilterRotation rotation,
                            GP_ProgressCallback *callback);

static inline GP_Context *GP_FilterRotate90(const GP_Context *src,
                                            GP_ProgressCallback *callback)
{
	return GP_FilterRotate(src, GP_ROTATE_90, callback);
}

static inline GP_Context *GP_FilterRotate180(const GP_Context *src,
                                             GP_ProgressCallback *callback)
{
	return GP_FilterRotate(src, GP_ROTATE_180, callback);
}

static inline GP_Context *GP_FilterRotate270(const GP_Context *src,
                                             GP_ProgressCallback *callback)
{
	return GP_FilterRotate(src, GP_ROTATE_270, callback);
}

#endif /* FILTERS_GP_ROTATE_H */
