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

#include "GP.h"

struct PixelTypeInfo {
	const char *type_name;		/* human-readable name */
	GP_ColorType color_type;	/* color type used for this pixel type */
	unsigned int bits;		/* how many bits the pixel occupies */
};

/* This table shows relations of pixel types to color types, their names
 * and bits per pixel.
 */
static struct PixelTypeInfo pixel_type_infos[] = {
	{ "Unknown pixel type", GP_NOCOLOR,	0 },
	{ "Palette 4bit",	GP_PAL4,	4 },
	{ "Palette 8bit",	GP_PAL8,	8 },
	{ "Grayscale 1bit",	GP_G1,		1 },
	{ "Grayscale 2bits",	GP_G2,		2 },
	{ "Grayscale 4bits",	GP_G4,		4 },
	{ "Grayscale 8bits",	GP_G8,		8 },
	{ "RGB 555",		GP_RGB555,	16 },
	{ "BGR 555",		GP_RGB555,	16 },
	{ "RGB 565",		GP_RGB565,	16 },
	{ "BGR 565",		GP_RGB565,	16 },
	{ "RGB 888",		GP_RGB888,	24 },
	{ "BGR 888",		GP_RGB888,	24 },
	{ "XRGB 8888",		GP_RGB888,	32 },
	{ "RGBX 8888",		GP_RGB888,	32 },
	{ "XBGR 8888",		GP_RGB888,	32 },
	{ "BGRX 8888",		GP_RGB888,	32 },
	{ "ARGB 8888",		GP_RGBA8888,	32 },
	{ "RGBA 8888",		GP_RGBA8888,	32 },
	{ "ABGR 8888",		GP_RGBA8888,	32 },
	{ "BGRA 8888",		GP_RGBA8888,	32 }
};

const char *GP_PixelTypeName(GP_PixelType type)
{
	if (type >= GP_PIXEL_MAX)
		return "INVALID TYPE";

	return pixel_type_infos[type].type_name;
}

uint32_t GP_PixelSize(GP_PixelType type)
{
	if (type >= GP_PIXEL_MAX)
		return 0;

	return pixel_type_infos[type].bits;
}

bool GP_PixelCmp(GP_Pixel *pixel1, GP_Pixel *pixel2)
{
	if (pixel1->type != pixel2->type)
		return false;

	if (pixel1->val == pixel2->val)
		return true;

	return false;
}

GP_ColorType GP_PixelTypeToColorType(GP_PixelType type)
{
	if (type >= GP_PIXEL_MAX)
		return GP_COLMAX;

	return pixel_type_infos[type].color_type;
}

#define CHECK_RET(ret) if (ret != GP_ESUCCESS && ret != GP_EUNPRECISE) return ret;

GP_RetCode GP_ColorToPixelType(GP_PixelType pixel_type, GP_Color color, GP_Pixel *pixel)
{
	if (pixel == NULL)
		return GP_ENULLPTR;

	pixel->type = pixel_type;

	GP_RetCode ret;

	switch (pixel->type) {
		case GP_PIXEL_PAL4:
			ret = GP_ColorConvert(&color, GP_PAL4);
			CHECK_RET(ret);
			pixel->val = color.pal4.index;
			return ret;
		break;
		case GP_PIXEL_PAL8:
			ret = GP_ColorConvert(&color, GP_PAL8);
			CHECK_RET(ret);
			pixel->val = color.pal8.index;
			return ret;
		break;
		case GP_PIXEL_G1:
			ret = GP_ColorConvert(&color, GP_G1);
			CHECK_RET(ret);
			pixel->val = color.g1.gray;
			return ret;
		break;
		case GP_PIXEL_G2:
			ret = GP_ColorConvert(&color, GP_G2);
			CHECK_RET(ret);
			pixel->val = color.g2.gray;
			return ret;
		break;
		case GP_PIXEL_G4:
			ret = GP_ColorConvert(&color, GP_G4);
			CHECK_RET(ret);
			pixel->val = color.g4.gray;
			return ret;
		break;
		case GP_PIXEL_G8:
			ret = GP_ColorConvert(&color, GP_G8);
			CHECK_RET(ret);
			pixel->val = color.g8.gray;
			return ret;
		break;
		case GP_PIXEL_RGB888:
		case GP_PIXEL_XRGB8888:
			ret = GP_ColorConvert(&color, GP_RGB888);
			CHECK_RET(ret);
			pixel->val = color.rgb888.red   << 0x10 |
			             color.rgb888.green << 0x08 | 
				     color.rgb888.blue;
			return ret;
		break;
		case GP_PIXEL_BGR888:
		case GP_PIXEL_XBGR8888:
			ret = GP_ColorConvert(&color, GP_RGB888);
			CHECK_RET(ret);
			pixel->val = color.rgb888.red           |
			             color.rgb888.green << 0x08 | 
				     color.rgb888.blue  << 0x10;
			return ret;
		break;
		case GP_PIXEL_RGBX8888:
			ret = GP_ColorConvert(&color, GP_RGB888);
			CHECK_RET(ret);
			pixel->val = color.rgb888.red   << 0x18 |
			             color.rgb888.green << 0x10 | 
			             color.rgb888.blue  << 0x8;
			return ret;
		break;
		case GP_PIXEL_BGRX8888:
			ret = GP_ColorConvert(&color, GP_RGB888);
			CHECK_RET(ret);
			pixel->val = color.rgb888.red   << 0x08 |
			             color.rgb888.green << 0x10 | 
				     color.rgb888.blue  << 0x18;
			return ret;
		break;
		case GP_PIXEL_ARGB8888:
			ret = GP_ColorConvert(&color, GP_RGBA8888);
			CHECK_RET(ret);
			pixel->val = color.rgba8888.red   << 0x10 |
			             color.rgba8888.green << 0x08 |
		                     color.rgba8888.blue          |
			             color.rgba8888.alpha << 0x18;
			return ret;
		break;
		case GP_PIXEL_RGBA8888:
			ret = GP_ColorConvert(&color, GP_RGBA8888);
			CHECK_RET(ret);
			pixel->val = color.rgba8888.red   << 0x18 |
			             color.rgba8888.green << 0x10 |
		                     color.rgba8888.blue  << 0x08 |
			             color.rgba8888.alpha;
			return ret;
		break;
		case GP_PIXEL_ABGR8888:
			ret = GP_ColorConvert(&color, GP_RGBA8888);
			CHECK_RET(ret);
			pixel->val = color.rgba8888.red           |
			             color.rgba8888.green << 0x08 |
			             color.rgba8888.blue  << 0x10 |
				     color.rgba8888.alpha << 0x18;
			return ret;
		break;
		case GP_PIXEL_BGRA8888:
			ret = GP_ColorConvert(&color, GP_RGBA8888);
			CHECK_RET(ret);
			pixel->val = color.rgba8888.red   << 0x08 |
			             color.rgba8888.green << 0x10 |
			             color.rgba8888.blue  << 0x18 |
			             color.rgba8888.alpha;
			return ret;
		break;
		case GP_PIXEL_MAX:
		break;
	}

	if (pixel->type >= GP_PIXEL_MAX)
		return GP_EINVAL;
	else
		return GP_ENOIMPL;
}

GP_RetCode GP_ColorToPixel(GP_Context *context, GP_Color color, GP_Pixel *pixel)
{
	if (context == NULL || pixel == NULL)
		return GP_ENULLPTR;

	return GP_ColorToPixelType(context->pixel_type, color, pixel);
}

GP_RetCode GP_ColorNameToPixelType(GP_PixelType pixel_type, GP_ColorName name, GP_Pixel *pixel)
{
	if (pixel == NULL)
		return GP_ENULLPTR;
	
	GP_Color color = GP_COLNAME_PACK(name);
	return GP_ColorToPixelType(pixel_type, color, pixel);
}

GP_RetCode GP_ColorNameToPixel(GP_Context *context, GP_ColorName name, GP_Pixel *pixel)
{
	if (context == NULL || pixel == NULL)
		return GP_ENULLPTR;

	GP_Color color = GP_COLNAME_PACK(name);
	return GP_ColorToPixelType(context->pixel_type, color, pixel);
}

GP_RetCode GP_PixelToColor(GP_Pixel pixel, GP_Color *color)
{
	switch (pixel.type) {
		case GP_PIXEL_PAL4:
			GP_PAL4_FILL(color, pixel.val);
			return GP_ESUCCESS;
		break;
		case GP_PIXEL_PAL8:
			GP_PAL8_FILL(color, pixel.val);
			return GP_ESUCCESS;
		break;
		case GP_PIXEL_G1:
			GP_G1_FILL(color, pixel.val);
			return GP_ESUCCESS;
		break;
		case GP_PIXEL_G2:
			GP_G2_FILL(color, pixel.val);
			return GP_ESUCCESS;
		break;
		case GP_PIXEL_G4:
			GP_G4_FILL(color, pixel.val);
			return GP_ESUCCESS;
		break;
		case GP_PIXEL_G8:
			GP_G8_FILL(color, pixel.val);
			return GP_ESUCCESS;
		break;
		case GP_PIXEL_RGB888:
		case GP_PIXEL_XRGB8888:
			GP_RGB888_FILL(color, (pixel.val >> 0x10) & 0xff,
			                      (pixel.val >> 0x08) & 0xff,
			                      (pixel.val & 0xff));
			return GP_ESUCCESS;
		break;
		case GP_PIXEL_BGR888:
		case GP_PIXEL_XBGR8888:
			GP_RGB888_FILL(color, (pixel.val & 0xff),
			                      (pixel.val >> 0x08) & 0xff,
			                      (pixel.val >> 0x10) & 0xff);
			
			return GP_ESUCCESS;
		break;
		case GP_PIXEL_MAX:
		break;
	}
	
	if (pixel.type >= GP_PIXEL_MAX)
		return GP_ENOIMPL;
	else
		return GP_EINVAL;
}
