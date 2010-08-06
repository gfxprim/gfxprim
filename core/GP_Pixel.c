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

#include "GP_Pixel.h"

static char *pixel_type_names[] = {
	"Palette 4bit",
	"Palette 8bit",
	"Grayscale 1bit",
	"Grayscale 2bits",
	"Grayscale 4bits",
	"Grayscale 8bit",
	"RGB 555",
	"BGR 555",
	"RGB 888",
	"BGR 888",
	"XRGB 8888",
	"RGBX 8888",
	"XBGR 8888",
	"BGRX 8888",
	"ARGB 8888",
	"RGBA 8888",
	"ABGR 8888",
	"BGRA 8888",
};

static GP_ColorType pixel_to_color_mapping[] = {
	GP_PAL4,
	GP_PAL8,
	GP_G1,
	GP_G2,
	GP_G4,
	GP_G8,
	GP_RGB555,
	GP_RGB555,
	/* RGB888 */
	GP_RGB888,
	GP_RGB888,
	/* RGB888 + padding */
	GP_RGB888,
	GP_RGB888,
	GP_RGB888,
	GP_RGB888,
	/* RGBA8888 */
	GP_RGBA8888,
	GP_RGBA8888,
	GP_RGBA8888,
	GP_RGBA8888,
};

static const uint32_t pixel_sizes[] = {
	4,
	8,
	1,
	2,
	4,
	8,
	15,
	15,
	/* RGB888 */
	24,
	24,
	/* RGB888 + padding */
	32,
	32,
	32,
	32,
	/* RGBA8888 */
	32,
	32,
	32,
	32,
};

const char *GP_PixelTypeName(GP_PixelType type)
{
	if (type >= GP_PIXEL_MAX)
		return "INVALID TYPE";

	return pixel_type_names[type];
}

uint32_t GP_PixelSize(GP_PixelType type)
{
	if (type >= GP_PIXEL_MAX)
		return 0;

	return pixel_sizes[type];
}

GP_ColorType GP_PixelTypeToColorType(GP_PixelType type)
{
	if (type >= GP_PIXEL_MAX)
		return GP_COLMAX;

	return pixel_to_color_mapping[type];
}

#define CHECK_RET(ret) if (ret != GP_ESUCCESS && ret != GP_EUNPRECISE) return ret;

GP_RetCode GP_ColorToPixel(GP_Color color, GP_PixelType pixel_type,
                           GP_Pixel *pixel)
{
	GP_RetCode ret;

	switch (pixel_type) {
		case GP_PIXEL_PAL4:
			ret = GP_ColorConvert(&color, GP_PAL4);
			CHECK_RET(ret);
			pixel->v8 = color.pal4.index;
			return ret;
		break;
		case GP_PIXEL_PAL8:
			ret = GP_ColorConvert(&color, GP_PAL8);
			CHECK_RET(ret);
			pixel->v8 = color.pal8.index;
			return ret;
		break;
		case GP_PIXEL_RGB888:
		case GP_PIXEL_XRGB8888:
			ret = GP_ColorConvert(&color, GP_RGB888);
			CHECK_RET(ret);
			pixel->v32 = color.rgb888.red   << 0x10 |
			             color.rgb888.green << 0x08 | 
				     color.rgb888.blue;
			return ret;
		break;
		case GP_PIXEL_BGR888:
		case GP_PIXEL_XBGR8888:
			ret = GP_ColorConvert(&color, GP_RGB888);
			CHECK_RET(ret);
			pixel->v32 = color.rgb888.red           |
			             color.rgb888.green << 0x08 | 
				     color.rgb888.blue  << 0x10;
			return ret;
		break;
		case GP_PIXEL_RGBX8888:
			ret = GP_ColorConvert(&color, GP_RGB888);
			CHECK_RET(ret);
			pixel->v32 = color.rgb888.red   << 0x18 |
			             color.rgb888.green << 0x10 | 
				     color.rgb888.blue  << 0x8;
			return ret;
		break;
		case GP_PIXEL_BGRX8888:
			ret = GP_ColorConvert(&color, GP_RGB888);
			CHECK_RET(ret);
			pixel->v32 = color.rgb888.red   << 0x08 |
			             color.rgb888.green << 0x10 | 
				     color.rgb888.blue  << 0x18;
			return ret;
		break;
		case GP_PIXEL_ARGB8888:
			ret = GP_ColorConvert(&color, GP_RGBA8888);
			CHECK_RET(ret);
			pixel->v32 = color.rgba8888.red   << 0x10 |
			             color.rgba8888.green << 0x08 |
				     color.rgba8888.blue          |
				     color.rgba8888.alpha << 0x18;
			return ret;
		break;
		case GP_PIXEL_RGBA8888:
			ret = GP_ColorConvert(&color, GP_RGBA8888);
			CHECK_RET(ret);
			pixel->v32 = color.rgba8888.red   << 0x18 |
			             color.rgba8888.green << 0x10 |
				     color.rgba8888.blue  << 0x08 |
				     color.rgba8888.alpha;
			return ret;
		break;
		case GP_PIXEL_ABGR8888:
			ret = GP_ColorConvert(&color, GP_RGBA8888);
			CHECK_RET(ret);
			pixel->v32 = color.rgba8888.red           |
			             color.rgba8888.green << 0x08 |
				     color.rgba8888.blue  << 0x10 |
				     color.rgba8888.alpha << 0x18;
			return ret;
		break;
		case GP_PIXEL_BGRA8888:
			ret = GP_ColorConvert(&color, GP_RGBA8888);
			CHECK_RET(ret);
			pixel->v32 = color.rgba8888.red   << 0x08 |
			             color.rgba8888.green << 0x10 |
				     color.rgba8888.blue  << 0x18 |
				     color.rgba8888.alpha;
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
