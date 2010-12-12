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
#include "algo/Circle.algo.h"
#include "GP_FnPerBpp.h"

/* Generate drawing functions for various bit depths. */
DEF_CIRCLE_FN(GP_Circle8bpp, GP_Context *, GP_Pixel, GP_PutPixel8bpp)
DEF_CIRCLE_FN(GP_Circle16bpp, GP_Context *, GP_Pixel, GP_PutPixel16bpp)
DEF_CIRCLE_FN(GP_Circle24bpp, GP_Context *, GP_Pixel, GP_PutPixel24bpp)
DEF_CIRCLE_FN(GP_Circle32bpp, GP_Context *, GP_Pixel, GP_PutPixel32bpp)

GP_RetCode GP_Circle(GP_Context *context, int xcenter, int ycenter,
                     unsigned int r, GP_Color color)
{
	if (!context)
		return GP_ENULLPTR;
	if (!GP_IS_CONTEXT_VALID(context))
		return GP_EBADCONTEXT;

	GP_Pixel pixel;
	pixel.type = context->pixel_type;
	GP_RetCode ret = GP_ColorToPixel(color, &pixel);

	GP_FN_PER_BPP(GP_Circle, xcenter, ycenter, r, pixel);

	return ret;
}

GP_RetCode GP_TCircle(GP_Context *context, int xcenter, int ycenter,
                      unsigned int r, GP_Color color)
{
	if (!context)
		return GP_ENULLPTR;
	if (!GP_IS_CONTEXT_VALID(context))
		return GP_EBADCONTEXT;

	/* Just recalculate center point */
	GP_TRANSFORM_POINT(context, xcenter, ycenter);
	return GP_Circle(context, xcenter, ycenter, r, color);
}
