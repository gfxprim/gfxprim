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

GP_RetCode GP_VLine(GP_Context *context, int x, int y0, int y1, GP_Color color)
{
	if (!context)
		return GP_ENULLPTR;
	if (!GP_IS_CONTEXT_VALID(context))
		return GP_EBADCONTEXT;

	/* handle swapped coordinates gracefully */
	if (y0 > y1)
		GP_SWAP(y0, y1);

	/* check if we are not completely outside the clipping rectangle */
	if (x < (int) context->clip_w_min
		|| x > (int) context->clip_w_max
		|| y0 > (int) context->clip_h_max
		|| y1 < (int) context->clip_h_min) {
		return GP_ESUCCESS;
	}
	
	/* clip the row value */
	y0 = GP_MAX(y0, (int) context->clip_h_min);
	y1 = GP_MIN(y1, (int) context->clip_h_max);

	/* Calculate the start address and height of the filled block */
	size_t height = 1 + y1 - y0;
	uint8_t *p = (uint8_t *) GP_PIXEL_ADDRESS(context, y0, x);
	
	/* Calculate pixel value from color */
	GP_Pixel pixel;
	pixel.type = context->pixel_type;
	GP_RetCode ret = GP_ColorToPixel(color, &pixel);

	size_t i;
	switch(context->bits_per_pixel) {
	case 32:
		for (i = 0; i < height; i++, p += context->bytes_per_row)
			GP_WritePixel32bpp(p, pixel.val);
		break;

	case 24:
		for (i = 0; i < height; i++, p += context->bytes_per_row)
			GP_WritePixel24bpp(p, pixel.val);
		break;

	case 16:
		for (i = 0; i < height; i++, p += context->bytes_per_row)
			GP_WritePixel16bpp(p, pixel.val);
		break;

	case 8:
		for (i = 0; i < height; i++, p += context->bytes_per_row)
			GP_WritePixel8bpp(p, pixel.val);
		break;

	default:
		return GP_ENOIMPL;
	}

	return ret;
}

GP_RetCode GP_TVLine(GP_Context *context, int x, int y0, int y1, GP_Color color)
{
	if (!context)
		return GP_ENULLPTR;
	if (!GP_IS_CONTEXT_VALID(context))
		return GP_EBADCONTEXT;

	if (context->axes_swap) {
		GP_TRANSFORM_Y(context, x);
		GP_TRANSFORM_X(context, y0);
		GP_TRANSFORM_X(context, y1);
		return GP_HLine(context, y0, y1, x, color);
	}

	GP_TRANSFORM_X(context, x);
	GP_TRANSFORM_Y(context, y0);
	GP_TRANSFORM_Y(context, y1);
	return GP_VLine(context, x, y0, y1, color);
}
