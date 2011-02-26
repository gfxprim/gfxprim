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

#include "GP_GetPixel.h"
#include "GP_FnPerBpp.h"

#define DO_GETPIXEL(bits) \
GP_RetCode GP_GetPixel##bits##bpp(GP_Context *context, int x, int y) \
{ \
	if (GP_PIXEL_IS_CLIPPED(context, x, y)) \
		return 0; \
\
	return GP_GETPIXEL_##bits##BPP(context, x, y); \
} \

DO_GETPIXEL(1)
DO_GETPIXEL(2)
DO_GETPIXEL(4)
DO_GETPIXEL(8)
DO_GETPIXEL(16)
DO_GETPIXEL(24)
DO_GETPIXEL(32)

GP_Pixel GP_GetPixel(GP_Context *context, int x, int y)
{
	GP_CHECK_CONTEXT(context);

	GP_FN_RET_PER_BPP(GP_GetPixel, context, x, y);

	return 0;
}

GP_Pixel GP_TGetPixel(GP_Context *context, int x, int y)
{
	GP_TRANSFORM_POINT(context, x, y);
	return GP_GetPixel(context, x, y);
}
