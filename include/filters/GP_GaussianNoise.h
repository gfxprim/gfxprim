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

    Additive Gaussian noise filters.

    The sigma and mu parameters define the noise parameters. The sigma defines
    amount of randomness, the mu defines offset. Both are defined as a ratio of
    the particular channel size.

  */

#ifndef FILTERS_GP_GAUSSIAN_NOISE_H
#define FILTERS_GP_GAUSSIAN_NOISE_H

#include "GP_Filter.h"

int GP_FilterGaussianNoiseAddEx(const GP_Pixmap *src,
                                GP_Coord x_src, GP_Coord y_src,
                                GP_Size w_src, GP_Size h_src,
                                GP_Pixmap *dst,
                                GP_Coord x_dst, GP_Coord y_dst,
                                float sigma, float mu,
                                GP_ProgressCallback *callback);

GP_Pixmap *GP_FilterGaussianNoiseAddExAlloc(const GP_Pixmap *src,
                                             GP_Coord x_src, GP_Coord y_src,
                                             GP_Size w_src, GP_Size h_src,
                                             float sigma, float mu,
                                             GP_ProgressCallback *callback);

static inline int GP_FilterGaussianNoiseAdd(const GP_Pixmap *src,
                                            GP_Pixmap *dst,
                                            float sigma, float mu,
                                            GP_ProgressCallback *callback)
{
	return GP_FilterGaussianNoiseAddEx(src, 0, 0, src->w, src->h,
	                                   dst, 0, 0, sigma, mu, callback);
}

static inline GP_Pixmap *
GP_FilterGaussianNoiseAddAlloc(const GP_Pixmap *src,
                               float sigma, float mu,
                               GP_ProgressCallback *callback)
{
	return GP_FilterGaussianNoiseAddExAlloc(src, 0, 0, src->w, src->h,
	                                        sigma, mu, callback);
}

#endif /* FILTERS_GP_GAUSSIAN_NOISE_H */
