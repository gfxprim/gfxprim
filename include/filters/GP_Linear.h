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

   Linear filters.

 */

#ifndef FILTERS_GP_LINEAR_H
#define FILTERS_GP_LINEAR_H

#include "GP_Filter.h"

/*
 * Gaussian blur
 *
 * The sigma parameters defines the blur radii in horizontal and vertical
 * direction.
 *
 * Internaly this is implemented as separable linear filter (calls vertical and
 * horizontal convolution with generated gaussian kernel).
 *
 * This variant could work in-place so it's perectly okay to call
 *
 * GP_FilterGaussianBlur_Raw(context, context, ...);
 */
int GP_FilterGaussianBlur_Raw(const GP_Context *src, GP_Context *dst,
                              float sigma_x, float sigma_y,
                              GP_ProgressCallback *callback);
/*
 * Gaussian blur.
 *
 * If dst is NULL, new bitmap is allocated.
 *
 * This variant could work in-place.
 *
 * Returns pointer to destination bitmap or NULL if allocation failed.
 */
GP_Context *GP_FilterGaussianBlur(const GP_Context *src, GP_Context *dst,
                                  float sigma_x, float sigma_y,
                                  GP_ProgressCallback *callback);

/*
 * Linear convolution.
 *
 * The kernel is array of kw * kh floats and is indexed as two directional
 * array.
 *
 * To define 3x3 average filter
 *
 * kernel[] = {
 *	1, 1, 1,
 *	1, 1, 1,
 *	1, 1, 1,
 * };
 *
 * kw = kh = 3
 *
 * This function works also in-place.
 */
int GP_FilterLinearConvolution_Raw(const GP_Context *src, GP_Context *dst,
                                   float kernel[], uint32_t kw, uint32_t kh,
                                   GP_ProgressCallback *callback);

/*
 * Special cases for convolution only in horizontal/vertical direction.
 *
 * These are about 10-30% faster than the generic implementation (depending on
 * the kernel size, bigger kernel == more savings).
 *
 * These are two are a base for bilinear filters.
 *
 * Both works also in-place.
 */
int GP_FilterHLinearConvolution_Raw(const GP_Context *src, GP_Context *dst,
                                    float kernel[], uint32_t kw,
                                    GP_ProgressCallback *callback);

int GP_FilterVLinearConvolution_Raw(const GP_Context *src, GP_Context *dst,
                                    float kernel[], uint32_t kh,
                                    GP_ProgressCallback *callback);

#endif /* FILTERS_GP_LINEAR_H */
