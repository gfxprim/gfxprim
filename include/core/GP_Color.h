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
 * Copyright (C) 2009-2011 Cyril Hrubis <metan@ucw.cz>                       *
 *                                                                           *
 *****************************************************************************/

 /*

   Color is enumeration of color names which may be converted into pixel values
   in desired pixel format. 

  */

#ifndef GP_COLOR_H
#define GP_COLOR_H

#include "GP_Context.h"
#include "GP_Pixel.h"

/*
 * Enumeration of color constants.
 */
typedef enum GP_Color {
	GP_COL_INVALID = -1,
	GP_COL_BLACK,
	GP_COL_RED,
	GP_COL_GREEN,
	GP_COL_BLUE,
	GP_COL_YELLOW,
	GP_COL_BROWN,
	GP_COL_ORANGE,
	GP_COL_GRAY_DARK,
	GP_COL_GRAY_LIGHT,
	GP_COL_PURPLE,
	GP_COL_WHITE,
	GP_COL_MAX,
} GP_Color;

/*
 * Converts GP_Color to GP_Pixel.
 */
GP_Pixel GP_ColorToPixel(GP_Color color, GP_PixelType pixel_type);

/*
 * Converts GP_Color to GP_Pixel. 
 */
static inline GP_Pixel GP_ColorToContextPixel(GP_Color color,
                                              GP_Context *context)
{
	return GP_ColorToPixel(color, context->pixel_type);
}

/*
 * Converts Color name (eg. string) to GP_Color.
 */
GP_Color GP_ColorNameToColor(const char *color_name);

/*
 * Converts GP_Color to color name.
 *
 * If color is not valid NULL is returned.
 */
const char *GP_ColorToColorName(GP_Color color);

/*
 * Converts Color name to Pixel.
 *
 * Returns true if conversion was successful false otherwise.
 */
bool GP_ColorNameToPixel(const char *color_name, GP_PixelType pixel_type,
                         GP_Pixel *pixel);

/*
 * Converts Color name to Pixel.
 * 
 * Returns true if conversion was successful false otherwise.
 */
static inline bool GP_ColorNameToContextPixel(const char *color_name,
                                              GP_Context *context,
                                              GP_Pixel *pixel)
{
	return GP_ColorNameToPixel(color_name, context->pixel_type, pixel);
}

/*
 * Loads all colors into array of GP_Pixel of size GP_COL_MAX.
 *
 * The colors are then used as pixels[GP_COL_XXX];
 */
void GP_ColorLoadPixels(GP_Pixel pixels[], GP_PixelType pixel_type);

/*
 * Loads all colors into array of GP_Pixel of size GP_COL_MAX.
 *
 * The colors are then used as pixels[GP_COL_XXX];
 */
static inline void GP_ColorLoadContextPixels(GP_Pixel pixels[],
                                             GP_Context *context)
{
	GP_ColorLoadPixels(pixels, context->pixel_type);
}

#endif /* GP_COLOR_H */
