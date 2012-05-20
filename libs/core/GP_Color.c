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

#include <stdint.h>
#include <string.h>

#include "GP_Convert.h"

#include "GP_Color.h"


static char *color_names[] = {
	"Black",
	"Red",
	"Green",
	"Blue",
	"Yellow",
	"Cyan",
	"Magenta",
	"Brown",
	"Orange",
	"DarkGray",
	"LightGray",
	"Purple",
	"White"
};

static uint8_t rgb888_colors[][3] = {
	{0x00, 0x00, 0x00}, /* Black      */
	{0xff, 0x00, 0x00}, /* Red        */
	{0x00, 0xff, 0x00}, /* Green      */
	{0x00, 0x00, 0xff}, /* Blue       */
	{0xff, 0xff, 0x00}, /* Yellow     */
	{0x00, 0xff, 0xff}, /* Cyan       */
	{0xff, 0x00, 0xff}, /* Magenta    */
	{0xa5, 0x2a, 0x2a}, /* Brown      */
	{0xff, 0xa5, 0x00}, /* Orange     */
	{0x7f, 0x7f, 0x7f}, /* DarkGray   */
	{0xbe, 0xbe, 0xbe}, /* LightGray  */
	{0xa0, 0x20, 0xf0}, /* Purple     */
	{0xff, 0xff, 0xff}, /* White      */
};

/* 3-2-3 RGB palette */
static uint8_t p8_colors[] = {
	0x00, /* Black     */
	0xe0, /* Red       */
	0x1c, /* Green     */
	0x03, /* Blue      */
        0xfc, /* Yellow    */
	0x1f, /* Cyan      */
	0xe7, /* Magenta   */
	0x88, /* Brown     */
	0xf0, /* Orange    */
	0x49, /* DarkGray  */
	0x92, /* LightGray */
	0x8a, /* Purple    */
	0xff, /* White     */
};

GP_Pixel GP_ColorToPixel(GP_Color color, GP_PixelType pixel_type)
{
	GP_ASSERT(color < GP_COL_MAX);
	GP_ASSERT(color >= 0);

	if (pixel_type == GP_PIXEL_P8)
		return p8_colors[color];

	return GP_RGBToPixel(rgb888_colors[color][0],
	                     rgb888_colors[color][1],
			     rgb888_colors[color][2],
			     pixel_type);
}

GP_Color GP_ColorNameToColor(const char *color_name)
{
	unsigned int i;

	for (i = 0; i < GP_COL_MAX; i++)
		if (!strcasecmp(color_name, color_names[i]))
			return i;

	return -1;
}

const char *GP_ColorToColorName(GP_Color color)
{
	if (color < 0 || color >= GP_COL_MAX)
		return NULL;

	return color_names[color];
}

bool GP_ColorNameToPixel(const char *color_name, GP_PixelType pixel_type,
                         GP_Pixel *pixel)
{
	GP_Color color = GP_ColorNameToColor(color_name);

	if (color == GP_COL_INVALID)
		return false;

	*pixel = GP_ColorToPixel(pixel_type, color);

	return true;
}

void GP_ColorLoadPixels(GP_Pixel pixels[], GP_PixelType pixel_type)
{
	unsigned int i;

	for (i = 0; i < GP_COL_MAX; i++)
		pixels[i] = GP_ColorToPixel(i, pixel_type);
}
