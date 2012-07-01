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

   Gaussian blur implementation.

 */

#ifndef FILTERS_GP_BLUR_H
#define FILTERS_GP_BLUR_H

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

#endif /* FILTERS_GP_BLUR_H */
