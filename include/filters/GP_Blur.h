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
 * Gaussian blur implemented using linear separable convolution.
 * 
 * The x_sigma defines the blur size in horizontal direction and y_sigma
 * defines blur on vertical direction.
 */

int GP_FilterGaussianBlurEx(const GP_Context *src,
                            GP_Coord x_src, GP_Coord y_src,
                            GP_Size w_src, GP_Size h_src,
                            GP_Context *dst,
			    GP_Coord x_dst, GP_Coord y_dst,
			    float x_sigma, float y_sigma,
			    GP_ProgressCallback *callback);

GP_Context *GP_FilterGaussianBlurExAlloc(const GP_Context *src,
                                         GP_Coord x_src, GP_Coord y_src,
                                         GP_Size w_src, GP_Size h_src,
			                 float x_sigma, float y_sigma,
			                 GP_ProgressCallback *callback);

static inline int GP_FilterGaussianBlur(const GP_Context *src, GP_Context *dst,
                                        float x_sigma, float y_sigma,
                                        GP_ProgressCallback *callback)
{
	return GP_FilterGaussianBlurEx(src, 0, 0, src->w, src->h, dst, 0, 0,
	                               x_sigma, y_sigma, callback);
}

static inline GP_Context *GP_FilterGaussianBlurAlloc(const GP_Context *src,
                                                     float x_sigma, float y_sigma,
                                                     GP_ProgressCallback *callback)
{
	return GP_FilterGaussianBlurExAlloc(src, 0, 0, src->w, src->h,
	                                    x_sigma, y_sigma, callback);
}

#endif /* FILTERS_GP_BLUR_H */
