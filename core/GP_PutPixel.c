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

/* Tests whether the coordinates are inside the clip rectangle;
 * if not, immediately exits with GP_ESUCCESS.
 */
#define CLIP_PIXEL(context, x, y) do { \
	if (x < (int) context->clip_w_min \
		|| x > (int) context->clip_w_max \
		|| y < (int) context->clip_h_min \
		|| y > (int) context->clip_h_max) { \
		return GP_ESUCCESS; /* clipped out */ \
	} \
} while(0);

/*
 * A set of PutPixel() calls used when we know the number of bits per pixel.
 * The pixel value is passed directly instead of color, as we should
 * already know what the pixel value is, and it would be redundant to
 * calculate it over and over.
 */

GP_RetCode GP_PutPixel8bpp(GP_Context *context, int x, int y, GP_Pixel pixel)
{
	CLIP_PIXEL(context, x, y);

	uint8_t *p = GP_PIXEL_ADDRESS(context, y, x);
	GP_WritePixel8bpp(p, pixel.val);
	return GP_ESUCCESS;
}

GP_RetCode GP_PutPixel16bpp(GP_Context *context, int x, int y, GP_Pixel pixel)
{
	CLIP_PIXEL(context, x, y);

	uint8_t *p = GP_PIXEL_ADDRESS(context, y, x);
	GP_WritePixel16bpp(p, pixel.val);
	return GP_ESUCCESS;
}

GP_RetCode GP_PutPixel24bpp(GP_Context *context, int x, int y, GP_Pixel pixel)
{
	CLIP_PIXEL(context, x, y);

	uint8_t *p = GP_PIXEL_ADDRESS(context, y, x);
	GP_WritePixel24bpp(p, pixel.val);
	return GP_ESUCCESS;
}

GP_RetCode GP_PutPixel32bpp(GP_Context *context, int x, int y, GP_Pixel pixel)
{
	CLIP_PIXEL(context, x, y);

	uint8_t *p = GP_PIXEL_ADDRESS(context, y, x);
	GP_WritePixel32bpp(p, pixel.val);
	return GP_ESUCCESS;
}

/*
 * A generic PutPixel call that automatically determines the number of
 * bits per pixel and converts color to pixel value appropriately.
 */
GP_RetCode GP_PutPixel(GP_Context *context, int x, int y, GP_Color color)
{
	if (!context)
		return GP_ENULLPTR;
	if (!GP_IS_CONTEXT_VALID(context))
		return GP_EBADCONTEXT;

	GP_Pixel pixel;
	GP_RetCode ret;
	pixel.type = context->pixel_type;
	ret = GP_ColorToPixel(color, &pixel);
	if (ret != GP_ESUCCESS)
		return ret;

	switch (context->bits_per_pixel) {
	case 32:
		return GP_PutPixel32bpp(context, x, y, pixel);
	case 24:
		return GP_PutPixel24bpp(context, x, y, pixel);
	case 16:
		return GP_PutPixel16bpp(context, x, y, pixel);
	case 8:
		return GP_PutPixel8bpp(context, x, y, pixel);
	}

	return GP_ENOIMPL;
}

GP_RetCode GP_TPutPixel(GP_Context *context, int x, int y, GP_Color color)
{
	if (!context)
		return GP_ENULLPTR;
	if (!GP_IS_CONTEXT_VALID(context))
		return GP_EBADCONTEXT;

	GP_TRANSFORM_POINT(context, x, y);
	return GP_PutPixel(context, x, y, color);
}
