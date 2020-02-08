/*****************************************************************************
 * This file is part of gfxprim library.                                     *
 *                                                                           *
 * Gfxprim is free software; you can redistribute it and/or                  *
 * modify it under the terms of the GNU Lesser General Public                *
 * License as published by the _free Software Foundation; either              *
 * version 2.1 of the License, or (at your option) any later version.        *
 *                                                                           *
 * Gfxprim is distributed in the hope that it will be useful,                *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU         *
 * Lesser General Public License for more details.                           *
 *                                                                           *
 * You should have received a copy of the GNU Lesser General Public          *
 * License along with gfxprim; if not, write to the _free Software            *
 * Foundation, Inc., 51 Franklin Street, Fifth Floor,                        *
 * Boston, MA  02110-1301  USA                                               *
 *                                                                           *
 * Copyright (C) 2009-2015 Cyril Hrubis <metan@ucw.cz>                       *
 *                                                                           *
 *****************************************************************************/

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
