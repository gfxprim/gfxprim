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
 * Copyright (C) 2009-2012 Cyril Hrubis <metan@ucw.cz>                       *
 *                                                                           *
 *****************************************************************************/

#include <string.h>

#include "GP_Debug.h"
#include "GP_Pixel.h" 

static const GP_PixelTypeChannel *
get_channel(const GP_PixelTypeDescription *desc, const char *name)
{
	unsigned int i;

	for (i = 0; i < desc->numchannels; i++)
		if (!strcmp(desc->channels[i].name, name))
			return &desc->channels[i];

	return NULL;
}

static int match(const GP_PixelTypeChannel *channel, GP_Pixel mask)
{
	if (channel == NULL) {
		GP_DEBUG(3, "%s gen %08x pass %08x", channel->name, 0, mask);
		return !mask;
	}

	GP_Pixel chmask = ~0;

	chmask >>= (GP_PIXEL_BITS - channel->size);
	chmask <<= channel->offset;
	GP_DEBUG(3, "%s gen %08x pass %08x", channel->name, chmask, mask);

	return (chmask == mask);
}

GP_PixelType GP_PixelTypeByName(const char *name)
{
	unsigned int i;

	for (i = 0; i < GP_PIXEL_MAX; i++)
		if (!strcasecmp(name, GP_PixelTypes[i].name))
			return i;

	return GP_PIXEL_UNKNOWN;
}

GP_PixelType GP_PixelRGBMatch(GP_Pixel rmask, GP_Pixel gmask,
                              GP_Pixel bmask, GP_Pixel amask,
			      uint8_t bits_per_pixel)
{
	unsigned int i;

	GP_DEBUG(1, "Matching Pixel R %08x G %08x B %08x A %08x size %u",
	            rmask, gmask, bmask, amask, bits_per_pixel);

	for (i = 0; i < GP_PIXEL_MAX; i++) {
		int res;

		const GP_PixelTypeChannel *r, *g, *b, *a;

		if (GP_PixelTypes[i].size != bits_per_pixel)
			continue;

		r = get_channel(&GP_PixelTypes[i], "R");
		g = get_channel(&GP_PixelTypes[i], "G");
		b = get_channel(&GP_PixelTypes[i], "B");
		a = get_channel(&GP_PixelTypes[i], "A");

		GP_DEBUG(2, "Trying Pixel %s %u",
		         GP_PixelTypes[i].name, bits_per_pixel);

		if (r)
			GP_DEBUG(3, "Matching R %i %i", r->size, r->offset);
		
		if (g)
			GP_DEBUG(3, "Matching G %i %i", g->size, g->offset);
		
		if (b)
			GP_DEBUG(3, "Matching B %i %i", b->size, b->offset);
		
		if (a)
			GP_DEBUG(3, "Matching A %i %i", a->size, a->offset);
	
		res = match(r, rmask) && match(g, gmask) &&
		      match(b, bmask) && match(a, amask);

		if (res) {
			GP_DEBUG(1, "Pixel found type id %u name '%s'",
			         GP_PixelTypes[i].type, GP_PixelTypes[i].name);
			return GP_PixelTypes[i].type;
		}
	}

	return GP_PIXEL_UNKNOWN;
}


GP_PixelType GP_PixelRGBLookup(uint32_t rsize, uint32_t roff,
                               uint32_t gsize, uint32_t goff,
			       uint32_t bsize, uint32_t boff,
			       uint32_t asize, uint32_t aoff,
			       uint8_t bits_per_pixel)
{
	unsigned int i;
	
	GP_DEBUG(1, "Looking up Pixel R %08x %08x G %08x %08x B %08x %08x "
	            "size %u", rsize, roff, gsize, goff, bsize, boff,
	             bits_per_pixel);

	for (i = 0; i < GP_PIXEL_MAX; i++) {
		const GP_PixelTypeChannel *r, *g, *b, *a;
		
		if (GP_PixelTypes[i].size != bits_per_pixel)
			continue;
		
		GP_DEBUG(2, "Trying Pixel %s %u",
		         GP_PixelTypes[i].name, bits_per_pixel);
		
		r = get_channel(&GP_PixelTypes[i], "R");
		g = get_channel(&GP_PixelTypes[i], "G");
		b = get_channel(&GP_PixelTypes[i], "B");
		a = get_channel(&GP_PixelTypes[i], "A");
	
		if (a == NULL && asize != 0)
			continue;
		
		if (a != NULL && (a->offset != aoff || a->size != asize))
			continue;

		if (r->offset == roff && r->size == rsize &&
		    g->offset == goff && g->size == gsize &&
		    b->offset == boff && b->size == bsize) {
			GP_DEBUG(1, "Pixel found type id %u name '%s'",
			         GP_PixelTypes[i].type, GP_PixelTypes[i].name);
			
			return GP_PixelTypes[i].type;
		 }
	}

	return GP_PIXEL_UNKNOWN;
}

int GP_PixelHasAlpha(GP_PixelType pixel_type)
{
	unsigned int i;

	GP_CHECK_VALID_PIXELTYPE(pixel_type);

	for (i = 0; i < GP_PixelTypes[pixel_type].numchannels; i++)
		if (!strcmp(GP_PixelTypes[pixel_type].channels[i].name, "A"))
			return 1;

	return 0;
}
