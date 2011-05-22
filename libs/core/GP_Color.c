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

#include <stdio.h>

#include "GP_Palette.h"
#include "GP_Color.h"

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

static const char *color_type_names[] = {
	"Invalid color"
	"ColorName",
	"Index to In Memory Palette",
	"HW Palete 4bit",
	"HW Palete 8bit",
	"Grayscale 1bit",
	"Grayscale 2bit",
	"Grayscale 4bit",
	"Grayscale 8bit",
	"RGB 555",
	"RGB 565",
	"RGB 666",
	"RGB 888",
	"RGBA 8888",
};

static enum GP_RetCode conv_from_name(GP_Color *color, GP_ColorType type)
{
	enum GP_ColorName i = color->name.name;

	if (i >= GP_COL_MAX)
		return GP_EINVAL;

	switch (type) {
		case GP_NOCOLOR:
			return GP_EINVAL;
		break;
		case GP_PAL4:
		case GP_PAL8:
			return GP_EINVAL;
		break;
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
		case GP_RGB565:
			GP_RGB565_FILL(color, rgb888_colors[i][0]>>3,
			                      rgb888_colors[i][1]>>2,
			                      rgb888_colors[i][2]>>3);
			return GP_ESUCCESS;
		break;
		case GP_RGB666:
			GP_RGB666_FILL(color, rgb888_colors[i][0]>>2,
			                      rgb888_colors[i][1]>>2,
			                      rgb888_colors[i][2]>>2);
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
		case GP_PALETTE:
		break;
	}

	if (type >= GP_COLMAX)
		return GP_EINVAL;
	else
		return GP_ENOIMPL;
}

const char *GP_ColorTypeName(GP_ColorType type)
{
	if (type >= GP_COLMAX)
		return "UNKNOWN";

	return color_type_names[type];
}

static enum GP_RetCode conv_from_g1(GP_Color *color, GP_ColorType type)
{
	struct GP_ColG1 *col = &color->g1;

	switch (type) {
		case GP_NOCOLOR:
			return GP_EINVAL;
		break;
		case GP_PAL4:
		case GP_PAL8:
			return GP_EINVAL;
		break;
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
		case GP_RGB565:
			GP_RGB565_FILL(color, col->gray * 0x1f,
			                      col->gray * 0x3f,
			                      col->gray * 0x1f);
			return GP_ESUCCESS;
		break;
		case GP_RGB666:
			GP_RGB666_FILL(color, col->gray * 0x3f,
			                      col->gray * 0x3f,
			                      col->gray * 0x3f);
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
		case GP_PALETTE:
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
		case GP_NOCOLOR:
			return GP_EINVAL;
		break;
		case GP_PAL4:
		case GP_PAL8:
			return GP_EINVAL;
		break;
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
		case GP_RGB565:
			GP_RGB565_FILL(color, col->gray * 0x1f / 0x03,
			                      col->gray * 0x3f / 0x03,
			                      col->gray * 0x1f / 0x03);
			return GP_ESUCCESS;
		break;
		case GP_RGB666:
			GP_RGB666_FILL(color, col->gray * 0x3f / 0x03,
			                      col->gray * 0x3f / 0x03,
			                      col->gray * 0x3f / 0x03);
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
		case GP_PALETTE:
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
		case GP_NOCOLOR:
			return GP_EINVAL;
		break;
		case GP_PAL4:
		case GP_PAL8:
			return GP_EINVAL;
		break;
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
		case GP_RGB565:
			GP_RGB565_FILL(color, col->gray * 0x1f / 0x0f,
			                      col->gray * 0x3f / 0x0f,
			                      col->gray * 0x1f / 0x0f);
			return GP_ESUCCESS;
		break;
		case GP_RGB666:
			GP_RGB666_FILL(color, col->gray * 0x3f / 0x0f,
			                      col->gray * 0x3f / 0x0f,
			                      col->gray * 0x3f / 0x0f);
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
		case GP_PALETTE:
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
		case GP_NOCOLOR:
			return GP_EINVAL;
		break;
		case GP_PAL4:
		case GP_PAL8:
			return GP_EINVAL;
		break;
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
		case GP_RGB565:
			GP_RGB565_FILL(color, col->gray * 0x1f / 0xff,
			                      col->gray * 0x3f / 0xff,
			                      col->gray * 0x1f / 0xff);
			return GP_ESUCCESS;
		break;
		case GP_RGB666:
			GP_RGB666_FILL(color, col->gray * 0x3f / 0xff,
			                      col->gray * 0x3f / 0xff,
			                      col->gray * 0x3f / 0xff);
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
		case GP_PALETTE:
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
		case GP_NOCOLOR:
			return GP_EINVAL;
		break;	
		case GP_PAL4:
		case GP_PAL8:
			return GP_EINVAL;
		break;
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
		case GP_RGB565:
			GP_RGB565_FILL(color, col->red, col->green << 1,
				col->blue);
			return GP_ESUCCESS;
		break;
		case GP_RGB666:
			GP_RGB666_FILL(color, 0x3f * col->red   / 0x1f,
			                      0x3f * col->green / 0x1f,
			                      0x3f * col->blue  / 0x1f);
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
		case GP_PALETTE:
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
		case GP_NOCOLOR:
			return GP_EINVAL;
		break;
		case GP_PAL4:
		case GP_PAL8:
			return GP_EINVAL;
		break;
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
			GP_G8_FILL(color, val);
			return GP_EUNPRECISE;
		break;
		case GP_RGB555:
			GP_RGB555_FILL(color, col->red>>3,
			                      col->green>>3,
			                      col->blue>>3);
			return GP_EUNPRECISE;
		break;
		case GP_RGB565:
			GP_RGB565_FILL(color, col->red>>3, col->green>>2,
				col->blue>>3);
			return GP_EUNPRECISE;
		break;
		case GP_RGB666:
			GP_RGB666_FILL(color, col->red>>2, col->green>>2,
				col->blue>>2);
			return GP_EUNPRECISE;
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
		case GP_PALETTE:
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
		case GP_NOCOLOR:
			return GP_EINVAL;
		break;
		case GP_PAL4:
		case GP_PAL8:
			return GP_EINVAL;
		break;
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
			GP_G8_FILL(color, val);
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
		case GP_PALETTE:
		break;
	}
	
	if (type >= GP_COLMAX)
		return GP_EINVAL;
	else
		return GP_ENOIMPL;
}

static enum GP_RetCode color_convert(GP_Color *color, GP_ColorType type)
{
	switch (color->type) {
		case GP_NOCOLOR:
			return GP_EINVAL;
		break;
		case GP_PAL4:
		case GP_PAL8:
			return GP_EINVAL;
		break;
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
		case GP_PALETTE:
		break;
	}

	if (color->type >= GP_COLMAX)
		return GP_EINVAL;
	else
		return GP_ENOIMPL;
}

GP_RetCode GP_ColorConvert(GP_Color *color, GP_ColorType type)
{
	if (!color)
		return GP_ENULLPTR;
	
	/* nothing to do */
	if (color->type == type)
		return GP_ESUCCESS;

	GP_RetCode ret;

	/* convert palette color to color */
	if (color->type == GP_PALETTE) {
		ret = GP_PaletteColorToColor(color);

		if (ret != GP_ESUCCESS)
			return ret;
	}

	/* convert color */
	return color_convert(color, type);
}

static void print_name(struct GP_ColName *color)
{
	printf(" TYPE    NAME\n");
	
	if (color->name > GP_COL_MAX) {
		printf("COLNAME invalid\n");
		return;
	}

	printf("COLNAME %s\n", color_names[color->name]);
}

static void print_palette(struct GP_ColPal *color)
{
	printf("Palette index %04u\n", color->index);
	GP_PalettePrint(color->palette);
}

static void print_pal4(struct GP_ColPal4 *color)
{
	printf("HW Palette index 0x%01u", color->index);
}

static void print_pal8(struct GP_ColPal8 *color)
{
	printf("HW Palette index 0x%02u", color->index);
}

static void print_g1(struct GP_ColG1 *color)
{
	printf(" TYPE   G\n");
	printf("  G1   0x%x\n", color->gray);
}

static void print_g2(struct GP_ColG2 *color)
{
	printf(" TYPE   G\n");
	printf("  G2   0x%x\n", color->gray);
}

static void print_g4(struct GP_ColG4 *color)
{
	printf(" TYPE   G\n");
	printf("  G4   0x%02x\n", color->gray);
}

static void print_g8(struct GP_ColG8 *color)
{
	printf(" TYPE   G\n");
	printf("  G8   0x%02x\n", color->gray);
}

static void print_rgb555(struct GP_ColRGB555 *color)
{
	printf(" TYPE   R    G    B\n");
	printf("RGB555 0x%02x 0x%02x 0x%02x\n", color->red,
	                                        color->green,
	                                        color->blue);
}

static void print_rgb565(struct GP_ColRGB565 *color)
{
	printf(" TYPE   R    G    B\n");
	printf("RGB565 0x%02x 0x%02x 0x%02x\n", color->red,
	                                        color->green,
	                                        color->blue);
}

static void print_rgb666(struct GP_ColRGB666 *color)
{
	printf(" TYPE   R    G    B\n");
	printf("RGB666 0x%02x 0x%02x 0x%02x\n", color->red,
	                                        color->green,
	                                        color->blue);
}
static void print_rgb888(struct GP_ColRGB888 *color)
{
	printf(" TYPE   R    G    B\n");
	printf("RGB888 0x%02x 0x%02x 0x%02x\n", color->red,
	                                        color->green,
	                                        color->blue);
}

static void print_rgba8888(struct GP_ColRGBA8888 *color)
{
	printf(" TYPE     R    G    B    A\n");
	printf("RGBA8888 0x%02x 0x%02x 0x%02x 0x%02x\n", color->red,
	                                                 color->green,
	                                                 color->blue,
	                                                 color->alpha);
}

void GP_ColorPrint(GP_Color *color)
{
	switch (color->type) {
		case GP_COLNAME:
			print_name(&color->name);
			return;
		break;
		case GP_PALETTE:
			print_palette(&color->pal);
			return;
		break;
		case GP_PAL4:
			print_pal4(&color->pal4);
			return;
		break;
		case GP_PAL8:
			print_pal8(&color->pal8);
		break;
		case GP_G1:
			print_g1(&color->g1);
			return;
		break;
		case GP_G2:
			print_g2(&color->g2);
			return;
		break;
		case GP_G4:
			print_g4(&color->g4);
			return;
		break;
		case GP_G8:
			print_g8(&color->g8);
			return;
		break;
		case GP_RGB555:
			print_rgb555(&color->rgb555);
			return;
		break;
		case GP_RGB565:
			print_rgb565(&color->rgb565);
			return;
		break;
		case GP_RGB666:
			print_rgb666(&color->rgb666);
			return;
		break;
		case GP_RGB888:
			print_rgb888(&color->rgb888);
			return;
		break;
		case GP_RGBA8888:
			print_rgba8888(&color->rgba8888);
			return;
		break;
		case GP_COLMAX:
		break;
	}

	printf("Invalid color type (%u)\n", color->type);
}
