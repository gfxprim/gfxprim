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
 * Copyright (C) 2009-2015 Cyril Hrubis <metan@ucw.cz>                       *
 *                                                                           *
 *****************************************************************************/

/*

  Statistic filters.

 */

#ifndef FILTERS_GP_STATS_H
#define FILTERS_GP_STATS_H

#include "GP_Filter.h"

typedef struct GP_HistogramChannel {
	const char *chan_name;
	GP_Pixel min;
	GP_Pixel max;
	uint32_t len;
	uint32_t hist[];
} GP_HistogramChannel;

typedef struct GP_Histogram {
	GP_PixelType pixel_type;
	GP_HistogramChannel *channels[];
} GP_Histogram;

/*
 * Allocates histogram for a given pixel type
 */
GP_Histogram *GP_HistogramAlloc(GP_PixelType pixel_type);

/*
 * Frees histogram.
 */
void GP_HistogramFree(GP_Histogram *self);

/*
 * Returns pointer to channel given channel name.
 */
GP_HistogramChannel *GP_HistogramChannelByName(GP_Histogram *self,
                                               const char *name);

/*
 * Computes histogram. Returns non-zero on failure (i.e. canceled by callback).
 */
int GP_FilterHistogram(GP_Histogram *self, const GP_Context *src,
                       GP_ProgressCallback *callback);

#endif /* FILTERS_GP_STATS_H */
