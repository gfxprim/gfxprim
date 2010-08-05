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

#include "GP_pixel.h"

static GP_ColorType pixel_to_color_mapping[] = {
	GP_G1,
	GP_G2,
	GP_G4,
	GP_G8,
	GP_RGB555,
	GP_RGB555,
	GP_RGB888,
	GP_RGB888,
	GP_RGB888,
	GP_RGB888,
	GP_RGB888,
	GP_RGB888,
	GP_RGBA8888,
	GP_RGBA8888,
	GP_RGBA8888,
	GP_RGBA8888,
};

GP_ColorType GP_PixelTypeToColorType(GP_PixelType type)
{
	if (type >= GP_PIXEL_MAX)
		return GP_COLMAX;

	return pixel_to_color_mapping[type];
}

GP_RetCode GP_ColorToPixel(GP_Color color, GP_PixelType pixel_type,
                           GP_Pixel *pixel)
{
	GP_RetCode ret;

	switch (pixel_type) {
		case GP_PIXEL_XRGB8888:
			ret = GP_ColorConvert(&color, GP_RGB888);
			pixel->v32 = color.rgb888.red   << 0x10 |
			             color.rgb888.green << 0x08 | 
				     color.rgb888.blue;
			return ret;
		break;
		case GP_PIXEL_MAX:
		break;
	}

	if (pixel_type >= GP_PIXEL_MAX)
		return GP_EINVAL;
	else
		return GP_ENOIMPL;
}
