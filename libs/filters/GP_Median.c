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

#include "GP_Median.h"

#include <string.h>

static inline void hist_inc(unsigned int *h, unsigned int x, unsigned int val)
{
	h[256 * x + val]++;
}

static inline void hist_dec(unsigned int *h, unsigned int x, unsigned int val)
{
	h[256 * x + val]--;
}

static inline void hist_sub(unsigned int *a, unsigned int *b, unsigned int x)
{
	int j;
	
	for (j = 0; j < 256; j++)
		a[j] -= b[256 * x + j];
}

static inline void hist_add(unsigned int *a, unsigned int *b, unsigned int x)
{
	int j;
	
	for (j = 0; j < 256; j++)
		a[j] += b[256 * x + j];
}

#define HIST_INC hist_inc
#define HIST_DEC hist_dec

static inline unsigned int hist_median(unsigned int *hist, unsigned int len,
                                       unsigned int trigger)
{
	unsigned int i;
	unsigned int acc = 0;

	for (i = 0; i < len; i++) {
		acc += hist[i];
		if (acc >= trigger)
			return i;
	}

	GP_BUG("Trigger not reached");
	return 0;
}

static int GP_FilterMedian_Raw(const GP_Context *src,
                               GP_Coord x_src, GP_Coord y_src,
                               GP_Size w_src, GP_Size h_src,
                               GP_Context *dst,
                               GP_Coord x_dst, GP_Coord y_dst,
		               int xmed, int ymed,
                               GP_ProgressCallback *callback)
{
	int i, x, y;

	GP_DEBUG(1, "Median filter size %ux%u xmed=%u ymed=%u",
	            w_src, h_src, 2 * xmed + 1, 2 * ymed + 1);
	
	/* The buffer is w + 2*xmed + 1 size because we read the last value but we don't use it */
	unsigned int size = (w_src + 2 * xmed + 1) * sizeof(int);

	/* Create and initalize arrays for row of histograms */
	GP_TempAllocCreate(temp, 3 * 256 * size);

	unsigned int *R = GP_TempAllocGet(temp, 256 * size);
	unsigned int *G = GP_TempAllocGet(temp, 256 * size);
	unsigned int *B = GP_TempAllocGet(temp, 256 * size);
	
	memset(R, 0, 256 * size);
	memset(G, 0, 256 * size);
	memset(B, 0, 256 * size);

	/* Prefill row of histograms */
	for (x = 0; x < (int)w_src + 2*xmed; x++) {
		int xi = GP_CLAMP(x_src + x - xmed, 0, (int)src->w - 1);
		
		for (y = -ymed; y <= ymed; y++) {
			int yi = GP_CLAMP(y_src + y, 0, (int)src->h - 1);
			
			GP_Pixel pix = GP_GetPixel_Raw_24BPP(src, xi, yi);
			
			HIST_INC(R, x, GP_Pixel_GET_R_RGB888(pix));
			HIST_INC(G, x, GP_Pixel_GET_G_RGB888(pix));	
			HIST_INC(B, x, GP_Pixel_GET_B_RGB888(pix));	
		}
	}

	/* Apply the median filter */
	for (y = 0; y < (int)h_src; y++) {
		unsigned int XR[256], XG[256], XB[256];
	
		memset(XR, 0, sizeof(XR));
		memset(XG, 0, sizeof(XG));
		memset(XB, 0, sizeof(XB));
	
		/* Compute first histogram */
		for (i = 0; i <= 2*xmed; i++) {
			hist_add(XR, R, i);
			hist_add(XG, G, i);
			hist_add(XB, B, i);
		}
		
		/* Generate row */
		for (x = 0; x < (int)w_src; x++) {
			int r = hist_median(XR, 256, (xmed + ymed + 1));
			int g = hist_median(XG, 256, (xmed + ymed + 1));
			int b = hist_median(XB, 256, (xmed + ymed + 1));

			GP_PutPixel_Raw_24BPP(dst, x_dst + x, y_dst + y,
			                      GP_Pixel_CREATE_RGB888(r, g, b));
		
			/* Recompute histograms */
			hist_sub(XR, R, x);
			hist_sub(XG, G, x);
			hist_sub(XB, B, x);

			hist_add(XR, R, (x + 2 * xmed + 1));
			hist_add(XG, G, (x + 2 * xmed + 1));
			hist_add(XB, B, (x + 2 * xmed + 1));
		}

		/* Recompute histograms, remove y - ymed pixel add y + ymed + 1 */
		for (x = 0; x < (int)w_src + 2*xmed; x++) {
			int xi = GP_CLAMP(x_src + x - xmed, 0, (int)src->w - 1);
			int yi = GP_CLAMP(y_src + y - ymed, 0, (int)src->h - 1);
			
			GP_Pixel pix = GP_GetPixel_Raw_24BPP(src, xi, yi);
		
			HIST_DEC(R, x, GP_Pixel_GET_R_RGB888(pix));	
			HIST_DEC(G, x, GP_Pixel_GET_G_RGB888(pix));	
			HIST_DEC(B, x, GP_Pixel_GET_B_RGB888(pix));	
			
			yi = GP_MIN(y_src + y + ymed + 1, (int)src->h - 1);
			
			pix = GP_GetPixel_Raw_24BPP(src, xi, yi);
			
			HIST_INC(R, x, GP_Pixel_GET_R_RGB888(pix));	
			HIST_INC(G, x, GP_Pixel_GET_G_RGB888(pix));	
			HIST_INC(B, x, GP_Pixel_GET_B_RGB888(pix));
		}
		
		if (GP_ProgressCallbackReport(callback, y, h_src, w_src)) {
			GP_TempAllocFree(temp);
			return 1;
		}
	}

	GP_TempAllocFree(temp);
	GP_ProgressCallbackDone(callback);

	return 0;
}

int GP_FilterMedianEx(const GP_Context *src,
                      GP_Coord x_src, GP_Coord y_src,
                      GP_Size w_src, GP_Size h_src,
                      GP_Context *dst,
                      GP_Coord x_dst, GP_Coord y_dst,
		      int xmed, int ymed,
                      GP_ProgressCallback *callback)
{
	GP_CHECK(src->pixel_type == dst->pixel_type);
	
	/* Check that destination is large enough */
	GP_CHECK(x_dst + (GP_Coord)w_src <= (GP_Coord)dst->w);
	GP_CHECK(y_dst + (GP_Coord)h_src <= (GP_Coord)dst->h);

	GP_CHECK(xmed >= 0 && ymed >= 0);

	return GP_FilterMedian_Raw(src, x_src, y_src, w_src, h_src,
	                           dst, x_dst, y_dst, xmed, ymed, callback);
}

GP_Context *GP_FilterMedianExAlloc(const GP_Context *src,
                                  GP_Coord x_src, GP_Coord y_src,
                                  GP_Size w_src, GP_Size h_src,
                                  int xmed, int ymed,
                                  GP_ProgressCallback *callback)
{
	int ret;

	GP_CHECK(xmed >= 0 && ymed >= 0);

	GP_Context *dst = GP_ContextAlloc(w_src, h_src, src->pixel_type);

	if (dst == NULL)
		return NULL;

	ret = GP_FilterMedian_Raw(src, x_src, y_src, w_src, h_src,
	                          dst, 0, 0, xmed, ymed, callback);

	if (ret) {
		GP_ContextFree(dst);
		return NULL;
	}

	return dst;
}
