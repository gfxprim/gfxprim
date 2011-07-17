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

static char *color_names[] = {
	"black     ",
	"red       ",
	"green     ",
	"blue      ",
	"yellow    ",
	"brown     ",
	"orange    ",
	"gray dark ",
	"gray light",
	"purple    ",
	"white     "
};

static uint8_t rgb888_colors[][3] = {
	{0x00, 0x00, 0x00}, /* black      */
	{0xff, 0x00, 0x00}, /* red        */
	{0x00, 0xff, 0x00}, /* green      */
	{0x00, 0x00, 0xff}, /* blue       */
	{0xff, 0xff, 0x00}, /* yellow     */
	{0xa5, 0x2a, 0x2a}, /* brown      */
	{0xff, 0xa5, 0x00}, /* orange     */
	{0x7f, 0x7f, 0x7f}, /* gray dark  */
	{0xbe, 0xbe, 0xbe}, /* gray light */
	{0xa0, 0x20, 0xf0}, /* purple     */
	{0xff, 0xff, 0xff}, /* white      */
};

GP_Pixel GP_ColorNameToPixel(GP_Context *context, GP_ColorName name)
{
	GP_ASSERT(name < GP_COL_MAX);

	return GP_RGBToPixel(rgb888_colors[name][0],
	                     rgb888_colors[name][1],
			     rgb888_colors[name][2], context->pixel_type);
}
