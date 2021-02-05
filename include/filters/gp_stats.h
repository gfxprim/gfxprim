// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2009-2015 Cyril Hrubis <metan@ucw.cz>
 */

/*

  Statistic filters.

 */

#ifndef FILTERS_GP_STATS_H
#define FILTERS_GP_STATS_H

#include <filters/gp_filter.h>

typedef struct gp_histogram_channel {
	const char *chan_name;
	gp_pixel min;
	gp_pixel max;
	uint32_t len;
	uint32_t hist[];
} gp_histogram_channel;

typedef struct gp_histogram {
	gp_pixel_type pixel_type;
	gp_histogram_channel *channels[];
} gp_histogram;

gp_histogram *gp_histogram_alloc(gp_pixel_type pixel_type);

void gp_histogram_free(gp_histogram *self);

gp_histogram_channel *gp_histogram_channel_by_name(gp_histogram *self,
                                                   const char *name);

/*
 * Computes histogram. Returns non-zero on failure (i.e. canceled by callback).
 */
int gp_filter_histogram(gp_histogram *self, const gp_pixmap *src,
                        gp_progress_cb *callback);

#endif /* FILTERS_GP_STATS_H */
