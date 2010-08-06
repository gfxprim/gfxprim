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

void GP_FillRow(GP_Context *context,
		int row, int first_column, int last_column, uint32_t value)
{
	GP_CHECK_CONTEXT(context);

	/* Handle swapped column indexes gracefully. */
	if (first_column > last_column) {
		GP_FillRow(context, row, last_column, first_column, value);
		return;
	}

	/* check if we are not completely outside limits */
	if (row < (int) context->clip_row_min
		|| row > (int) context->clip_row_max
		|| first_column > (int) context->clip_column_max
		|| last_column < (int) context->clip_column_min) {
		return;
	}

	/* clip the column value */
	first_column = GP_MAX(first_column, (int) context->clip_column_min);
	last_column = GP_MIN(last_column, (int) context->clip_column_max);

	size_t column_count = 1 + last_column - first_column;

	/* Calculate the address of the start of the filled block */
	void *start = GP_PIXEL_ADDRESS(context, row, first_column);

	switch(context->bits_per_pixel) {
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
	
	default:
		GP_ABORT("Unsupported value of context->bits_per_pixel");
	}
}
