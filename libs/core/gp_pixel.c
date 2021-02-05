// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2009-2010 Jiri "BlueBear" Dluhos
 *                         <jiri.bluebear.dluhos@gmail.com>
 *
 * Copyright (C) 2009-2013 Cyril Hrubis <metan@ucw.cz>
 */

#include <string.h>

#include <core/gp_debug.h>
#include <core/gp_pixel.h>

static const gp_pixel_channel *
get_channel(const gp_pixel_type_desc *desc, const char *name)
{
	unsigned int i;

	for (i = 0; i < desc->numchannels; i++)
		if (!strcmp(desc->channels[i].name, name))
			return &desc->channels[i];

	return NULL;
}

static int match(const gp_pixel_channel *channel, gp_pixel mask)
{
	if (channel == NULL) {
		GP_DEBUG(3, "%s gen %08x pass %08x", channel->name, 0, mask);
		return !mask;
	}

	gp_pixel chmask = ~0;

	chmask >>= (GP_PIXEL_BITS - channel->size);
	chmask <<= channel->offset;
	GP_DEBUG(3, "%s gen %08x pass %08x", channel->name, chmask, mask);

	return (chmask == mask);
}

gp_pixel_type gp_pixel_type_by_name(const char *name)
{
	unsigned int i;

	for (i = 0; i < GP_PIXEL_MAX; i++)
		if (!strcasecmp(name, gp_pixel_types[i].name))
			return i;

	return GP_PIXEL_UNKNOWN;
}

gp_pixel_type gp_pixel_rgb_match(gp_pixel rmask, gp_pixel gmask,
                                gp_pixel bmask, gp_pixel amask,
			        uint8_t bits_per_pixel)
{
	unsigned int i;

	GP_DEBUG(1, "Matching Pixel R %08x G %08x B %08x A %08x size %u",
	            rmask, gmask, bmask, amask, bits_per_pixel);

	for (i = 0; i < GP_PIXEL_MAX; i++) {
		int res;
		const gp_pixel_channel *r, *g, *b, *a;

		if (!gp_pixel_has_flags(i, GP_PIXEL_IS_RGB))
			continue;

		if (gp_pixel_types[i].size != bits_per_pixel)
			continue;

		r = get_channel(&gp_pixel_types[i], "R");
		g = get_channel(&gp_pixel_types[i], "G");
		b = get_channel(&gp_pixel_types[i], "B");
		a = get_channel(&gp_pixel_types[i], "A");

		GP_DEBUG(2, "Trying Pixel %s %u",
		         gp_pixel_types[i].name, bits_per_pixel);

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
			         gp_pixel_types[i].type, gp_pixel_types[i].name);
			return gp_pixel_types[i].type;
		}
	}

	return GP_PIXEL_UNKNOWN;
}


gp_pixel_type gp_pixel_rgb_lookup(uint32_t rsize, uint32_t roff,
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
		const gp_pixel_channel *r, *g, *b, *a;

		if (!gp_pixel_has_flags(i, GP_PIXEL_IS_RGB))
			continue;

		if (gp_pixel_types[i].size != bits_per_pixel)
			continue;

		GP_DEBUG(2, "Trying Pixel %s %u",
		         gp_pixel_types[i].name, bits_per_pixel);

		r = get_channel(&gp_pixel_types[i], "R");
		g = get_channel(&gp_pixel_types[i], "G");
		b = get_channel(&gp_pixel_types[i], "B");
		a = get_channel(&gp_pixel_types[i], "A");

		if (a == NULL && asize != 0)
			continue;

		if (a != NULL && (a->offset != aoff || a->size != asize))
			continue;

		if (r->offset == roff && r->size == rsize &&
		    g->offset == goff && g->size == gsize &&
		    b->offset == boff && b->size == bsize) {
			GP_DEBUG(1, "Pixel found type id %u name '%s'",
			         gp_pixel_types[i].type,
			         gp_pixel_types[i].name);

			return gp_pixel_types[i].type;
		 }
	}

	return GP_PIXEL_UNKNOWN;
}

int gp_pixel_has_flags(gp_pixel_type pixel_type, gp_pixel_flags flags)
{
	gp_pixel_flags my_flags = gp_pixel_types[pixel_type].flags;

	return flags == (my_flags & flags);
}
