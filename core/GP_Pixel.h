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
#include "GP_RetCode.h"

struct GP_Context;

typedef enum GP_PixelType {
	/* Unknown pixel type */
	GP_PIXEL_UNKNOWN = 0,

	/* Palete */
	GP_PIXEL_PAL4_LE,
	GP_PIXEL_PAL4_BE,
	GP_PIXEL_PAL8,

	/* Grayscale */
	GP_PIXEL_G1_LE,
	GP_PIXEL_G1_BE,
	GP_PIXEL_G2_LE,
	GP_PIXEL_G2_BE,
	GP_PIXEL_G4_LE,
	GP_PIXEL_G4_BE,
	GP_PIXEL_G8,

	/* RGB 555 - 15 bits per pixel, 1 bit of padding */
	GP_PIXEL_RGB555,
	GP_PIXEL_BGR555,

	/* RGB 565 - 16 bits per pixel */
	GP_PIXEL_RGB565,
	GP_PIXEL_BGR565,

	/* RGB - 24bits per pixel */
	GP_PIXEL_RGB888,
	GP_PIXEL_BGR888,

	/* RGB + 32bits per pixel, 8 bits of padding */
	GP_PIXEL_XRGB8888,
	GP_PIXEL_RGBX8888,
	GP_PIXEL_XBGR8888,
	GP_PIXEL_BGRX8888,

	/* RGB + alpha */
	GP_PIXEL_ARGB8888,
	GP_PIXEL_RGBA8888,
	GP_PIXEL_ABGR8888,
	GP_PIXEL_BGRA8888,

	GP_PIXEL_MAX,
} GP_PixelType;

typedef uint32_t GP_Pixel;

/* information about ordering of pixels in byte for 1, 2 and 4 bpp */
/* used in a one bit variable in GP_Context */
typedef enum { 	
	/* less significant bits contain pixels with lower indices */ 
	GP_BIT_ENDIAN_LE = 0,
	/* more significant bits contain pixels with lower indices */ 
	GP_BIT_ENDIAN_BE,     
} GP_BIT_ENDIAN; 

/*
 * Convert pixel type to name.
 */
const char *GP_PixelTypeName(GP_PixelType type);

/*
 * Returns number of bits per pixel.
 */
uint32_t GP_PixelSize(GP_PixelType type);

/*
 * Returns GP_PixelType to GP_ColorType mapping.
 */
GP_ColorType GP_PixelTypeToColorType(GP_PixelType type);

/*
 * Converts a color to the specified pixel type.
 */
GP_RetCode GP_ColorToPixelType(GP_PixelType pixel_type, GP_Color color, GP_Pixel *pixel);

/*
 * Converts a color to a pixel value suitable for the specified context.
 */
GP_RetCode GP_ColorToPixel(struct GP_Context *context, GP_Color color, GP_Pixel *pixel);

/*
 *
 */
GP_RetCode GP_ColorNameToPixel(struct GP_Context *context, GP_ColorName name, GP_Pixel *pixel);

/*
 * Converts a color name to the specified pixel type.
 */
GP_RetCode GP_ColorNameToPixelType(GP_PixelType pixel_type, GP_ColorName name, GP_Pixel *pixel);

/*
 * Converts a color specified by its R, G, B components to a pixel value
 * compatible with the specified context.
 */
GP_RetCode GP_RGBToPixel(struct GP_Context *context, uint8_t r, uint8_t g, uint8_t b, GP_Pixel *pixel);

#endif /* GP_PIXEL_H */
