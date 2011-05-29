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

#ifndef GP_PIXEL_H
#define GP_PIXEL_H

#include <stdbool.h>
#include <stdint.h>

#include "GP_Color.h"
#include "GP_Common.h"
#include "GP_RetCode.h"

struct GP_Context;

/*
 *  GP_PixelType  is typedef enum of PixelTypes, 
 * 
 * each named GP_PIXEL_<TYPENAME>, such as GP_PIXEL_GRB888
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

/* Generated headers */
#include "GP_Pixel.gen.h"
#include "GP_Pixel_Scale.gen.h"

/*
 * Information about ordering of pixels in byte for 1, 2 and 4 bpp 
 * used in a one bit variable in GP_Context 
 */

typedef enum { 	
	/* less significant bits contain pixels with lower indices */ 
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

typedef struct {
  char name[8];           /* Channel name */
  uint8_t offset;             /* Offset in bits */
  uint8_t size;               /* Bit-size */
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

typedef struct {
  GP_PixelType type;        /* Number of the type */
  const char name[16];      /* Name */
  uint8_t size;	            /* Size in bits */
  GP_BIT_ENDIAN bit_endian; /* Order of pixels in a byte */
  uint8_t numchannels;      /* Number of channels */
  /* String describing the bit-representaton (as in "RRRRRGGGGGGBBBBB")*/
  const char bitmap[sizeof(GP_Pixel) * 8 + 1];  
  /* Individual channels */
  const GP_PixelTypeChannel channels[GP_PIXELTYPE_MAX_CHANNELS]; 
} GP_PixelTypeDescription;

/*
 * Array of size GP_PIXEL_MAX describing known pixel types
 */

extern const GP_PixelTypeDescription const GP_PixelTypes[];

/*
 * Convert pixel type to name.
 */

static inline const char *GP_PixelTypeName(GP_PixelType type)
{
  GP_CHECK(type < GP_PIXEL_MAX);
  return GP_PixelTypes[type].name;
}

/*
 * Returns number of bits per pixel.
 */

static inline uint32_t GP_PixelSize(GP_PixelType type)
{
  GP_CHECK(type < GP_PIXEL_MAX);
  return GP_PixelTypes[type].size;
}


/* Below -- TODO sync with new pixel type */

/*
 * Returns GP_PixelType to GP_ColorType mapping.
 */
//GP_ColorType GP_PixelTypeToColorType(GP_PixelType type);

/*
 * Converts a color to the specified pixel type.
 */
//GP_RetCode GP_ColorToPixelType(GP_PixelType pixel_type, GP_Color color, GP_Pixel *pixel);

/*
 * Converts a color to a pixel value suitable for the specified context.
 */
//GP_RetCode GP_ColorToPixel(struct GP_Context *context, GP_Color color, GP_Pixel *pixel);

/*
 *
 */
//GP_RetCode GP_ColorNameToPixel(struct GP_Context *context, GP_ColorName name, GP_Pixel *pixel);

/*
 * Converts a color name to the specified pixel type.
 */
//GP_RetCode GP_ColorNameToPixelType(GP_PixelType pixel_type, GP_ColorName name, GP_Pixel *pixel);

/*
 * Converts a color specified by its R, G, B components to a pixel value
 * compatible with the specified context.
 */
GP_RetCode GP_RGBToPixel(struct GP_Context *context, uint8_t r, uint8_t g, uint8_t b, GP_Pixel *pixel);

#endif /* GP_PIXEL_H */
