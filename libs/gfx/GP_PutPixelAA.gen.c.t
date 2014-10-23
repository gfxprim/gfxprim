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
 * Copyright (C) 2009-2013 Cyril Hrubis <metan@ucw.cz>                       *
 *                                                                           *
 *****************************************************************************/

/* Anti Aliased Put Pixel */

#include "core/GP_Context.h"
#include "core/GP_MixPixels.h"
#include "core/GP_FixedPoint.h"
#include "core/GP_GammaCorrection.h"

#include "gfx/GP_HLine.h"
#include "gfx/GP_VLine.h"

#define FP_TO_PERC(a) (GP_FP_ROUND_TO_INT((a) * 255))

void GP_PutPixelAA_Raw(GP_Context *context, GP_Coord x, GP_Coord y,
                       GP_Pixel pixel)
{
	GP_Coord int_x = GP_FP_TO_INT(x);
	GP_Coord int_y = GP_FP_TO_INT(y);
	GP_Coord frac_x = GP_FP_FRAC(x);
	GP_Coord frac_y = GP_FP_FRAC(y);
	uint8_t perc;

	perc = FP_TO_PERC(GP_FP_MUL(GP_FP_1 - frac_x, GP_FP_1 - frac_y));
	GP_MixPixel_Raw_Clipped(context, int_x, int_y, pixel, perc);

	perc = FP_TO_PERC(GP_FP_MUL(frac_x, GP_FP_1 - frac_y));
	GP_MixPixel_Raw_Clipped(context, int_x + 1, int_y, pixel, perc);

	perc = FP_TO_PERC(GP_FP_MUL(GP_FP_1 - frac_x, frac_y));
	GP_MixPixel_Raw_Clipped(context, int_x, int_y + 1, pixel, perc);

	perc = FP_TO_PERC(GP_FP_MUL(frac_x, frac_y));
	GP_MixPixel_Raw_Clipped(context, int_x + 1, int_y + 1, pixel, perc);
}

void GP_PutPixelAA_Raw_Clipped(GP_Context *context, GP_Coord x, GP_Coord y,
                               GP_Pixel pixel)
{
	GP_PutPixelAA_Raw(context, x, y, pixel);
}

void GP_PutPixelAA(GP_Context *context, GP_Coord x, GP_Coord y, GP_Pixel pixel)
{
	GP_TRANSFORM_POINT_FP(context, x, y);

	GP_PutPixelAA_Raw_Clipped(context, x, y, pixel);
}
