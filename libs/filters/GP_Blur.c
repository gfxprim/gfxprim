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

int GP_FilterGaussianBlur_Raw(const GP_Context *src, GP_Context *dst,
                              float sigma_x, float sigma_y,
                              GP_ProgressCallback *callback)
{
	unsigned int size_x = gaussian_kernel_size(sigma_x);
	unsigned int size_y = gaussian_kernel_size(sigma_y);
	
	GP_DEBUG(1, "Gaussian blur sigma_x=%2.3f sigma_y=%2.3f kernel %ix%i image %ux%u",
	            sigma_x, sigma_y, size_x, size_y, src->w, src->h);
	
	GP_ProgressCallback *new_callback = NULL;

	GP_ProgressCallback gaussian_callback = {
		.callback = gaussian_callback_horiz,
		.priv = callback
	};

	if (callback != NULL)
		new_callback = &gaussian_callback;

	/* compute kernel and apply in horizontal direction */
	if (sigma_x > 0) {
		float kernel_x[size_x];
		float sum = gaussian_kernel_init(sigma_x, kernel_x);
	
		if (GP_FilterHLinearConvolution_Raw(src, 0, 0, src->w, src->h,
		                                    dst, 0, 0, kernel_x, size_x,
		                                    sum, new_callback))
			return 1;
	}
	
	if (new_callback != NULL)
		new_callback->callback = gaussian_callback_vert;

	/* compute kernel and apply in vertical direction */
	if (sigma_y > 0) {
		float kernel_y[size_y];
		float sum = gaussian_kernel_init(sigma_y, kernel_y);
		
		if (GP_FilterVLinearConvolution_Raw(dst, 0, 0, src->w, src->h,
		                                    dst, 0, 0, kernel_y, size_y,
		                                    sum, new_callback))
			return 1;
	}

	GP_ProgressCallbackDone(callback);
	return 0;
}

GP_Context *GP_FilterGaussianBlur(const GP_Context *src, GP_Context *dst,
                                  float sigma_x, float sigma_y,
                                  GP_ProgressCallback *callback)
{
	/* TODO: templatetize  */
	if (src->pixel_type != GP_PIXEL_RGB888)
		return NULL;
	
	if (dst == NULL) {
		dst = GP_ContextCopy(src, 0);

		if (dst == NULL)
			return NULL;
	} else {
		GP_ASSERT(src->pixel_type == dst->pixel_type,
		          "The src and dst pixel types must match");
		GP_ASSERT(src->w <= dst->w && src->h <= dst->h,
		          "Destination is not big enough");
	}

	GP_FilterGaussianBlur_Raw(src, dst, sigma_x, sigma_y, callback);

	return dst;
}
