// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2009-2010 Jiri "BlueBear" Dluhos
 *                         <jiri.bluebear.dluhos@gmail.com>
 *
 * Copyright (C) 2009-2024 Cyril Hrubis <metan@ucw.cz>
 *
 * Copyright (C) 2011      Tomas Gavenciak <gavento@ucw.cz>
 */

/**
 * @file gp_pixel.h
 * @brief A pixel description.
 */

#ifndef CORE_GP_PIXEL_H
#define CORE_GP_PIXEL_H

#include <stdint.h>

#include <core/gp_types.h>
#include <core/gp_common.h>

#define GP_PIXEL_BITS (sizeof(gp_pixel) * 8)

#include <core/gp_pixel.gen.h>
#include <core/gp_pixel_alias.h>
#include <core/gp_get_set_bits.h>

/**
 * @brief Description of one pixel channel.
 *
 * Assumes all the channel names to be at most 7 chars long
 *
 * The common channel names are:
 *
 *  R, G, B     - RGB as usual
 *  C, M, Y, K  - CMYK
 *  V           - value, for grayscale
 *  A           - opacity (0=transparent)
 *  P           - palette (index)
 */
typedef struct gp_pixel_channel {
	/** Pixel channel name */
	char name[8];
	/** Channel offset in bits */
	uint8_t offset;
	/** Channel size in bits */
	uint8_t size;
} gp_pixel_channel;

/**
 * @brief Pixel type flags for various pixel properties.
 */
typedef enum gp_pixel_flags {
	/** @brief Pixel has an alpha channel */
	GP_PIXEL_HAS_ALPHA = 0x01,
	/** @brief Pixel has RGB channels */
	GP_PIXEL_IS_RGB = 0x02,
	/** @brief Pixel is palette */
	GP_PIXEL_IS_PALETTE = 0x04,
	/** @brief Pixel has CMYK channels */
	GP_PIXEL_IS_CMYK = 0x08,
	/** @brief Pixel is grayscale */
	GP_PIXEL_IS_GRAYSCALE = 0x10,
} gp_pixel_flags;

/**
 * @brief A description of a gp_pixel_type
 * Assumes name with at most 15 chars
 */
struct gp_pixel_type_desc {
	/** @brief An id of the pixel type */
	gp_pixel_type type;
	/** @brief A name e.g. xRGB8888 */
	const char name[16];
	/** @brief A pixel size in bits */
	uint8_t size;
	/** @brief A pixel packing */
	uint8_t pack;
	/** @brief A number of channels */
	uint8_t numchannels;
	/** @brief Bitwise or of gp_pixel_flags */
	gp_pixel_flags flags;
	/** @brief String describing the bit-representaton (as in "RRRRRGGGGGGBBBBB")*/
	const char bitmap[GP_PIXEL_BITS + 1];
	/** @brief An individual channel descriptions */
	const gp_pixel_channel channels[GP_PIXEL_CHANS_MAX];
};

/**
 * @brief Array with description for all pixel types.
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

/**
 * @brief Returns channel mask for a given pixel type and channel name.
 *
 * @pixel_type A pixel type.
 * @chan_name A channel name, e.g. "R" or "A".
 *
 * @return A channel mask or zero if channel is not present.
 */
gp_pixel gp_pixel_chan_mask(gp_pixel_type pixel_type, const char *chan_name);

/**
 * @brief Does per-channel saturated addition.
 *
 * The alpha channel is left untouched.
 *
 * @pixel_type A pixel type.
 * @pixel A pixel value.
 * @add An addition in percets of the maximal channel value.
 *
 * @return A pixel value after addition.
 */
gp_pixel gp_pixel_chans_add(gp_pixel_type pixel_type, gp_pixel pixel, int add);

#endif /* CORE_GP_PIXEL_H */
