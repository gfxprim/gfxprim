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

void GP_FillColumn(GP_Context *context, int column, int first_row, int last_row,
                   GP_Color color)
{
	GP_CHECK_CONTEXT(context);
	GP_RetCode ret;
	GP_Pixel pixel;

	/* handle swapped coordinates gracefully */
	if (first_row > last_row) {
		GP_FillColumn(context, column, last_row, first_row, color);
		return;
	}

	/* check if we are not completely outside the clipping rectangle */
	if (column < (int) context->clip_column_min
		|| column > (int) context->clip_column_max
		|| first_row > (int) context->clip_row_max
		|| last_row < (int) context->clip_row_min) {
		return;
	}
	
	/* clip the row value */
	first_row = GP_MAX(first_row, (int) context->clip_row_min);
	last_row = GP_MIN(last_row, (int) context->clip_row_max);

	size_t row_count = 1 + last_row - first_row;

	/* Calculate the address of the start of the filled block */
	uint8_t *p = (uint8_t *) GP_PIXEL_ADDRESS(context, first_row, column);
	
	/* Calculate pixel value from color */
	pixel.type = context->pixel_type;
	ret        = GP_ColorToPixel(color, &pixel);

	size_t i;
	switch(context->bits_per_pixel) {
	case 32:
		for (i = 0; i < row_count; i++, p += context->bytes_per_row)
			GP_WritePixel32bpp(p, pixel.val);
		break;

	case 24:
		for (i = 0; i < row_count; i++, p += context->bytes_per_row)
			GP_WritePixel24bpp(p, pixel.val);
		break;

	case 16:
		for (i = 0; i < row_count; i++, p += context->bytes_per_row)
			GP_WritePixel16bpp(p, pixel.val);
		break;

	case 8:
		for (i = 0; i < row_count; i++, p += context->bytes_per_row)
			GP_WritePixel8bpp(p, pixel.val);
		break;

	default:
		GP_ABORT("Unsupported value of context->bytes_per_row");
	}
}
