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

  gp_pixmap filters.

 */

#ifndef FILTERS_GP_FILTERS_H
#define FILTERS_GP_FILTERS_H

/* Point filters, brightness, contrast ... */
#include <filters/gp_point.h>

/* Addition, difference, min, max ... */
#include <filters/gp_arithmetic.h>

/* Histograms, ... */
#include <filters/gp_stats.h>

/* Image rotations (90 180 270 grads) and mirroring */
#include <filters/gp_rotate.h>

/* Linear convolution Raw API */
#include <filters/gp_linear.h>

/* Convolution filters */
#include <filters/gp_convolution.h>

/* Blur filters */
#include <filters/gp_blur.h>

/* Edge detection filters */
#include <filters/gp_edge_detection.h>

/* Image scaling (resampling) */
#include <filters/gp_resize.h>
#include <filters/gp_resize_nn.h>
#include <filters/gp_resize_linear.h>
#include <filters/gp_resize_cubic.h>

/* Bitmap dithering */
#include <filters/gp_dither.h>

/* Laplace based filters */
#include <filters/gp_laplace.h>

/* Median filter */
#include <filters/gp_median.h>

/* Weighted Median filter */
#include <filters/gp_weighted_median.h>

/* Sigma Mean filter */
#include <filters/gp_sigma.h>

/* Gaussian noise filter */
#include <filters/gp_gaussian_noise.h>

/* Multi tone point filters */
#include <filters/gp_multi_tone.h>
#include <filters/gp_sepia.h>

#endif /* FILTERS_GP_FILTERS_H */
