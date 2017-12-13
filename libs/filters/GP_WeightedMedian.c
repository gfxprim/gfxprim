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
#include <string.h>

#include <core/GP_Pixmap.h>
#include <core/GP_GetPutPixel.h>
#include <core/GP_TempAlloc.h>
#include <core/GP_Clamp.h>
#include <core/GP_Debug.h>
#include <filters/GP_WeightedMedian.h>


static unsigned int sum_weights(gp_median_weights *weights)
{
	unsigned int i;
	unsigned int sum = 0;

	for (i = 0; i < weights->w * weights->h; i++)
		sum += weights->weights[i];

	return sum;
}

static inline void hist_add(unsigned int *hist, unsigned int val,
                            unsigned int count)
{
	hist[val] += count;
}

static inline unsigned int hist_med(unsigned int *hist, unsigned int size,
                                    unsigned int threshold)
{
	unsigned int i;
	unsigned int acc = 0;

	for (i = 0; i < size; i++) {
		acc += hist[i];
		if (acc >= threshold)
			return i;
	}

	GP_BUG("Threshold not reached");
	return 0;
}

static inline void hist_clear(unsigned int *hist, unsigned int size)
{
	memset(hist, 0, sizeof(unsigned int) * size);
}

static inline unsigned int get_weight(gp_median_weights *weights,
                                      unsigned int x, unsigned int y)
{
	return weights->weights[y * weights->w + x];
}

static int gp_filter_weighted_median_raw(const gp_pixmap *src,
                                         gp_coord x_src, gp_coord y_src,
                                         gp_size w_src, gp_size h_src,
                                         gp_pixmap *dst,
                                         gp_coord x_dst, gp_coord y_dst,
                                         gp_median_weights *weights,
                                         gp_progress_cb *callback)
{
	int x, y, sum = sum_weights(weights);
	unsigned int x1, y1;

	if (src->pixel_type != GP_PIXEL_RGB888) {
		errno = ENOSYS;
		return -1;
	}

	GP_DEBUG(1, "Weighted Median filter size %ux%u xmed=%u ymed=%u sum=%u",
	            w_src, h_src, weights->w, weights->h, sum);

	unsigned int w = w_src +  weights->w;
	unsigned int size = w * weights->h;

	gp_temp_alloc_create(temp, 3 * size * sizeof(unsigned int));

	unsigned int *R = gp_temp_alloc_get(temp, size * sizeof(unsigned int));
	unsigned int *G = gp_temp_alloc_get(temp, size * sizeof(unsigned int));
	unsigned int *B = gp_temp_alloc_get(temp, size * sizeof(unsigned int));

	/* prefil the sampled array */
	for (x = 0; x < (int)w; x++) {
		int xi = GP_CLAMP(x_src + x - (int)weights->w/2, 0, (int)src->w - 1);

		for (y = 0; y < (int)weights->h; y++) {
			int yi = GP_CLAMP(y_src + y - (int)weights->h, 0, (int)src->h - 1);

			gp_pixel pix = gp_getpixel_raw_24BPP(src, xi, yi);

			R[y * w + x] = GP_PIXEL_GET_R_RGB888(pix);
			G[y * w + x] = GP_PIXEL_GET_G_RGB888(pix);
			B[y * w + x] = GP_PIXEL_GET_B_RGB888(pix);
		}
	}

	unsigned int hist_R[256];
	unsigned int hist_G[256];
	unsigned int hist_B[256];

	hist_clear(hist_R, 256);
	hist_clear(hist_G, 256);
	hist_clear(hist_B, 256);

	/* Apply the weighted median filter */
	for (y = 0; y < (int)h_src; y++) {
		for (x = 0; x < (int)w_src; x++) {
			/* compute weighted histogram and then median */
			for (x1 = 0; x1 < weights->w; x1++) {
				for (y1 = 0; y1 < weights->h; y1++) {
					unsigned int weight = get_weight(weights, x1, y1);
					hist_add(hist_R, R[y1 * w + x + x1], weight);
					hist_add(hist_G, G[y1 * w + x + x1], weight);
					hist_add(hist_B, B[y1 * w + x + x1], weight);
				}
			}

			unsigned int r = hist_med(hist_R, 256, sum/2);
			unsigned int g = hist_med(hist_G, 256, sum/2);
			unsigned int b = hist_med(hist_B, 256, sum/2);

			gp_putpixel_raw_24BPP(dst, x_dst + x, y_dst + y,
			                      GP_PIXEL_CREATE_RGB888(r, g, b));

			hist_clear(hist_R, 256);
			hist_clear(hist_G, 256);
			hist_clear(hist_B, 256);
		}

		for (x = 0; x < (int)w; x++) {
			int xi = GP_CLAMP(x_src + x - (int)weights->w/2, 0, (int)src->w - 1);

			for (y1 = 0; y1 < weights->h; y1++) {
				int yi = GP_CLAMP(y_src + y + (int)y1 - (int)weights->h/2, 0, (int)src->h - 1);

				gp_pixel pix = gp_getpixel_raw_24BPP(src, xi, yi);

				R[y1 * w + x] = GP_PIXEL_GET_R_RGB888(pix);
				G[y1 * w + x] = GP_PIXEL_GET_G_RGB888(pix);
				B[y1 * w + x] = GP_PIXEL_GET_B_RGB888(pix);
			}
		}

		if (gp_progress_cb_report(callback, y, h_src, w_src)) {
			gp_temp_alloc_free(temp);
			return 1;
		}
	}

	gp_temp_alloc_free(temp);
	gp_progress_cb_done(callback);

	return 0;
}

int gp_filter_weighted_median_ex(const gp_pixmap *src,
                                 gp_coord x_src, gp_coord y_src,
                                 gp_size w_src, gp_size h_src,
                                 gp_pixmap *dst,
                                 gp_coord x_dst, gp_coord y_dst,
                                 gp_median_weights *weights,
                                 gp_progress_cb *callback)
{
	GP_CHECK(src->pixel_type == dst->pixel_type);

	/* Check that destination is large enough */
	GP_CHECK(x_dst + (gp_coord)w_src <= (gp_coord)dst->w);
	GP_CHECK(y_dst + (gp_coord)h_src <= (gp_coord)dst->h);

	//GP_CHECK(xmed >= 0 && ymed >= 0);

	return gp_filter_weighted_median_raw(src, x_src, y_src, w_src, h_src,
	                                     dst, x_dst, y_dst, weights, callback);
}

gp_pixmap *gp_filter_weighted_median_ex_alloc(const gp_pixmap *src,
                                              gp_coord x_src, gp_coord y_src,
                                              gp_size w_src, gp_size h_src,
                                              gp_median_weights *weights,
                                              gp_progress_cb *callback)
{
	int ret;

	//GP_CHECK(xmed >= 0 && ymed >= 0);

	gp_pixmap *dst = gp_pixmap_alloc(w_src, h_src, src->pixel_type);

	if (dst == NULL)
		return NULL;

	ret = gp_filter_weighted_median_raw(src, x_src, y_src, w_src, h_src,
	                                    dst, 0, 0, weights, callback);

	if (ret) {
		gp_pixmap_free(dst);
		return NULL;
	}

	return dst;
}
