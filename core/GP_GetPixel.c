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

uint32_t GP_GetPixel(GP_Context *context, int x, int y)
{
	GP_CHECK_CONTEXT(context);

	uint8_t *p;
	if (context->vertical_rows) {
		if (x < (int) context->clip_row_min
			|| x > (int) context->clip_row_max
			|| y < (int) context->clip_column_min
			|| y > (int) context->clip_column_max) {
			return 0;			/* clipped out */
		}
		p = GP_PIXEL_ADDRESS(context, x, y);
	} else {
		if (x < (int) context->clip_column_min
			|| x > (int) context->clip_column_max
			|| y < (int) context->clip_row_min
			|| y > (int) context->clip_row_max) {
			return 0;			/* clipped out */
		}
		p = GP_PIXEL_ADDRESS(context, y, x);
	}

	switch (context->bits_per_pixel) {
	case 32:
		return GP_ReadPixel32bpp(p);
	
	case 24:
		return GP_ReadPixel24bpp(p);
	
	case 16:
		return GP_ReadPixel16bpp(p);
	
	case 8:
		return GP_ReadPixel8bpp(p);
	
	default:
		GP_ABORT("Unsupported value of context->bits_per_pixel");
	}

	return 0; /* unreached */
}
