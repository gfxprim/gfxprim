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

/* Anti Aliased Line */

#include "core/GP_Pixmap.h"
#include "core/GP_MixPixels.h"
#include "core/GP_FixedPoint.h"
#include "core/GP_GammaCorrection.h"

#include "gfx/GP_HLineAA.h"
#include "gfx/GP_VLineAA.h"

#define FP_TO_PERC(a) (GP_FP_ROUND_TO_INT((a) * 255))

static inline void line_aa_x(gp_pixmap *pixmap,
                             gp_coord x0, gp_coord y0,
                             gp_coord x1, gp_coord y1, gp_pixel pixel)
{
	gp_coord xend, yend, xgap, xpx0, ypx0, xpx1, ypx1;
	uint8_t perc;

	int64_t dx = x1 - x0;
	int64_t dy = y1 - y0;

	if (x1 < x0) {
		GP_SWAP(x0, x1);
		GP_SWAP(y0, y1);
	}

	xend = GP_FP_ROUND(x1);
	yend = y1 + GP_FP_DIV(GP_FP_MUL(dy, xend - x1), dx);
	xgap = GP_FP_FRAC(x1 + GP_FP_1_2);
	xpx1 = GP_FP_TO_INT(xend);
	ypx1 = GP_FP_TO_INT(yend);

	perc = FP_TO_PERC(GP_FP_MUL(GP_FP_RFRAC(yend), xgap));
	gp_mix_pixel_raw_clipped(pixmap, xpx1, ypx1, pixel, perc);
	perc = FP_TO_PERC(GP_FP_MUL(GP_FP_FRAC(yend), xgap));
	gp_mix_pixel_raw_clipped(pixmap, xpx1, ypx1+1, pixel, perc);

	xend = GP_FP_ROUND(x0);
	yend = y0 + GP_FP_DIV(GP_FP_MUL(dy, xend - x0), dx);
	xgap = GP_FP_RFRAC(x0 + GP_FP_1_2);
	xpx0 = GP_FP_TO_INT(xend);
	ypx0 = GP_FP_TO_INT(yend);

	perc = FP_TO_PERC(GP_FP_MUL(GP_FP_RFRAC(yend), xgap));
	gp_mix_pixel_raw_clipped(pixmap, xpx0, ypx0, pixel, perc);
	perc = FP_TO_PERC(GP_FP_MUL(GP_FP_FRAC(yend), xgap));
	gp_mix_pixel_raw_clipped(pixmap, xpx0, ypx0+1, pixel, perc);

	gp_coord x;
	gp_coord intery;

	for (x = xpx0 + 1; x < xpx1; x++) {
		intery = yend + GP_FP_DIV((x - xpx0) * dy, dx);

		perc = FP_TO_PERC(GP_FP_RFRAC(intery));
		gp_mix_pixel_raw_clipped(pixmap, x, GP_FP_TO_INT(intery), pixel, perc);
		perc = FP_TO_PERC(GP_FP_FRAC(intery));
		gp_mix_pixel_raw_clipped(pixmap, x, GP_FP_TO_INT(intery)+1, pixel, perc);
	}
}

static inline void line_aa_y(gp_pixmap *pixmap,
                             gp_coord x0, gp_coord y0,
                             gp_coord x1, gp_coord y1, gp_pixel pixel)
{
	gp_coord xend, yend, ygap, xpx0, ypx0, xpx1, ypx1;
	uint8_t perc;

	int64_t dx = x1 - x0;
	int64_t dy = y1 - y0;

	if (y1 < y0) {
		GP_SWAP(x0, x1);
		GP_SWAP(y0, y1);
	}

	yend = GP_FP_ROUND(y1);
	xend = x1 + GP_FP_DIV(GP_FP_MUL(dx, yend - y1), dy);
	ygap = GP_FP_FRAC(y1 + GP_FP_1_2);
	ypx1 = GP_FP_TO_INT(yend);
	xpx1 = GP_FP_TO_INT(xend);

	perc = FP_TO_PERC(GP_FP_MUL(GP_FP_RFRAC(xend), ygap));
	gp_mix_pixel_raw_clipped(pixmap, xpx1, ypx1, pixel, perc);
	perc = FP_TO_PERC(GP_FP_MUL(GP_FP_FRAC(xend), ygap));
	gp_mix_pixel_raw_clipped(pixmap, xpx1, ypx1+1, pixel, perc);

	yend = GP_FP_ROUND(y0);
	xend = x0 + GP_FP_DIV(GP_FP_MUL(dx, yend - y0), dy);
	ygap = GP_FP_RFRAC(y0 + GP_FP_1_2);
	ypx0 = GP_FP_TO_INT(yend);
	xpx0 = GP_FP_TO_INT(xend);

	perc = FP_TO_PERC(GP_FP_MUL(GP_FP_RFRAC(xend), ygap));
	gp_mix_pixel_raw_clipped(pixmap, xpx0, ypx0, pixel, perc);
	perc = FP_TO_PERC(GP_FP_MUL(GP_FP_FRAC(xend), ygap));
	gp_mix_pixel_raw_clipped(pixmap, xpx0, ypx0+1, pixel, perc);

	gp_coord y;
	gp_coord intery;

	for (y = ypx0 + 1; y < ypx1; y++) {
		intery = xend + GP_FP_DIV((y - ypx0) * dx, dy);

		perc = FP_TO_PERC(GP_FP_RFRAC(intery));
		gp_mix_pixel_raw_clipped(pixmap, GP_FP_TO_INT(intery), y, pixel, perc);
		perc = FP_TO_PERC(GP_FP_FRAC(intery));
		gp_mix_pixel_raw_clipped(pixmap, GP_FP_TO_INT(intery)+1, y, pixel, perc);
	}
}

void gp_line_aa_raw(gp_pixmap *pixmap, gp_coord x0, gp_coord y0,
                    gp_coord x1, gp_coord y1, gp_pixel pixel)
{
	int64_t dx = x1 - x0;
	int64_t dy = y1 - y0;

	if (dy == 0) {
		gp_hline_aa_raw(pixmap, x0, x1, y0, pixel);
		return;
	}

	if (dx == 0) {
		gp_vline_aa_raw(pixmap, x0, y0, y1, pixel);
		return;
	}

	if (GP_ABS(dx) < GP_ABS(dy))
		line_aa_y(pixmap, x0, y0, x1, y1, pixel);
	else
		line_aa_x(pixmap, x0, y0, x1, y1, pixel);
}
