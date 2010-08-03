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

#include "GP_color.h"

/* WHITE, RED, BLUE, GREEN, BLACK */

static uint8_t rgb888_cols[][3] = {
	{0xff, 0xff, 0xff},
	{0xff, 0x00, 0x00},
	{0x00, 0xff, 0x00},
	{0x00, 0x00, 0xff},
	{0x00, 0x00, 0x00},
};

static uint8_t rgb555_cols[][3] = {
	{0x10, 0x10, 0x10},
	{0x10, 0x00, 0x00},
	{0x00, 0x10, 0x00},
	{0x00, 0x00, 0x10},
	{0x00, 0x00, 0x00},
};

static enum GP_RetCode conv_from_name(GP_Color *color, GP_ColorType type)
{
	enum GP_ColorName i = color->colname.name;

	if (i >= GP_COL_MAX)
		return GP_EINVAL;

	switch (type) {
		case GP_RGB888:
			GP_RGB888_FILL(color, rgb888_cols[i][0],
			               rgb888_cols[i][1], rgb888_cols[i][2]);
			return GP_ESUCCESS;
		break;
		case GP_RGBA8888:
			GP_RGBA8888_FILL(color, rgb888_cols[i][0],
			                 rgb888_cols[i][1], rgb888_cols[i][2],
					 0xff);
			return GP_ESUCCESS;
		break;
		case GP_RGB555:
			GP_RGB555_FILL(color, rgb555_cols[i][0], rgb555_cols[i][1],
			                      rgb555_cols[i][2]);
			return GP_ESUCCESS;
		break;
		case GP_COLNAME:
		case GP_COLMAX:
		break;
	}

	if (type >= GP_COLMAX)
		return GP_EINVAL;
	else
		return GP_ENOIMPL;
}

static enum GP_RetCode conv_from_rgb888(GP_Color *color, GP_ColorType type)
{
	struct GP_ColRGB888 *col = &color->rgb888;

	switch (type) {
		case GP_RGBA8888:
			GP_RGBA8888_FILL(color, col->red, col->green,
			                 col->blue, 0xff);
			return GP_ESUCCESS;
		break;
		case GP_RGB555:
			GP_RGB555_FILL(color, col->red / 8, col->green / 8,
			               col->blue / 8);
			return GP_ESUCCESS;
		break;
		
		case GP_COLMAX:
		case GP_COLNAME:
		case GP_RGB888:
		break;
	}

	if (type >= GP_COLMAX)
		return GP_EINVAL;
	else
		return GP_ENOIMPL;
}

static enum GP_RetCode conv_from_rgba8888(GP_Color *color, GP_ColorType type)
{
	struct GP_ColRGBA8888 *col = &color->rgba8888;

	switch (type) {
		case GP_RGB555:
			GP_RGB555_FILL(color, col->red / 8, col->green / 8,
			               col->blue / 8);
			return GP_EUNPRECISE;
		break;
		case GP_RGB888:
			GP_RGB888_FILL(color, col->red, col->green,
			               col->blue);
			return GP_EUNPRECISE;
		break;
		case GP_COLMAX:
		case GP_COLNAME:
		case GP_RGBA8888:
		break;
	}
	
	if (type >= GP_COLMAX)
		return GP_EINVAL;
	else
		return GP_ENOIMPL;
}

static enum GP_RetCode conv_from_rgb555(GP_Color *color, GP_ColorType type)
{
	struct GP_ColRGB555 *col = &color->rgb555;

	switch (type) {
		case GP_RGB888:
			GP_RGB888_FILL(color, col->red * 8, col->green * 8,
			               col->blue * 8);
			return GP_ESUCCESS;
		break;
		case GP_RGBA8888:
			GP_RGBA8888_FILL(color, col->red * 8, col->green * 8,
			                 col->blue * 8, 0xff);
			return GP_ESUCCESS;
		break;
		case GP_COLMAX:
		case GP_COLNAME:
		case GP_RGB555:
		break;
	}
	
	if (type >= GP_COLMAX)
		return GP_EINVAL;
	else
		return GP_ENOIMPL;
}

enum GP_RetCode GP_ColorConvert(GP_Color *color, GP_ColorType type)
{
	/* nothing to do */
	if (color->type == type)
		return GP_ESUCCESS;

	switch (color->type) {
		case GP_COLNAME:
			return conv_from_name(color, type);
		break;
		case GP_RGB888:
			return conv_from_rgb888(color, type);
		break;
		case GP_RGBA8888:
			return conv_from_rgba8888(color, type);
		break;
		case GP_RGB555:
			return conv_from_rgb555(color, type);
		break;
		case GP_COLMAX:
		break;
	}

	if (color->type >= GP_COLMAX)
		return GP_EINVAL;
	else
		return GP_ENOIMPL;
}
