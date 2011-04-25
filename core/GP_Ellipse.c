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
#include "algo/Ellipse.algo.h"
#include "GP_FnPerBpp.h"

/* Generate drawing functions for various bit depths. */
DEF_ELLIPSE_FN(GP_Ellipse1bpp,  GP_Context *, GP_Pixel, GP_PutPixel1bpp);
DEF_ELLIPSE_FN(GP_Ellipse2bpp,  GP_Context *, GP_Pixel, GP_PutPixel2bpp);
DEF_ELLIPSE_FN(GP_Ellipse4bpp,  GP_Context *, GP_Pixel, GP_PutPixel4bpp);
DEF_ELLIPSE_FN(GP_Ellipse8bpp,  GP_Context *, GP_Pixel, GP_PutPixel8bpp);
DEF_ELLIPSE_FN(GP_Ellipse16bpp, GP_Context *, GP_Pixel, GP_PutPixel16bpp);
DEF_ELLIPSE_FN(GP_Ellipse24bpp, GP_Context *, GP_Pixel, GP_PutPixel24bpp);
DEF_ELLIPSE_FN(GP_Ellipse32bpp, GP_Context *, GP_Pixel, GP_PutPixel32bpp);

void GP_Ellipse(GP_Context *context, int xcenter, int ycenter,
                unsigned int a, unsigned int b, GP_Pixel pixel)
{
	GP_CHECK_CONTEXT(context);

	GP_FN_PER_BPP(GP_Ellipse, context, xcenter, ycenter, a, b, pixel);
}

void GP_TEllipse(GP_Context *context, int xcenter, int ycenter,
                 unsigned int a, unsigned int b, GP_Pixel pixel)
{
	GP_CHECK_CONTEXT(context);
	
	/* recalculate center point and swap a and b when axes are swapped */
	GP_TRANSFORM_POINT(context, xcenter, ycenter);
	GP_TRANSFORM_SWAP(context, a, b);
	
	GP_Ellipse(context, xcenter, ycenter, a, b, pixel);
}
