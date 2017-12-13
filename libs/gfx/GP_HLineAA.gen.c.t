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

/* Anti Aliased Horizontal Line */

#include "core/GP_Pixmap.h"
#include "core/GP_MixPixels.h"
#include "core/GP_FixedPoint.h"
#include "core/GP_GammaCorrection.h"

#define FP_TO_PERC(a) (GP_FP_ROUND_TO_INT((a) * 255))

/*
 * Computes pixel in buffer that we should start drawing in.
 *
 * This is different at the start and the end of the HLine. The difference is
 * in where do integer coordinates belong (gfxprim puts integer coordinates in
 * the middle of pixels this is where the +1/2 comes from.
 */
#define TO_X_S(x) GP_FP_FLOOR_TO_INT((x) + GP_FP_1_2)
#define TO_X_E(x) GP_FP_CEIL_TO_INT((x) - GP_FP_1_2)

void gp_hline_aa_raw(gp_pixmap *pixmap, gp_coord x0, gp_coord x1,
                     gp_coord y, gp_pixel pixel)
{
	/* Nothing to draw */
	if (x0 == x1)
		return;

	if (x1 < x0)
		GP_SWAP(x1, x0);

	gp_coord int_x0 = TO_X_S(x0);
	gp_coord int_x1 = TO_X_E(x1);
	gp_coord int_y  = GP_FP_FLOOR_TO_INT(y);

//	printf("%f %f %f -> %i %i %i\n", GP_FP_TO_FLOAT(x0), GP_FP_TO_FLOAT(x1), GP_FP_TO_FLOAT(y), int_x0, int_x1, int_y);

	/* Draw the four starting and ending pixels */
	unsigned int perc;
	unsigned int w;

	w = GP_FP_RFRAC(x0 + GP_FP_1_2);

	perc = FP_TO_PERC(GP_FP_MUL(GP_FP_RFRAC(y), w));
	gp_mix_pixel_raw_clipped(pixmap, int_x0, int_y, pixel, perc);

	perc = FP_TO_PERC(GP_FP_MUL(GP_FP_FRAC(y), w));
	gp_mix_pixel_raw_clipped(pixmap, int_x0, int_y+1, pixel, perc);

	if (int_x0 != int_x1) {
		w = GP_FP_FRAC(x1 + GP_FP_1_2);

		perc = FP_TO_PERC(GP_FP_MUL(GP_FP_RFRAC(y), w));
		gp_mix_pixel_raw_clipped(pixmap, int_x1, int_y, pixel, perc);

		perc = FP_TO_PERC(GP_FP_MUL(GP_FP_FRAC(y), w));
		gp_mix_pixel_raw_clipped(pixmap, int_x1, int_y+1, pixel, perc);
	}

	gp_coord x;

	/* Now fill the inner part of the HLine */
	uint8_t up = FP_TO_PERC(GP_FP_RFRAC(y));
	uint8_t lp = FP_TO_PERC(GP_FP_FRAC(y));

	for (x = int_x0 + 1; x < int_x1; x++) {
		gp_mix_pixel_raw_clipped(pixmap, x, int_y, pixel, up);
		gp_mix_pixel_raw_clipped(pixmap, x, int_y+1, pixel, lp);
	}
}
