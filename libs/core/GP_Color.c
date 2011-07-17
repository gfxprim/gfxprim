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

#include "GP_Convert.h"

#include "GP_Color.h"

#include <stdint.h>
#include <string.h>

static char *color_names[] = {
	"Black",
	"Red",
	"Green",
	"Blue",
	"Yellow",
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
	{0xa5, 0x2a, 0x2a}, /* Brown      */
	{0xff, 0xa5, 0x00}, /* Orange     */
	{0x7f, 0x7f, 0x7f}, /* DarkGray   */
	{0xbe, 0xbe, 0xbe}, /* LigthGray  */
	{0xa0, 0x20, 0xf0}, /* Purple     */
	{0xff, 0xff, 0xff}, /* White      */
};

GP_Pixel GP_ColorToPixel(GP_Context *context, GP_Color col)
{
	GP_ASSERT(col < GP_COL_MAX);
	GP_ASSERT(col >= 0);

	return GP_RGBToPixel(rgb888_colors[col][0],
	                     rgb888_colors[col][1],
			     rgb888_colors[col][2], context->pixel_type);
}

GP_Color GP_ColorNameToColor(const char *color_name)
{
	unsigned int i;

	for (i = 0; i < GP_COL_MAX; i++)
		if (!strcasecmp(color_name, color_names[i]))
			return i;

	return -1;
}

const char *GP_ColorToColorName(GP_Color col)
{
	if (col < 0 || col >= GP_COL_MAX)
		return NULL;

	return color_names[col];
}

GP_Pixel GP_ColorNameToPixel(GP_Context *context, const char *color_name)
{
	GP_Color col = GP_ColorNameToColor(color_name);

	if (col < 0)
		return 0;

	return GP_ColorToPixel(context, col);
}
