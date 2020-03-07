// SPDX-License-Identifier: GPL-2.1-or-later
/*
 * Copyright (C) 2009-2013 Cyril Hrubis <metan@ucw.cz>
 */

/* Anti Aliased Vertical Line */

#include "core/gp_pixmap.h"
#include <core/gp_mix_pixels.h>
#include <core/gp_fixed_point.h>
#include <core/gp_gamma_correction.h>

#define FP_TO_PERC(a) (GP_FP_ROUND_TO_INT((a) * 255))

void gp_vline_aa_raw(gp_pixmap *pixmap, gp_coord x, gp_coord y0,
                     gp_coord y1, gp_pixel pixel)
{
	if (y1 < y0)
		GP_SWAP(y1, y0);

	y0 -= GP_FP_1_2;
	y1 += GP_FP_1_2;
	x  -= GP_FP_1_2;

	gp_coord int_y0 = GP_FP_TO_INT(y0);
	gp_coord int_y1 = GP_FP_TO_INT(y1);
	gp_coord int_x  = GP_FP_TO_INT(x);

	/* Line is shorter than two pixels */
	if (int_y0 == int_y1) {
		//TODO
		return;
	}

	/* Draw the starting and ending pixel */
	uint8_t perc;

	perc = FP_TO_PERC(GP_FP_MUL(GP_FP_RFRAC(x), GP_FP_RFRAC(y0)));
	gp_mix_pixel_raw_clipped(pixmap, int_x, int_y0, pixel, perc);

	perc = FP_TO_PERC(GP_FP_MUL(GP_FP_FRAC(x), GP_FP_RFRAC(y0)));
	gp_mix_pixel_raw_clipped(pixmap, int_x+1, int_y0, pixel, perc);

	perc = FP_TO_PERC(GP_FP_MUL(GP_FP_RFRAC(x), GP_FP_FRAC(y1)));
	gp_mix_pixel_raw_clipped(pixmap, int_x, int_y1, pixel, perc);

	perc = FP_TO_PERC(GP_FP_MUL(GP_FP_FRAC(x), GP_FP_FRAC(y1)));
	gp_mix_pixel_raw_clipped(pixmap, int_x+1, int_y1, pixel, perc);

	/* Draw the middle pixels */
	uint8_t up = FP_TO_PERC(GP_FP_RFRAC(x));
	uint8_t lp = FP_TO_PERC(GP_FP_FRAC(x));

	gp_coord y;

	for (y = int_y0 + 1; y < int_y1; y++) {
		gp_mix_pixel_raw_clipped(pixmap, int_x, y, pixel, up);
		gp_mix_pixel_raw_clipped(pixmap, int_x+1, y, pixel, lp);
	}
}
