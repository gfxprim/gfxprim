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

#include "core/GP_Context.h"
#include "core/GP_GetPutPixel.h"

#include "core/GP_Debug.h"

#include "GP_Linear.h"

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
	
		if (GP_FilterHLinearConvolution_Raw(src, dst, kernel_x, size_x,
		                                    sum, new_callback))
			return 1;
	}
	
	if (new_callback != NULL)
		new_callback->callback = gaussian_callback_vert;

	/* compute kernel and apply in vertical direction */
	if (sigma_y > 0) {
		float kernel_y[size_y];
		float sum = gaussian_kernel_init(sigma_y, kernel_y);
		
		if (GP_FilterVLinearConvolution_Raw(dst, dst, kernel_y, size_y,
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

#define MUL 1024

#define CLAMP(val) do {    \
	if (val > 255)     \
		val = 255; \
	if (val < 0)       \
		val = 0;   \
} while (0)

int GP_FilterHLinearConvolution_Raw(const GP_Context *src, GP_Context *dst,
                                    float kernel[], uint32_t kw, float kern_div,
                                    GP_ProgressCallback *callback)
{
	GP_Coord x, y;
	uint32_t i;
	int32_t ikernel[kw], ikern_div;
	uint32_t size = dst->w + kw - 1;
	
	GP_DEBUG(1, "Horizontal linear convolution kernel width %i image %ux%u",
	            kw, src->w, src->h);

	for (i = 0; i < kw; i++)
		ikernel[i] = kernel[i] * MUL + 0.5;

	ikern_div = kern_div * MUL + 0.5;

	/* do linear convolution */	
	for (y = 0; y < (GP_Coord)dst->h; y++) {
		uint8_t R[size], G[size], B[size];

		/* Fetch the whole row */
		GP_Pixel pix = GP_GetPixel_Raw_24BPP(src, 0, y);

		for (i = 0; i < kw/2; i++) {
			R[i] = GP_Pixel_GET_R_RGB888(pix);
			G[i] = GP_Pixel_GET_G_RGB888(pix);
			B[i] = GP_Pixel_GET_B_RGB888(pix);
		}
			
		for (i = 0; i < src->w; i++) {
			pix = GP_GetPixel_Raw_24BPP(src, i, y);

			R[i+kw/2] = GP_Pixel_GET_R_RGB888(pix);
			G[i+kw/2] = GP_Pixel_GET_G_RGB888(pix);
			B[i+kw/2] = GP_Pixel_GET_B_RGB888(pix);
		}
	
		for (i = src->w + kw/2; i < size; i++) {
			R[i] = GP_Pixel_GET_R_RGB888(pix);
			G[i] = GP_Pixel_GET_G_RGB888(pix);
			B[i] = GP_Pixel_GET_B_RGB888(pix);
		}

		for (x = 0; x < (GP_Coord)dst->w; x++) {
			int32_t r = MUL/2, g = MUL/2, b = MUL/2;

			/* count the pixel value from neighbours weighted by kernel */
			for (i = 0; i < kw; i++) {
				r += R[i + x] * ikernel[i];
				g += G[i + x] * ikernel[i];
				b += B[i + x] * ikernel[i];
			}
			
			/* divide the result */
			r /= ikern_div;
			g /= ikern_div;
			b /= ikern_div;
			
			/* and clamp just to be extra sure */
			CLAMP(r);
			CLAMP(g);
			CLAMP(b);

			GP_PutPixel_Raw_24BPP(dst, x, y,
			                      GP_Pixel_CREATE_RGB888(r, g, b));
		}
		
		if (GP_ProgressCallbackReport(callback, y, dst->h, dst->w))
			return 1;
	}

	GP_ProgressCallbackDone(callback);
	return 0;
}

int GP_FilterVLinearConvolution_Raw(const GP_Context *src, GP_Context *dst,
                                    float kernel[], uint32_t kh, float kern_div,
                                    GP_ProgressCallback *callback)
{
	GP_Coord x, y;
	uint32_t i;
	int32_t ikernel[kh], ikern_div;
	uint32_t size = dst->h + kh - 1;

	for (i = 0; i < kh; i++)
		ikernel[i] = kernel[i] * MUL + 0.5;

	GP_DEBUG(1, "Vertical linear convolution kernel width %i image %ux%u",
	            kh, src->w, src->h);

	ikern_div = kern_div * MUL + 0.5;

	/* do linear convolution */	
	for (x = 0; x < (GP_Coord)dst->w; x++) {
		uint8_t R[size], G[size], B[size];

		/* Fetch the whole column */
		GP_Pixel pix = GP_GetPixel_Raw_24BPP(src, x, 0);

		for (i = 0; i < kh/2; i++) {
			R[i] = GP_Pixel_GET_R_RGB888(pix);
			G[i] = GP_Pixel_GET_G_RGB888(pix);
			B[i] = GP_Pixel_GET_B_RGB888(pix);
		}
			
		for (i = 0; i < src->h; i++) {
			pix = GP_GetPixel_Raw_24BPP(src, x, i);

			R[i+kh/2] = GP_Pixel_GET_R_RGB888(pix);
			G[i+kh/2] = GP_Pixel_GET_G_RGB888(pix);
			B[i+kh/2] = GP_Pixel_GET_B_RGB888(pix);
		}
	
		for (i = src->h + kh/2; i < size; i++) {
			R[i] = GP_Pixel_GET_R_RGB888(pix);
			G[i] = GP_Pixel_GET_G_RGB888(pix);
			B[i] = GP_Pixel_GET_B_RGB888(pix);
		}

		for (y = 0; y < (GP_Coord)dst->h; y++) {
			int32_t r = MUL/2, g = MUL/2, b = MUL/2;
			
			/* count the pixel value from neighbours weighted by kernel */
			for (i = 0; i < kh; i++) {
				r += R[y + i] * ikernel[i];
				g += G[y + i] * ikernel[i];
				b += B[y + i] * ikernel[i];
			}

			/* divide the result */
			r /= ikern_div;
			g /= ikern_div;
			b /= ikern_div;
			
			/* and clamp just to be extra sure */
			CLAMP(r);
			CLAMP(g);
			CLAMP(b);

			GP_PutPixel_Raw_24BPP(dst, x, y,
			                      GP_Pixel_CREATE_RGB888(r, g, b));
		}
		
		if (GP_ProgressCallbackReport(callback, x, dst->w, dst->h))
			return 1;
	}

	GP_ProgressCallbackDone(callback);
	return 0;
}

static int h_callback(GP_ProgressCallback *self)
{
	GP_ProgressCallback *callback = self->priv;

	callback->percentage = self->percentage / 2;
	return callback->callback(callback);
}

static int v_callback(GP_ProgressCallback *self)
{
	GP_ProgressCallback *callback = self->priv;

	callback->percentage = self->percentage / 2 + 50;
	return callback->callback(callback);
}

int GP_FilterVHLinearConvolution_Raw(const GP_Context *src, GP_Context *dst,
                                     float hkernel[], uint32_t kw, float hkern_div,
                                     float vkernel[], uint32_t kh, float vkern_div,
                                     GP_ProgressCallback *callback)
{
	GP_ProgressCallback *new_callback;

	GP_ProgressCallback conv_callback = {
		.callback = h_callback,
		.priv = callback,
	};

	new_callback = callback ? &conv_callback : NULL;

	if (GP_FilterVLinearConvolution_Raw(src, dst, hkernel, kw, hkern_div, new_callback))
		return 1;
	
	conv_callback.callback = v_callback;
	
	if (GP_FilterHLinearConvolution_Raw(src, dst, vkernel, kh, vkern_div, new_callback))
		return 1;
	
	GP_ProgressCallbackDone(callback);
	return 0;
}

/*
 * Linear convolution.
 *
 * Can be used in-place.
 */
int GP_FilterLinearConvolution_Raw(const GP_Context *src, GP_Context *dst,
                                   float kernel[], uint32_t kw, uint32_t kh,
                                   float kern_div, GP_ProgressCallback *callback)
{
	GP_Coord x, y;
	uint32_t i, j;

	GP_DEBUG(1, "Linear convolution kernel %ix%i image %ux%u",
	            kw, kh, src->w, src->h);

	/* do linear convolution */	
	for (y = 0; y < (GP_Coord)dst->h; y++) {
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

		for (x = 0; x < (GP_Coord)dst->w; x++) {
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

			/* divide the result */
			r /= kern_div;
			g /= kern_div;
			b /= kern_div;

			/* and clamp just to be extra sure */
			if (r > 255)
				r = 255;
			if (r < 0)
				r = 0;
			if (g > 255)
				g = 255;
			if (g < 0)
				g = 0;
			if (b > 255)
				b = 255;
			if (b < 0)
				b = 0;

			pix = GP_Pixel_CREATE_RGB888((uint32_t)r, (uint32_t)g, (uint32_t)b);

			GP_PutPixel_Raw_24BPP(dst, x, y, pix);
		
			idx++;

			if (idx >= (int)kw)
				idx = 0;
		}
		
		if (GP_ProgressCallbackReport(callback, y, dst->h, dst->w))
			return 1;
	}

	GP_ProgressCallbackDone(callback);
	return 0;
}
