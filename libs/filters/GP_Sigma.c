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
 * Copyright (C) 2009-2013 Cyril Hrubis <metan@ucw.cz>                       *
 *                                                                           *
 *****************************************************************************/

#include <errno.h>

#include "core/GP_Context.h"
#include "core/GP_GetPutPixel.h"
#include "core/GP_TempAlloc.h"
#include "core/GP_Clamp.h"
#include "core/GP_Common.h"

#include "core/GP_Debug.h"

#include "GP_Sigma.h"

#include <string.h>

static int GP_FilterSigma_Raw(const GP_Context *src,
                              GP_Coord x_src, GP_Coord y_src,
                              GP_Size w_src, GP_Size h_src,
                              GP_Context *dst,
                              GP_Coord x_dst, GP_Coord y_dst,
                              int xrad, int yrad,
                              unsigned int min, float sigma,
                              GP_ProgressCallback *callback)
{
	int x, y;
	unsigned int x1, y1;
	
	if (src->pixel_type != GP_PIXEL_RGB888) {
		errno = ENOSYS;
		return -1;
	}

	GP_DEBUG(1, "Sigma Mean filter size %ux%u xrad=%u yrad=%u sigma=%.2f",
	         w_src, h_src, xrad, yrad, sigma);

	unsigned int R_sigma = 255 * sigma;
	unsigned int G_sigma = 255 * sigma;
	unsigned int B_sigma = 255 * sigma;

	unsigned int xdiam = 2 * xrad + 1;
	unsigned int ydiam = 2 * yrad + 1;

	unsigned int w = w_src + xdiam; 
	unsigned int size = w * ydiam;

	GP_TempAllocCreate(temp, 3 * size * sizeof(unsigned int));

	unsigned int *R = GP_TempAllocGet(temp, size * sizeof(unsigned int));
	unsigned int *G = GP_TempAllocGet(temp, size * sizeof(unsigned int));
	unsigned int *B = GP_TempAllocGet(temp, size * sizeof(unsigned int));
	
	/* prefil the sampled array */
	for (x = 0; x < (int)w; x++) {
		int xi = GP_CLAMP(x_src + x - xrad, 0, (int)src->w - 1);
			
		for (y = 0; y < (int)ydiam; y++) {
			int yi = GP_CLAMP(y_src + y - yrad, 0, (int)src->h - 1);

			GP_Pixel pix = GP_GetPixel_Raw_24BPP(src, xi, yi);

			R[y * w + x] = GP_Pixel_GET_R_RGB888(pix);
			G[y * w + x] = GP_Pixel_GET_G_RGB888(pix);
			B[y * w + x] = GP_Pixel_GET_B_RGB888(pix);
		}
	}

	unsigned int R_sum;
	unsigned int G_sum;
	unsigned int B_sum;

	unsigned int R_ssum;
	unsigned int G_ssum;
	unsigned int B_ssum;

	unsigned int R_cnt;
	unsigned int G_cnt;
	unsigned int B_cnt;

	unsigned int cnt = xdiam * ydiam - 1;

	/* center pixel ypsilon in the buffer */
	unsigned int yc = yrad;
	/* last sampled ypsilon in the buffer */
	unsigned int yl = 0;

	/* Apply the sigma mean filter */
	for (y = 0; y < (int)h_src; y++) {
		for (x = 0; x < (int)w_src; x++) {
			/* Get center pixel */
			unsigned int R_center = R[yc * w + x + xrad];
			unsigned int G_center = G[yc * w + x + xrad];
			unsigned int B_center = B[yc * w + x + xrad];

			/* Reset sum counters */
			R_sum = 0;
			G_sum = 0;
			B_sum = 0;
			
			R_ssum = 0;
			G_ssum = 0;
			B_ssum = 0;

			R_cnt = 0;
			G_cnt = 0;
			B_cnt = 0;

			for (x1 = 0; x1 < xdiam; x1++) {
				for (y1 = 0; y1 < ydiam; y1++) {
					unsigned int R_cur = R[y1 * w + x + x1];
					unsigned int G_cur = G[y1 * w + x + x1];
					unsigned int B_cur = B[y1 * w + x + x1];

					R_sum += R_cur;
					G_sum += G_cur;
					B_sum += B_cur;

					if (abs(R_cur - R_center) < R_sigma) {
						R_ssum += R_cur;
						R_cnt++;
					}

					if (abs(G_cur - G_center) < G_sigma) {
						G_ssum += G_cur;
						G_cnt++;
					}
					
					if (abs(B_cur - B_center) < B_sigma) {
						B_ssum += B_cur;
						B_cnt++;
					}
				}
			}
			
			R_sum -= R_center;
			G_sum -= G_center;
			B_sum -= B_center;

			unsigned int r;
			unsigned int g;
			unsigned int b;
		
			if (R_cnt >= min)
				r = R_ssum / R_cnt;
			else
				r = R_sum / cnt;
			
			if (G_cnt >= min)
				g = G_ssum / G_cnt;
			else
				g = G_sum / cnt;

			if (B_cnt >= min)
				b = B_ssum / B_cnt;
			else
				b = B_sum / cnt;
	
			GP_PutPixel_Raw_24BPP(dst, x_dst + x, y_dst + y,
			                      GP_Pixel_CREATE_RGB888(r, g, b));
		}
		
		int yi = GP_CLAMP(y_src + y + yrad + 1, 0, (int)src->h - 1);
		
		for (x = 0; x < (int)w; x++) {
			int xi = GP_CLAMP(x_src + x - xrad, 0, (int)src->w - 1);
			
			GP_Pixel pix = GP_GetPixel_Raw_24BPP(src, xi, yi);
			
			R[yl * w + x] = GP_Pixel_GET_R_RGB888(pix);
			G[yl * w + x] = GP_Pixel_GET_G_RGB888(pix);
			B[yl * w + x] = GP_Pixel_GET_B_RGB888(pix);
			
		}
		
		yc = (yc+1) % ydiam;
		yl = (yl+1) % ydiam;
		
		if (GP_ProgressCallbackReport(callback, y, h_src, w_src)) {
			GP_TempAllocFree(temp);
			return 1;
		}
	}

	GP_TempAllocFree(temp);
	GP_ProgressCallbackDone(callback);

	return 0;
}

int GP_FilterSigmaEx(const GP_Context *src,
                     GP_Coord x_src, GP_Coord y_src,
                     GP_Size w_src, GP_Size h_src,
                     GP_Context *dst,
                     GP_Coord x_dst, GP_Coord y_dst,
                     int xrad, int yrad,
                     unsigned int min, float sigma,
		     GP_ProgressCallback *callback)
{
	GP_CHECK(src->pixel_type == dst->pixel_type);
	
	/* Check that destination is large enough */
	GP_CHECK(x_dst + (GP_Coord)w_src <= (GP_Coord)dst->w);
	GP_CHECK(y_dst + (GP_Coord)h_src <= (GP_Coord)dst->h);

	GP_CHECK(xrad >= 0 && yrad >= 0);

	return GP_FilterSigma_Raw(src, x_src, y_src, w_src, h_src,
	                          dst, x_dst, y_dst, xrad, yrad, min, sigma,
	                          callback);
}

GP_Context *GP_FilterSigmaExAlloc(const GP_Context *src,
                                  GP_Coord x_src, GP_Coord y_src,
                                  GP_Size w_src, GP_Size h_src,
                                  int xrad, int yrad,
                                  unsigned int min, float sigma,
                                  GP_ProgressCallback *callback)
{
	int ret;

	GP_CHECK(xrad >= 0 && yrad >= 0);

	GP_Context *dst = GP_ContextAlloc(w_src, h_src, src->pixel_type);

	if (dst == NULL)
		return NULL;

	ret = GP_FilterSigma_Raw(src, x_src, y_src, w_src, h_src,
	                         dst, 0, 0, xrad, yrad, min, sigma, callback);

	if (ret) {
		GP_ContextFree(dst);
		return NULL;
	}

	return dst;
}
