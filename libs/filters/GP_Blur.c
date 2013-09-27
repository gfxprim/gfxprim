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

#include <math.h>

#include "core/GP_Debug.h"

#include "GP_Linear.h"
#include "GP_LinearThreads.h"

#include "GP_Blur.h"

static inline unsigned int gaussian_kernel_size(float sigma)
{
	int center = 3 * sigma;

	return 2 * center + 1;
}

static inline float gaussian_kernel_init(float sigma, float *kernel)
{
	int i, center = 3 * sigma;
	int N = 2 * center + 1;
	float ret = 0;

	double sigma2 = sigma * sigma;

	for (i = 0; i < N; i++) {
		double r = center - i;
		kernel[i] = exp(-0.5 * (r * r) / sigma2);
		ret += kernel[i];
	}

	return ret;
}

static int gaussian_callback_horiz(GP_ProgressCallback *self)
{
	GP_ProgressCallback *callback = self->priv;

	callback->percentage = self->percentage / 2;
	return callback->callback(callback);
}

static int gaussian_callback_vert(GP_ProgressCallback *self)
{
	GP_ProgressCallback *callback = self->priv;

	callback->percentage = self->percentage / 2 + 50;
	return callback->callback(callback);
}

int GP_FilterGaussianBlur_Raw(const GP_Context *src,
                              GP_Coord x_src, GP_Coord y_src,
                              GP_Size w_src, GP_Size h_src,
			      GP_Context *dst,
                              GP_Coord x_dst, GP_Coord y_dst,
                              float x_sigma, float y_sigma,
                              GP_ProgressCallback *callback)
{
	unsigned int size_x = gaussian_kernel_size(x_sigma);
	unsigned int size_y = gaussian_kernel_size(y_sigma);

	GP_DEBUG(1, "Gaussian blur x_sigma=%2.3f y_sigma=%2.3f kernel %ix%i image %ux%u",
	            x_sigma, y_sigma, size_x, size_y, w_src, h_src);

	GP_ProgressCallback *new_callback = NULL;

	GP_ProgressCallback gaussian_callback = {
		.callback = gaussian_callback_horiz,
		.priv = callback
	};

	if (callback != NULL)
		new_callback = &gaussian_callback;

	/* compute kernel and apply in horizontal direction */
	if (x_sigma > 0) {
		float kernel_x[size_x];
		float sum = gaussian_kernel_init(x_sigma, kernel_x);

		GP_ConvolutionParams params = {
			.src = src,
			.x_src = x_src,
			.y_src = y_src,
			.w_src = w_src,
			.h_src = h_src,
			.dst = dst,
			.x_dst = x_dst,
			.y_dst = y_dst,
			.kernel = kernel_x,
			.kw = size_x,
			.kh = 1,
			.kern_div = sum,
			.callback = new_callback,
		};

		if (GP_FilterHConvolutionMP_Raw(&params))
			return 1;
	}

	if (new_callback != NULL)
		new_callback->callback = gaussian_callback_vert;

	/* compute kernel and apply in vertical direction */
	if (y_sigma > 0) {
		float kernel_y[size_y];
		float sum = gaussian_kernel_init(y_sigma, kernel_y);

		GP_ConvolutionParams params = {
			.src = src,
			.x_src = x_src,
			.y_src = y_src,
			.w_src = w_src,
			.h_src = h_src,
			.dst = dst,
			.x_dst = x_dst,
			.y_dst = y_dst,
			.kernel = kernel_y,
			.kw = 1,
			.kh = size_y,
			.kern_div = sum,
			.callback = new_callback,
		};

		if (GP_FilterVConvolutionMP_Raw(&params))
			return 1;
	}

	GP_ProgressCallbackDone(callback);
	return 0;
}

int GP_FilterGaussianBlurEx(const GP_Context *src,
                            GP_Coord x_src, GP_Coord y_src,
                            GP_Size w_src, GP_Size h_src,
                            GP_Context *dst,
                            GP_Coord x_dst, GP_Coord y_dst,
                            float x_sigma, float y_sigma,
                            GP_ProgressCallback *callback)
{
	GP_CHECK(src->pixel_type == dst->pixel_type);

	/* Check that destination is large enough */
	GP_CHECK(x_dst + (GP_Coord)w_src <= (GP_Coord)dst->w);
	GP_CHECK(y_dst + (GP_Coord)h_src <= (GP_Coord)dst->h);

	return GP_FilterGaussianBlur_Raw(src, x_src, y_src, w_src, h_src,
	                                 dst, x_dst, y_dst,
	                                 x_sigma, y_sigma, callback);
}

GP_Context *GP_FilterGaussianBlurExAlloc(const GP_Context *src,
                                         GP_Coord x_src, GP_Coord y_src,
                                         GP_Size w_src, GP_Size h_src,
				         float x_sigma, float y_sigma,
                                         GP_ProgressCallback *callback)
{
	GP_Context *dst = GP_ContextAlloc(w_src, h_src, src->pixel_type);

	if (dst == NULL)
		return NULL;

	if (GP_FilterGaussianBlur_Raw(src, x_src, y_src, w_src, h_src, dst,
	                                0, 0, x_sigma, y_sigma, callback)) {
		GP_ContextFree(dst);
		return NULL;
	}

	return dst;
}
