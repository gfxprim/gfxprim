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

#include "GP_RetCode.h"
#include "GP_Color.h"

typedef enum GP_PixelType {
	/* Palete */
	GP_PIXEL_PAL4,
	GP_PIXEL_PAL8,
	/* Grayscale */
	GP_PIXEL_G1,
	GP_PIXEL_G2,
	GP_PIXEL_G4,
	GP_PIXEL_G8,
	/* RGB 555 */
	GP_PIXEL_RGB555,
	GP_PIXEL_BGR555,
	/* RGB */
	GP_PIXEL_RGB888,
	GP_PIXEL_BGR888,
	/* RGB + padding */
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

typedef struct GP_Pixel {
	enum GP_PixelType type;
	uint32_t val;
} GP_Pixel;

/*
 * Convert pixel type to name.
 */
const char *GP_PixelTypeName(GP_PixelType type);

/*
 * Returns number of bits per pixel.
 */
uint32_t GP_PixelSize(GP_PixelType type);

/*
 * Returns true for exaclty same pixels.
 */
bool GP_PixelCmp(GP_Pixel *pixel1, GP_Pixel *pixel2);

/*
 * Returns GP_PixelType to GP_ColorType mapping.
 */
GP_ColorType GP_PixelTypeToColorType(GP_PixelType type);

/*
 * Fills pixel accodingly to color, pixel.type MUST be prefilled.
 */
GP_RetCode GP_ColorToPixel(GP_Color color, GP_Pixel *pixel);

/*
 * Converts GP_Pixel to GP_Color.
 */
GP_RetCode GP_PixelToColor(GP_Pixel pixel, GP_Color *color);

#endif /* GP_PIXEL_H */
