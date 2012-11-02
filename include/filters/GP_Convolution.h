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

  Convolution filters.

 */

#ifndef FILTERS_GP_CONVOLUTION_H
#define FILTERS_GP_CONVOLUTION_H

#include "GP_Filter.h"
#include "GP_Linear.h"

/*
 * 2D convolution kernel.
 *
 * The kernel array size must be w * h.
 * 
 * The div is used to divide the resulting value which is commonly used for
 * normalization.
 *
 * Example box smoothing filter kernel initialization:
 *
 * float box_filter[] = {
 * 	1, 1, 1,
 * 	1, 1, 1,
 * 	1, 1, 1,
 * };
 *
 * GP_FilterKernel2D box_kernel = {
 * 	.w = 3,
 * 	.h = 3,
 * 	.div = 9,
 * 	.kernel = box_filter,
 * };
 */
typedef struct GP_FilterKernel2D {
	unsigned int w;
	unsigned int h;
	float div;
	float *kernel;
} GP_FilterKernel2D;

/*
 * Extended convolution filter.
 *
 * Works on rectangle in src defined by x_src, y_src, w_src and h_src.
 *
 * The result is stored into dst strating from x_dst and y_dst.
 *
 */
int GP_FilterConvolutionEx(const GP_Context *src,
                           GP_Coord x_src, GP_Coord y_src,
                           GP_Size w_src, GP_Coord h_src,
                           GP_Context *dst,
                           GP_Coord x_dst, GP_Coord y_dst,
                           const GP_FilterKernel2D *kernel,
                           GP_ProgressCallback *callback);

/*
 * Extended convolution filter.
 *
 * Works on rectangle in src defined by x_src, y_src, w_src and h_src.
 *
 * Allocates context of a w_src x h_src.
 */
GP_Context *GP_FilterConvolutionExAlloc(const GP_Context *src,
                                        GP_Coord x_src, GP_Coord y_src,
                                        GP_Size w_src, GP_Size h_src,
                                        const GP_FilterKernel2D *kernel,
                                        GP_ProgressCallback *callback);


static inline int GP_FilterConvolution(const GP_Context *src, GP_Context *dst,
                                       const GP_FilterKernel2D *kernel,
                                       GP_ProgressCallback *callback)
{
	return GP_FilterConvolutionEx(src, 0, 0, dst->w, dst->h, dst, 0, 0,
	                              kernel, callback);
}

static inline GP_Context *GP_FilterConvolutionAlloc(const GP_Context *src,
                                                    const GP_FilterKernel2D *kernel,
                                                    GP_ProgressCallback *callback)
{
	return GP_FilterConvolutionExAlloc(src, 0, 0, src->w, src->h,
                                           kernel, callback);
}

/*
 * Prints a kernel into the stdout.
 */
static inline void GP_FilterKernel2DPrint(const GP_FilterKernel2D *kernel)
{
	GP_FilterKernelPrint_Raw(kernel->kernel, kernel->w, kernel->h,
	                         kernel->div);
}

#endif /* FILTERS_GP_CONVOLUTION_H */
