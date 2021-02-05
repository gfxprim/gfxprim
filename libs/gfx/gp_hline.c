// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2009-2011 Jiri "BlueBear" Dluhos
 *                         <jiri.bluebear.dluhos@gmail.com>
 *
 * Copyright (C) 2009-2012 Cyril Hrubis <metan@ucw.cz>
 */

#include "core/gp_pixmap.h"
#include <core/gp_transform.h>

#include <gfx/gp_hline.h>
#include <gfx/gp_vline.h>

void gp_hline_xxy_raw(gp_pixmap *pixmap, gp_coord x0, gp_coord x1,
                     gp_coord y, gp_pixel pixel)
{
	GP_CHECK_PIXMAP(pixmap);

	GP_FN_PER_BPP_PIXMAP(gp_hline_raw, pixmap, pixmap, x0, x1, y,
	                      pixel);
}

void gp_hline_xyw_raw(gp_pixmap *pixmap, gp_coord x, gp_coord y, gp_size w,
                     gp_pixel pixel)
{
	if (w == 0)
		return;

	gp_hline_xxy_raw(pixmap, x, x + w - 1, y, pixel);
}

void gp_hline_xxy(gp_pixmap *pixmap, gp_coord x0, gp_coord x1, gp_coord y,
                     gp_pixel pixel)
{
	GP_CHECK_PIXMAP(pixmap);

	if (pixmap->axes_swap) {
		GP_TRANSFORM_Y(pixmap, x0);
		GP_TRANSFORM_Y(pixmap, x1);
		GP_TRANSFORM_X(pixmap, y);
		gp_vline_raw(pixmap, y, x0, x1, pixel);
	} else {
		GP_TRANSFORM_X(pixmap, x0);
		GP_TRANSFORM_X(pixmap, x1);
		GP_TRANSFORM_Y(pixmap, y);
		gp_hline_raw(pixmap, x0, x1, y, pixel);
	}
}

void gp_hline_xyw(gp_pixmap *pixmap, gp_coord x, gp_coord y, gp_size w,
                 gp_pixel pixel)
{
	if (w == 0)
		return;

	gp_hline_xxy(pixmap, x, x + w - 1, y, pixel);
}
