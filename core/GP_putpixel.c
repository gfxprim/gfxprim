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

void GP_PutPixel(struct GP_BufferInfo *buffer, struct GP_ClipInfo *clip,
		int x, int y, uint32_t value)
{
	int min_row, max_row, min_column, max_column;
	GP_GetClipLimits(buffer, clip, &min_row, &max_row, &min_column,
			&max_column);

	uint8_t *p;
	if (buffer->rows_are_vertical) {
		if (x < min_row || x > max_row
			|| y < min_column || y > max_column) {
			return;		/* clipped out */
		}
		p = GP_PIXEL_ADDRESS(buffer, x, y);
	} else {
		if (x < min_column || x > max_column
			|| y < min_row || y > max_row) {
			return;		/* clipped out */
		}
		p = GP_PIXEL_ADDRESS(buffer, y, x);
	}

	switch (buffer->bits_per_pixel) {
	case 32:
		GP_WritePixel32bpp(p, value);
		break;
	
	case 24:
		GP_WritePixel24bpp(p, value);
		break;
	
	case 16:
		GP_WritePixel16bpp(p, value);
		break;
	
	case 8:
		GP_WritePixel8bpp(p, value);
		break;
	}
}
