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

#include "GP_FnPerBpp.h"

#define DO_PUTPIXEL(bits) \
GP_RetCode GP_PutPixel##bits##bpp(GP_Context *context, int x, int y, GP_Pixel pixel) \
{ \
	if (GP_PIXEL_IS_CLIPPED(context, x, y)) \
		return GP_ESUCCESS; \
\
	GP_PUTPIXEL_##bits##BPP(context, x, y, pixel); \
\
	return GP_ESUCCESS; \
} \

DO_PUTPIXEL(1)
DO_PUTPIXEL(2)
DO_PUTPIXEL(4)
DO_PUTPIXEL(8)
DO_PUTPIXEL(16)
DO_PUTPIXEL(24)
DO_PUTPIXEL(32)

/*
 * A generic PutPixel call that automatically determines the number of
 * bits per pixel.
 */
GP_RetCode GP_PutPixel(GP_Context *context, int x, int y, GP_Pixel pixel)
{
	if (!context)
		return GP_ENULLPTR;
	if (!GP_IS_CONTEXT_VALID(context))
		return GP_EBADCONTEXT;

	GP_FN_PER_BPP(GP_PutPixel, context, x, y, pixel);
}

GP_RetCode GP_TPutPixel(GP_Context *context, int x, int y, GP_Pixel pixel)
{
	if (!context)
		return GP_ENULLPTR;
	if (!GP_IS_CONTEXT_VALID(context))
		return GP_EBADCONTEXT;

	GP_TRANSFORM_POINT(context, x, y);
	return GP_PutPixel(context, x, y, pixel);
}
