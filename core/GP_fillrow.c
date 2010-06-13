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

#include <stdio.h>

void GP_FillRow(struct GP_BufferInfo *buffer, struct GP_ClipInfo *clip,
		int row, int first_column, int last_column, uint32_t value)
{
	int min_row, max_row, min_column, max_column;
	GP_GetClipLimits(buffer, clip, &min_row, &max_row, &min_column,
			&max_column);

	/* check if we are not completely outside limits */
	if (row < min_row || row > max_row
		|| first_column > max_column || last_column < min_column) {
		return;
	}

	/* clip the column value */
	first_column = GP_MAX(first_column, min_column);
	last_column = GP_MIN(last_column, max_column);

	/* bail out if the coordinates are backwards */
	if (first_column > last_column) {
		return;
	}

	size_t column_count = 1 + last_column - first_column;

	/* Calculate the address of the start of the filled block */
	void *start = GP_PIXEL_ADDRESS(buffer, row, first_column);

	switch(buffer->bits_per_pixel) {
	case 32:
		GP_WritePixels32bpp(start, column_count, value);
		break;
	
	case 24:
		GP_WritePixels24bpp(start, column_count, value);
		break;
	
	case 16:
		GP_WritePixels16bpp(start, column_count, (uint16_t) value);
		break;
	
	case 8:
		GP_WritePixels8bpp(start, column_count, (uint8_t) value);
		break;
	}
}
