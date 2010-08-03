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

static uint8_t rgb888_colors[][3] = {
	{0x00, 0x00, 0x00}, /* black  */
	{0xff, 0x00, 0x00}, /* red    */
	{0x00, 0xff, 0x00}, /* green  */
	{0x00, 0x00, 0xff}, /* blue   */
	{0xff, 0xff, 0x00}, /* yellow */
	{0xa5, 0x2a, 0x2a}, /* brown  */
	{0xff, 0xa5, 0x00}, /* orange */
	{0xbe, 0xbe, 0xbe}, /* gray1  */
	{0x7f, 0x7f, 0x7f}, /* gray2  */
	{0xa0, 0x20, 0xf0}, /* purple */
	{0xff, 0xff, 0xff}, /* white */
};

static enum GP_RetCode conv_from_name(GP_Color *color, GP_ColorType type)
{
	enum GP_ColorName i = color->colname.name;

	if (i >= GP_COL_MAX)
		return GP_EINVAL;

	switch (type) {
		case GP_G1:
		break;
		case GP_G2:
		break;
		case GP_G4:
			GP_G4_FILL(color, ((rgb888_colors[i][0] +
			                    rgb888_colors[i][1] +
			                    rgb888_colors[i][2])/3)>>4);
			return GP_EUNPRECISE;
		break;
		case GP_G8:
			GP_G8_FILL(color, (rgb888_colors[i][0] +
			                   rgb888_colors[i][1] +
			                   rgb888_colors[i][2])/3);
			return GP_EUNPRECISE;
		break;
		case GP_RGB555:
			GP_RGB555_FILL(color, rgb888_colors[i][0]>>3,
			                      rgb888_colors[i][1]>>3,
			                      rgb888_colors[i][2]>>3);
			return GP_ESUCCESS;
		break;
		case GP_RGB888:
			GP_RGB888_FILL(color, rgb888_colors[i][0],
			                      rgb888_colors[i][1],
			                      rgb888_colors[i][2]);
			return GP_ESUCCESS;
		break;
		case GP_RGBA8888:
			GP_RGBA8888_FILL(color, rgb888_colors[i][0],
			                        rgb888_colors[i][1],
			                        rgb888_colors[i][2], 0xff);
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

static enum GP_RetCode conv_from_g1(GP_Color *color, GP_ColorType type)
{
	struct GP_ColG1 *col = &color->g1;

	switch (type) {
		case GP_G1:
			return GP_ESUCCESS;
		break;
		case GP_G2:
			GP_G2_FILL(color, col->gray * 0x03);
			return GP_ESUCCESS;
		break;
		case GP_G4:
			GP_G4_FILL(color, col->gray * 0x0f);
			return GP_ESUCCESS;
		break;
		case GP_G8:
			GP_G8_FILL(color, col->gray * 0xff);
			return GP_ESUCCESS;
		break;
		case GP_RGB555:
			GP_RGB555_FILL(color, col->gray * 0x1f,
			                      col->gray * 0x1f,
			                      col->gray * 0x1f);
			return GP_ESUCCESS;
		break;
		case GP_RGB888:
			GP_RGB888_FILL(color, col->gray * 0xff,
			                      col->gray * 0xff,
			                      col->gray * 0xff);
			return GP_ESUCCESS;
		break;
		case GP_RGBA8888:
			GP_RGBA8888_FILL(color, col->gray * 0xff,
			                        col->gray * 0xff,
			                        col->gray * 0xff, 0xff);
			return GP_ESUCCESS;
		break;
		case GP_COLMAX:
		case GP_COLNAME:
		break;
	}
	
	if (type >= GP_COLMAX)
		return GP_EINVAL;
	else
		return GP_ENOIMPL;
}

static enum GP_RetCode conv_from_g2(GP_Color *color, GP_ColorType type)
{
	struct GP_ColG2 *col = &color->g2;

	switch (type) {
		case GP_G1:
			GP_G1_FILL(color, col->gray>>2);
			return GP_EUNPRECISE;
		break;
		case GP_G2:
			return GP_ESUCCESS;
		break;
		case GP_G4:
			GP_G4_FILL(color, col->gray * 0x0f / 0x03);
			return GP_ESUCCESS;
		break;
		case GP_G8:
			GP_G8_FILL(color, col->gray * 0xff / 0x03);
			return GP_ESUCCESS;
		break;
		case GP_RGB555:
			GP_RGB555_FILL(color, col->gray * 0x1f / 0x03,
			                      col->gray * 0x1f / 0x03,
			                      col->gray * 0x1f / 0x03);
			return GP_ESUCCESS;
		break;
		case GP_RGB888:
			GP_RGB888_FILL(color, col->gray * 0xff / 0x03, 
			                      col->gray * 0xff / 0x03,
			                      col->gray * 0xff / 0x03);
			return GP_ESUCCESS;
		break;
		case GP_RGBA8888:
			GP_RGBA8888_FILL(color, col->gray * 0xff / 0x03,
			                        col->gray * 0xff / 0x03,
			                        col->gray * 0xff / 0x03, 0xff);
			return GP_ESUCCESS;
		break;
		case GP_COLMAX:
		case GP_COLNAME:
		break;
	}
	
	if (type >= GP_COLMAX)
		return GP_EINVAL;
	else
		return GP_ENOIMPL;
}

static enum GP_RetCode conv_from_g4(GP_Color *color, GP_ColorType type)
{
	struct GP_ColG4 *col = &color->g4;

	switch (type) {
		case GP_G1:
			GP_G1_FILL(color, col->gray>>3);
			return GP_EUNPRECISE;
		break;
		case GP_G2:
			GP_G2_FILL(color, col->gray>>2);
			return GP_EUNPRECISE;
		break;
		case GP_G4:
			return GP_ESUCCESS;
		break;
		case GP_G8:
			GP_G8_FILL(color, col->gray * 0xff / 0x0f);
			return GP_ESUCCESS;
		break;
		case GP_RGB555:
			GP_RGB555_FILL(color, col->gray * 0x1f / 0x0f,
			                      col->gray * 0x1f / 0x0f,
			                      col->gray * 0x1f / 0x0f);
			return GP_ESUCCESS;
		break;
		case GP_RGB888:
			GP_RGB888_FILL(color, col->gray * 0xff / 0x0f, 
			                      col->gray * 0xff / 0x0f,
			                      col->gray * 0xff / 0x0f);
			return GP_ESUCCESS;
		break;
		case GP_RGBA8888:
			GP_RGBA8888_FILL(color, col->gray * 0xff / 0x0f,
			                        col->gray * 0xff / 0x0f,
			                        col->gray * 0xff / 0x0f, 0xff);
			return GP_ESUCCESS;
		break;
		case GP_COLMAX:
		case GP_COLNAME:
		break;
	}
	
	if (type >= GP_COLMAX)
		return GP_EINVAL;
	else
		return GP_ENOIMPL;
}

static enum GP_RetCode conv_from_g8(GP_Color *color, GP_ColorType type)
{
	struct GP_ColG8 *col = &color->g8;

	switch (type) {
		case GP_G1:
			GP_G1_FILL(color, col->gray / 8);
			return GP_EUNPRECISE;
		break;
		case GP_G2:
			GP_G2_FILL(color, col->gray / 4);
			return GP_EUNPRECISE;
		break;
		case GP_G4:
			GP_G4_FILL(color, col->gray / 2);
			return GP_EUNPRECISE;
		break;
		case GP_G8:
			return GP_ESUCCESS;
		break;
		case GP_RGB555:
			GP_RGB555_FILL(color, col->gray * 0x1f / 0xff,
			                      col->gray * 0x1f / 0xff,
			                      col->gray * 0x1f / 0xff);
			return GP_ESUCCESS;
		break;
		case GP_RGB888:
			GP_RGB888_FILL(color, col->gray, 
			                      col->gray,
			                      col->gray);
			return GP_ESUCCESS;
		break;
		case GP_RGBA8888:
			GP_RGBA8888_FILL(color, col->gray,
			                        col->gray,
			                        col->gray, 0xff);
			return GP_ESUCCESS;
		break;
		case GP_COLMAX:
		case GP_COLNAME:
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
	uint8_t val;

	switch (type) {
		case GP_G1:
			val = ((col->red + col->green + col->blue) / 3)>>4;
			GP_G1_FILL(color, val);
			return GP_EUNPRECISE;
		break;
		case GP_G2:
			val = ((col->red + col->green + col->blue) / 3)>>3;
			GP_G2_FILL(color, val);
			return GP_EUNPRECISE;
		break;
		case GP_G4:
			val = ((col->red + col->green + col->blue) / 3)>>2;
			GP_G4_FILL(color, val);
			return GP_EUNPRECISE;
		break;
		case GP_G8:
			val = (0xff * (col->red + col->green + col->blue)) /
			      (3 * 0x1f);
			GP_G8_FILL(color, val);
			return GP_EUNPRECISE;
		break;
		case GP_RGB555:
			return GP_ESUCCESS;
		break;
		case GP_RGB888:
			GP_RGB888_FILL(color, 0xff * col->red   / 0x1f,
			                      0xff * col->green / 0x1f,
			                      0xff * col->blue  / 0x1f);
			return GP_ESUCCESS;
		break;
		case GP_RGBA8888:
			GP_RGBA8888_FILL(color, 0xff * col->red   / 0x1f,
			                        0xff * col->green / 0x1f,
			                        0xff * col->blue  / 0x1f, 0xff);
			return GP_ESUCCESS;
		break;
		case GP_COLMAX:
		case GP_COLNAME:
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
	uint8_t val;

	switch (type) {
		case GP_G1:
			val = ((col->red + col->green + col->blue)/3)>>7;
			GP_G1_FILL(color, val);
			return GP_EUNPRECISE;
		break;
		case GP_G2:
			val = ((col->red + col->green + col->blue)/3)>>6;
			GP_G2_FILL(color, val);
			return GP_EUNPRECISE;
		break;
		case GP_G4:
			val = ((col->red + col->green + col->blue)/3)>>4;
			GP_G4_FILL(color, val);
			return GP_EUNPRECISE;
		break;
		case GP_G8:

		break;
		case GP_RGB555:
			GP_RGB555_FILL(color, col->red>>3,
			                      col->green>>3,
			                      col->blue>>3);
			return GP_ESUCCESS;
		break;
		case GP_RGB888:
			return GP_ESUCCESS;
		break;
		case GP_RGBA8888:
			GP_RGBA8888_FILL(color, col->red, col->green,
			                 col->blue, 0xff);
			return GP_ESUCCESS;
		break;
		
		case GP_COLMAX:
		case GP_COLNAME:
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
	uint8_t val;

	switch (type) {
		case GP_G1:
			val = ((col->red + col->green + col->blue)/3)>>7;
			GP_G1_FILL(color, val);
			return GP_EUNPRECISE;
		break;
		case GP_G2:
			val = ((col->red + col->green + col->blue)/3)>>6;
			GP_G2_FILL(color, val);
			return GP_EUNPRECISE;
		break;
		case GP_G4:
			val = ((col->red + col->green + col->blue)/3)>>4;
			GP_G4_FILL(color, val);
			return GP_EUNPRECISE;
		break;
		case GP_G8:
			val = ((col->red + col->green + col->blue)/3);
			GP_G4_FILL(color, val);
			return GP_EUNPRECISE;
		break;
		case GP_RGB555:
			GP_RGB555_FILL(color, col->red>>3,
			                      col->green>>3,
			                      col->blue>>3);
			return GP_EUNPRECISE;
		break;
		case GP_RGB888:
			GP_RGB888_FILL(color, col->red,
			                      col->green,
			                      col->blue);
			return GP_EUNPRECISE;
		break;
		case GP_RGBA8888:
			return GP_ESUCCESS;
		break;
		case GP_COLMAX:
		case GP_COLNAME:
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
		case GP_G1:
			return conv_from_g1(color, type);
		break;
		case GP_G2:
			return conv_from_g2(color, type);
		break;
		case GP_G4:
			return conv_from_g4(color, type);
		break;
		case GP_G8:
			return conv_from_g8(color, type);
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
