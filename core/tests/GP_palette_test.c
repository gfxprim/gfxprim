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

 /*

   Palette compile-time test.

  */

#include <GP.h>

struct GP_RGB888 palette_colors[] = {
	{0xff, 0x00, 0xff},
	{0x0a, 0xdf, 0xee},
	{0x00, 0x00, 0x00},
	{0xff, 0xff, 0xff},
};

int main(void)
{
	union GP_Palette palette = GP_PAL_RGB888_PACK(palette_colors);
	union GP_Color color = GP_PALINDEX_PACK(&palette, 1); 

	printf("Color #1\n");
	GP_ColorPrint(&color);
	
	GP_ColorConvert(&color, GP_RGB888);
	
	printf("\nColor #2\n");
	GP_ColorPrint(&color);
	
	GP_ColorConvert(&color, GP_RGBA8888);
	
	printf("\nColor #3\n");
	GP_ColorPrint(&color);
	
	GP_ColorConvert(&color, GP_RGB555);
	
	printf("\nColor #4\n");
	GP_ColorPrint(&color);
	
	GP_ColorConvert(&color, GP_G8);
	
	printf("\nColor #5\n");
	GP_ColorPrint(&color);

	printf("\nColor #6\n");
	GP_COLNAME_FILL(&color, GP_COL_YELLOW);
	GP_ColorPrint(&color);

	return 0;
}
