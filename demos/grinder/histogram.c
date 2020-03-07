// SPDX-License-Identifier: GPL-2.1-or-later
/*
 * Copyright (C) 2009-2015 Cyril Hrubis <metan@ucw.cz>
 */

#include "histogram.h"

void histogram_to_png(const gp_pixmap *src, const char *filename)
{
	gp_histogram *hist;

	hist = gp_histogram_alloc(src->pixel_type);
	if (!hist) {
		fprintf(stderr, "Failed to allocate histogram\n");
		return;
	}

	gp_filter_histogram(hist, src, NULL);

	unsigned int i, j;

	gp_pixmap *res = gp_pixmap_alloc(257 * 4, 256, GP_PIXEL_RGB888);

	gp_fill(res, 0xffffff);

	gp_histogram_channel *hist_r = gp_histogram_channel_by_name(hist, "R");

	for (i = 0; i < hist_r->len; i++)
		gp_vline_xyh(res, i, 256, -255.00 * hist_r->hist[i] / hist_r->max + 0.5 , 0xff0000);

	gp_histogram_channel *hist_g = gp_histogram_channel_by_name(hist, "G");

	for (i = 0; i < hist_g->len; i++)
		gp_vline_xyh(res, i+257, 256, -255.00 * hist_g->hist[i] / hist_g->max + 0.5 , 0x00ff00);

	gp_histogram_channel *hist_b = gp_histogram_channel_by_name(hist, "B");

	for (i = 0; i < hist_b->len; i++)
		gp_vline_xyh(res, i+514, 256, -255.00 * hist_b->hist[i] / hist_b->max + 0.5 , 0x0000ff);

	uint32_t max = GP_MAX3(hist_r->max, hist_g->max, hist_b->max);

	for (i = 0; i < hist_r->len; i++) {
		for (j = 0; j < hist_r->len; j++) {
			gp_pixel pix = 0;

			if (255 * hist_r->hist[i] / max + 0.5 > j)
				pix |= 0xff0000;

			if (255 * hist_g->hist[i] / max + 0.5 > j)
				pix |= 0x00ff00;

			if (255 * hist_b->hist[i] / max + 0.5 > j)
				pix |= 0x0000ff;

			gp_putpixel(res, i + 771, 256 - j, pix);
		}
	}

	gp_save_png(res, filename, NULL);
	gp_pixmap_free(res);
	gp_histogram_free(hist);
}
