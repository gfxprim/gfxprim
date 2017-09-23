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

   Linear Convolution _Raw filters.

 */

#ifndef FILTERS_GP_LINEAR_H
#define FILTERS_GP_LINEAR_H

#include "GP_Filter.h"

/*
 * Linear convolution.
 *
 * The kernel is array of kw * kh floats and is indexed as two directional
 * array.
 *
 * The src coordinates and size defines rectangle in the source on which the
 * filter operates.
 *
 * The dst coodinates defines start pixel of in the destination pixmap.
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
 * kern_div = 9
 */
int GP_FilterLinearConvolution_Raw(const GP_Pixmap *src,
                                   GP_Coord x_src, GP_Coord y_src,
                                   GP_Size w_src, GP_Size h_src,
                                   GP_Pixmap *dst,
                                   GP_Coord x_dst, GP_Coord y_dst,
                                   float kernel[], uint32_t kw, uint32_t kh,
                                   float kern_div, GP_ProgressCallback *callback);

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
int GP_FilterHLinearConvolution_Raw(const GP_Pixmap *src,
                                    GP_Coord x_src, GP_Coord y_src,
                                    GP_Size w_src, GP_Size h_src,
                                    GP_Pixmap *dst,
                                    GP_Coord x_dst, GP_Coord y_dst,
                                    float kernel[], uint32_t kw, float kern_div,
                                    GP_ProgressCallback *callback);

int GP_FilterVLinearConvolution_Raw(const GP_Pixmap *src,
                                    GP_Coord x_src, GP_Coord y_src,
                                    GP_Size w_src, GP_Size h_src,
                                    GP_Pixmap *dst,
                                    GP_Coord x_dst, GP_Coord y_dst,
                                    float kernel[], uint32_t kh, float kern_div,
                                    GP_ProgressCallback *callback);

/*
 * Applies both horizontal and vertical convolution and takes care of the
 * correct progress callback (both horizontal and vertical kernels are expected
 * to be similar in size).
 */
int GP_FilterVHLinearConvolution_Raw(const GP_Pixmap *src,
                                     GP_Coord x_src, GP_Coord y_src,
                                     GP_Size w_src, GP_Size h_src,
                                     GP_Pixmap *dst,
                                     GP_Coord x_dst, GP_Coord y_dst,
                                     float hkernel[], uint32_t kw, float hkern_div,
                                     float vkernel[], uint32_t kh, float vkern_div,
                                     GP_ProgressCallback *callback);

/*
 * Prints a kernel into the stdout.
 */
void GP_FilterKernelPrint_Raw(float kernel[], int kw, int kh, float kern_div);



typedef struct GP_ConvolutionParams {
	const GP_Pixmap *src;
	GP_Coord x_src;
	GP_Coord y_src;
	GP_Size w_src;
	GP_Size h_src;

	GP_Pixmap *dst;
	GP_Coord x_dst;
	GP_Coord y_dst;

	float *kernel;
	unsigned int kw;
	unsigned int kh;
	float kern_div;

	GP_ProgressCallback *callback;
} GP_ConvolutionParams;

static inline int GP_FilterConvolution_Raw(const struct GP_ConvolutionParams *params)
{
	return GP_FilterLinearConvolution_Raw(params->src, params->x_src,
	                                      params->y_src, params->w_src,
	                                      params->h_src, params->dst,
	                                      params->x_dst, params->y_dst,
	                                      params->kernel, params->kw,
	                                      params->kh, params->kern_div,
	                                      params->callback);
}

static inline int GP_FilterVConvolution_Raw(const struct GP_ConvolutionParams *params)
{

	return GP_FilterVLinearConvolution_Raw(params->src, params->x_src,
	                                       params->y_src, params->w_src,
	                                       params->h_src, params->dst,
	                                       params->x_dst, params->y_dst,
	                                       params->kernel, params->kh,
	                                       params->kern_div,
	                                       params->callback);
}

static inline int GP_FilterHConvolution_Raw(const struct GP_ConvolutionParams *params)
{
	return GP_FilterHLinearConvolution_Raw(params->src, params->x_src,
	                                       params->y_src, params->w_src,
	                                       params->h_src, params->dst,
	                                       params->x_dst, params->y_dst,
	                                       params->kernel, params->kw,
	                                       params->kern_div,
	                                       params->callback);
}

#endif /* FILTERS_GP_LINEAR_H */
