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

GP_RetCode GP_HLine(GP_Context *context, int x0, int x1, int y, GP_Color color)
{
	GP_CHECK_CONTEXT(context);

	/* Handle swapped coordinates gracefully. */
	if (x0 > x1)
		GP_SWAP(x0, x1);

	/* check if we are not completely outside limits */
	if (y < (int) context->clip_h_min
		|| y > (int) context->clip_h_max
		|| x0 > (int) context->clip_w_max
		|| x1 < (int) context->clip_w_min) {
		return GP_EINVAL;
	}

	/* clip the X value */
	x0 = GP_MAX(x0, (int) context->clip_w_min);
	x1 = GP_MIN(x1, (int) context->clip_w_max);

	/* calculate the length and start of the filled block */
	size_t length = 1 + x1 - x0;
	void *start = GP_PIXEL_ADDRESS(context, y, x0);
	
	/* Calculate pixel value from color */
	GP_Pixel pixel;
	pixel.type = context->pixel_type;
	GP_RetCode ret = GP_ColorToPixel(color, &pixel);

	switch(context->bits_per_pixel) {
	case 32:
		GP_WritePixels32bpp(start, length, pixel.val);
		break;
	case 24:
		GP_WritePixels24bpp(start, length, pixel.val);
		break;
	case 16:
		GP_WritePixels16bpp(start, length, pixel.val);
		break;
	case 8:
		GP_WritePixels8bpp(start, length, pixel.val);
		break;
	default:
		return GP_ENOIMPL;
	}

	return ret;
}

GP_RetCode GP_THLine(GP_Context *context, int x0, int x1, int y, GP_Color color)
{
	if (context->axes_swap) {
		GP_TRANSFORM_Y(context, x0);
		GP_TRANSFORM_Y(context, x1);
		GP_TRANSFORM_X(context, y);
		return GP_VLine(context, y, x0, x1, color);
	}

	GP_TRANSFORM_X(context, x0);
	GP_TRANSFORM_X(context, x1);
	GP_TRANSFORM_Y(context, y);
	return GP_HLine(context, x0, x1, y, color);
}
