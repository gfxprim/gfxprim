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

#include "core/GP_Context.h"
#include "core/GP_GetPutPixel.h"
#include "core/GP_TempAlloc.h"
#include "core/GP_Clamp.h"

#include "core/GP_Debug.h"

#include "GP_Linear.h"

#define MUL 1024

int GP_FilterHLinearConvolution_Raw(const GP_Context *src,
                                    GP_Coord x_src, GP_Coord y_src,
                                    GP_Size w_src, GP_Size h_src,
                                    GP_Context *dst,
                                    GP_Coord x_dst, GP_Coord y_dst,
				    float kernel[], uint32_t kw, float kern_div,
				    GP_ProgressCallback *callback)
{
	GP_Coord x, y;
	uint32_t i;
	int32_t ikernel[kw], ikern_div;
	uint32_t size = w_src + kw - 1;

	GP_DEBUG(1, "Horizontal linear convolution kernel width %u "
	            "offset %ix%i rectangle %ux%u",
		    kw, x_src, y_src, w_src, h_src);

	for (i = 0; i < kw; i++)
		ikernel[i] = kernel[i] * MUL + 0.5;

	ikern_div = kern_div * MUL + 0.5;

	/* Create temporary buffers */
	GP_TempAllocCreate(temp, 3 * size * sizeof(int));

	int *R = GP_TempAllocGet(temp, size * sizeof(int));
	int *G = GP_TempAllocGet(temp, size * sizeof(int));
	int *B = GP_TempAllocGet(temp, size * sizeof(int));

	/* Do horizontal linear convolution */	
	for (y = 0; y < (GP_Coord)h_src; y++) {
		int yi = GP_MIN(y_src + y, (int)src->h - 1);

		/* Fetch the whole row */
		GP_Pixel pix = GP_GetPixel_Raw_24BPP(src, 0, yi);

		int xi = x_src - kw/2;
		i = 0;

		/* Copy border pixel until the source image starts */
		while (xi <= 0 && i < size) {
			R[i] = GP_Pixel_GET_R_RGB888(pix);
			G[i] = GP_Pixel_GET_G_RGB888(pix);
			B[i] = GP_Pixel_GET_B_RGB888(pix);
			
			i++;
			xi++;
		}

		/* Use as much source image pixels as possible */
		while (xi < (int)src->w && i < size) {
			pix = GP_GetPixel_Raw_24BPP(src, xi, yi);

			R[i] = GP_Pixel_GET_R_RGB888(pix);
			G[i] = GP_Pixel_GET_G_RGB888(pix);
			B[i] = GP_Pixel_GET_B_RGB888(pix);
			
			i++;
			xi++;
		}

		/* Copy the rest the border pixel when we are out again */
		while (i < size) {
			R[i] = GP_Pixel_GET_R_RGB888(pix);
			G[i] = GP_Pixel_GET_G_RGB888(pix);
			B[i] = GP_Pixel_GET_B_RGB888(pix);
		
			i++;
		}

		for (x = 0; x < (GP_Coord)w_src; x++) {
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
			r = GP_CLAMP(r, 0, 255);
			g = GP_CLAMP(g, 0, 255);
			b = GP_CLAMP(b, 0, 255);

			GP_PutPixel_Raw_24BPP(dst, x_dst + x, y_dst + y,
			                      GP_Pixel_CREATE_RGB888(r, g, b));
		}

		if (GP_ProgressCallbackReport(callback, y, dst->h, dst->w)) {
			GP_TempAllocFree(temp);
			return 1;
		}
	}
			
	GP_TempAllocFree(temp);

	GP_ProgressCallbackDone(callback);
	return 0;
}

int GP_FilterVLinearConvolution_Raw(const GP_Context *src,
                                    GP_Coord x_src, GP_Coord y_src,
                                    GP_Size w_src, GP_Size h_src,
                                    GP_Context *dst,
                                    GP_Coord x_dst, GP_Coord y_dst,
                                    float kernel[], uint32_t kh, float kern_div,
                                    GP_ProgressCallback *callback)
{
	GP_Coord x, y;
	uint32_t i;
	int32_t ikernel[kh], ikern_div;
	uint32_t size = h_src + kh - 1;

	for (i = 0; i < kh; i++)
		ikernel[i] = kernel[i] * MUL + 0.5;

	GP_DEBUG(1, "Vertical linear convolution kernel width %u "
	            "offset %ix%i rectangle %ux%u",
		    kh, x_src, y_src, w_src, h_src);

	ikern_div = kern_div * MUL + 0.5;
	
	/* Create temporary buffers */
	GP_TempAllocCreate(temp, 3 * size * sizeof(int));

	int *R = GP_TempAllocGet(temp, size * sizeof(int));
	int *G = GP_TempAllocGet(temp, size * sizeof(int));
	int *B = GP_TempAllocGet(temp, size * sizeof(int));

	/* Do vertical linear convolution */	
	for (x = 0; x < (GP_Coord)w_src; x++) {
		int xi = GP_MIN(x_src + x, (int)src->w - 1);
		
		/* Fetch the whole row */
		GP_Pixel pix = GP_GetPixel_Raw_24BPP(src, xi, 0);

		int yi = y_src - kh/2;
		i = 0;

		/* Copy border pixel until the source image starts */
		while (yi <= 0 && i < size) {
			R[i] = GP_Pixel_GET_R_RGB888(pix);
			G[i] = GP_Pixel_GET_G_RGB888(pix);
			B[i] = GP_Pixel_GET_B_RGB888(pix);
			
			i++;
			yi++;
		}

		/* Use as much source image pixels as possible */
		while (yi < (int)src->h && i < size) {
			pix = GP_GetPixel_Raw_24BPP(src, xi, yi);

			R[i] = GP_Pixel_GET_R_RGB888(pix);
			G[i] = GP_Pixel_GET_G_RGB888(pix);
			B[i] = GP_Pixel_GET_B_RGB888(pix);
			
			i++;
			yi++;
		}

		/* Copy the rest the border pixel when we are out again */
		while (i < size) {
			R[i] = GP_Pixel_GET_R_RGB888(pix);
			G[i] = GP_Pixel_GET_G_RGB888(pix);
			B[i] = GP_Pixel_GET_B_RGB888(pix);
		
			i++;
		}

		for (y = 0; y < (GP_Coord)h_src; y++) {
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
			r = GP_CLAMP(r, 0, 255);
			g = GP_CLAMP(g, 0, 255);
			b = GP_CLAMP(b, 0, 255);

			GP_PutPixel_Raw_24BPP(dst, x_dst + x, y_dst + y,
			                      GP_Pixel_CREATE_RGB888(r, g, b));
		}
		
		if (GP_ProgressCallbackReport(callback, x, dst->w, dst->h)) {
			GP_TempAllocFree(temp);
			return 1;
		}
	}

	GP_TempAllocFree(temp);
	
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

int GP_FilterVHLinearConvolution_Raw(const GP_Context *src,
                                     GP_Coord x_src, GP_Coord y_src,
                                     GP_Size w_src, GP_Size h_src,
                                     GP_Context *dst,
                                     GP_Coord x_dst, GP_Coord y_dst,
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

	if (GP_FilterVLinearConvolution_Raw(src, x_src, y_src, w_src, h_src,
	                                    dst, x_dst, y_dst,
	                                    hkernel, kw, hkern_div,
	                                    new_callback))
		return 1;
	
	conv_callback.callback = v_callback;
	
	if (GP_FilterHLinearConvolution_Raw(dst, x_src, y_src, w_src, h_src,
	                                    dst, x_dst, y_dst,
	                                    vkernel, kh, vkern_div,
	                                    new_callback))
		return 1;
	
	GP_ProgressCallbackDone(callback);
	return 0;
}

int GP_FilterLinearConvolution_Raw(const GP_Context *src,
                                   GP_Coord x_src, GP_Coord y_src,
                                   GP_Size w_src, GP_Size h_src,
                                   GP_Context *dst,
                                   GP_Coord x_dst, GP_Coord y_dst,
                                   float kernel[], uint32_t kw, uint32_t kh,
                                   float kern_div, GP_ProgressCallback *callback)
{
	GP_Coord x, y;
	unsigned int i, j;

	GP_DEBUG(1, "Linear convolution kernel %ix%i rectangle %ux%u",
	            kw, kh, w_src, h_src);

	/* Do linear convolution */	
	for (y = 0; y < (GP_Coord)h_src; y++) {
		uint32_t R[kw][kh], G[kw][kh], B[kw][kh];
		GP_Pixel pix;

		/* Prefill the buffer on the start */
		for (j = 0; j < kh; j++) {
			for (i = 0; i < kw - 1; i++) {
				int xi = x_src + i - kw/2;
				int yi = y_src + y + j - kh/2;

				xi = GP_CLAMP(xi, 0, (int)src->w - 1);
				yi = GP_CLAMP(yi, 0, (int)src->h - 1);	
				
				pix = GP_GetPixel_Raw_24BPP(src, xi, yi);

				R[i][j] = GP_Pixel_GET_R_RGB888(pix);
				G[i][j] = GP_Pixel_GET_G_RGB888(pix);
				B[i][j] = GP_Pixel_GET_B_RGB888(pix);
			}
		}

		int idx = kw - 1;

		for (x = 0; x < (GP_Coord)w_src; x++) {
			float r = 0, g = 0, b = 0;

			for (j = 0; j < kh; j++) {
				int xi = x_src + x + kw/2;
				int yi = y_src + y + j - kh/2;
				
				xi = GP_CLAMP(xi, 0, (int)src->w - 1);
				yi = GP_CLAMP(yi, 0, (int)src->h - 1);	

				pix = GP_GetPixel_Raw_24BPP(src, xi, yi);

				R[idx][j] = GP_Pixel_GET_R_RGB888(pix);
				G[idx][j] = GP_Pixel_GET_G_RGB888(pix);
				B[idx][j] = GP_Pixel_GET_B_RGB888(pix);
			}
			
			/* Count the pixel value from neighbours weighted by kernel */
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
			r = GP_CLAMP((int)r, 0, 255);
			g = GP_CLAMP((int)g, 0, 255);
			b = GP_CLAMP((int)b, 0, 255);

			pix = GP_Pixel_CREATE_RGB888((uint32_t)r, (uint32_t)g, (uint32_t)b);

			GP_PutPixel_Raw_24BPP(dst, x_dst + x, y_dst + y, pix);
		
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

void GP_FilterKernelPrint_Raw(float kernel[], int kw, int kh, float kern_div)
{
	int i, j;

	for (i = 0; i < kw; i++) {

		if (i == kw/2)
			printf("% 8.2f * | ", 1/kern_div);
		else
			printf("           | ");

		for (j = 0; j < kh; j++)
			printf("% 8.2f  ", kernel[j + i * kw]);

		printf("|\n");
	}
}
