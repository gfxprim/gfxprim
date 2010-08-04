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

#include "GP_color.h"
#include "GP_palette.h"

enum GP_RetCode GP_PaletteColorToColor(GP_Color *color)
{
        struct GP_ColPal *pal;
	GP_Palette *palette;
	uint16_t index;

	if (color->type != GP_PALETTE)
		return GP_EINVAL;
	
	pal     = &color->pal;
	palette = pal->palette;
	index   = pal->index;

        switch (pal->palette->type) {
                case GP_RGB888:
                        if (index >= palette->rgb888.size)
                                return GP_EINVAL;
			
			GP_RGB888_FILL(color, palette->rgb888.colors[index].red,
			                      palette->rgb888.colors[index].green,
			                      palette->rgb888.colors[index].blue);
                        return GP_ESUCCESS;
                break;
                default:
                        return GP_ENOIMPL;
        }
}

static void print_rgb888(struct GP_PalRGB888 *palette)
{
	uint16_t i;

	printf("palette format rgb888 (size = %u)\n", palette->size);
	printf(" NR    R    G    B\n");

	for (i = 0; i < palette->size; i++) {
		printf("%04u: 0x%.2x 0x%.2x 0x%.2x\n", i,
		                                      palette->colors[i].red,
		                                      palette->colors[i].green,
		                                      palette->colors[i].blue);
	}
}

void GP_PalettePrint(GP_Palette *palette)
{
	switch (palette->type) {
		case GP_RGB888:
			print_rgb888(&palette->rgb888);
		break;
		default:
			break;
	}
}
