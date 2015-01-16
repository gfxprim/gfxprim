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
 * Copyright (C) 2009-2010 Jiri "BlueBear" Dluhos                            *
 *                         <jiri.bluebear.dluhos@gmail.com>                  *
 *                                                                           *
 * Copyright (C) 2009-2011 Cyril Hrubis <metan@ucw.cz>                       *
 *                                                                           *
 *****************************************************************************/

/*

  GP_Context filters.

 */

#ifndef FILTERS_GP_FILTERS_H
#define FILTERS_GP_FILTERS_H

/* Point filters, brightness, contrast ... */
#include "filters/GP_Point.h"

/* Addition, difference, min, max ... */
#include "filters/GP_Arithmetic.h"

/* Histograms, ... */
#include "filters/GP_Stats.h"

/* Image rotations (90 180 270 grads) and mirroring */
#include "filters/GP_Rotate.h"

/* Linear convolution Raw API */
#include "filters/GP_Linear.h"

/* Convolution filters */
#include "filters/GP_Convolution.h"

/* Blur filters */
#include "filters/GP_Blur.h"

/* Edge detection filters */
#include "filters/GP_EdgeDetection.h"

/* Image scaling (resampling) */
#include "filters/GP_Resize.h"
#include "filters/GP_ResizeNN.h"
#include "filters/GP_ResizeLinear.h"
#include "filters/GP_ResizeCubic.h"

/* Bitmap dithering */
#include "filters/GP_Dither.h"

/* Laplace based filters */
#include "filters/GP_Laplace.h"

/* Median filter */
#include "filters/GP_Median.h"

/* Weighted Median filter */
#include "filters/GP_WeightedMedian.h"

/* Sigma Mean filter */
#include "filters/GP_Sigma.h"

/* Gaussian noise filter */
#include "filters/GP_GaussianNoise.h"

/* Multi tone point filters */
#include "filters/GP_MultiTone.h"
#include "filters/GP_Sepia.h"

#endif /* FILTERS_GP_FILTERS_H */
