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

#include <math.h>

#include <GP_Context.h>
#include <GP_GetPutPixel.h>

#include <GP_Debug.h>

#include <GP_Linear.h>

static void GP_FilterLinearConvolution(const GP_Context *src, GP_Context *res,
                                       GP_ProgressCallback *callback,
                                       float kernel[], uint32_t kw, uint32_t kh);

static inline unsigned int gaussian_kernel_size(float sigma)
{
	int center = 3 * sigma;

	return 2 * center + 1;
}

static inline void gaussian_kernel_init(float sigma, float *kernel)
{
	int i, center = 3 * sigma;
	int N = 2 * center + 1;
	
	double sigma2 = sigma * sigma;
	
	for (i = 0; i < N; i++) {
		double r = center - i;
		kernel[i] = exp(-0.5 * (r * r) / sigma2);
	}
}

static void gaussian_callback_1(GP_ProgressCallback *self)
{
	GP_ProgressCallbackReport(self->priv, self->percentage/2);
}

static void gaussian_callback_2(GP_ProgressCallback *self)
{
	GP_ProgressCallbackReport(self->priv, self->percentage/2 + 50);
}

void GP_FilterGaussianBlur_Raw(GP_Context *src, GP_Context *res,
                               GP_ProgressCallback *callback,
                               float sigma_x, float sigma_y)
{
	unsigned int size_x = gaussian_kernel_size(sigma_x);
	unsigned int size_y = gaussian_kernel_size(sigma_y);
	
	GP_DEBUG(1, "Gaussian blur sigma_x=%2.3f sigma_y=%2.3f kernel %ix%i image %ux%u",
	            sigma_x, sigma_y, size_x, size_y, src->w, src->h);
	
	GP_ProgressCallback *new_callback = NULL;

	GP_ProgressCallback gaussian_callback = {
		.callback = gaussian_callback_1,
		.priv = callback
	};

	if (callback != NULL)
		new_callback = &gaussian_callback;

	/* compute kernel and apply on horizontal direction */
	if (sigma_x > 0) {
		float kernel_x[size_x];
		gaussian_kernel_init(sigma_x, kernel_x);
		GP_FilterLinearConvolution(src, res, new_callback, kernel_x, size_x, 1);
	}
	
	if (new_callback != NULL)
		new_callback->callback = gaussian_callback_2;

	/* compute kernel and apply on vertical direction */
	if (sigma_y > 0) {
		float kernel_y[size_y];
		gaussian_kernel_init(sigma_y, kernel_y);
		GP_FilterLinearConvolution(res, res, new_callback, kernel_y, 1, size_y);
	}

	GP_ProgressCallbackDone(callback);
}

GP_Context *GP_FilterGaussianBlur(GP_Context *src,
                                  GP_ProgressCallback *callback,
                                  float sigma_x, float sigma_y)
{
	GP_Context *res;
	
	if (src->pixel_type != GP_PIXEL_RGB888)
		return NULL;

	res = GP_ContextCopy(src, 0);

	if (res == NULL) {
		GP_DEBUG(1, "Malloc failed :(");
		return NULL;
	}

	GP_FilterGaussianBlur_Raw(src, res, callback, sigma_x, sigma_y);

	return res;
}

static void GP_FilterLinearConvolution(const GP_Context *src, GP_Context *res,
                                       GP_ProgressCallback *callback,
                                       float kernel[], uint32_t kw, uint32_t kh)
{
	float kernel_sum = 0;
	GP_Coord x, y;
	uint32_t i, j;

	GP_DEBUG(1, "Linear convolution kernel %ix%i image %ux%u",
	            kw, kh, src->w, src->h);

	/* count kernel sum for normalization */
	for (i = 0; i < kw * kh; i++)
		kernel_sum += kernel[i];

	/* do linear convolution */	
	for (y = 0; y < (GP_Coord)res->h; y++) {
		GP_Pixel pix;
		uint32_t R[kw][kh], G[kw][kh], B[kw][kh];

		/* prefill the buffer on the start */
		for (j = 0; j < kh; j++) {
			for (i = 0; i < kw - 1; i++) {
				int cx = i - kw/2;
				int cy = y + j - kh/2;

				if (cx < 0)
					cx = 0;
				
				if (cy < 0)
					cy = 0;
				
				if (cy >= (int)src->h)
					cy = src->h - 1;
				
				pix = GP_GetPixel_Raw_24BPP(src, cx, cy);

				R[i][j] = GP_Pixel_GET_R_RGB888(pix);
				G[i][j] = GP_Pixel_GET_G_RGB888(pix);
				B[i][j] = GP_Pixel_GET_B_RGB888(pix);
			}
		}

		int idx = kw - 1;

		for (x = 0; x < (GP_Coord)res->w; x++) {
			float r = 0, g = 0, b = 0;

			for (j = 0; j < kh; j++) {
				int cy = y + j - kh/2;
				int cx = x + kw/2;

				if (cy < 0)
					cy = 0;

				if (cy >= (int)src->h)
					cy = src->h - 1;

				if (cx >= (int)src->w)
					cx = src->w - 1;

				pix = GP_GetPixel_Raw_24BPP(src, cx, cy);

				R[idx][j] = GP_Pixel_GET_R_RGB888(pix);
				G[idx][j] = GP_Pixel_GET_G_RGB888(pix);
				B[idx][j] = GP_Pixel_GET_B_RGB888(pix);
			}
			
			/* count the pixel value from neighbours weighted by kernel */
			for (i = 0; i < kw; i++) {
				int k;

				if ((int)i < idx + 1)
					k = kw - idx - 1 + i;
				else
					k = i - idx - 1;

				for (j = 0; j < kh; j++) {
					r += R[i][j] * kernel[k + j * kw];
					g += G[i][j] * kernel[k + j * kw];
					b += B[i][j] * kernel[k + j * kw];
				}
			}

			/* normalize the result */
			r /= kernel_sum;
			g /= kernel_sum;
			b /= kernel_sum;

			/* and clamp just to be extra sure */
			if (r > 255)
				r = 255;
			if (g > 255)
				g = 255;
			if (b > 255)
				b = 255;

			pix = GP_Pixel_CREATE_RGB888((uint32_t)r, (uint32_t)g, (uint32_t)b);

			GP_PutPixel_Raw_24BPP(res, x, y, pix);
		
			idx++;

			if (idx >= (int)kw)
				idx = 0;
		}
		
		if (callback != NULL && y % 100 == 0)
			GP_ProgressCallbackReport(callback, 100.00 * y/res->h);
	}

	GP_ProgressCallbackDone(callback);
}
