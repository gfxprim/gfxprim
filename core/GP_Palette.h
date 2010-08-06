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

#ifndef GP_PALETTE_H
#define GP_PALETTE_H

#include <stdint.h>

#include "GP_RetCode.h"
#include "GP_Color.h"

/*
 * Usage:
 *
 * struct GP_RGB888 pal_cols[] = {
 *	{0xff, 0x00, 0xff},
 *	...
 * };
 *
 * GP_Palette my_palette = GP_PAL_RGB888_PACK(pal_cols);
 */
#define GP_PAL_RGB888_PACK(cols) {.rgb888 = {GP_RGB888,                     \
                                  sizeof (cols) / sizeof (struct GP_RGB888),\
                                  cols}}

struct GP_RGB888 {
	uint8_t red;
	uint8_t green;
	uint8_t blue;
};

struct GP_PalRGB888 {
	enum GP_ColorType type;
	uint16_t size;
	struct GP_RGB888 *colors;
};

typedef union GP_Palette {
	enum GP_ColorType type;
	struct GP_PalRGB888 rgb888;
} GP_Palette;

/*
 * Converts palette color to direct color.
 */
enum GP_RetCode GP_PaletteColorToColor(GP_Color *color);

/*
 * Print palette into stdout in human-readable format.
 */
void GP_PalettePrint(GP_Palette *palette);

#endif /* GP_PALETTE_H */
