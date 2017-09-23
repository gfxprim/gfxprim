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

/* Anti Aliased Vertical Line */

#include "core/GP_Pixmap.h"
#include "core/GP_MixPixels.h"
#include "core/GP_FixedPoint.h"
#include "core/GP_GammaCorrection.h"

#define FP_TO_PERC(a) (GP_FP_ROUND_TO_INT((a) * 255))

void GP_VLineAA_Raw(GP_Pixmap *pixmap, GP_Coord x, GP_Coord y0,
                    GP_Coord y1, GP_Pixel pixel)
{
	if (y1 < y0)
		GP_SWAP(y1, y0);

	y0 -= GP_FP_1_2;
	y1 += GP_FP_1_2;
	x  -= GP_FP_1_2;

	GP_Coord int_y0 = GP_FP_TO_INT(y0);
	GP_Coord int_y1 = GP_FP_TO_INT(y1);
	GP_Coord int_x  = GP_FP_TO_INT(x);

	/* Line is shorter than two pixels */
	if (int_y0 == int_y1) {
		//TODO
		return;
	}

	/* Draw the starting and ending pixel */
	uint8_t perc;

	perc = FP_TO_PERC(GP_FP_MUL(GP_FP_RFRAC(x), GP_FP_RFRAC(y0)));
	GP_MixPixel_Raw_Clipped(pixmap, int_x, int_y0, pixel, perc);

	perc = FP_TO_PERC(GP_FP_MUL(GP_FP_FRAC(x), GP_FP_RFRAC(y0)));
	GP_MixPixel_Raw_Clipped(pixmap, int_x+1, int_y0, pixel, perc);

	perc = FP_TO_PERC(GP_FP_MUL(GP_FP_RFRAC(x), GP_FP_FRAC(y1)));
	GP_MixPixel_Raw_Clipped(pixmap, int_x, int_y1, pixel, perc);

	perc = FP_TO_PERC(GP_FP_MUL(GP_FP_FRAC(x), GP_FP_FRAC(y1)));
	GP_MixPixel_Raw_Clipped(pixmap, int_x+1, int_y1, pixel, perc);

	/* Draw the middle pixels */
	uint8_t up = FP_TO_PERC(GP_FP_RFRAC(x));
	uint8_t lp = FP_TO_PERC(GP_FP_FRAC(x));

	GP_Coord y;

	for (y = int_y0 + 1; y < int_y1; y++) {
		GP_MixPixel_Raw_Clipped(pixmap, int_x, y, pixel, up);
		GP_MixPixel_Raw_Clipped(pixmap, int_x+1, y, pixel, lp);
	}
}
