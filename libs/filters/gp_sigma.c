// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2009-2013 Cyril Hrubis <metan@ucw.cz>
 */

#include <errno.h>
#include <string.h>

#include <core/gp_common.h>
#include <core/gp_pixmap.h>
#include <core/gp_get_put_pixel.h>
#include <core/gp_temp_alloc.h>
#include <core/gp_clamp.h>
#include <core/gp_debug.h>
#include <filters/gp_sigma.h>

static int gp_filter_sigma_raw(const gp_pixmap *src,
                               gp_coord x_src, gp_coord y_src,
                               gp_size w_src, gp_size h_src,
                               gp_pixmap *dst,
                               gp_coord x_dst, gp_coord y_dst,
                               int xrad, int yrad,
                               unsigned int min, float sigma,
                               gp_progress_cb *callback)
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

	gp_temp_alloc_create(temp, 3 * size * sizeof(unsigned int));

	unsigned int *R = gp_temp_alloc_get(temp, size * sizeof(unsigned int));
	unsigned int *G = gp_temp_alloc_get(temp, size * sizeof(unsigned int));
	unsigned int *B = gp_temp_alloc_get(temp, size * sizeof(unsigned int));

	/* prefil the sampled array */
	for (x = 0; x < (int)w; x++) {
		int xi = GP_CLAMP(x_src + x - xrad, 0, (int)src->w - 1);

		for (y = 0; y < (int)ydiam; y++) {
			int yi = GP_CLAMP(y_src + y - yrad, 0, (int)src->h - 1);

			gp_pixel pix = gp_getpixel_raw_24BPP(src, xi, yi);

			R[y * w + x] = GP_PIXEL_GET_R_RGB888(pix);
			G[y * w + x] = GP_PIXEL_GET_G_RGB888(pix);
			B[y * w + x] = GP_PIXEL_GET_B_RGB888(pix);
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
			int R_center = R[yc * w + x + xrad];
			int G_center = G[yc * w + x + xrad];
			int B_center = B[yc * w + x + xrad];

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
					int R_cur = R[y1 * w + x + x1];
					int G_cur = G[y1 * w + x + x1];
					int B_cur = B[y1 * w + x + x1];

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

			gp_putpixel_raw_24BPP(dst, x_dst + x, y_dst + y,
			                      GP_PIXEL_CREATE_RGB888(r, g, b));
		}

		int yi = GP_CLAMP(y_src + y + yrad + 1, 0, (int)src->h - 1);

		for (x = 0; x < (int)w; x++) {
			int xi = GP_CLAMP(x_src + x - xrad, 0, (int)src->w - 1);

			gp_pixel pix = gp_getpixel_raw_24BPP(src, xi, yi);

			R[yl * w + x] = GP_PIXEL_GET_R_RGB888(pix);
			G[yl * w + x] = GP_PIXEL_GET_G_RGB888(pix);
			B[yl * w + x] = GP_PIXEL_GET_B_RGB888(pix);
		}

		yc = (yc+1) % ydiam;
		yl = (yl+1) % ydiam;

		if (gp_progress_cb_report(callback, y, h_src, w_src)) {
			gp_temp_alloc_free(temp);
			return 1;
		}
	}

	gp_temp_alloc_free(temp);
	gp_progress_cb_done(callback);

	return 0;
}

int gp_filter_sigma_ex(const gp_pixmap *src,
                       gp_coord x_src, gp_coord y_src,
                       gp_size w_src, gp_size h_src,
                       gp_pixmap *dst,
                       gp_coord x_dst, gp_coord y_dst,
                       int xrad, int yrad,
                       unsigned int min, float sigma,
                       gp_progress_cb *callback)
{
	GP_CHECK(src->pixel_type == dst->pixel_type);

	/* Check that destination is large enough */
	GP_CHECK(x_dst + (gp_coord)w_src <= (gp_coord)dst->w);
	GP_CHECK(y_dst + (gp_coord)h_src <= (gp_coord)dst->h);

	GP_CHECK(xrad >= 0 && yrad >= 0);

	return gp_filter_sigma_raw(src, x_src, y_src, w_src, h_src,
	                           dst, x_dst, y_dst, xrad, yrad, min, sigma,
	                           callback);
}

gp_pixmap *gp_filter_sigma_ex_alloc(const gp_pixmap *src,
                                    gp_coord x_src, gp_coord y_src,
                                    gp_size w_src, gp_size h_src,
                                    int xrad, int yrad,
                                    unsigned int min, float sigma,
                                    gp_progress_cb *callback)
{
	int ret;

	GP_CHECK(xrad >= 0 && yrad >= 0);

	gp_pixmap *dst = gp_pixmap_alloc(w_src, h_src, src->pixel_type);

	if (dst == NULL)
		return NULL;

	ret = gp_filter_sigma_raw(src, x_src, y_src, w_src, h_src,
	                          dst, 0, 0, xrad, yrad, min, sigma, callback);

	if (ret) {
		gp_pixmap_free(dst);
		return NULL;
	}

	return dst;
}
