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

#include <string.h>

#include <GP_Debug.h>

#include "GP_Stats.h"

int GP_FilterHistogram_Raw(const GP_Context *src, GP_FilterParam histogram[],
                           GP_ProgressCallback *callback);

int GP_FilterHistogram(const GP_Context *src, GP_FilterParam histogram[],
                       GP_ProgressCallback *callback)
{
	int ret;

	ret = GP_FilterHistogram_Raw(src, histogram, callback);

	if (ret)
		return ret;

	unsigned int i;

	for (i = 0; histogram[i].channel_name[0] != '\0'; i++) {
		unsigned int j;
		GP_Histogram *hist = histogram[i].val.ptr;

		hist->max = hist->hist[0];
		hist->min = hist->hist[0];

		for (j = 1; j < hist->len; j++) {
			if (hist->hist[j] > hist->max)
				hist->max = hist->hist[j];

			if (hist->hist[j] < hist->min)
				hist->min = hist->hist[j];
		}
	}

	return 0;
}

void GP_FilterHistogramAlloc(GP_PixelType type, GP_FilterParam params[])
{
	uint32_t i;

	GP_FilterParamSetPtrAll(params, NULL);

	const GP_PixelTypeChannel *channels = GP_PixelTypes[type].channels;

	for (i = 0; i < GP_PixelTypes[type].numchannels; i++) {
		size_t chan_size = 1<<channels[i].size;

		GP_Histogram *hist = malloc(sizeof(struct GP_Histogram) +
		                            sizeof(uint32_t) * chan_size);

		if (hist == NULL) {
			GP_FilterHistogramFree(params);
			return;
		}

		hist->len = chan_size;
		memset(hist->hist, 0, sizeof(uint32_t) * chan_size);

		(GP_FilterParamChannel(params, channels[i].name))->val.ptr = hist;
	}
}
