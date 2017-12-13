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

#include <string.h>
#include <errno.h>

#include <core/GP_Debug.h>
#include <filters/GP_Stats.h>

gp_histogram *gp_histogram_alloc(gp_pixel_type pixel_type)
{
	size_t hsize, size = 0;
	unsigned int i;
	gp_histogram *hist;

	GP_DEBUG(1, "Allocating histogram for %s",
	         gp_pixel_type_name(pixel_type));

	hsize = sizeof(gp_histogram) +
	        gp_pixel_channel_count(pixel_type) * sizeof(void*);

	for (i = 0; i < gp_pixel_channel_count(pixel_type); i++) {
		size += sizeof(gp_histogram_channel) +
			sizeof(uint32_t) * (1<<gp_pixel_channel_bits(pixel_type, i));
	}

	hist = malloc(hsize + size);
	if (!hist) {
		GP_WARN("Malloc failed :(");
		errno = ENOMEM;
		return NULL;
	}

	hist->pixel_type = pixel_type;

	for (i = 0; i < gp_pixel_channel_count(pixel_type); i++) {
		size_t chan_size = 1<<gp_pixel_channel_bits(pixel_type, i);

		hist->channels[i] = (void*)hist + hsize;

		hsize += sizeof(gp_histogram_channel) +
			sizeof(uint32_t) * chan_size;

		hist->channels[i]->len = chan_size;
		hist->channels[i]->chan_name = gp_pixel_channel_name(pixel_type, i);
	}

	return hist;
}

gp_histogram_channel *gp_histogram_channel_by_name(gp_histogram *self,
		                                   const char *name)
{
	unsigned int i;

	for (i = 0; i < gp_pixel_channel_count(self->pixel_type); i++) {
		if (!strcmp(self->channels[i]->chan_name, name))
			return self->channels[i];
	}

	return NULL;
}

void gp_histogram_free(gp_histogram *self)
{
	GP_DEBUG(1, "Freeing histogram %p", self);
	free(self);
}
