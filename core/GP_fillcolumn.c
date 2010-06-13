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

void GP_FillColumn(struct GP_BufferInfo *buffer, struct GP_ClipInfo *clip,
		int column, int first_row, int last_row, uint32_t value)
{
	int min_row, max_row, min_column, max_column;
	GP_GetClipLimits(buffer, clip, &min_row, &max_row,
			&min_column, &max_column);

	/* check if we are not completely outside limits */
	if (column < min_column || column > max_column
		|| first_row > max_row || last_row < min_row) {
		return;
	}

	/* clip the row value */
	first_row = GP_MAX(first_row, min_row);
	last_row = GP_MIN(last_row, max_row);

	/* bail out if the coordinates are backwards */
	if (first_row > last_row) {
		return;
	}

	size_t row_count = 1 + last_row - first_row;

	/* Calculate the address of the start of the filled block */
	uint8_t *p = (uint8_t *) GP_PIXEL_ADDRESS(buffer, first_row, column);

	size_t i;
	switch(buffer->bits_per_pixel) {
	case 32:
		for (i = 0; i < row_count; i++, p += buffer->bytes_per_row) {
			GP_WritePixel32bpp(p, value);
		}
		break;
	
	case 24:
		for (i = 0; i < row_count; i++, p += buffer->bytes_per_row) {
			GP_WritePixel24bpp(p, value);
		}
		break;
	
	case 16:
		for (i = 0; i < row_count; i++, p += buffer->bytes_per_row) {
			GP_WritePixel16bpp(p, (uint16_t) value);
		}
		break;
	
	case 8:
		for (i = 0; i < row_count; i++, p += buffer->bytes_per_row) {
			GP_WritePixel8bpp(p, (uint8_t) value);
		}
		break;
	}
}
