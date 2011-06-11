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
 * Copyright (C) 2011      Tomas Gavenciak <gavento@ucw.cz>                  *
 *                                                                           *
 *****************************************************************************/

#ifndef CORE_GP_CONVERT_H
#define CORE_GP_CONVERT_H

#include "GP_Common.h"
#include "GP_Context.h"

/* Generated headers */
#include "GP_Convert.gen.h"
#include "GP_Convert_Scale.gen.h"

/*
 * Generated function to convert RGB888 to any type.
 * Does not work with palette types.
 */
GP_Pixel GP_RGB888ToPixel(GP_Pixel pixel, GP_PixelType type);

/*
 * Generated function to convert RGBA8888 to any type.
 * Does not work with palette types.
 */
GP_Pixel GP_RGBA8888ToPixel(GP_Pixel pixel, GP_PixelType type);

/*
 * Generated function to convert to RGB888 from any type.
 * Does not work with palette types.
 */
GP_Pixel GP_PixelToRGB888(GP_Pixel pixel, GP_PixelType type);

/*
 * Generated function to convert to RGBA8888 from any type.
 * Does not work with palette types.
 */
GP_Pixel GP_PixelToRGBA8888(GP_Pixel pixel, GP_PixelType type);

/*
 * Converts a color specified by its R, G, B components to a specified type.
 */
static inline GP_Pixel GP_RGBToPixel(uint8_t r, uint8_t g, uint8_t b, GP_PixelType type)
{
	GP_Pixel p = GP_Pixel_CREATE_RGB888(r, g, b);
	return GP_RGB888ToPixel(p, type);
}

/*
 * Converts a color specified by its R, G, B, A components to a specified type.
 */
static inline GP_Pixel GP_RGBAToPixel(uint8_t r, uint8_t g, uint8_t b, uint8_t a, GP_PixelType type)
{
	GP_Pixel p = GP_Pixel_CREATE_RGBA8888(r, g, b, a);
	return GP_RGBA8888ToPixel(p, type);
}

/*
 * Converts a color specified by its R, G, B components to a pixel value
 * compatible with the specified context.
 */
static inline GP_Pixel GP_RGBToContextPixel(uint8_t r, uint8_t g, uint8_t b, 
					    const GP_Context *context)
{
	return GP_RGBToPixel(r, g, b, context->pixel_type);
}

/*
 * Converts a color specified by its R, G, B, A components to a pixel value
 * compatible with the specified context.
 */
static inline GP_Pixel GP_RGBAToContextPixel(uint8_t r, uint8_t g, uint8_t b, uint8_t a, const GP_Context *context)
{
	return GP_RGBAToPixel(r, g, b, a, context->pixel_type);
}

/*
 * Convert between any pixel types (excl. palette types) via RGBA8888
 */
static inline GP_Pixel GP_ConvertPixel(GP_Pixel pixel, GP_PixelType from, GP_PixelType to)
{
	return GP_RGBA8888ToPixel(GP_PixelToRGBA8888(from, pixel), to);
}
 
/*
 * Convert between pixel types of given contexts (excl. palette types) via RGBA8888.
 * 
 */
static inline GP_Pixel GP_ConvertContextPixel(GP_Pixel pixel, const GP_Context *from, const GP_Context *to)
{
	return GP_RGBA8888ToPixel(GP_PixelToRGBA8888(from->pixel_type, pixel), to->pixel_type);
}

#endif /* CORE_GP_CONVERT_H */
