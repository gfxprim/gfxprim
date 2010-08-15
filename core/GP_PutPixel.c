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

GP_RetCode GP_PutPixel(GP_Context *context, int x, int y, GP_Color color)
{
	GP_CHECK_CONTEXT(context);
	GP_RetCode ret;
	GP_Pixel pixel;
	
	pixel.type = context->pixel_type;
	ret = GP_ColorToPixel(color, &pixel);
	
	uint8_t *p;

	if (x < (int) context->clip_w_min
		|| x > (int) context->clip_w_max
		|| y < (int) context->clip_h_min
		|| y > (int) context->clip_h_max) {
		return GP_EINVAL; /* clipped out */
	}
	
	p = GP_PIXEL_ADDRESS(context, y, x);

	switch (context->bits_per_pixel) {
	case 32:
		GP_WritePixel32bpp(p, pixel.val);
		break;
	case 24:
		GP_WritePixel24bpp(p, pixel.val);
		break;
	
	case 16:
		GP_WritePixel16bpp(p, pixel.val);
		break;
	
	case 8:
		GP_WritePixel8bpp(p, pixel.val);
		break;
	
	default:
		GP_ABORT("Unsupported value of context->bits_per_pixel");
		return GP_ENOIMPL;
	}

	return ret;
}

GP_RetCode GP_TPutPixel(GP_Context *context, int x, int y, GP_Color color)
{
	GP_TRANSFORM_POINT(context, x, y);
	return GP_PutPixel(context, x, y, color);
}
