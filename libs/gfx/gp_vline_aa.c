// SPDX-License-Identifier: GPL-2.1-or-later
/*
 * Copyright (C) 2009-2012 Cyril Hrubis <metan@ucw.cz>
 */

#include <core/gp_get_put_pixel.h>
#include <core/gp_fn_per_bpp.h>

#include <gfx/gp_vline_aa.h>
#include <gfx/gp_hline_aa.h>

void gp_vline_aa(gp_pixmap *pixmap, gp_coord x, gp_coord y0,
                gp_coord y1, gp_pixel pixel)
{
	GP_CHECK_PIXMAP(pixmap);

	if (pixmap->axes_swap) {
		GP_TRANSFORM_Y_FP(pixmap, x);
		GP_TRANSFORM_X_FP(pixmap, y0);
		GP_TRANSFORM_X_FP(pixmap, y1);
		gp_hline_aa_raw(pixmap, y0, y1, x, pixel);
	} else {
		GP_TRANSFORM_X_FP(pixmap, x);
		GP_TRANSFORM_Y_FP(pixmap, y0);
		GP_TRANSFORM_Y_FP(pixmap, y1);
		gp_vline_aa_raw(pixmap, x, y0, y1, pixel);
	}
}
