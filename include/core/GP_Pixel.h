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
 * Copyright (C) 2009-2015 Cyril Hrubis <metan@ucw.cz>                       *
 *                                                                           *
 * Copyright (C) 2011      Tomas Gavenciak <gavento@ucw.cz>                  *
 *                                                                           *
 *****************************************************************************/

#ifndef CORE_GP_PIXEL_H
#define CORE_GP_PIXEL_H

#include <stdint.h>

#include <core/GP_Types.h>
#include <core/GP_Common.h>
#include <core/GP_FnPerBpp.h>

/*
 * gp_pixel_type is an enum of pixel types,
 *
 * each named GP_PIXEL_<TYPENAME>, such as GP_PIXEL_RGB888
 * see the beginning of GP_Pixel.gen.h for a complete list
 *
 * The type always contains GP_PIXEL_UNKNOWN = 0 and
 * GP_PIXEL_MAX as the last value (also the number of valid types)
 * The types are numbered to use the entire range 0 .. GP_PIXEL_MAX-1
 */
#define GP_PIXEL_BITS (sizeof(gp_pixel) * 8)

/* Generated header */
#include "GP_Pixel.gen.h"
#include "GP_GetSetBits.h"

/*
 * Information about ordering of pixels in byte for 1, 2 and 4 bpp
 * used in a one bit variable in gp_pixmap
 */
typedef enum {
	/* less significant bits contain pixels with lower indices */
	/* also used for irrelevant bit-endian */
	GP_BIT_ENDIAN_LE = 0,
	/* more significant bits contain pixels with lower indices */
	GP_BIT_ENDIAN_BE,
} GP_BIT_ENDIAN;

/*
 * Description of one channel
 * Assumes all the channel names to be at most 7 chars long
 *
 * The common channel names are:
 *  R, G, B  - as usual
 *  V        - value, for grayscale
 *  A        - opacity (0=transparent)
 *  P        - palette (index)
 */
typedef struct gp_pixel_channel {
	char name[8];    /* Channel name */
	uint8_t offset;  /* Offset in bits */
	uint8_t size;    /* Bit-size */
} gp_pixel_channel;

/*
 * Maximum number of channels in a PixelType
 */
#define GP_PIXELTYPE_MAX_CHANNELS 8

/*
 * Pixel type flags for various pixel properties.
 */
typedef enum gp_pixel_flags {
	GP_PIXEL_HAS_ALPHA = 0x01,
	GP_PIXEL_IS_RGB = 0x02,
	GP_PIXEL_IS_PALETTE = 0x04,
	GP_PIXEL_IS_CMYK = 0x08,
	GP_PIXEL_IS_GRAYSCALE = 0x10,
} gp_pixel_flags;

/*
 * Description of one PixelType
 * Assumes name with at most 15 chars
 * Assumes at most 8 channels
 */
struct gp_pixel_type_desc {
	gp_pixel_type type;        /* Number of the type */
	const char name[16];      /* Name */
	uint8_t size;	          /* Size in bits */
	GP_BIT_ENDIAN bit_endian; /* Order of pixels in a byte */
	uint8_t numchannels;      /* Number of channels */
	gp_pixel_flags flags;
	/* String describing the bit-representaton (as in "RRRRRGGGGGGBBBBB")*/
	const char bitmap[GP_PIXEL_BITS + 1];
	/* Individual channels */
	const gp_pixel_channel channels[GP_PIXELTYPE_MAX_CHANNELS];
};

/*
 * Array of size GP_PIXEL_MAX describing known pixel types
 */
extern const gp_pixel_type_desc gp_pixel_types[GP_PIXEL_MAX];

#define GP_VALID_PIXELTYPE(type) (((type) > 0) && ((type) < GP_PIXEL_MAX))

#define GP_CHECK_VALID_PIXELTYPE(type) \
	GP_CHECK(GP_VALID_PIXELTYPE(type), "Invalid PixelType %d", (type))

/*
 * Convert pixel type to name.
 */
static inline const char *gp_pixel_type_name(gp_pixel_type type)
{
	GP_CHECK_VALID_PIXELTYPE(type);
	return gp_pixel_types[type].name;
}

/*
 * Returns number of bits per pixel.
 */
static inline uint32_t gp_pixel_size(gp_pixel_type type)
{
	GP_CHECK_VALID_PIXELTYPE(type);
	return gp_pixel_types[type].size;
}

static inline const gp_pixel_type_desc *gp_pixel_desc(gp_pixel_type type)
{
	GP_CHECK_VALID_PIXELTYPE(type);
	return &gp_pixel_types[type];
}

static inline unsigned int gp_pixel_channel_count(gp_pixel_type type)
{
	GP_CHECK_VALID_PIXELTYPE(type);
	return gp_pixel_types[type].numchannels;
}

static inline uint8_t gp_pixel_channel_bits(gp_pixel_type type, uint8_t channel)
{
	GP_CHECK_VALID_PIXELTYPE(type);
	return gp_pixel_types[type].channels[channel].size;
}

static inline const char *gp_pixel_channel_name(gp_pixel_type type,
                                                uint8_t channel)
{
	GP_CHECK_VALID_PIXELTYPE(type);
	return gp_pixel_types[type].channels[channel].name;
}

/*
 * Print a human-readable representation of a pixel value to a string.
 * Arguments as for snprintf().
 */
void gp_pixel_snprint(char *buf, size_t len, gp_pixel pixel, gp_pixel_type type);

/*
 * Prints human-readable representation of pixel value into the stdout.
 */
void gp_pixel_print(gp_pixel pixel, gp_pixel_type type);

/*
 * Returns pixel type for passed human-readable name (e.g. RGB888).
 */
gp_pixel_type gp_pixel_type_by_name(const char *name);

/*
 * Match pixel type to known pixel types.
 *
 * Returns either valid PixelType or GP_PIXEL_UNKNOWN
 */
gp_pixel_type gp_pixel_rgb_match(gp_pixel rmask, gp_pixel gmask,
                                 gp_pixel bmask, gp_pixel amask,
			         uint8_t bits_per_pixel);

/*
 * Similar to GP_PixelRGBMatch but works with offsets and sizes
 *
 * Returns either valid PixelType or GP_PIXEL_UNKNOWN
 */
gp_pixel_type gp_pixel_rgb_lookup(uint32_t rsize, uint32_t rof,
                                  uint32_t gsize, uint32_t goff,
			          uint32_t bsize, uint32_t boff,
			          uint32_t asize, uint32_t aoff,
			          uint8_t bits_per_pixel);

/*
 * Functions to determine pixel attributes.
 *
 * Call as:
 *
 * if (gp_pixel_has_flags(pixel_type, GP_PIXEL_IS_RGB | GP_PIXEL_HAS_ALPHA))
 *	...
 */
int gp_pixel_has_flags(gp_pixel_type pixel_type, gp_pixel_flags flags);

#endif /* CORE_GP_PIXEL_H */
