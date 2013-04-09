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
 * Copyright (C) 2009-2013 Cyril Hrubis <metan@ucw.cz>                       *
 *                                                                           *
 * Copyright (C) 2011      Tomas Gavenciak <gavento@ucw.cz>                  *
 *                                                                           *
 *****************************************************************************/

#ifndef CORE_GP_PIXEL_H
#define CORE_GP_PIXEL_H

#include <stdbool.h>
#include <stdint.h>

#include "GP_Common.h"
#include "GP_FnPerBpp.h"

struct GP_Context;

/*
 *  GP_PixelType  is typedef enum of PixelTypes, 
 * 
 * each named GP_PIXEL_<TYPENAME>, such as GP_PIXEL_RGB888
 * see the beginning of GP_Pixel.gen.h for a complete list
 *
 * The type always contains GP_PIXEL_UNKNOWN = 0 and
 * GP_PIXEL_MAX as the last value (also the number of valid types)
 * The types are numbered to use the entire range 0 .. GP_PIXEL_MAX-1
 */

/*
 * GP_Pixel is just uint32_t
 */
typedef uint32_t GP_Pixel;
#define GP_PIXEL_BITS (sizeof(GP_Pixel) * 8)

/* Generated header */
#include "GP_Pixel.gen.h"
#include "GP_GetSetBits.h"

/*
 * Information about ordering of pixels in byte for 1, 2 and 4 bpp 
 * used in a one bit variable in GP_Context 
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
typedef struct GP_PixelTypeChannel {
	char name[8];    /* Channel name */
	uint8_t offset;  /* Offset in bits */
	uint8_t size;    /* Bit-size */
} GP_PixelTypeChannel;

/*
 * Maximum number of channels in a PixelType
 */
#define GP_PIXELTYPE_MAX_CHANNELS 8

/*
 * Description of one PixelType
 * Assumes name with at most 15 chars
 * Assumes at most 8 channels
 */
typedef struct GP_PixelTypeDescription {
	GP_PixelType type;        /* Number of the type */
	const char name[16];      /* Name */
	uint8_t size;	          /* Size in bits */
	GP_BIT_ENDIAN bit_endian; /* Order of pixels in a byte */
	uint8_t numchannels;      /* Number of channels */
	/* String describing the bit-representaton (as in "RRRRRGGGGGGBBBBB")*/
	const char bitmap[GP_PIXEL_BITS + 1];
	/* Individual channels */
	const GP_PixelTypeChannel channels[GP_PIXELTYPE_MAX_CHANNELS];
} GP_PixelTypeDescription;

/*
 * Array of size GP_PIXEL_MAX describing known pixel types
 */
extern const GP_PixelTypeDescription const GP_PixelTypes[GP_PIXEL_MAX];

#define GP_VALID_PIXELTYPE(type) (((type) > 0) && ((type) < GP_PIXEL_MAX))

#define GP_CHECK_VALID_PIXELTYPE(type) \
	GP_CHECK(GP_VALID_PIXELTYPE(type), "Invalid PixelType %d", (type))

/*
 * Convert pixel type to name.
 */
static inline const char *GP_PixelTypeName(GP_PixelType type)
{
	GP_CHECK_VALID_PIXELTYPE(type);
	return GP_PixelTypes[type].name;
}

/*
 * Returns number of bits per pixel.
 */
static inline uint32_t GP_PixelSize(GP_PixelType type)
{
	GP_CHECK_VALID_PIXELTYPE(type);
	return GP_PixelTypes[type].size;
}

static inline const GP_PixelTypeDescription *GP_PixelTypeDesc(GP_PixelType type)
{
	GP_CHECK_VALID_PIXELTYPE(type);
	return &GP_PixelTypes[type];
}

/*
 * Print a human-readable representation of a pixel value to a string.
 * Arguments as for snprintf().
 */
static inline void GP_PixelSNPrint(char *buf, size_t len, GP_Pixel pixel,
                                   GP_PixelType type)
{
	GP_FN_PER_PIXELTYPE(GP_PixelSNPrint, type, buf, len, pixel);
}

/*
 * "printf" out a human-readable representation of pixel value.
 */
static inline void GP_PixelPrint(GP_Pixel pixel, GP_PixelType type)
{
	char buf[256];
	GP_PixelSNPrint(buf, 256, pixel, type);
	printf("%s", buf);
}

/*
 * Returns pixel type for passed human-readable name (e.g. RGB888).
 */
GP_PixelType GP_PixelTypeByName(const char *name);

/*
 * Match pixel type to known pixel types.
 *
 * Returns either valid PixelType or GP_PIXEL_UNKNOWN 
 */
GP_PixelType GP_PixelRGBMatch(GP_Pixel rmask, GP_Pixel gmask,
                              GP_Pixel bmask, GP_Pixel amask,
			      uint8_t bits_per_pixel);

/*
 * Similar to GP_PixelRGBMatch but works with offsets and sizes
 *
 * Returns either valid PixelType or GP_PIXEL_UNKNOWN 
 */
GP_PixelType GP_PixelRGBLookup(uint32_t rsize, uint32_t roff,
                               uint32_t gsize, uint32_t goff,
			       uint32_t bsize, uint32_t boff,
			       uint32_t asize, uint32_t aoff,
			       uint8_t bits_per_pixel);

#endif /* CORE_GP_PIXEL_H */
