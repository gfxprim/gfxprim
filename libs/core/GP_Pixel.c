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
 * Copyright (C) 2009-2010 Jiri "BlueBear" Dluhos                            *
 *                         <jiri.bluebear.dluhos@gmail.com>                  *
 *                                                                           *
 * Copyright (C) 2009-2010 Cyril Hrubis <metan@ucw.cz>                       *
 *                                                                           *
 *****************************************************************************/

#include <string.h>

#include "GP_Pixel.h" 

static const GP_PixelTypeChannel *get_channel(const GP_PixelTypeDescription *desc,
                                              const char *name)
{
	unsigned int i;

	for (i = 0; i < desc->numchannels; i++)
		if (!strcmp(desc->channels[i].name, name))
			return &desc->channels[i];

	return NULL;
}

static int match(const GP_PixelTypeChannel *channel, GP_Pixel mask)
{
	if (channel == NULL)
		return !mask;

	GP_Pixel chmask = ~0;

	chmask >>= (GP_PIXEL_BITS - channel->size);
	chmask <<= channel->offset;

	return (chmask == mask);
}

GP_PixelType GP_PixelRGBMatch(GP_Pixel rmask, GP_Pixel gmask,
                              GP_Pixel bmask, GP_Pixel amask)
{
	unsigned int i;

	for (i = 0; i < GP_PIXEL_MAX; i++) {
		int res;

		const GP_PixelTypeChannel *r, *g, *b, *a;

		r = get_channel(&GP_PixelTypes[i], "R");
		g = get_channel(&GP_PixelTypes[i], "G");
		b = get_channel(&GP_PixelTypes[i], "B");
		a = get_channel(&GP_PixelTypes[i], "A");
	
		res = match(r, rmask) && match(g, gmask) &&
		      match(b, bmask) && match(a, amask);

		if (res)
			return GP_PixelTypes[i].type;
	}

	return GP_PIXEL_UNKNOWN;
}
