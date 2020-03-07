// SPDX-License-Identifier: GPL-2.1-or-later
/*
 * Copyright (C) 2009-2013 Cyril Hrubis <metan@ucw.cz>
 */

#include <errno.h>

#include "core/gp_pixmap.h"
#include <core/gp_get_put_pixel.h>
#include <core/gp_temp_alloc.h>
#include "core/gp_clamp.h"
#include <core/gp_debug.h>

#include <filters/gp_median.h>

#include <string.h>

struct hist8 {
	unsigned int coarse[16];
	unsigned int fine[16][16];
};

struct hist8u {
	unsigned int coarse[16];
	unsigned int fine[16][16];
	unsigned int lx[16];
};

static inline void hist8_inc(struct hist8 *h, unsigned int x, unsigned int val)
{
	h[x].coarse[val>>4]++;
	h[x].fine[val>>4][val&0x0f]++;
}

static inline void hist8_dec(struct hist8 *h, unsigned int x, unsigned int val)
{
	h[x].coarse[val>>4]--;
	h[x].fine[val>>4][val&0x0f]--;
}

static inline void hist8_add(struct hist8u *out, struct hist8 *in, unsigned int x)
{
	int i;

	for (i = 0; i < 16; i += 4) {
		out->coarse[i + 0] += in[x].coarse[i + 0];
		out->coarse[i + 1] += in[x].coarse[i + 1];
		out->coarse[i + 2] += in[x].coarse[i + 2];
		out->coarse[i + 3] += in[x].coarse[i + 3];
	}
}

static inline void hist8_add_fine(struct hist8u *out, struct hist8 *in, unsigned int x)
{
	int i, j;

	for (i = 0; i < 16; i += 4) {
		out->coarse[i + 0] += in[x].coarse[i + 0];
		out->coarse[i + 1] += in[x].coarse[i + 1];
		out->coarse[i + 2] += in[x].coarse[i + 2];
		out->coarse[i + 3] += in[x].coarse[i + 3];

		for (j = 0; j < 16; j++) {
			out->fine[i + 0][j] += in[x].fine[i + 0][j];
			out->fine[i + 1][j] += in[x].fine[i + 1][j];
			out->fine[i + 2][j] += in[x].fine[i + 2][j];
			out->fine[i + 3][j] += in[x].fine[i + 3][j];
		}
	}
}

static inline void hist8_sub(struct hist8u *out, struct hist8 *in, unsigned int x)
{
	int i;

	for (i = 0; i < 16; i += 4) {
		out->coarse[i + 0] -= in[x].coarse[i + 0];
		out->coarse[i + 1] -= in[x].coarse[i + 1];
		out->coarse[i + 2] -= in[x].coarse[i + 2];
		out->coarse[i + 3] -= in[x].coarse[i + 3];
	}
}

/*
 * Updates only one specified fine part of the histogram.
 *
 * The structure hist8u remebers when was particular fine row updated so we either
 * generate it from scatch or update depending on the number of needed operations.
 */
static inline void hist8_update(struct hist8u *h, unsigned int i,
                                struct hist8  *row, unsigned int x, unsigned int xmed)
{
	unsigned int j, k;
	unsigned int lx = h->lx[i];
	unsigned int dx = x - lx;

	if (dx > 2*xmed) {
		/* if last update was long ago clear it and load again */
		for (j = 0; j < 16; j+=4) {
			h->fine[i][j + 0] = 0;
			h->fine[i][j + 1] = 0;
			h->fine[i][j + 2] = 0;
			h->fine[i][j + 3] = 0;
		}

		for (j = 0; j < 16; j++)
			for (k = 0; k <= 2*xmed; k++)
				h->fine[i][j + 0] += row[x + k].fine[i][j + 0];
	} else {
		/* update only missing bits */
		for (j = 0; j < 16; j++) {
			for (k = 0; k < dx; k++) {
				h->fine[i][j] -= row[lx + k].fine[i][j];
				h->fine[i][j] += row[lx + k + 2*xmed + 1].fine[i][j];
			}
		}

	}

	h->lx[i] = x;
}


static inline unsigned int hist8_median(struct hist8u *h, struct hist8 *row,
                                        unsigned int x, int xmed, unsigned int trigger)
{
	unsigned int i, j;
	unsigned int acc = 0;

	for (i = 0; i < 16; i++) {
		acc += h->coarse[i];

		if (acc >= trigger) {
			acc -= h->coarse[i];

			/* update fine on position i */
			hist8_update(h, i, row, x, xmed);

			for (j = 0; j < 16; j++) {
				acc += h->fine[i][j];

				if (acc >= trigger)
					return (i<<4) | j;
			}
		}
	}

	GP_BUG("Trigger not reached");
	return 0;
}

static int gp_filter_median_raw(const gp_pixmap *src,
                                gp_coord x_src, gp_coord y_src,
                                gp_size w_src, gp_size h_src,
                                gp_pixmap *dst,
                                gp_coord x_dst, gp_coord y_dst,
		                int xmed, int ymed,
                                gp_progress_cb *callback)
{
	int i, x, y;
	unsigned int trigger = ((2*xmed+1)*(2*ymed+1))/2;

	if (src->pixel_type != GP_PIXEL_RGB888) {
		errno = ENOSYS;
		return -1;
	}

	GP_DEBUG(1, "Median filter size %ux%u xmed=%u ymed=%u",
	            w_src, h_src, 2 * xmed + 1, 2 * ymed + 1);

	/* The buffer is w + 2*xmed + 1 size because we read the last value but we don't use it */
	unsigned int size = (w_src + 2 * xmed + 1);

	/* Create and initalize arrays for row of histograms */
	gp_temp_alloc_create(temp, 3 * sizeof(struct hist8) * size + 3 * sizeof(struct hist8u));

	struct hist8 *R = gp_temp_alloc_get(temp, sizeof(struct hist8) * size);
	struct hist8 *G = gp_temp_alloc_get(temp, sizeof(struct hist8) * size);
	struct hist8 *B = gp_temp_alloc_get(temp, sizeof(struct hist8) * size);

	memset(R, 0, sizeof(*R) * size);
	memset(G, 0, sizeof(*G) * size);
	memset(B, 0, sizeof(*B) * size);

	struct hist8u *XR = gp_temp_alloc_get(temp, sizeof(struct hist8u));
	struct hist8u *XG = gp_temp_alloc_get(temp, sizeof(struct hist8u));
	struct hist8u *XB = gp_temp_alloc_get(temp, sizeof(struct hist8u));

	/* Prefill row of histograms */
	for (x = 0; x < (int)w_src + 2*xmed; x++) {
		int xi = GP_CLAMP(x_src + x - xmed, 0, (int)src->w - 1);

		for (y = -ymed; y <= ymed; y++) {
			int yi = GP_CLAMP(y_src + y, 0, (int)src->h - 1);

			gp_pixel pix = gp_getpixel_raw_24BPP(src, xi, yi);

			hist8_inc(R, x, GP_PIXEL_GET_R_RGB888(pix));
			hist8_inc(G, x, GP_PIXEL_GET_G_RGB888(pix));
			hist8_inc(B, x, GP_PIXEL_GET_B_RGB888(pix));
		}
	}

	/* Apply the median filter */
	for (y = 0; y < (int)h_src; y++) {
		memset(XR, 0, sizeof(*XR));
		memset(XG, 0, sizeof(*XG));
		memset(XB, 0, sizeof(*XB));

		/* Compute first histogram */
		for (i = 0; i <= 2*xmed; i++) {
			hist8_add_fine(XR, R, i);
			hist8_add_fine(XG, G, i);
			hist8_add_fine(XB, B, i);
		}

		/* Generate row */
		for (x = 0; x < (int)w_src; x++) {
			int r = hist8_median(XR, R, x, xmed, trigger);
			int g = hist8_median(XG, G, x, xmed, trigger);
			int b = hist8_median(XB, B, x, xmed, trigger);

			gp_putpixel_raw_24BPP(dst, x_dst + x, y_dst + y,
			                      GP_PIXEL_CREATE_RGB888(r, g, b));

			/* Recompute histograms */
			hist8_sub(XR, R, x);
			hist8_sub(XG, G, x);
			hist8_sub(XB, B, x);

			hist8_add(XR, R, (x + 2 * xmed + 1));
			hist8_add(XG, G, (x + 2 * xmed + 1));
			hist8_add(XB, B, (x + 2 * xmed + 1));
		}

		/* Recompute histograms, remove y - ymed pixel add y + ymed + 1 */
		for (x = 0; x < (int)w_src + 2*xmed; x++) {
			int xi = GP_CLAMP(x_src + x - xmed, 0, (int)src->w - 1);
			int yi = GP_CLAMP(y_src + y - ymed, 0, (int)src->h - 1);

			gp_pixel pix = gp_getpixel_raw_24BPP(src, xi, yi);

			hist8_dec(R, x, GP_PIXEL_GET_R_RGB888(pix));
			hist8_dec(G, x, GP_PIXEL_GET_G_RGB888(pix));
			hist8_dec(B, x, GP_PIXEL_GET_B_RGB888(pix));

			yi = GP_MIN(y_src + y + ymed + 1, (int)src->h - 1);

			pix = gp_getpixel_raw_24BPP(src, xi, yi);

			hist8_inc(R, x, GP_PIXEL_GET_R_RGB888(pix));
			hist8_inc(G, x, GP_PIXEL_GET_G_RGB888(pix));
			hist8_inc(B, x, GP_PIXEL_GET_B_RGB888(pix));
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

int gp_filter_median_ex(const gp_pixmap *src,
                        gp_coord x_src, gp_coord y_src,
                        gp_size w_src, gp_size h_src,
                        gp_pixmap *dst,
                        gp_coord x_dst, gp_coord y_dst,
		        int xmed, int ymed,
                        gp_progress_cb *callback)
{
	GP_CHECK(src->pixel_type == dst->pixel_type);

	/* Check that destination is large enough */
	GP_CHECK(x_dst + (gp_coord)w_src <= (gp_coord)dst->w);
	GP_CHECK(y_dst + (gp_coord)h_src <= (gp_coord)dst->h);

	GP_CHECK(xmed >= 0 && ymed >= 0);

	return gp_filter_median_raw(src, x_src, y_src, w_src, h_src,
	                            dst, x_dst, y_dst, xmed, ymed, callback);
}

gp_pixmap *gp_filter_median_ex_alloc(const gp_pixmap *src,
                                     gp_coord x_src, gp_coord y_src,
                                     gp_size w_src, gp_size h_src,
                                     int xmed, int ymed,
                                     gp_progress_cb *callback)
{
	int ret;

	GP_CHECK(xmed >= 0 && ymed >= 0);

	gp_pixmap *dst = gp_pixmap_alloc(w_src, h_src, src->pixel_type);

	if (dst == NULL)
		return NULL;

	ret = gp_filter_median_raw(src, x_src, y_src, w_src, h_src,
	                          dst, 0, 0, xmed, ymed, callback);

	if (ret) {
		gp_pixmap_free(dst);
		return NULL;
	}

	return dst;
}
