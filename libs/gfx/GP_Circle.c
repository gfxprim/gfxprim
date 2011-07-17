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

#include "GP_Gfx.h"
#include "algo/Circle.algo.h"
#include "core/GP_FnPerBpp.h"

/* Generate drawing functions for various bit depths. */
DEF_CIRCLE_FN(GP_Circle1bpp,  GP_Context *, GP_Pixel, GP_PutPixel1bpp)
DEF_CIRCLE_FN(GP_Circle2bpp,  GP_Context *, GP_Pixel, GP_PutPixel2bpp)
DEF_CIRCLE_FN(GP_Circle4bpp,  GP_Context *, GP_Pixel, GP_PutPixel4bpp)
DEF_CIRCLE_FN(GP_Circle8bpp,  GP_Context *, GP_Pixel, GP_PutPixel8bpp)
DEF_CIRCLE_FN(GP_Circle16bpp, GP_Context *, GP_Pixel, GP_PutPixel16bpp)
DEF_CIRCLE_FN(GP_Circle24bpp, GP_Context *, GP_Pixel, GP_PutPixel24bpp)
DEF_CIRCLE_FN(GP_Circle32bpp, GP_Context *, GP_Pixel, GP_PutPixel32bpp)

void GP_Circle(GP_Context *context, int xcenter, int ycenter,
               unsigned int r, GP_Pixel pixel)
{
	GP_CHECK_CONTEXT(context);

	GP_FN_PER_BPP(GP_Circle, context->bpp, context,
	              xcenter, ycenter, r, pixel);
}

void GP_TCircle(GP_Context *context, int xcenter, int ycenter,
               unsigned int r, GP_Pixel pixel)
{
	GP_CHECK_CONTEXT(context);
	
	GP_TRANSFORM_POINT(context, xcenter, ycenter);
	
	GP_Circle(context, xcenter, ycenter, r, pixel);
}

#include "algo/FillCircle.algo.h"

/* Generate drawing functions for various bit depths. */
DEF_FILLCIRCLE_FN(GP_FillCircle1bpp,  GP_Context *, GP_Pixel, GP_HLine1bpp)
DEF_FILLCIRCLE_FN(GP_FillCircle2bpp,  GP_Context *, GP_Pixel, GP_HLine2bpp)
DEF_FILLCIRCLE_FN(GP_FillCircle4bpp,  GP_Context *, GP_Pixel, GP_HLine4bpp)
DEF_FILLCIRCLE_FN(GP_FillCircle8bpp,  GP_Context *, GP_Pixel, GP_HLine8bpp)
DEF_FILLCIRCLE_FN(GP_FillCircle16bpp, GP_Context *, GP_Pixel, GP_HLine16bpp)
DEF_FILLCIRCLE_FN(GP_FillCircle24bpp, GP_Context *, GP_Pixel, GP_HLine24bpp)
DEF_FILLCIRCLE_FN(GP_FillCircle32bpp, GP_Context *, GP_Pixel, GP_HLine32bpp)

void GP_FillCircle(GP_Context *context, int xcenter, int ycenter,
                   unsigned int r, GP_Pixel pixel)
{
	GP_CHECK_CONTEXT(context);

	GP_FN_PER_BPP(GP_FillCircle, context->bpp, context,
	              xcenter, ycenter, r, pixel);
}

void GP_TFillCircle(GP_Context *context, int xcenter, int ycenter,
                    unsigned int r, GP_Pixel pixel)
{
	GP_CHECK_CONTEXT(context);
	
	GP_TRANSFORM_POINT(context, xcenter, ycenter);
	
	GP_FillCircle(context, xcenter, ycenter, r, pixel);
}

#include "algo/FillRing.algo.h"

/* Generate drawing functions for various bit depths. */
DEF_FILLRING_FN(GP_FillRing1bpp,  GP_Context *, GP_Pixel, GP_HLine1bpp)
DEF_FILLRING_FN(GP_FillRing2bpp,  GP_Context *, GP_Pixel, GP_HLine2bpp)
DEF_FILLRING_FN(GP_FillRing4bpp,  GP_Context *, GP_Pixel, GP_HLine4bpp)
DEF_FILLRING_FN(GP_FillRing8bpp,  GP_Context *, GP_Pixel, GP_HLine8bpp)
DEF_FILLRING_FN(GP_FillRing16bpp, GP_Context *, GP_Pixel, GP_HLine16bpp)
DEF_FILLRING_FN(GP_FillRing24bpp, GP_Context *, GP_Pixel, GP_HLine24bpp)
DEF_FILLRING_FN(GP_FillRing32bpp, GP_Context *, GP_Pixel, GP_HLine32bpp)

void GP_FillRing(GP_Context *context, int xcenter, int ycenter,
                   unsigned int outer_r, unsigned int inner_r, GP_Pixel pixel)
{
	GP_CHECK_CONTEXT(context);

	GP_FN_PER_BPP(GP_FillRing, context->bpp, context,
	              xcenter, ycenter, outer_r, inner_r, pixel);
}

void GP_TFillRing(GP_Context *context, int xcenter, int ycenter,
                    unsigned int outer_r, unsigned int inner_r, GP_Pixel pixel)
{
	GP_CHECK_CONTEXT(context);
	
	GP_TRANSFORM_POINT(context, xcenter, ycenter);
	
	GP_FillRing(context, xcenter, ycenter, outer_r, inner_r, pixel);
}
