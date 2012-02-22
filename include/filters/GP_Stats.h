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
 * Copyright (C) 2009-2011 Cyril Hrubis <metan@ucw.cz>                       *
 *                                                                           *
 *****************************************************************************/

/*

  Statistic filters.

 */

#ifndef FILTERS_GP_STATS_H
#define FILTERS_GP_STATS_H

#include "GP_Filter.h"

typedef struct GP_Histogram {
	uint32_t min;
	uint32_t max;
	uint32_t len;
	uint32_t hist[];
} GP_Histogram;

/*
 * Histogram filter.
 *
 * The filter param is expected to hold pointers to struct GP_Histogram
 */
int GP_FilterHistogram(const GP_Context *src, GP_FilterParam histogram[],
                       GP_ProgressCallback *callback);

/*
 * Allocate and initalize struct GP_Histogram for each channel and stores the
 * pointer to filter params array. The pixel type must match the params[]
 * channels.
 */
void GP_FilterHistogramAlloc(GP_PixelType type, GP_FilterParam params[]);

/*
 * Free the histogram arrays.
 */
static inline void GP_FilterHistogramFree(GP_FilterParam params[])
{
	GP_FilterParamFreePtrAll(params);
}

#endif /* FILTERS_GP_STATS_H */
